#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <mutex>
#include <memory>
#include <filesystem>
#include <thread>
#include <chrono>

namespace UltraImageViewer {
namespace Utils {

/**
 * Log levels
 */
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

/**
 * Log entry with metadata
 */
struct LogEntry {
    LogLevel level;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::string timestamp;
    std::thread::id threadId;

    LogEntry() : line(0), threadId(std::this_thread::get_id()) {}
};

/**
 * Structured logger for application logging
 * Features:
 * - Multiple log levels
 * - Thread-safe
 * - File and console output
 * - Structured logging support
 */
class Logger {
public:
    // Get singleton instance
    static Logger& GetInstance();

    // Initialize logger
    void Initialize(const std::filesystem::path& logFile = L"");
    void Shutdown();

    // Logging methods
    void Log(LogLevel level, const std::string& message);
    void Log(LogLevel level, const std::string& message, const char* file, int line, const char* function);

    // Convenience methods
    void Trace(const std::string& msg) { Log(LogLevel::Trace, msg); }
    void Debug(const std::string& msg) { Log(LogLevel::Debug, msg); }
    void Info(const std::string& msg) { Log(LogLevel::Info, msg); }
    void Warning(const std::string& msg) { Log(LogLevel::Warning, msg); }
    void Error(const std::string& msg) { Log(LogLevel::Error, msg); }
    void Fatal(const std::string& msg) { Log(LogLevel::Fatal, msg); }

    // Configure
    void SetMinimumLevel(LogLevel level) { minimumLevel_ = level; }
    LogLevel GetMinimumLevel() const { return minimumLevel_; }
    void EnableConsoleOutput(bool enable) { consoleOutput_ = enable; }
    void EnableFileOutput(bool enable) { fileOutput_ = enable; }

    // Flush log buffer
    void Flush();

private:
    Logger();
    ~Logger();

    // Singleton - prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Helper methods
    std::string LevelToString(LogLevel level) const;
    std::string GetCurrentTimestamp() const;
    void WriteToConsole(const LogEntry& entry);
    void WriteToFile(const LogEntry& entry);
    void FormatMessage(std::ostringstream& oss, const LogEntry& entry);

    // Configuration
    LogLevel minimumLevel_;
    bool consoleOutput_;
    bool fileOutput_;
    bool initialized_;

    // File output
    std::filesystem::path logFilePath_;
    std::ofstream logFile_;
    std::mutex fileMutex_;

    // Console output
    std::mutex consoleMutex_;
};

/**
 * RAII helper for scoped logging
 * Logs function entry/exit automatically
 */
class ScopedLog {
public:
    ScopedLog(const std::string& function, LogLevel level = LogLevel::Trace);
    ~ScopedLog();

private:
    std::string function_;
    LogLevel level_;
    std::chrono::steady_clock::time_point startTime_;
};

/**
 * Stream-based logging macro
 * Usage: LOG_INFO << "Message " << value;
 */
#define LOG_STREAM_HELPER(logger, level) \
    if (logger.GetMinimumLevel() <= level) \
        std::ostringstream().flush()

// Logging macros with file/line info
#define LOG_TRACE(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Trace, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

#define LOG_DEBUG(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Debug, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

#define LOG_INFO(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Info, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

#define LOG_WARNING(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Warning, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

#define LOG_ERROR(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Error, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

#define LOG_FATAL(msg) \
    UltraImageViewer::Utils::Logger::GetInstance().Log( \
        UltraImageViewer::Utils::LogLevel::Fatal, \
        std::string(msg), __FILE__, __LINE__, __FUNCTION__)

// Scoped logging
#define SCOPED_LOG() \
    UltraImageViewer::Utils::ScopedLog __log__(__FUNCTION__)

#define SCOPED_LOG_LEVEL(level) \
    UltraImageViewer::Utils::ScopedLog __log__(__FUNCTION__, level)

} // namespace Utils
} // namespace UltraImageViewer
