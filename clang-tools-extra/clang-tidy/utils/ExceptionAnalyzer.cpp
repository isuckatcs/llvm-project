//===--- ExceptionAnalyzer.cpp - clang-tidy -------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ExceptionAnalyzer.h"

namespace clang {
namespace tidy {
namespace utils {

void ExceptionAnalyzer::ExceptionInfo::registerException(
    const Type *ExceptionType) {
  assert(ExceptionType != nullptr && "Only valid types are accepted");
  Behaviour = State::Throwing;
  ThrownExceptions.insert(ExceptionType);
}

void ExceptionAnalyzer::ExceptionInfo::registerExceptions(
    const Throwables &Exceptions) {
  if (Exceptions.size() == 0)
    return;
  Behaviour = State::Throwing;
  ThrownExceptions.insert(Exceptions.begin(), Exceptions.end());
}

ExceptionAnalyzer::ExceptionInfo &ExceptionAnalyzer::ExceptionInfo::merge(
    const ExceptionAnalyzer::ExceptionInfo &Other) {
  // Only the following two cases require an update to the local
  // 'Behaviour'. If the local entity is already throwing there will be no
  // change and if the other entity is throwing the merged entity will throw
  // as well.
  // If one of both entities is 'Unknown' and the other one does not throw
  // the merged entity is 'Unknown' as well.
  if (Other.Behaviour == State::Throwing)
    Behaviour = State::Throwing;
  else if (Other.Behaviour == State::Unknown && Behaviour == State::NotThrowing)
    Behaviour = State::Unknown;

  ContainsUnknown = ContainsUnknown || Other.ContainsUnknown;
  ThrownExceptions.insert(Other.ThrownExceptions.begin(),
                          Other.ThrownExceptions.end());
  return *this;
}

static bool isBaseOf(const Type *DerivedType, const Type *BaseType) {
  const auto *DerivedClass = DerivedType->getAsCXXRecordDecl();
  const auto *BaseClass = BaseType->getAsCXXRecordDecl();
  if (!DerivedClass || !BaseClass)
    return false;

  return !DerivedClass->forallBases(
      [BaseClass](const CXXRecordDecl *Cur) { return Cur != BaseClass; });
}

// Checks if T1 is convertible to T2.
static bool isMultiLevelConvertiblePointer(QualType P1, QualType P2,
                                           unsigned CurrentLevel = 0,
                                           bool IsP2ConstSoFar = false) {
  if (CurrentLevel == 0) {
    assert(P1->isPointerType() && "P1 is not a pointer type");
    assert(P2->isPointerType() && "P2 is not a pointer type");

    QualType P1PointeeTy = P1->getAs<PointerType>()->getPointeeType();
    QualType P2PointeeTy = P2->getAs<PointerType>()->getPointeeType();

    if (P1PointeeTy->isArrayType() && P2PointeeTy->isArrayType()) {
      P1PointeeTy = P1PointeeTy->getAsArrayTypeUnsafe()->getElementType();
      P2PointeeTy = P2PointeeTy->getAsArrayTypeUnsafe()->getElementType();
    }

    // If the pointer is not a multi-level pointer, perform
    // conversion checks.
    if (!P1PointeeTy->isPointerType() || !P2PointeeTy->isPointerType()) {
      const Type *P1PointeeUQTy = P1PointeeTy->getUnqualifiedDesugaredType();
      const Type *P2PointeeUQTy = P2PointeeTy->getUnqualifiedDesugaredType();

      Qualifiers P1Quals = P1PointeeTy.getQualifiers();
      Qualifiers P2Quals = P2PointeeTy.getQualifiers();

      return (P1PointeeUQTy == P2PointeeUQTy ||
              isBaseOf(P1PointeeUQTy, P2PointeeUQTy)) &&
             P2Quals.isStrictSupersetOf(P1Quals);
    }

    return isMultiLevelConvertiblePointer(P1PointeeTy, P2PointeeTy,
                                          CurrentLevel + 1, true);
  }

  bool convertible = true;

  if (P1->isArrayType() && P2->isArrayType()) {
    // At every level that array type is involved in, at least
    // one array type has unknown bound, or both array types
    // have same size.
    if (P1->isConstantArrayType() && P2->isConstantArrayType())
      convertible &=
          cast<ConstantArrayType>(P1->getAsArrayTypeUnsafe())->getSize() ==
          cast<ConstantArrayType>(P2->getAsArrayTypeUnsafe())->getSize();

    // If there is an array type of unknown bound at some level
    // (other than level zero) of P1, there is an array type of
    // unknown bound in the same level of P2;
    if (P1->isIncompleteArrayType())
      convertible &= P2->isIncompleteArrayType();

    // ... [or there is an array type of known bound in P1 and
    // an array type of unknown bound in P2 (since C++20)] then
    // there must be a 'const' at every single level (other than
    // level zero) of P2 up until the current level.
    if (P1->isConstantArrayType() && P2->isIncompleteArrayType())
      convertible &= IsP2ConstSoFar;

    P1 = P1->getAsArrayTypeUnsafe()->getElementType();
    P2 = P2->getAsArrayTypeUnsafe()->getElementType();
  }

  // If at the current level P2 is more cv-qualified than P1 [...],
  // then there must be a 'const' at every single level (other than level zero)
  // of P2 up until the current level
  if (P2.getQualifiers().isStrictSupersetOf(P1.getQualifiers()))
    convertible &= IsP2ConstSoFar;

  // If the pointers don't have the same amount of levels, they are not
  // convertible to each other.
  if (!P1->isPointerType() || !P2->isPointerType())
    return convertible && P1->getUnqualifiedDesugaredType() ==
                              P2->getUnqualifiedDesugaredType();

  // If the current level (other than level zero) in P1 is 'const' qualified,
  // the same level in P2 must also be 'const' qualified.
  if (P1.isConstQualified())
    convertible &= P2.isConstQualified();

  // If the current level (other than level zero) in P1 is 'volatile' qualified,
  // the same level in P2 must also be 'volatile' qualified.
  if (P1.isVolatileQualified())
    convertible &= P2.isVolatileQualified();

  IsP2ConstSoFar &= P2.isConstQualified();
  return convertible && isMultiLevelConvertiblePointer(
                            P1->getAs<PointerType>()->getPointeeType(),
                            P2->getAs<PointerType>()->getPointeeType(),
                            CurrentLevel + 1, IsP2ConstSoFar);
}

bool ExceptionAnalyzer::ExceptionInfo::filterByCatch(const Type *BaseClass) {
  llvm::SmallVector<const Type *, 8> TypesToDelete;
  for (const Type *T : ThrownExceptions) {
    if (T == BaseClass || isBaseOf(T, BaseClass))
      TypesToDelete.push_back(T);
    else if (T->isPointerType() && BaseClass->isPointerType()) {
      if (isMultiLevelConvertiblePointer(QualType(T, 0),
                                         QualType(BaseClass, 0))) {
        TypesToDelete.push_back(T);
      }
    }
  }

  for (const Type *T : TypesToDelete)
    ThrownExceptions.erase(T);

  reevaluateBehaviour();
  return TypesToDelete.size() > 0;
}

ExceptionAnalyzer::ExceptionInfo &
ExceptionAnalyzer::ExceptionInfo::filterIgnoredExceptions(
    const llvm::StringSet<> &IgnoredTypes, bool IgnoreBadAlloc) {
  llvm::SmallVector<const Type *, 8> TypesToDelete;
  // Note: Using a 'SmallSet' with 'llvm::remove_if()' is not possible.
  // Therefore this slightly hacky implementation is required.
  for (const Type *T : ThrownExceptions) {
    if (const auto *TD = T->getAsTagDecl()) {
      if (TD->getDeclName().isIdentifier()) {
        if ((IgnoreBadAlloc &&
             (TD->getName() == "bad_alloc" && TD->isInStdNamespace())) ||
            (IgnoredTypes.count(TD->getName()) > 0))
          TypesToDelete.push_back(T);
      }
    }
  }
  for (const Type *T : TypesToDelete)
    ThrownExceptions.erase(T);

  reevaluateBehaviour();
  return *this;
}

void ExceptionAnalyzer::ExceptionInfo::clear() {
  Behaviour = State::NotThrowing;
  ContainsUnknown = false;
  ThrownExceptions.clear();
}

void ExceptionAnalyzer::ExceptionInfo::reevaluateBehaviour() {
  if (ThrownExceptions.size() == 0)
    if (ContainsUnknown)
      Behaviour = State::Unknown;
    else
      Behaviour = State::NotThrowing;
  else
    Behaviour = State::Throwing;
}

ExceptionAnalyzer::ExceptionInfo ExceptionAnalyzer::throwsException(
    const FunctionDecl *Func,
    llvm::SmallSet<const FunctionDecl *, 32> &CallStack) {
  if (CallStack.count(Func))
    return ExceptionInfo::createNonThrowing();

  if (const Stmt *Body = Func->getBody()) {
    CallStack.insert(Func);
    ExceptionInfo Result =
        throwsException(Body, ExceptionInfo::Throwables(), CallStack);

    // For a constructor, we also have to check the initializers.
    if (const auto *Ctor = dyn_cast<CXXConstructorDecl>(Func)) {
      for (const CXXCtorInitializer *Init : Ctor->inits()) {
        ExceptionInfo Excs = throwsException(
            Init->getInit(), ExceptionInfo::Throwables(), CallStack);
        Result.merge(Excs);
      }
    }

    CallStack.erase(Func);
    return Result;
  }

  auto Result = ExceptionInfo::createUnknown();
  if (const auto *FPT = Func->getType()->getAs<FunctionProtoType>()) {
    for (const QualType &Ex : FPT->exceptions())
      Result.registerException(Ex.getTypePtr());
  }
  return Result;
}

/// Analyzes a single statement on it's throwing behaviour. This is in principle
/// possible except some 'Unknown' functions are called.
ExceptionAnalyzer::ExceptionInfo ExceptionAnalyzer::throwsException(
    const Stmt *St, const ExceptionInfo::Throwables &Caught,
    llvm::SmallSet<const FunctionDecl *, 32> &CallStack) {
  auto Results = ExceptionInfo::createNonThrowing();
  if (!St)
    return Results;

  if (const auto *Throw = dyn_cast<CXXThrowExpr>(St)) {
    if (const auto *ThrownExpr = Throw->getSubExpr()) {
      const auto *ThrownType =
          ThrownExpr->getType()->getUnqualifiedDesugaredType();
      if (ThrownType->isReferenceType())
        ThrownType = ThrownType->castAs<ReferenceType>()
                         ->getPointeeType()
                         ->getUnqualifiedDesugaredType();
      Results.registerException(
          ThrownExpr->getType()->getUnqualifiedDesugaredType());
    } else
      // A rethrow of a caught exception happens which makes it possible
      // to throw all exception that are caught in the 'catch' clause of
      // the parent try-catch block.
      Results.registerExceptions(Caught);
  } else if (const auto *Try = dyn_cast<CXXTryStmt>(St)) {
    ExceptionInfo Uncaught =
        throwsException(Try->getTryBlock(), Caught, CallStack);
    for (unsigned I = 0; I < Try->getNumHandlers(); ++I) {
      const CXXCatchStmt *Catch = Try->getHandler(I);

      // Everything is catched through 'catch(...)'.
      if (!Catch->getExceptionDecl()) {
        ExceptionInfo Rethrown = throwsException(
            Catch->getHandlerBlock(), Uncaught.getExceptionTypes(), CallStack);
        Results.merge(Rethrown);
        Uncaught.clear();
      } else {
        const auto *CaughtType =
            Catch->getCaughtType()->getUnqualifiedDesugaredType();
        if (CaughtType->isReferenceType()) {
          CaughtType = CaughtType->castAs<ReferenceType>()
                           ->getPointeeType()
                           ->getUnqualifiedDesugaredType();
        }

        // If the caught exception will catch multiple previously potential
        // thrown types (because it's sensitive to inheritance) the throwing
        // situation changes. First of all filter the exception types and
        // analyze if the baseclass-exception is rethrown.
        if (Uncaught.filterByCatch(CaughtType)) {
          ExceptionInfo::Throwables CaughtExceptions;
          CaughtExceptions.insert(CaughtType);
          ExceptionInfo Rethrown = throwsException(Catch->getHandlerBlock(),
                                                   CaughtExceptions, CallStack);
          Results.merge(Rethrown);
        }
      }
    }
    Results.merge(Uncaught);
  } else if (const auto *Call = dyn_cast<CallExpr>(St)) {
    if (const FunctionDecl *Func = Call->getDirectCallee()) {
      ExceptionInfo Excs = throwsException(Func, CallStack);
      Results.merge(Excs);
    }
  } else if (const auto *Construct = dyn_cast<CXXConstructExpr>(St)) {
    ExceptionInfo Excs =
        throwsException(Construct->getConstructor(), CallStack);
    Results.merge(Excs);
  } else if (const auto *DefaultInit = dyn_cast<CXXDefaultInitExpr>(St)) {
    ExceptionInfo Excs =
        throwsException(DefaultInit->getExpr(), Caught, CallStack);
    Results.merge(Excs);
  } else {
    for (const Stmt *Child : St->children()) {
      ExceptionInfo Excs = throwsException(Child, Caught, CallStack);
      Results.merge(Excs);
    }
  }
  return Results;
}

ExceptionAnalyzer::ExceptionInfo
ExceptionAnalyzer::analyzeImpl(const FunctionDecl *Func) {
  ExceptionInfo ExceptionList;

  // Check if the function has already been analyzed and reuse that result.
  if (FunctionCache.count(Func) == 0) {
    llvm::SmallSet<const FunctionDecl *, 32> CallStack;
    ExceptionList = throwsException(Func, CallStack);

    // Cache the result of the analysis. This is done prior to filtering
    // because it is best to keep as much information as possible.
    // The results here might be relevant to different analysis passes
    // with different needs as well.
    FunctionCache.insert(std::make_pair(Func, ExceptionList));
  } else
    ExceptionList = FunctionCache[Func];

  return ExceptionList;
}

ExceptionAnalyzer::ExceptionInfo
ExceptionAnalyzer::analyzeImpl(const Stmt *Stmt) {
  llvm::SmallSet<const FunctionDecl *, 32> CallStack;
  return throwsException(Stmt, ExceptionInfo::Throwables(), CallStack);
}

template <typename T>
ExceptionAnalyzer::ExceptionInfo
ExceptionAnalyzer::analyzeDispatch(const T *Node) {
  ExceptionInfo ExceptionList = analyzeImpl(Node);

  if (ExceptionList.getBehaviour() == State::NotThrowing ||
      ExceptionList.getBehaviour() == State::Unknown)
    return ExceptionList;

  // Remove all ignored exceptions from the list of exceptions that can be
  // thrown.
  ExceptionList.filterIgnoredExceptions(IgnoredExceptions, IgnoreBadAlloc);

  return ExceptionList;
}

ExceptionAnalyzer::ExceptionInfo
ExceptionAnalyzer::analyze(const FunctionDecl *Func) {
  return analyzeDispatch(Func);
}

ExceptionAnalyzer::ExceptionInfo
ExceptionAnalyzer::analyze(const Stmt *Stmt) {
  return analyzeDispatch(Stmt);
}

} // namespace utils
} // namespace tidy

} // namespace clang
