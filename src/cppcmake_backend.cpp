#include "cppcmake_backend.hpp"

void CppCmake::Make::setVar(std::string&& key, std::string&& val) {
    mappings_.emplace_back(key, val);
    this->cppCmakeMain_->state_.bindings_.AddBinding(key, val);
}

void CppCmake::Make::setDefault(std::string&& def) {
    this->default_ = def;
}

void CppCmake::Make::addRule(Rule&& rule) {
    this->rules_.emplace_back(rule);
}

void CppCmake::Make::addBuildTarget(BuildTarget&& build) {
    this->builds_.emplace_back(build);
}

std::string CppCmake::Make::generate_string_() {
    std::string out;
    out += "cxx = " + this->cxx_ + "\n";
    out += "cflags = " + this->cflags_ + "\n\n";

    for (const auto& m : mappings_) {
        out += m.first + " = " + m.second + "\n";
    }

    // rule
    for (auto r : this->rules_) {
        out += "rule " + r.name + "\n";
        out += "  ";
        out += "command = " + r.command + "\n";
        out += "  ";
        out += "description = " + r.description + "\n\n";
    }

    for (auto b : this->builds_) {
        out += "build ";
        out += b.src + ": " + b.target + "\n";
        out += "\n";
    }

    out += "default " + this->default_;
    out += "\n";
    return out;
}

//"cxx = g++\ncflags = -Wall -std=c++11\nsrc_dir = .\nbuild_dir = build\n\nrule ensure_dir\n  command = mkdir -p $out\n  description = Ensure directory $out exists\n\nbuild ${build_dir}: ensure_dir\n\nrule compile\n  command = $cxx $cflags -c $in -o $out\n  description = Compile $out\n\nrule link\n  command = $cxx $in -o $out\n  description = Link $out\n\nbuild ${build_dir}/my_lib.o: compile ${src_dir}/lib/my_lib.cpp\nbuild ${build_dir}/main.o: compile ${src_dir}/main.cpp | ${build_dir}\n\nbuild my_app: link ${build_dir}/main.o ${build_dir}/my_lib.o\n\ndefault my_app\n"

void CppCmake::Make::build(int argc, char** argv) {
    // Use exit() instead of return in this function to avoid potentially
    // expensive cleanup when destructing CppCmakeMain.


    options_.input_file = "build.ninja";

    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    this->cppcmake_command_ = argv[0];

    int exit_code = ReadFlags(&argc, &argv, &options_, &config_);
    if (exit_code >= 0)
        exit(exit_code);

    Status* status = Status::factory(config_);

    if (options_.working_dir) {
        // The formatting of this std::string, complete with funny quotes, is
        // so Emacs can properly identify that the cwd has changed for
        // subsequent commands.
        // Don't print this if a tool is being used, so that tool output
        // can be piped into a file without this std::string showing up.
        if (!options_.tool && config_.verbosity != BuildConfig::NO_STATUS_UPDATE)
            status->Info("Entering directory `%s'", options_.working_dir);
        if (chdir(options_.working_dir) < 0) {
            Fatal("chdir to '%s' - %s", options_.working_dir, strerror(errno));
        }
    }

    if (options_.tool && options_.tool->when == CppCmake::Tool::RUN_AFTER_FLAGS) {
        // None of the RUN_AFTER_FLAGS actually use a CppCmakeMain, but it's needed
        // by other tools.
        CppCmake::CppCmakeMain cppcmake(this->cppcmake_command_, config_);
        exit((cppcmake.*options_.tool->func)(&options_, argc, argv));
    }

    // Limit number of rebuilds, to prevent infinite loops.
    const int kCycleLimit = 100;
    for (int cycle = 1; cycle <= kCycleLimit; ++cycle) {
        CppCmake::CppCmakeMain cppcmake(this->cppcmake_command_, this->config_);

        ManifestParserOptions parser_opts;
        if (options_.phony_cycle_should_err) {
            parser_opts.phony_cycle_action_ = kPhonyCycleActionError;
        }
        ManifestParser parser(&cppcmake.state_, &cppcmake.disk_interface_, parser_opts);
        std::string err;
        if (!parser.Load(this->generate_string_(), &err)) {
            status->Error("%s", err.c_str());
            exit(1);
        }
        options_.input_file = "build.nija";

        if (options_.tool && options_.tool->when == CppCmake::Tool::RUN_AFTER_LOAD)
            exit((cppcmake.*options_.tool->func)(&options_, argc, argv));

        if (!cppcmake.EnsureBuildDirExists())
            exit(1);

        if (!cppcmake.OpenBuildLog() || !cppcmake.OpenDepsLog())
            exit(1);

        if (options_.tool && options_.tool->when == CppCmake::Tool::RUN_AFTER_LOGS)
            exit((cppcmake.*options_.tool->func)(&options_, argc, argv));

        // Attempt to rebuild the manifest before building anything else
        if (cppcmake.RebuildManifest(options_.input_file, &err, status)) {
            // In dry_run mode the regeneration will succeed without changing the
            // manifest forever. Better to return immediately.
            if (config_.dry_run)
                exit(0);
            // Start the build over with the new manifest.
            continue;
        } else if (!err.empty()) {
            status->Error("rebuilding '%s': %s", options_.input_file, err.c_str());
            exit(1);
        }

        cppcmake.ParsePreviousElapsedTimes();

        int result = cppcmake.RunBuild(argc, argv, status);
        if (g_metrics)
            cppcmake.DumpMetrics();
        exit(result);
    }

    status->Error("manifest '%s' still dirty after %d tries, perhaps system time is not set", options_.input_file,
                  kCycleLimit);
    exit(1);
}

CppCmake::Make::Make() {
    this->cppCmakeMain_ = std::make_unique<CppCmake::CppCmakeMain>(this->cppcmake_command_, this->config_);
}
