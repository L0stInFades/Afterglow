#include "animation/AnimationEngine.hpp"
#include <algorithm>

namespace UltraImageViewer {
namespace Animation {

AnimationEngine::AnimationEngine() = default;
AnimationEngine::~AnimationEngine() = default;

void AnimationEngine::Update(float deltaTime)
{
    for (auto it = springs_.begin(); it != springs_.end(); ) {
        auto& ms = *it;
        ms.animation->Update(deltaTime);

        if (ms.onUpdate) {
            ms.onUpdate(ms.animation->GetValue());
        }

        if (ms.animation->IsFinished() && ms.isManaged) {
            if (ms.onComplete) {
                ms.onComplete();
            }
            it = springs_.erase(it);
        } else {
            ++it;
        }
    }
}

bool AnimationEngine::HasActiveAnimations() const
{
    for (const auto& ms : springs_) {
        if (!ms.animation->IsFinished()) {
            return true;
        }
    }
    return false;
}

SpringAnimation* AnimationEngine::CreateSpring(const SpringConfig& config)
{
    ManagedSpring ms;
    ms.animation = std::make_unique<SpringAnimation>(config);
    ms.isManaged = false;
    auto* ptr = ms.animation.get();
    springs_.push_back(std::move(ms));
    return ptr;
}

void AnimationEngine::RemoveAnimation(SpringAnimation* anim)
{
    springs_.erase(
        std::remove_if(springs_.begin(), springs_.end(),
            [anim](const ManagedSpring& ms) { return ms.animation.get() == anim; }),
        springs_.end()
    );
}

void AnimationEngine::AnimateValue(float from, float to, const SpringConfig& config,
                                   ValueCallback onUpdate, CompletionCallback onComplete)
{
    ManagedSpring ms;
    ms.animation = std::make_unique<SpringAnimation>(config);
    ms.animation->SetValue(from);
    ms.animation->SetTarget(to);
    ms.onUpdate = std::move(onUpdate);
    ms.onComplete = std::move(onComplete);
    ms.isManaged = true;
    springs_.push_back(std::move(ms));
}

void AnimationEngine::Clear()
{
    springs_.clear();
}

} // namespace Animation
} // namespace UltraImageViewer
