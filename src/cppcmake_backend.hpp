#ifndef CPPCMAKE_CPPCMAKE_BACKEND_HPP
#define CPPCMAKE_CPPCMAKE_BACKEND_HPP

#include <string>
#include <unordered_map>
#include <vector>

// example of input string
//"cxx = g++\n"
//"cflags = -Wall -Wextra -O2\n\n"
//"rule compile\n"
//"  command = $cxx $cflags -c $in -o $out\n"
//"  description = Compiling $out\n\n"
//"rule link\n"
//"  command = $cxx $in -o $out\n"
//"  description = Linking $out\n\n"
//"build helloworld.o: compile helloworld.cpp\n"
//"build helloworld: link helloworld.o\n\n"
//"default helloworld\n";
namespace CppCmake {
    namespace Backend {
        struct Rule {
            std::string name;
            std::string command;
            std::string description;
            //return a string for log
            std::string toString() const {
                return "Rule(Name: " + name + ", Command: " + command + ", Description: " + description + ")";
            }
        };

        struct Build {
            std::string target;
            std::string rule;
            std::string source;
            std::string toString() const {
                return "Build(target: " + target + ", rule: " + rule + ", source: " + source + ")";
            }
        };

        struct BuildNinjaConfig {
            std::string cxx;
            std::string cflags;
            std::unordered_map<std::string, Rule> rules;
            std::vector<Build> builds;
            std::string defaultTarget;

            void setCXX(const std::string& compiler) {
                cxx = compiler;
            }

            void setCFlags(const std::string& flags) {
                cflags = flags;
            }

            void addRule(const std::string& name, const std::string& command, const std::string& description) {
                rules[name] = Rule{name, command, description};
            }

            void addBuild(const std::string& target, const std::string& rule, const std::string& source) {
                builds.push_back({target, rule, source});
            }

            void setDefaultTarget(const std::string& target) {
                defaultTarget = target;
            }

            Rule getRule(const std::string& name) const {
//                return rules[name];// should not using [] operation
                auto iterator = rules.find(name);
                if (iterator != rules.end()) {
                    return iterator->second;
                } else {
                    return Rule();
                }
            }

            std::vector<Build> getBuilds() const {
                return builds;
            }

            std::string getDefaultTarget() const {
                return defaultTarget;
            }
        };

        struct KVP {
            std::string cxx;
            std::string cflags;

//            CppCmake::ManifestParser generateParser(); // --> ManifestParser::Parse(with  data inside of KVP)
        };

    }
};

#endif //CPPCMAKE_CPPCMAKE_BACKEND_HPP
