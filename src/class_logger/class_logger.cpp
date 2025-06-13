#include"class_logger.hpp"
#include<iostream>
#include"example.hpp"

// implementing the public method from Logger at include/class_logger.hpp
void Logger::class_logging(const std::string& msg) {
    public_print("[FROM CLASS]: " + msg); // i can use the public function!!! bc i implemented in the include/
}