#include <numeric>
#include "../CppCmake/CppCmake/backend.hpp"

#include <format>

struct Populator {
  struct EnvRule {
    std::string name;
    std::vector<std::pair<std::string, std::string>> command;
    std::optional<std::vector<std::pair<std::string, std::string>>> depfile;
    std::optional<std::vector<std::pair<std::string, std::string>>> deps;
    std::optional<std::vector<std::pair<std::string, std::string>>> msvc_deps_prefix;
    std::optional<std::vector<std::pair<std::string, std::string>>> description;
    std::optional<std::vector<std::pair<std::string, std::string>>> generator;
    std::optional<std::vector<std::pair<std::string, std::string>>> restat;
    std::optional<std::vector<std::pair<std::string, std::string>>> rspfile;
    std::optional<std::vector<std::pair<std::string, std::string>>> rspfile_content;
  };

  BindingEnv* env_;
  explicit Populator(State* state);
  bool addBinding(std::string key, std::vector<std::pair<std::string, std::string>> value);
  [[nodiscard]] std::string getBinding(std::string key) const;

  bool addRule(Populator::EnvRule&& env_rule);
};

Populator::Populator(State* state) : env_{&state->bindings_} {}

bool Populator::addBinding(std::string key, std::vector<std::pair<std::string, std::string>> value) {
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
    throw std::runtime_error(std::format("duplicate rule '{}'", env_rule.name));
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
      .command = std::vector<std::pair<std::string, std::string>>{{"g++", "RAW"}, {"-Itest/include", "RAW"}, {"-c", "RAW"}, {"in", "SPECIAL"}, {"-o", "RAW"}, {"out", "SPECIAL"}},
      .depfile = std::vector<std::pair<std::string, std::string>>{{"out", "SPECIAL"}, {".d", "RAW"}},
      .deps = std::vector<std::pair<std::string, std::string>>{{"gcc", "RAW"}},
      .msvc_deps_prefix = std::vector<std::pair<std::string, std::string>>{{"Note:", "RAW"}},
      .description = std::vector<std::pair<std::string, std::string>>{{"Compiling", "RAW"}, {"in", "SPECIAL"}, {"to", "RAW"}, {"out", "SPECIAL"}},
      .generator = std::vector<std::pair<std::string, std::string>>{{"true", "RAW"}},
      .restat = std::vector<std::pair<std::string, std::string>>{{"true", "RAW"}},
      .rspfile = std::vector<std::pair<std::string, std::string>>{{"out", "SPECIAL"}, {".rsp", "RAW"}},
      .rspfile_content = std::vector<std::pair<std::string, std::string>>{{"--input", "RAW"}, {"in", "SPECIAL"}, {"--output", "RAW"}, {"out", "RAW"}},
  });


  make.setVar("cxx", "g++");
  make.setVar("cx", "$cxx random random2");
  make.setVar("cx1", "$cx");
  make.setVar("cflags", "-I./include -O2");
  make.addRule({.name = "compile",
                .command = "g++ -Itest/include -c $in -o $out",
                .depfile = "$out.d",
                .deps = "gcc",
                .msvc_deps_prefix = "Note:",
                .description = "Compiling $in to $out",
                .generator = "true",
                .restat = "true",
                .rspfile = "$out.rsp",
                .rspfile_content = "--input $in --output $out"});

  make.addRule({.name = "link", .command = "$cxx $in -o $out", .description = "Linking $out"});
  make.addBuildTarget({.src = "obj/hello.o", .target = "compile src/hello.cpp\n    cflags = -O3"});
  make.addBuildTarget({.src = "obj/main.o", .target = "compile src/main.cpp"});
  make.addBuildTarget({.src = "hello", .target = "link obj/hello.o obj/main.o"});
  make.setDefault("hello");

  make.build(argc, argv);
}
