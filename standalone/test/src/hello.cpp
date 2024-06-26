// src/hello.cpp
#include "../include/hello.h"
#include <iostream>
#include "../include/add.h"

void say_hello() {
  std::cout  << add_two(1,2) << std::endl;
  std::cout << "Hello, Ninja!" << std::endl;
}
