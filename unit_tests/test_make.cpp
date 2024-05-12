#include <cassert>
#include <iostream>
#include <vector>

#include "../src/cppcmake_backend.hpp"
#include "../src/cppcmake_utils.hpp"

class TestMake {
 public:
  static void testSetVar() {
    CppCmake::Make make;
    make.setVar("cxx", "g++");
    // Assuming getVar is a method that retrieves the value of a variable.
    assert(make.getVar("cxx") == "g++");
    std::cout << "testSetVar passed.\n";
  }

  static void testAddRule() {
    CppCmake::Make make;
    CppCmake::Rule compileRule = {
        .name = "compile", .command = "$cxx -c $cflags -o $out $in", .description = "Compiling $in"};
    make.addRule(std::move(compileRule));
    // Assuming getRule is a method that retrieves a rule by name.
    assert(make.getRule("compile").command == "$cxx -c $cflags -o $out $in");
    std::cout << "testAddRule passed.\n";
  }

  static void testAddBuildTarget() {
    CppCmake::Make make;
    CppCmake::BuildTarget buildTarget = {.src = "obj/hello.o", .target = "compile src/hello.cpp"};
    make.addBuildTarget(std::move(buildTarget));
    // Assuming getBuildTarget is a method that retrieves a build target by source.
    assert(make.getBuildTarget("obj/hello.o").target == "compile src/hello.cpp");
    std::cout << "testAddBuildTarget passed.\n";
  }

  static void testSetDefault() {
    CppCmake::Make make;
    make.setDefault("hello");
    // Assuming getDefault is a method that retrieves the default build target.
    assert(make.getDefault() == "hello");
    std::cout << "testSetDefault passed.\n";
  }
};

int main() {
  TestMake::testSetVar();
  TestMake::testAddRule();
  TestMake::testAddBuildTarget();
  TestMake::testSetDefault();

  return 0;
}
