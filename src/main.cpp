#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>
#include <ctime>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    template<typename... Args>
    void log(Level level, Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::stringstream message;
        message << getTimestamp() << " [" << getLevelString(level) << "] ";
        (message << ... << std::forward<Args>(args));
        message << '\n';
        
        std::cout << message.str();
        if (file_.is_open()) {
            file_ << message.str();
            file_.flush();
        }
    }

    void setLogFile(const std::string& filename) {
        if (file_.is_open()) {
            file_.close();
        }
        file_.open(filename, std::ios::app);
    }

    

private:
    Logger() = default;
    ~Logger() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    static const char* getLevelString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            case Level::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    std::mutex mutex_;
    std::ofstream file_;
};

int main() {
    auto& logger = Logger::getInstance();
    logger.setLogFile("app.log");

    logger.log(Logger::Level::INFO, "Application started");
    logger.log(Logger::Level::DEBUG, "Debug message: ", 42);
    logger.log(Logger::Level::WARNING, "Warning: ", "Resource running low");
    logger.log(Logger::Level::ERROR, "Error occurred: ", "Connection failed");

    return 0;
}
