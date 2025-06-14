#include "clang/Sema/SummaryAttribute.h"
#include "clang/Sema/SummaryContext.h"

namespace clang {
void NoWriteGlobalAttr::Callback::run(
    const ast_matchers::MatchFinder::MatchResult &Result) {
  const auto *Assignment = Result.Nodes.getNodeAs<BinaryOperator>("assignment");
  if (!Assignment)
    return;

  WriteGlobal = true;
}

bool NoWriteGlobalAttr::infer(const FunctionDecl *FD) const {
  using namespace ast_matchers;
  MatchFinder Finder;
  Callback CB;

  Finder.addMatcher(
      functionDecl(forEachDescendant(
          binaryOperator(isAssignmentOperator(),
                         hasLHS(declRefExpr(to(varDecl(hasGlobalStorage())))))
              .bind("assignment"))),
      &CB);
  Finder.match(*FD, FD->getASTContext());
  return !CB.WriteGlobal;
}

bool NoWriteGlobalAttr::merge(const FunctionSummary &Caller,
                              const FunctionSummary &Callee) const {
  return Caller.getAttributes().count(this) &&
         Callee.getAttributes().count(this);
}
} // namespace clang