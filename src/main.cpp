#include "example.hpp"
#include "class_logger.hpp"

int main() {
    public_print(std::string("this is a print!"));

    Logger l;
    l.class_logging(std::string("this is another log! but from the class"));
}