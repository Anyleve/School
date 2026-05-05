#pragma once

#include <fstream>
#include <mutex>
#include <string>

class Logger final {
public:
    enum class Level { Debug, Info, Warn, Error };

    static Logger& instance();

    void set_log_file(std::string path);
    void set_min_level(Level level);

    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

    void log(Level level, const std::string& msg);

private:
    Logger();

    static std::string level_to_string(Level level);
    static std::string now_string();

    std::mutex m_;
    std::ofstream file_;
    std::string path_;
    Level min_level_ = Level::Info;
};

