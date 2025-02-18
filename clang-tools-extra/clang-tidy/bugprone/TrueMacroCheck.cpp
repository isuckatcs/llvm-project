//===--- TrueMacroCheck.cpp - clang-tidy ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TrueMacroCheck.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

using namespace clang::ast_matchers;

namespace clang::tidy::bugprone {
namespace {

class MacroCallback : public PPCallbacks {
  static constexpr const char *TrueMacroSpelling = "true";

public:
  MacroCallback(TrueMacroCheck *Check, Preprocessor *PP)
      : Check(Check), PP(PP) {}
  void MacroDefined(const Token &MacroNameTok,
                    const MacroDirective *MD) override {
    if (TrueDefined)
      return;

    const MacroInfo *MI = MD->getMacroInfo();
    for (const Token &Tok : MI->tokens()) {
      if (PP->getSpelling(Tok) == TrueMacroSpelling)
        emitDiagnostics(Tok.getLocation(),
                        {Tok.getLocation(), Tok.getEndLoc()});
    }

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

    if (!TrueDefined && Condition == TrueMacroSpelling) {
      emitDiagnostics(ConditionRange.getBegin(), ConditionRange);
      return;
    }

    for (auto &&Identifier : identifiersInCondition(Condition)) {
      if (!TrueDefined && Identifier == TrueMacroSpelling) {
        emitDiagnostics(Loc, {}, true);
        break;
      }
    }
  }

private:
  void emitDiagnostics(SourceLocation Loc, SourceRange ReplaceRange,
                       bool InCondition = false) {
    DiagnosticBuilder Builder =
        Check->diag(Loc, "in C 'true'%select{| in the condition}0 is treated "
                         "as an undefined "
                         "macro and evaluates to a falsy value; "
                         "consider replacing it with '1'")
        << InCondition;

    if (!InCondition)
      Builder << FixItHint::CreateReplacement(ReplaceRange, "1");
  }

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
  Preprocessor *PP;
};
} // namespace

void TrueMacroCheck::registerPPCallbacks(const SourceManager &SM,
                                         Preprocessor *PP,
                                         Preprocessor *ModuleExpanderPP) {
  PP->addPPCallbacks(std::make_unique<MacroCallback>(this, PP));
}

} // namespace clang::tidy::bugprone
