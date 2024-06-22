#ifndef CPPCMAKE_BACKEND_HPP
#define CPPCMAKE_BACKEND_HPP

#include <string>
#include <vector>

#include "util_funcs.hpp"

/*
 *
command = g++ -Itest/include -c $in -o $out
depfile = $out.d
deps = gcc
msvc_deps_prefix = Note:
description = Compiling $in to $out
generator = true
restat = true
rspfile = $out.rsp
rspfile_content = --input $in --output $out
 */
namespace CppCmake {

    struct Rule {
        std::string name;
        std::string command;
        std::string depfile;
        std::string deps;
        std::string msvc_deps_prefix;
        std::string description;
        std::string generator;
        std::string restat;
        std::string rspfile;
        std::string rspfile_content;
    };

    struct BuildTarget {
        std::string src;
        std::string target;
    };

    class Make {
    public:

        void setVar(std::string &&key, std::string &&val);

        void setDefault(std::string &&def);

        void addRule(CppCmake::Rule &&rule);

        void addBuildTarget(CppCmake::BuildTarget &&build);

        NORETURN void build(int argc, char **argv);

    private:
        std::string cxx_;
        std::string def_;
        std::string src_;
        std::string cflags_;
        std::string default_;
        std::string build_dir_;

        std::vector<std::pair<std::string, std::string>> mappings_;
        std::vector<CppCmake::Rule> rules_;
        std::vector<CppCmake::BuildTarget> builds_;

        std::string generate_string_();
    };


};

#endif  //CPPCMAKE_BACKEND_HPP
