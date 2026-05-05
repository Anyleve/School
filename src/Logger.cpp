#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() = default;

void Logger::set_log_file(std::string path) {
    std::lock_guard<std::mutex> lock(m_);
    path_ = std::move(path);
    file_.close();
    file_.open(path_, std::ios::app);
}

void Logger::set_min_level(Level level) {
    std::lock_guard<std::mutex> lock(m_);
    min_level_ = level;
}

void Logger::debug(const std::string& msg) { log(Level::Debug, msg); }
void Logger::info(const std::string& msg) { log(Level::Info, msg); }
void Logger::warn(const std::string& msg) { log(Level::Warn, msg); }
void Logger::error(const std::string& msg) { log(Level::Error, msg); }

void Logger::log(Level level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_);
    if (static_cast<int>(level) < static_cast<int>(min_level_)) {
        return;
    }

    const std::string line = now_string() + " [" + level_to_string(level) + "] " + msg;

    if (file_.is_open()) {
        file_ << line << "\n";
        file_.flush();
    }

    if (level == Level::Error) {
        std::cerr << line << "\n";
    } else {
        std::cout << line << "\n";
    }
}

std::string Logger::level_to_string(Level level) {
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
    }
    return "INFO";
}

std::string Logger::now_string() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const std::time_t t = clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

