// RUN: %clang_analyze_cc1 %s \
// RUN:   -analyzer-checker=cplusplus \
// RUN:   -verify

void checkUndefinedElmenetCountValue()
{
    int n;

    int *arr = new int[n]; // expected-warning{{Undefined element count!}}
}

void checkUndefinedElmenetCountMultiDimensionalValue()
{
    int n;

    auto *arr = new int[n][5]; // expected-warning{{Undefined element count!}}
}

void checkUndefinedElmenetCountReference()
{
    int n;

    int& ref = n;

    int *arr = new int[ref]; // expected-warning{{Undefined element count!}}
}

void checkUndefinedElmenetCountMultiDimensionalReference()
{
    int n;

    int& ref = n;

    auto *arr = new int[ref][5]; // expected-warning{{Undefined element count!}}
}

int foo()
{
    int n;

    return n;
}

void checkUndefinedElmenetCountFunction()
{
    int *arr = new int[foo()]; // expected-warning{{Undefined element count!}}
}

void checkUndefinedElmenetCountMultiDimensionalFunction()
{
    auto *arr = new int[foo()][5]; // expected-warning{{Undefined element count!}}
}
