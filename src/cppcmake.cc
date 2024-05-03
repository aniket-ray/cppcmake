#include "cppcmake_backend.hpp"


int main(int argc, char **argv) {
//    CppCmake::real_main(argc, argv);
    CppCmake::Make make;

    make.setCxx("g++");
    make.setCflags("-Wall -std=c++11");
    make.setSrc(".");
    make.setBuildDir("build");

    make.addRule({
            .name = "ensure_dir",
            .command = "/Users/aniket/NYU/CppCmake/test",
            .description = "Ensure directory $out exists"
    });

    make.addRule({
            .name = "compile",
            .command = "$cxx $cflags -c $in -o $out",
            .description = "Compile $out"
    });

    make.addRule({
            .name = "link",
            .command = "$cxx $in -o $out",
            .description = "Link $out"
    });

    make.addBuildTarget({
            .src = "helloworld.o",
            .target = "compile helloworld.cpp"
    });

    make.addBuildTarget({
            .src = "helloworld",
            .target = "link helloworld.o"
    });

    make.setDefault("helloworld");

    make.build(argc, argv);

}
