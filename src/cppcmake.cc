#include "cppcmake_backend.hpp"


int main(int argc, char **argv) {
//    CppCmake::real_main(argc, argv);
    CppCmake::Make make;

    make.setVar("cxx", "g++");
    make.setVar("cflags", "-I./include -O2");


    make.addRule({
            .name = "compile",
            .command = "$cxx -c $cflags -o $out $in",
            .description = "Compiling $in"
    });

    make.addRule({
            .name = "link",
            .command = "$cxx $in -o $out",
            .description = "Linking $out"
    });



    make.addBuildTarget({
            .src = "obj/hello.o",
            .target = "compile src/hello.cpp"
    });

    make.addBuildTarget({
            .src = "obj/main.o",
            .target = "compile src/main.cpp"
    });
    make.addBuildTarget({
            .src = "hello",
            .target = "link obj/hello.o obj/main.o"
    });

    make.setDefault("hello");

    make.build(argc, argv);

}
