// example of public header, can be used by other modules

#pragma once // substitutes #ifndef, #define, #endif

#include <string>

void public_print(const std::string& message); // implemented on src/example_module/example_internal.cpp

