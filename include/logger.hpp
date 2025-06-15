#include <iostream>

enum class LogLevel {INFO, WARNING, ERROR};

void Log(LogLevel level, const std::string& msg) {
    std::string levelStr;
    switch (level) {
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARNING: levelStr = "WARN"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
    }

    std::cout << "[" << levelStr << "] " << msg << std::endl;
}