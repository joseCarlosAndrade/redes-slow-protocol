#pragma once
#include <iostream>

enum class LogLevel {INFO, WARNING, ERROR};

void Log(LogLevel level, const std::string& msg) ;