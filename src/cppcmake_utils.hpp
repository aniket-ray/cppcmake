#ifndef CPPCMAKE_CPPCMAKE_UTILS_HPP
#define CPPCMAKE_CPPCMAKE_UTILS_HPP

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>

#include <getopt.h>
#include <unistd.h>


#include "../deps/build.h"
#include "../deps/build_log.h"
#include "../deps/deps_log.h"
#include "../deps/clean.h"
#include "../deps/debug_flags.h"
#include "../deps/disk_interface.h"
#include "../deps/graphviz.h"
#include "../deps/json.h"
#include "../deps/manifest_parser.h"
#include "../deps/metrics.h"
#include "../deps/missing_deps.h"
#include "../deps/state.h"
#include "../deps/status.h"
#include "../deps/version.h"

namespace CppCmake {
    enum PrintCommandMode {
        PCM_Single, PCM_All
    };
    enum EvaluateCommandMode {
        ECM_NORMAL,
        ECM_EXPAND_RSPFILE
    };

    struct Tool;

    struct Options {
        const char *input_file;
        const char *working_dir;
        const Tool *tool;
        bool phony_cycle_should_err;
    };

    struct CppCmakeMain : public BuildLogUser {
        CppCmakeMain(const char *cppcmake_command, const BuildConfig &config) :
                cppcmake_command_(cppcmake_command), config_(config),
                start_time_millis_(GetTimeMillis()) {}

        /// Command line used to run CppCmake.
        const char *cppcmake_command_;

        /// Build configuration set from flags (e.g. parallelism).
        const BuildConfig &config_;

        /// Loaded state (rules, nodes).
        State state_;

        /// Functions for accessing the disk.
        RealDiskInterface disk_interface_;

        /// The build directory, used for storing the build log etc.
        std::string build_dir_;

        BuildLog build_log_;
        DepsLog deps_log_;

        /// The type of functions that are the entry points to tools (subcommands).
        typedef int (CppCmakeMain::*ToolFunc)(const Options *, int, char **);

        /// Get the Node for a given command-line path, handling features like
        /// spell correction.
        Node *CollectTarget(const char *cpath, std::string *err);

        /// CollectTarget for all command-line arguments, filling in \a targets.
        bool CollectTargetsFromArgs(int argc, char *argv[],
                                    std::vector<Node *> *targets, std::string *err);

        // The various subcommands, run via "-t XXX".
        int ToolGraph(const Options *options, int argc, char *argv[]);

        int ToolQuery(const Options *options, int argc, char *argv[]);

        int ToolDeps(const Options *options, int argc, char *argv[]);

        int ToolMissingDeps(const Options *options, int argc, char *argv[]);

        int ToolBrowse(const Options *options, int argc, char *argv[]);

        int ToolMSVC(const Options *options, int argc, char *argv[]);

        int ToolTargets(const Options *options, int argc, char *argv[]);

        int ToolCommands(const Options *options, int argc, char *argv[]);

        int ToolInputs(const Options *options, int argc, char *argv[]);

        int ToolClean(const Options *options, int argc, char *argv[]);

        int ToolCleanDead(const Options *options, int argc, char *argv[]);

        int ToolCompilationDatabase(const Options *options, int argc, char *argv[]);

        int ToolRecompact(const Options *options, int argc, char *argv[]);

        int ToolRestat(const Options *options, int argc, char *argv[]);

        int ToolUrtle(const Options *options, int argc, char **argv);

        int ToolRules(const Options *options, int argc, char *argv[]);

        int ToolWinCodePage(const Options *options, int argc, char *argv[]);

        /// Open the build log.
        /// @return false on error.
        bool OpenBuildLog(bool recompact_only = false);

        /// Open the deps log: load it, then open for writing.
        /// @return false on error.
        bool OpenDepsLog(bool recompact_only = false);

        /// Ensure the build directory exists, creating it if necessary.
        /// @return false on error.
        bool EnsureBuildDirExists();

        /// Rebuild the manifest, if necessary.
        /// Fills in \a err on error.
        /// @return true if the manifest was rebuilt.
        bool RebuildManifest(const char *input_file, std::string *err, Status *status);

        /// For each edge, lookup in build log how long it took last time,
        /// and record that in the edge itself. It will be used for ETA predicton.
        void ParsePreviousElapsedTimes();

        /// Build the targets listed on the command line.
        /// @return an exit code.
        int RunBuild(int argc, char **argv, Status *status);

        /// Dump the output requested by '-d stats'.
        void DumpMetrics();

        virtual bool IsPathDead(StringPiece s) const {
            Node *n = state_.LookupNode(s);
            if (n && n->in_edge())
                return false;
            // Just checking n isn't enough: If an old output is both in the build log
            // and in the deps log, it will have a Node object in state_.  (It will also
            // have an in edge if one of its inputs is another output that's in the deps
            // log, but having a deps edge product an output that's input to another deps
            // edge is rare, and the first recompaction will delete all old outputs from
            // the deps log, and then a second recompaction will clear the build log,
            // which seems good enough for this corner case.)
            // Do keep entries around for files which still exist on disk, for
            // generators that want to use this information.
            std::string err;
            TimeStamp mtime = disk_interface_.Stat(s.AsString(), &err);
            if (mtime == -1)
                Error("%s", err.c_str());  // Log and ignore Stat() errors.
            return mtime == 0;
        }

        int64_t start_time_millis_;
    };

    struct Tool {
        /// Short name of the tool.
        const char *name;

        /// Description (shown in "-t list").
        const char *desc;

        /// When to run the tool.
        enum {
            /// Run after parsing the command-line flags and potentially changing
            /// the current working directory (as early as possible).
            RUN_AFTER_FLAGS,

            /// Run after loading build.cppcmake.
            RUN_AFTER_LOAD,

            /// Run after loading the build/deps logs.
            RUN_AFTER_LOGS,
        } when;

        /// Implementation of the tool.
        CppCmakeMain::ToolFunc func;
    };

    void Usage(const BuildConfig &config);

    int GuessParallelism();

    class DeferGuessParallelism {
    public:
        bool needGuess;
        BuildConfig *config;

        explicit DeferGuessParallelism(BuildConfig *config)
                : needGuess(true), config(config) {}

        void Refresh() {
            if (needGuess) {
                needGuess = false;
                config->parallelism = GuessParallelism();
            }
        }

        ~DeferGuessParallelism() { Refresh(); }
    };

    int ToolTargetsList(const std::vector<Node *> &nodes, int depth, int indent);

    int ToolTargetsSourceList(State *state);

    int ToolTargetsList(State *state, const std::string &rule_name);

    int ToolTargetsList(State *state);

    bool WarningEnable(const std::string &name, Options *options);

    void PrintCommands(Edge *edge, EdgeSet *seen, PrintCommandMode mode);

    void CollectInputs(Edge *edge, std::set<Edge *> *seen, std::vector<std::string> *result);

    std::string EvaluateCommandWithRspfile(const Edge *edge, const EvaluateCommandMode mode);

    void printCompdb(const char *const directory, const Edge *const edge, const EvaluateCommandMode eval_mode);

    const Tool *ChooseTool(const std::string &tool_name);

    bool DebugEnable(const std::string &name);

    int ReadFlags(int *argc, char ***argv, Options *options, BuildConfig *config);

    NORETURN void real_main(int argc, char **argv);
};

#endif //CPPCMAKE_CPPCMAKE_UTILS_HPP
