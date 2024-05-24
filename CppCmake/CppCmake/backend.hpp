#ifndef CPPCMAKE_BACKEND_HPP
#define CPPCMAKE_BACKEND_HPP

#include <string>
#include <vector>

#include "util_funcs.hpp"

namespace CppCmake {

    struct Rule {
        std::string name;
        std::string command;
        std::string description;
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
