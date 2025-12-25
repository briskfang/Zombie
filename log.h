#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <ctime>


enum class LogLevel{ ERROR = 0, WARNING, INFO, DEBUG };


class Logger
{
    private:
        std::ofstream file;
        LogLevel currentLevel = LogLevel::INFO;
        static Logger& instance();
        static std::string levelToString(LogLevel level);
        static std::string timestamp();

    public: 
        static void init(const std::string& filename, LogLevel level);
        static void setLevel(LogLevel level);
        static void log(LogLevel level, const std::string& message);
        
};


// Log example:  2025-12-25 14:22:11 [INFO] [main.cpp:34] Game starts
// at lease one operand of operator "+" must be a std::string for string concatenation to work.
// that's why we use std::string("[") at the beginning, but use "]", instead of std::string("]")

#define LOG_ERROR(msg) \
    Logger::log(LogLevel::ERROR,   std::string("[") + __FILE__ + ":" + std::to_string(__LINE__) + "]" + msg);

#define LOG_WARNING(msg) \
    Logger::log(LogLevel::WARNING, std::string("[") + __FILE__ + ":" + std::to_string(__LINE__) + "]" + msg);

#define LOG_INFO(msg) \
    Logger::log(LogLevel::INFO,    std::string("[") + __FILE__ + ":" + std::to_string(__LINE__) + "]" + msg);

#define LOG_DEBUG(msg) \
    Logger::log(LogLevel::DEBUG,   std::string("[") + __FILE__ + ":" + std::to_string(__LINE__) + "]" + msg);




