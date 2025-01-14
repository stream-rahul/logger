#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>
#include <ctime>
#include <limits> // For INT_MAX and INT_MIN

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

    // Add function for arithmetic operations
    template<typename T>
    T add(T a, T b) {
        T result = a + b;
        log(Level::DEBUG, "add(", a, ", ", b, ") = ", result);
        return result;
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

    // Test the add function with integers
    int intResult = logger.add(5, 10);
    logger.log(Logger::Level::INFO, "Integer addition result: ", intResult);

    // Test the add function with floating-point numbers
    double doubleResult = logger.add(3.14, 2.71);
    logger.log(Logger::Level::INFO, "Floating-point addition result: ", doubleResult);

    // Test edge cases
    int maxInt = std::numeric_limits<int>::max();
    int minInt = std::numeric_limits<int>::min();
    logger.add(maxInt, 1); // Overflow test
    logger.add(minInt, -1); // Underflow test

    return 0;
}