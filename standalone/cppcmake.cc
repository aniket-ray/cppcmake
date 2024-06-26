#include <numeric>
#include "../CppCmake/CppCmake/backend.hpp"

#include <optional>

using ExposedEvalString = std::pair<std::string, std::string>;
using ExposedEvalStrings = std::vector<ExposedEvalString>;

struct Populator {
  struct EnvRule {
    std::string name;
    ExposedEvalStrings command;
    std::optional<ExposedEvalStrings> depfile;
    std::optional<ExposedEvalStrings> deps;
    std::optional<ExposedEvalStrings> msvc_deps_prefix;
    std::optional<ExposedEvalStrings> description;
    std::optional<ExposedEvalStrings> generator;
    std::optional<ExposedEvalStrings> restat;
    std::optional<ExposedEvalStrings> rspfile;
    std::optional<ExposedEvalStrings> rspfile_content;
  };

  BindingEnv* env_;
  explicit Populator(State* state);
  bool addBinding(std::string key, ExposedEvalStrings value);
  [[nodiscard]] std::string getBinding(std::string key) const;

  bool addRule(Populator::EnvRule&& env_rule);
  bool addBuildTarget(std::string rule_name, std::vector<ExposedEvalStrings> ins, std::vector<ExposedEvalStrings> outs, std::pair<std::string, ExposedEvalStrings> env_bindings
                      //               ExposedEvalString context_vars
  );
};

Populator::Populator(State* state) : env_{&state->bindings_} {}

bool Populator::addBinding(std::string key, ExposedEvalStrings value) {
  std::vector<std::string> buffer;

  for (auto i = value.begin(); i != value.end(); i++) {
    if (i->second == "RAW") {
      buffer.push_back(i->first);
    } else if (i->second == "SPECIAL") {
      // TODO (Aniket): IDK whether to handle empty lookups or not. Currently, not handling.
      //                Reminder: BindingEnv::LookupVariable returns null string
      buffer.push_back(this->getBinding(i->first));
    }
  }

  env_->AddBinding(key, std::accumulate(std::next(buffer.begin()), buffer.end(), buffer[0], [](const std::string& a, const std::string& b) { return a + " " + b; }));
  return true;
}

std::string Populator::getBinding(std::string key) const {
  return env_->LookupVariable(key);
}

bool Populator::addRule(Populator::EnvRule&& env_rule) {
  // TODO (aniket): Proper error handling. Rn doing crude error handling for demo
  if (env_rule.name.empty()) {
    throw std::runtime_error("expected rule name");
  }

  if (env_->LookupRuleCurrentScope(env_rule.name)) {
    throw std::runtime_error((std::stringstream() << "duplicate rule '" << env_rule.name << "'").str());
  }

  Rule* rule = new Rule(env_rule.name);

  rule->AddBinding("command", EvalString::createInstance(env_rule.command));

  if (env_rule.depfile.has_value())
    rule->AddBinding("depfile", EvalString::createInstance(env_rule.depfile.value()));

  if (env_rule.deps.has_value())
    rule->AddBinding("deps", EvalString::createInstance(env_rule.deps.value()));

  if (env_rule.msvc_deps_prefix.has_value())
    rule->AddBinding("msvc_deps_prefix", EvalString::createInstance(env_rule.msvc_deps_prefix.value()));

  if (env_rule.description.has_value())
    rule->AddBinding("description", EvalString::createInstance(env_rule.description.value()));

  if (env_rule.generator.has_value())
    rule->AddBinding("generator", EvalString::createInstance(env_rule.generator.value()));

  if (env_rule.restat.has_value())
    rule->AddBinding("restat", EvalString::createInstance(env_rule.restat.value()));

  if (env_rule.rspfile.has_value() && env_rule.rspfile_content.has_value()) {
    rule->AddBinding("rspfile", EvalString::createInstance(env_rule.rspfile.value()));
    rule->AddBinding("rspfile_content", EvalString::createInstance(env_rule.rspfile_content.value()));
  } else {
    throw std::runtime_error("rspfile and rspfile_content need to be both specified");
  }

  env_->AddRule(rule);
  return true;
}

//bool Populator::addBuildTarget(std::string rule_name, ExposedEvalStrings ins, ExposedEvalStrings outs) {}

int main(int argc, char** argv) {
  CppCmake::Make make;

  BuildConfig config;
  CppCmake::CppCmakeMain cppcmake_sidecheck("/Users/aniket/NYU/CppCmake/cmake-build-debug/cppcmake", config);

  Populator populator(&cppcmake_sidecheck.state_);
  populator.addBinding("cxx", {std::make_pair("g++", "RAW")});
  populator.addBinding("cx", {{"cxx", "SPECIAL"}, {"random", "RAW"}, {"random2", "RAW"}});
  populator.addBinding("cx1", {std::make_pair("cx", "SPECIAL")});
  populator.addBinding("cflags", {std::make_pair("-I./include -O2", "RAW")});

  populator.addRule({
      .name = "compile",
      .command = ExposedEvalStrings{{"g++", "RAW"}, {"-Itest/include", "RAW"}, {"-c", "RAW"}, {"in", "SPECIAL"}, {"-o", "RAW"}, {"out", "SPECIAL"}},
      .depfile = ExposedEvalStrings{{"out", "SPECIAL"}, {".d", "RAW"}},
      .deps = ExposedEvalStrings{{"gcc", "RAW"}},
      .msvc_deps_prefix = ExposedEvalStrings{{"Note:", "RAW"}},
      .description = ExposedEvalStrings{{"Compiling", "RAW"}, {"in", "SPECIAL"}, {"to", "RAW"}, {"out", "SPECIAL"}},
      .generator = ExposedEvalStrings{{"true", "RAW"}},
      .restat = ExposedEvalStrings{{"true", "RAW"}},
      .rspfile = ExposedEvalStrings{{"out", "SPECIAL"}, {".rsp", "RAW"}},
      .rspfile_content = ExposedEvalStrings{{"--input", "RAW"}, {"in", "SPECIAL"}, {"--output", "RAW"}, {"out", "RAW"}},
  });

//  populator.addBuildTarget("compile", {{{"src/hello.cpp", "RAW"}}, {{"include/add.h", "RAW"}}}, {{{"obj/hello.o", "RAW"}}});

  /*
     * build_location = src
     *
     * build hello.o : compile $build_location/hello.cpp
    */
  make.setVar("cxx", "g++");
  make.setVar("cx", "$cxx random random2");
  make.setVar("cx1", "$cx");
  make.setVar("cflags", "-I./include -O2");
  make.setVar("sample_file", "src/hello.cpp");
  make.setVar("src_path", "src");

  make.addRule({
      .name = "compile",
      .command = "g++ -Itest/include -c $in -o $out",
      //                .depfile = "$out.d",
      //                .deps = "gcc",
      //                .msvc_deps_prefix = "Note:",
      .description = "Compiling $in to $out",
      //                .generator = "true",
      //                .restat = "true",
      //                .rspfile = "$out.rsp",
      //                .rspfile_content = "--input $in --output $out"
  });

  make.addRule({.name = "link", .command = "$cxx $in -o $out", .description = "Linking $out"});
  make.addBuildTarget({.src = "obj/hello.o", .target = "compile $sample_file | include/add.h\n  cxx = clang++"});
  make.addBuildTarget({.src = "obj/add.o", .target = "compile  src/add.cpp | include/add.h"});
  make.addBuildTarget({.src = "obj/main.o", .target = "compile src/main.cpp | include/add.h"});
  make.addBuildTarget({.src = "hello", .target = "link obj/hello.o obj/main.o obj/add.o"});
  make.setDefault("hello");

  make.build(argc, argv);
}
