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

struct Test {
  int v = 1;
  int u = 2;
};

namespace std {
template <typename T> struct tuple_size {};
template <>
struct tuple_size<Test> {
  static const std::size_t value = 2;
};

template <std::size_t I, typename T> struct tuple_element {};
template <std::size_t I>
struct tuple_element<I, Test> {
  using type = int;
};

} // namespace std

template <std::size_t I>
int get(Test t) {
  if (I == 0) {
    t.u = 10;
    return t.v;
  } else {
    t.v = 20;
    return t.u;
  }
}

void b(void) {
  Test p;
  auto [u, v] = p;

  u = 8;

  int x = u;

  clang_analyzer_eval(x == 8); // expected-warning{{TRUE}}

  clang_analyzer_eval(u == 8); // expected-warning{{TRUE}}
  clang_analyzer_eval(v == 2); // expected-warning{{TRUE}}

  clang_analyzer_eval(p.u == 2); // expected-warning{{TRUE}}
  clang_analyzer_eval(p.v == 1); // expected-warning{{TRUE}}
}
