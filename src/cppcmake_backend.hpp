#ifndef CPPCMAKE_CPPCMAKE_BACKEND_HPP
#define CPPCMAKE_CPPCMAKE_BACKEND_HPP

#include <string>
#include <vector>

#include "cppcmake_utils.hpp"

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
        void setCxx(std::string &&compiler);

        void setCflags(std::string &&flags);

        void setSrc(std::string &&src);

        void setBuildDir(std::string &&build_dir);

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

        std::vector<CppCmake::Rule> rules_;
        std::vector<CppCmake::BuildTarget> builds_;

        std::string generate_string_();
    };


};

#endif //CPPCMAKE_CPPCMAKE_BACKEND_HPP
