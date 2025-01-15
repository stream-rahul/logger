#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>
#include <ctime>
#include <map>
#include <deque>
#include <memory>
#include <limits>  // For INT_MAX and INT_MIN
#include <cstddef> // For size_t

// Forward declare Logger::Level for Filter interface
class Logger;
enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// Filter Interface - Complete definition
class Filter
{
public:
    virtual bool shouldLog(LogLevel level,
                           const std::string &message,
                           const std::map<LogLevel, size_t> &levelCounts) = 0;
    virtual ~Filter() = default;
};

class Logger
{
public:
    using Level = LogLevel; // Alias for consistency

    struct LogMessage
    {
        Level level;
        std::string message;
    };

    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    template <typename... Args>
    void log(Level level, Args &&...args)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::stringstream messageStream;
        messageStream << getTimestamp() << " [" << getLevelString(level) << "] ";
        (messageStream << ... << std::forward<Args>(args));
        std::string msg = messageStream.str() + '\n';

        // Apply the filter (if set)
        if (filter_ && !filter_->shouldLog(level, msg, levelCounts_))
        {
            return; // Skip logging if the filter rejects the message
        }

        // Log the message
        std::cout << msg;
        if (file_.is_open())
        {
            file_ << msg;
            file_.flush();
        }

        // Update counts and message history
        levelCounts_[level]++;
        messages_.push_back({level, msg});
        if (messages_.size() > maxMessageHistory_)
        {
            const LogMessage &oldest = messages_.front();
            levelCounts_[oldest.level]--;
            messages_.pop_front();
        }
    }

    void setLogFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open())
        {
            file_.close();
        }
        file_.open(filename, std::ios::app);
    }

    void setFilter(std::unique_ptr<Filter> filter)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        filter_ = std::move(filter);
    }

private:
    Logger() : maxMessageHistory_(1000) {} // Set a maximum history size
    ~Logger()
    {
        if (file_.is_open())
        {
            file_.close();
        }
    }

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    static std::string getTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    static const char *getLevelString(Level level)
    {
        switch (level)
        {
        case Level::DEBUG:
            return "DEBUG";
        case Level::INFO:
            return "INFO";
        case Level::WARNING:
            return "WARNING";
        case Level::ERROR:
            return "ERROR";
        case Level::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
        }
    }

    std::mutex mutex_;
    std::ofstream file_;
    std::map<Level, size_t> levelCounts_;
    std::deque<LogMessage> messages_;
    size_t maxMessageHistory_;
    std::unique_ptr<Filter> filter_;
};

// CountFilter Implementation
class CountFilter final : public Filter
{
public:
    CountFilter(size_t threshold, LogLevel level)
        : threshold_(threshold), level_(level) {}

    bool shouldLog(LogLevel level,
                   const std::string &message,
                   const std::map<LogLevel, size_t> &levelCounts) override
    {
        (void)message; // Unused parameter
        if (level != level_)
        {
            return true; // Only filter specific level
        }
        auto it = levelCounts.find(level_);
        if (it == levelCounts.end())
        {
            return true; // No count yet, allow logging
        }
        return it->second < threshold_;
    }

private:
    const size_t threshold_;
    const LogLevel level_;
};

// Main Function for Testing
int main()
{
    auto &logger = Logger::getInstance();
    logger.setLogFile("app.log");

    // Set a CountFilter for WARNING messages with a threshold of 2
    logger.setFilter(std::make_unique<CountFilter>(2, Logger::Level::WARNING));

    // Test logging with the filter
    logger.log(Logger::Level::INFO, "Application started");

    logger.log(Logger::Level::WARNING, "Warning 1: Resource running low");   // Logged
    logger.log(Logger::Level::WARNING, "Warning 2: Disk space almost full"); // Logged
    logger.log(Logger::Level::WARNING, "Warning 3: Network latency high");   // Not logged (threshold reached)

    logger.log(Logger::Level::ERROR, "Error occurred: Connection failed"); // Logged (no filter for ERROR)

    return 0;
}