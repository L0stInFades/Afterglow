#include "core/ThreadPool.hpp"
#include <algorithm>
#include <string>
#include <windows.h>

namespace UltraImageViewer {
namespace Core {

ThreadPool::ThreadPool(uint32_t numThreads)
{
    if (numThreads == 0) {
        uint32_t hw = std::thread::hardware_concurrency();
        // Reserve one core for the render thread, minimum 2 workers
        numThreads = (hw > 2) ? hw - 1 : 2;
    }
    threadCount_ = numThreads;

    threads_.reserve(numThreads);
    for (uint32_t i = 0; i < numThreads; ++i) {
        threads_.emplace_back([this, i](std::stop_token) { WorkerFunc(i); });
    }

    OutputDebugStringA(("[ThreadPool] Started with " +
        std::to_string(numThreads) + " workers\n").c_str());
}

ThreadPool::~ThreadPool()
{
    shutdown_.store(true, std::memory_order_release);
    cv_.notify_all();

    for (auto& t : threads_) {
        if (t.joinable()) {
            t.request_stop();
            t.join();
        }
    }
    threads_.clear();

    OutputDebugStringA(("[ThreadPool] Shutdown. Completed " +
        std::to_string(completed_.load()) + " tasks total\n").c_str());
}

void ThreadPool::Submit(std::function<void()> fn, TaskPriority p)
{
    {
        std::lock_guard lock(mutex_);
        lanes_[static_cast<int>(p)].queue.push_back(std::move(fn));
    }
    pending_.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void ThreadPool::SubmitFront(std::function<void()> fn, TaskPriority p)
{
    {
        std::lock_guard lock(mutex_);
        lanes_[static_cast<int>(p)].queue.push_front(std::move(fn));
    }
    pending_.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
}

void ThreadPool::SubmitBatch(std::vector<std::function<void()>>& fns, TaskPriority p)
{
    if (fns.empty()) return;

    uint32_t count = static_cast<uint32_t>(fns.size());
    {
        std::lock_guard lock(mutex_);
        auto& q = lanes_[static_cast<int>(p)].queue;
        for (auto& fn : fns) {
            q.push_back(std::move(fn));
        }
    }
    pending_.fetch_add(count, std::memory_order_relaxed);
    cv_.notify_all();
}

void ThreadPool::PurgeAll()
{
    std::lock_guard lock(mutex_);
    uint32_t purged = 0;
    for (int i = 0; i < kLaneCount; ++i) {
        purged += static_cast<uint32_t>(lanes_[i].queue.size());
        lanes_[i].queue.clear();
    }
    // Adjust pending count (saturating subtract)
    uint32_t old = pending_.load(std::memory_order_relaxed);
    while (old > 0 && !pending_.compare_exchange_weak(old,
           (old >= purged) ? old - purged : 0, std::memory_order_relaxed)) {}

    // Wake WaitIdle() if all work is done
    if (pending_.load(std::memory_order_relaxed) == 0 &&
        active_.load(std::memory_order_relaxed) == 0) {
        idleCV_.notify_all();
    }
}

void ThreadPool::PurgePriority(TaskPriority p)
{
    std::lock_guard lock(mutex_);
    auto& q = lanes_[static_cast<int>(p)].queue;
    uint32_t purged = static_cast<uint32_t>(q.size());
    q.clear();

    uint32_t old = pending_.load(std::memory_order_relaxed);
    while (old > 0 && !pending_.compare_exchange_weak(old,
           (old >= purged) ? old - purged : 0, std::memory_order_relaxed)) {}

    if (pending_.load(std::memory_order_relaxed) == 0 &&
        active_.load(std::memory_order_relaxed) == 0) {
        idleCV_.notify_all();
    }
}

void ThreadPool::WaitIdle()
{
    std::unique_lock lock(mutex_);
    idleCV_.wait(lock, [this] {
        return pending_.load(std::memory_order_relaxed) == 0 &&
               active_.load(std::memory_order_relaxed) == 0;
    });
}

std::optional<std::function<void()>> ThreadPool::TryDequeue()
{
    std::lock_guard lock(mutex_);
    for (int i = 0; i < kLaneCount; ++i) {
        auto& q = lanes_[i].queue;
        if (!q.empty()) {
            auto fn = std::move(q.front());
            q.pop_front();
            return fn;
        }
    }
    return std::nullopt;
}

void ThreadPool::WorkerFunc(uint32_t /*index*/)
{
    while (!shutdown_.load(std::memory_order_acquire)) {

        // Phase 1: Spin — try to grab a task with no syscall
        for (int spin = 0; spin < kSpinCount; ++spin) {
            auto task = TryDequeue();
            if (task) {
                pending_.fetch_sub(1, std::memory_order_relaxed);
                active_.fetch_add(1, std::memory_order_relaxed);
                (*task)();
                active_.fetch_sub(1, std::memory_order_relaxed);
                completed_.fetch_add(1, std::memory_order_relaxed);

                // Notify WaitIdle if everything drained
                if (pending_.load(std::memory_order_relaxed) == 0 &&
                    active_.load(std::memory_order_relaxed) == 0) {
                    idleCV_.notify_all();
                }
                goto next_cycle;  // restart hot loop
            }
        }

        // Phase 2: Yield — pause the CPU pipeline briefly
        for (int y = 0; y < kYieldCount; ++y) {
            _mm_pause();
            auto task = TryDequeue();
            if (task) {
                pending_.fetch_sub(1, std::memory_order_relaxed);
                active_.fetch_add(1, std::memory_order_relaxed);
                (*task)();
                active_.fetch_sub(1, std::memory_order_relaxed);
                completed_.fetch_add(1, std::memory_order_relaxed);

                if (pending_.load(std::memory_order_relaxed) == 0 &&
                    active_.load(std::memory_order_relaxed) == 0) {
                    idleCV_.notify_all();
                }
                goto next_cycle;
            }
        }

        // Phase 3: Sleep — wait on condition variable (syscall)
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock, [this] {
                // Check if any lane has work
                for (int i = 0; i < kLaneCount; ++i) {
                    if (!lanes_[i].queue.empty()) return true;
                }
                return shutdown_.load(std::memory_order_relaxed);
            });
        }
        continue;

    next_cycle:;
    }
}

} // namespace Core
} // namespace UltraImageViewer
