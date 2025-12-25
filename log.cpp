#include "log.h"


// instance()
Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}

// levelToString()
std::string Logger::levelToString(LogLevel level)
{
    switch(level)
    {
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::DEBUG:   return "DEBUG";

    }
    return "UNKNOWN";
}

// timestamp()
std::string Logger::timestamp()
{
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}


// init()
void Logger::init(const std::string& filename, LogLevel level = LogLevel::INFO)
{
    instance().file.open(filename, std::ios::out | std::ios::app);
    instance().currentLevel = level;
}


// setLevel()
void Logger::setLevel(LogLevel level)
{
    instance().currentLevel = level;
}

// log()
void Logger::log(LogLevel level, const std::string& message)
{
    if(level > instance().currentLevel)
        return ;
    std::string line = timestamp() + "[" + levelToString(level) + "]" + message;
    if(instance().file.is_open())
    {
        instance().file << line << std::endl;
    }
    else
    {
    std::cout << line << std::endl;
    }
}