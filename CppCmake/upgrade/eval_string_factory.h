#ifndef CPPCMAKE_EVAL_STRING_FACTORY_H
#define CPPCMAKE_EVAL_STRING_FACTORY_H

#include "../CppCmake/eval_env.h"

// Factory builder class
class EvalStringBuilder {
 public:
  EvalStringBuilder& AddText(StringPiece text) {
    eval_string_.AddText(text);
    return *this;
  }

  EvalStringBuilder& AddSpecial(StringPiece text) {
    eval_string_.AddSpecial(text);
    return *this;
  }

  [[nodiscard]] EvalString Build() const { return eval_string_; }

 private:
  EvalString eval_string_;
};

#endif  //CPPCMAKE_EVAL_STRING_FACTORY_H
