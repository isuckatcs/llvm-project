// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -analyzer-config c++-inlining=destructors -verify -std=c++11 %s
// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -analyzer-config c++-inlining=destructors -verify -std=c++17 %s

void clang_analyzer_eval(bool);
void clang_analyzer_checkInlined(bool);

int a, b, c, d;

struct InlineDtor {
  static int cnt;
  static int dtorCalled;
  ~InlineDtor() {
    switch (dtorCalled % 4) {
    case 0:
      a = cnt++;
      break;
    case 1:
      b = cnt++;
      break;
    case 2:
      c = cnt++;
      break;
    case 3:
      d = cnt++;
      break;
    }

    ++dtorCalled;
  }
};

int InlineDtor::cnt = 0;
int InlineDtor::dtorCalled = 0;

void foo() {
  InlineDtor::cnt = 0;
  InlineDtor::dtorCalled = 0;
  InlineDtor arr[4];
}

void testAutoDtor() {
  foo();

  clang_analyzer_eval(a == 0); // expected-warning {{TRUE}}
  clang_analyzer_eval(b == 1); // expected-warning {{TRUE}}
  clang_analyzer_eval(c == 2); // expected-warning {{TRUE}}
  clang_analyzer_eval(d == 3); // expected-warning {{TRUE}}
}

void testDeleteDtor() {
  InlineDtor::cnt = 10;
  InlineDtor::dtorCalled = 0;

  InlineDtor *arr = new InlineDtor[4];
  delete[] arr;

  clang_analyzer_eval(a == 10); // expected-warning {{TRUE}}
  clang_analyzer_eval(b == 11); // expected-warning {{TRUE}}
  clang_analyzer_eval(c == 12); // expected-warning {{TRUE}}
  clang_analyzer_eval(d == 13); // expected-warning {{TRUE}}
}

struct MemberDtor {
  InlineDtor arr[4];
};

void testMemberDtor() {
  InlineDtor::cnt = 5;
  InlineDtor::dtorCalled = 0;

  MemberDtor *MD = new MemberDtor{};
  delete MD;

  clang_analyzer_eval(a == 5); // expected-warning {{TRUE}}
  clang_analyzer_eval(b == 6); // expected-warning {{TRUE}}
  clang_analyzer_eval(c == 7); // expected-warning {{TRUE}}
  clang_analyzer_eval(d == 8); // expected-warning {{TRUE}}
}

struct MultipleMemberDtor
{
  InlineDtor arr[4];
  InlineDtor arr2[4];
};

void testMultipleMemberDtor() {
  InlineDtor::cnt = 30;
  InlineDtor::dtorCalled = 0;

  MultipleMemberDtor *MD = new MultipleMemberDtor{};
  delete MD;

  clang_analyzer_eval(a == 34); // expected-warning {{TRUE}}
  clang_analyzer_eval(b == 35); // expected-warning {{TRUE}}
  clang_analyzer_eval(c == 36); // expected-warning {{TRUE}}
  clang_analyzer_eval(d == 37); // expected-warning {{TRUE}}
}

int EvalOrderArr[4];

struct EvalOrder
{
  int ctor = 0;
  static int dtorCalled;
  static int ctorCalled;

  EvalOrder() { ctor = ctorCalled++; };

  ~EvalOrder() { EvalOrderArr[ctor] = dtorCalled++; }
};

int EvalOrder::ctorCalled = 0;
int EvalOrder::dtorCalled = 0;

void dtorEvaluationOrder(){
  EvalOrder::ctorCalled = 0;
  EvalOrder::dtorCalled = 0;
  
  EvalOrder* eptr = new EvalOrder[4];
  delete[] eptr;

  clang_analyzer_eval(EvalOrder::dtorCalled == 4); // expected-warning {{TRUE}}
  clang_analyzer_eval(EvalOrder::dtorCalled == EvalOrder::ctorCalled); // expected-warning {{TRUE}}

  clang_analyzer_eval(EvalOrderArr[0] == 3); // expected-warning {{TRUE}}
  clang_analyzer_eval(EvalOrderArr[1] == 2); // expected-warning {{TRUE}}
  clang_analyzer_eval(EvalOrderArr[2] == 1); // expected-warning {{TRUE}}
  clang_analyzer_eval(EvalOrderArr[3] == 0); // expected-warning {{TRUE}}
}
