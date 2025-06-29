#include "clang/Summary/SummaryConsumer.h"
#include "clang/Summary/SummaryContext.h"
#include "clang/Summary/SummaryYamlMappings.h"

namespace clang {
void JSONPrintingSummaryConsumer::ProcessFunctionSummary(
    const FunctionSummary &Summary) {
  JOS.object([&] {
    JOS.attribute("id", llvm::json::Value(Summary.getID()));
    JOS.attributeObject("attrs", [&] {
      JOS.attributeArray("function", [&] {
        for (auto &&Attr : Summary.getAttributes()) {
          JOS.value(llvm::json::Value(Attr->serialize()));
        }
      });
    });
    JOS.attributeObject("calls", [&] {
      JOS.attribute("opaque", llvm::json::Value(Summary.callsOpaqueObject()));
      JOS.attributeArray("functions", [&] {
        for (auto &&Call : Summary.getCalls()) {
          JOS.object([&] { JOS.attribute("id", llvm::json::Value(Call)); });
        }
      });
    });
  });
}

void YAMLPrintingSummaryConsumer::ProcessEndOfSourceFile() {
  YOS << ((SummaryContext *)SummaryCtx)->FunctionSummaries;
  OS.flush();
}
} // namespace clang