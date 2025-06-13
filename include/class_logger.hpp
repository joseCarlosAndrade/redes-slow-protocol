#pragma once

#include <string>

// example of how to use public classes (that are seen from all modules)
class Logger {
    public:
        void class_logging(const std::string& msg); // implemented on src/class_logger/class_logger.cpp
};