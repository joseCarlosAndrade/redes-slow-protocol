#include "example_internal.hpp" // internal header, only this module can see
#include"example.hpp" // implmements the public function so that other modules can use it
#include<iostream>

// internally implemented
void internal_print(const std::string& message) {
    std::cout << "Internal LOG: " << message << std::endl;
}

// implements the method of the public example.hpp so other modules can access it. They dont worry
// about this implementation
void public_print(const std::string& message) {
    internal_print(message);
}
