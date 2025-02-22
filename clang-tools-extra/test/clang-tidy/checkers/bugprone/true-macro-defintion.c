// RUN: %check_clang_tidy -std=c99-or-later %s bugprone-true-macro %t

#define FOO true
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1

#if true
// CHECK-MESSAGES: :[[@LINE-1]]:5: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1
#endif

#if false || true
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: in C 'true' in the condition is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
#endif

#define true 1

#define FOO true

#if true
#endif

#if false || true
#endif

#undef true

#define FOO true
// CHECK-MESSAGES: :[[@LINE-1]]:13: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1

#if true
// CHECK-MESSAGES: :[[@LINE-1]]:5: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1
#endif

#if false || true
// CHECK-MESSAGES: :[[@LINE-1]]:2: warning: in C 'true' in the condition is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
#endif

#define true true
// CHECK-MESSAGES: :[[@LINE-1]]:14: warning: in C 'true' is treated as an undefined macro and evaluates to a falsy value; consider replacing it with '1' [bugprone-true-macro]
// CHECK-FIXES: 1
