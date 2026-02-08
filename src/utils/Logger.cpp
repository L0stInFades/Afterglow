#include "utils/Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <windows.h>
#include <cstdlib>

namespace UltraImageViewer {
namespace Utils {

// Logger implementation
Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : minimumLevel_(LogLevel::Info)
    , consoleOutput_(true)
    , fileOutput_(false)
    , initialized_(false)
{
}

Logger::~Logger()
{
    Shutdown();
}

void Logger::Initialize(const std::filesystem::path& logFile)
{
    if (initialized_) {
        return;
    }

    if (!logFile.empty()) {
        logFilePath_ = logFile;
        logFile_.open(logFile_, std::ios::out | std::ios::app);
        fileOutput_ = logFile_.is_open();
    }

    initialized_ = true;

    Log(LogLevel::Info, "Logger initialized");
}

void Logger::Shutdown()
{
    if (!initialized_) {
        return;
    }

    Log(LogLevel::Info, "Logger shutting down");

    if (logFile_.is_open()) {
        logFile_.close();
    }

    initialized_ = false;
}

void Logger::Log(LogLevel level, const std::string& message)
{
    if (!initialized_ || level < minimumLevel_) {
        return;
    }

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.timestamp = GetCurrentTimestamp();
    entry.threadId = std::this_thread::get_id();

    if (consoleOutput_) {
        WriteToConsole(entry);
    }

    if (fileOutput_ && logFile_.is_open()) {
        WriteToFile(entry);
    }
}

void Logger::Log(LogLevel level, const std::string& message, const char* file, int line, const char* function)
{
    if (!initialized_ || level < minimumLevel_) {
        return;
    }

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.file = file ? file : "";
    entry.line = line;
    entry.function = function ? function : "";
    entry.timestamp = GetCurrentTimestamp();
    entry.threadId = std::this_thread::get_id();

    if (consoleOutput_) {
        WriteToConsole(entry);
    }

    if (fileOutput_ && logFile_.is_open()) {
        WriteToFile(entry);
    }

    // Fatal errors should terminate
    if (level == LogLevel::Fatal) {
        Flush();
        std::abort();
    }
}

std::string Logger::LevelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Trace:   return "TRACE";
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "?????";
    }
}

std::string Logger::GetCurrentTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

void Logger::WriteToConsole(const LogEntry& entry)
{
    std::lock_guard<std::mutex> lock(consoleMutex_);

    std::ostringstream oss;
    FormatMessage(oss, entry);

    // Color codes for Windows console
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(hConsole, &consoleInfo);

        WORD attributes = consoleInfo.wAttributes;
        switch (entry.level) {
            case LogLevel::Error:
            case LogLevel::Fatal:
                attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Warning:
                attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Info:
                attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                break;
            case LogLevel::Debug:
            case LogLevel::Trace:
                attributes = FOREGROUND_INTENSITY;
                break;
            default:
                break;
        }

        SetConsoleTextAttribute(hConsole, attributes);
    }

    std::cout << oss.str() << std::endl;

    // Reset attributes
    if (hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}

void Logger::WriteToFile(const LogEntry& entry)
{
    std::lock_guard<std::mutex> lock(fileMutex_);

    std::ostringstream oss;
    FormatMessage(oss, entry);

    logFile_ << oss.str() << std::endl;
    logFile_.flush();
}

void Logger::FormatMessage(std::ostringstream& oss, const LogEntry& entry)
{
    oss << "[" << entry.timestamp << "] ";
    oss << "[" << LevelToString(entry.level) << "] ";
    oss << "[Thread " << entry.threadId << "] ";

    if (!entry.function.empty()) {
        oss << "[" << entry.function << "] ";
    }

    oss << entry.message;
}

void Logger::Flush()
{
    if (logFile_.is_open()) {
        logFile_.flush();
    }
    std::cout.flush();
}

// ScopedLog implementation
ScopedLog::ScopedLog(const std::string& function, LogLevel level)
    : function_(function)
    , level_(level)
    , startTime_(std::chrono::steady_clock::now())
{
    Logger::GetInstance().Log(level_, "Entering: " + function_);
}

ScopedLog::~ScopedLog()
{
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime_).count();

    std::ostringstream oss;
    oss << "Exiting: " << function_ << " (took " << duration << " μs)";

    Logger::GetInstance().Log(level_, oss.str());
}

} // namespace Utils
} // namespace UltraImageViewer
