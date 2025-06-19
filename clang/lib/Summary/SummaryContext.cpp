#include "clang/Summary/SummaryContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Summary/SummaryAttribute.h"
#include "clang/Summary/SummaryConsumer.h"
#include <set>

namespace clang {
namespace {
SmallVector<char> GetUSR(const FunctionDecl *FD) {
  SmallVector<char> USR;
  index::generateUSRForDecl(FD, USR);
  return USR;
}

class CallCollector : public ast_matchers::MatchFinder::MatchCallback {
  std::set<SmallVector<char>> Calls;
  bool callsOpaqueSymbol = false;

  virtual void
  run(const ast_matchers::MatchFinder::MatchResult &Result) override {
    const auto *Call = Result.Nodes.getNodeAs<CallExpr>("call");
    if (!Call)
      return;

    const auto *Callee =
        llvm::dyn_cast_or_null<FunctionDecl>(Call->getCalleeDecl());
    if (!Callee) {
      callsOpaqueSymbol = true;
      return;
    }

    if (const auto *MD = llvm::dyn_cast<CXXMethodDecl>(Callee);
        MD && MD->isVirtual()) {
      callsOpaqueSymbol = true;
      return;
    }

    Calls.emplace(GetUSR(Callee));
  }

public:
  std::pair<std::set<SmallVector<char>>, bool> collect(const FunctionDecl *FD) {
    using namespace ast_matchers;
    MatchFinder Finder;

    Finder.addMatcher(functionDecl(forEachDescendant(callExpr().bind("call"))),
                      this);
    Finder.match(*FD, FD->getASTContext());

    return {Calls, callsOpaqueSymbol};
  }
};
} // namespace

FunctionSummary::FunctionSummary(SmallVector<char> ID,
                                 std::set<const SummaryAttr *> FunctionAttrs,
                                 std::set<SmallVector<char>> Calls,
                                 bool CallsOpaque)
    : ID(std::move(ID)), Attrs(std::move(FunctionAttrs)),
      Calls(std::move(Calls)), CallsOpaque(CallsOpaque) {}

template <typename T> void SummaryContext::registerAttr() {
  std::unique_ptr<T> attr(new T());
  SummaryAttrKind Kind = attr->getKind();

  if (KindToAttribute.count(Kind))
    return;

  KindToAttribute[Kind] = Attributes.emplace_back(std::move(attr)).get();
}

SummaryContext::SummaryContext() { registerAttr<NoWriteGlobalAttr>(); }

void SummaryContext::CreateSummary(SmallVector<char> ID,
                                   std::set<const SummaryAttr *> Attrs,
                                   std::set<SmallVector<char>> Calls,
                                   bool CallsOpaque) {
  auto Summary = std::make_unique<FunctionSummary>(
      std::move(ID), std::move(Attrs), std::move(Calls), CallsOpaque);
  auto *SummaryPtr = FunctionSummaries.emplace_back(std::move(Summary)).get();
  IDToSummary[SummaryPtr->getID()] = SummaryPtr;
}

const FunctionSummary *
SummaryContext::GetSummary(const FunctionDecl *FD) const {
  auto USR = GetUSR(FD);
  return IDToSummary.count(USR) ? IDToSummary.at(USR) : nullptr;
}

void SummaryContext::SummarizeFunctionBody(const FunctionDecl *FD) {
  std::set<const SummaryAttr *> Attrs;

  for (auto &&Attr : Attributes) {
    if (Attr->infer(FD))
      Attrs.emplace(Attr.get());
  }

  auto [calls, opaque] = CallCollector().collect(FD);

  CreateSummary(GetUSR(FD), std::move(Attrs), std::move(calls), opaque);
}

void SummaryContext::ParseSummaryFromJSON(const llvm::json::Array &Summary) {
  for (auto it = Summary.begin(); it != Summary.end(); ++it) {
    const llvm::json::Object *FunctionSummary = it->getAsObject();

    SmallString<128> ID(*FunctionSummary->getString("id"));
    std::set<const SummaryAttr *> FunctionAttrs;
    const llvm::json::Array *FunctionAttributes =
        FunctionSummary->getObject("attrs")->getArray("function");
    for (auto attrIt = FunctionAttributes->begin();
         attrIt != FunctionAttributes->end(); ++attrIt) {
      for (auto &&Attr : Attributes) {
        if (Attr->parse(*attrIt->getAsString()))
          FunctionAttrs.emplace(Attr.get());
      }
    }

    std::set<SmallVector<char>> Calls;
    const llvm::json::Object *CallsObject = FunctionSummary->getObject("calls");
    bool callsOpaue = *CallsObject->getBoolean("opaque");

    const llvm::json::Array *CallEntries = CallsObject->getArray("functions");
    for (auto callIt = CallEntries->begin(); callIt != CallEntries->end();
         ++callIt) {
      auto *Obj = callIt->getAsObject();
      Calls.emplace(SmallString<128>(*Obj->getString("id")));
    }

    CreateSummary(std::move(ID), std::move(FunctionAttrs), std::move(Calls),
                  callsOpaue);
  }
}

bool SummaryContext::ReduceFunctionSummary(FunctionSummary &Function) {
  bool changed = false;

  for (auto &&call : Function.getCalls()) {
    std::set<const SummaryAttr *> reducedAttrs;

    const FunctionSummary *callSummary =
        IDToSummary.count(call) ? IDToSummary[call] : nullptr;
    for (auto &&Attr : Attributes) {
      if (Attr->merge(Function, callSummary))
        reducedAttrs.emplace(Attr.get());
    }

    if (reducedAttrs != Function.getAttributes()) {
      Function.replaceAttributes(std::move(reducedAttrs));
      changed = true;
    }
  }

  return changed;
}

void SummaryContext::ReduceSummaries() {
  bool changed = true;
  while (changed) {
    changed = false;

    for (auto &&Function : FunctionSummaries)
      changed |= ReduceFunctionSummary(*Function);
  }
}
} // namespace clang
