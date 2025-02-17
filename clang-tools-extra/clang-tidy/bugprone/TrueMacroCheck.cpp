//===--- TrueMacroCheck.cpp - clang-tidy ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TrueMacroCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include <iostream>

using namespace clang::ast_matchers;

namespace clang::tidy::bugprone {
namespace {

class MacroCallback : public PPCallbacks {
  static constexpr const char *TrueMacroSpelling = "true";

public:
  MacroCallback(TrueMacroCheck *Check, const SourceManager &SM,
                Preprocessor *PP)
      : Check(Check), SM(&SM), PP(PP) {}
  void MacroDefined(const Token &MacroNameTok,
                    const MacroDirective *MD) override {
    if (PP->getSpelling(MacroNameTok) == TrueMacroSpelling)
      TrueDefined = true;
  }

  virtual void MacroUndefined(const Token &MacroNameTok,
                              const MacroDefinition &MD,
                              const MacroDirective *Undef) override {
    if (PP->getSpelling(MacroNameTok) == TrueMacroSpelling)
      TrueDefined = false;
  }

  virtual void If(SourceLocation Loc, SourceRange ConditionRange,
                  ConditionValueKind ConditionValue) override {
    StringRef Condition =
        Lexer::getSourceText(CharSourceRange::getTokenRange(ConditionRange),
                             PP->getSourceManager(), PP->getLangOpts());

    for (auto &&Identifier : identifiersInCondition(Condition))
      std::cout << Identifier.str() << ' ' << Identifier.size() << '\n';
  }

private:
  void emitDiagnostic() {}

  std::vector<StringRef> identifiersInCondition(StringRef Condition) {
    const static auto Start = [](char C) {
      return C == '_' || ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z');
    };

    const static auto Continue = [](char C) {
      return ('0' <= C && C <= '9') || Start(C);
    };

    std::vector<StringRef> Identifiers;
    const char *Str = nullptr;

    for (size_t I = 0; I < Condition.size(); ++I) {
      char C = Condition[I];

      if (!Str && Start(C)) {
        Str = Condition.begin() + I;
      } else if (Str && !Continue(C)) {
        Identifiers.emplace_back(Str, Condition.begin() + I - Str);
        Str = nullptr;
      }
    }

    if (Str)
      Identifiers.emplace_back(Str, Condition.end() - Str);

    return Identifiers;
  }

  bool TrueDefined = false;

  TrueMacroCheck *Check;
  const SourceManager *SM;
  Preprocessor *PP;
};
} // namespace

void TrueMacroCheck::registerPPCallbacks(const SourceManager &SM,
                                         Preprocessor *PP,
                                         Preprocessor *ModuleExpanderPP) {
  PP->addPPCallbacks(std::make_unique<MacroCallback>(this, SM, PP));
}

} // namespace clang::tidy::bugprone
