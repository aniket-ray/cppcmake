#include "cppcmake_utils.hpp"
#include "cppcmake_backend.hpp"
#include <iostream>
int main(int argc, char **argv) {

    CppCmake::Backend::BuildNinjaConfig config;
    config.setCXX("g++");
    config.setCFlags("-Wall -Wextra -O2");

    config.addRule("compile", "$cxx $cflags -c $in -o $out", "Compiling $out");
    config.addRule("link", "$cxx $in -o $out", "Linking $out");

    config.addBuild("helloworld.o", "compile", "helloworld.cpp");
    config.addBuild("helloworld", "link", "helloworld.o");

    config.setDefaultTarget("helloworld");

    std::cout << "Compiler: " << config.cxx << std::endl;
    std::cout << "Compiler Flags: " << config.cflags << std::endl;
    std::cout << "Default Target: " << config.getDefaultTarget() << std::endl;
    std::cout << "Compile Rule: " << config.getRule("compile").toString() << std::endl;

//    CppCmake::real_main(argc, argv);
}
