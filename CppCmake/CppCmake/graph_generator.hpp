#ifndef CPPCMAKE_GRAPH_GENERATOR_HPP
#define CPPCMAKE_GRAPH_GENERATOR_HPP

#include "graph.h"
#include "state.h"
#include "util.h"
#include "version.h"

class GraphGenerator {
 public:
  GraphGenerator(State* state, std::string err);

  void addBindings(std::string key, std::string value);

  GraphGenerator() = delete;
  GraphGenerator(GraphGenerator&&) = delete;
  GraphGenerator(const GraphGenerator&) = delete;
  GraphGenerator operator=(GraphGenerator&&) = delete;
  GraphGenerator operator=(const GraphGenerator&) = delete;

 private:
  BindingEnv* env_;
  std::string* err_;
};

#endif
