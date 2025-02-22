// RUN: %check_clang_tidy -std=c99-or-later %s bugprone-true-macro %t

#define FOO true
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1
