#ifndef CPPCMAKE_CPPCMAKE_BACKEND_HPP
#define CPPCMAKE_CPPCMAKE_BACKEND_HPP

#include <string>

namespace CppCmake {
    namespace Backend {
        struct KVP {
            std::string cxx;
            std::string cflags;

            CppCmake::ManifestParser generateParser(); // --> ManifestParser::Parse(with  data inside of KVP)
        };

    }
};

#endif //CPPCMAKE_CPPCMAKE_BACKEND_HPP
