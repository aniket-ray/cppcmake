#include "graph_generator.hpp"
#include <sstream>
#include "util_funcs.hpp"

GraphGenerator::GraphGenerator(State* state, std::string err) : env_{&state->bindings_}, err_(&err) {}

void GraphGenerator::addBindings(std::string key, std::string value) {
  if (key.empty()) {
    *err_ += "Key cannot be empty";
    return;
  }

  if (value.empty()) {
    *err_ += "Value cannot be empty";
    return;
  }

  EvalString let_value;
  CppCmake::tokenizer(value, &let_value, err_);
  std::string evaluted_string = let_value.Evaluate(env_);
  env_->AddBinding(key, evaluted_string);
}

/*
$$ - valid
$ - invalid
${asd - invalid
${foo} - valid
foo - valid
*/