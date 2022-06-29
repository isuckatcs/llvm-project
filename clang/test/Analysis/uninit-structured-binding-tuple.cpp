// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection -std=c++17 -verify %s

#include "Inputs/system-header-simulator-cxx.h"

void clang_analyzer_eval(bool);

void a(void) {
  std::pair<int, int> p = {1, 2};

  auto [u, v] = p;

  int x = u;
  int y = v;

  clang_analyzer_eval(x == 1); // expected-warning{{TRUE}}
  clang_analyzer_eval(y == 2); // expected-warning{{TRUE}}
}
