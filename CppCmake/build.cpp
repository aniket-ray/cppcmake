#include "build.hpp"

cppcmake::cppcmake::cppcmake(const std::string& project_name, const std::string& project_version = "1.0")
    : project_name_{project_name}, project_version_{project_version} {}