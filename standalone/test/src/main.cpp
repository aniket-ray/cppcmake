// src/main.cpp
#include "../include/hello.h"
#include <iostream>
#include "../include/add.h"

int main() {
    say_hello();
  std::cout  << add_two(1,2) << std::endl;
    return 0;
}
