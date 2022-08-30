// RUN: %clang_analyze_cc1 -analyzer-checker=debug.ExprInspection -std=c++17 -verify %s

void clang_analyzer_eval(bool);

struct s {
  int a;
  int b;
};

void a(void) {
  s tst;

  auto [i, j] = tst;

  clang_analyzer_eval(i); // expected-warning{{UNDEFINED}}
  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

void b(void) {
  s tst;
  tst.a = 1;

  auto [i, j] = tst;

  clang_analyzer_eval(i == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

void c(void) {
  s tst;

  auto &[i, j] = tst;

  clang_analyzer_eval(i); // expected-warning{{UNDEFINED}}
  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

void d(void) {
  s tst;
  tst.a = 1;

  auto &[i, j] = tst;

  clang_analyzer_eval(i == 1); // expected-warning{{TRUE}}
  i = 2;
  clang_analyzer_eval(tst.a == 2); // expected-warning{{TRUE}}

  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

void e(void) {
  s tst;
  tst.a = 1;

  auto &[i, j] = tst;

  clang_analyzer_eval(i == 1); // expected-warning{{TRUE}}

  tst.b = 2;
  clang_analyzer_eval(j == 2); // expected-warning{{TRUE}}
}

void f(void) {
  s tst;

  auto &&[i, j] = tst;

  clang_analyzer_eval(i); // expected-warning{{UNDEFINED}}
  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

void g(void) {
  s tst;
  tst.a = 1;

  auto &&[i, j] = tst;

  clang_analyzer_eval(i == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(j); // expected-warning{{UNDEFINED}}
}

struct s2 {
  int a = 1;
  int b = 2;
};

struct s3 {
  s x;
  s2 y;
};

void h(void) {
  s3 tst;

  clang_analyzer_eval(tst.y.a == 1); // expected-warning{{TRUE}}

  auto [i, j] = tst;

  clang_analyzer_eval(i.a); // expected-warning{{UNDEFINED}}
  clang_analyzer_eval(i.b); // expected-warning{{UNDEFINED}}

  clang_analyzer_eval(j.a == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(j.b == 2); // expected-warning{{TRUE}}
}

void i(void) {
  s3 tst;

  clang_analyzer_eval(tst.y.a == 1); // expected-warning{{TRUE}}

  auto &[i, j] = tst;
  j.a = 3;

  clang_analyzer_eval(tst.y.a == 3); // expected-warning{{TRUE}}
  clang_analyzer_eval(tst.y.b == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(j.b == 2);     // expected-warning{{TRUE}}
}
