#ifndef CPPCMAKE_BUILD_H
#define CPPCMAKE_BUILD_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace cppcmake {
    enum class rules {
    // can be copied from ninja validators
    };

    struct BuildParams {
        std::vector<std::string> ins;
        std::vector<std::string> outs;
        cppcmake::rules rule;
    };

    class cppcmake {
    public:
        cppcmake(const std::string &project_name, const std::string &project_version);

        ~cppcmake();

        bool generate_ninja_dependecy_graph_(void);

        // Remove constructors
        cppcmake(const cppcmake &) = delete;

        cppcmake(cppcmake &&) = delete;

        cppcmake operator=(const cppcmake &) = delete;

        cppcmake operator=(cppcmake &&) = delete;

        void add_library(std::string lib_name, std::vector<std::string> lib_files);

        void add_executable(std::string exec, std::vector<std::string> source_files);

        void target_compile_definitions(std::vector<std::string> defs);

        // TODO: version needs to be validated with regex
        void setNinjaVersion(std::string version);

        void setProjectVersion(std::string version);

    private:
        std::filesystem::path workspace_scope_;
        // Where to check for the source files
        std::string project_name_;
        std::string project_version_;
        std::string ninja_version_ = "1.11.0";
        std::unordered_map<std::string, std::vector<std::string>> lib_src_map_;
        // Contains (key [string]) Library Name = (values [str, str, ...]) Library Files
        std::vector<std::string> proj_headers_;
        std::vector<std::string> proj_src_;

        static std::unordered_set<std::string> CONFIG_OPTIONS;
    };
};

#endif //CPPCMAKE_BUILD_H

