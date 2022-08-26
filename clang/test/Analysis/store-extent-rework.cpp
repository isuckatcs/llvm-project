// RUN: %clang_analyze_cc1 -analyzer-checker=core,debug.ExprInspection,alpha.unix.cstring -verify %s

void clang_analyzer_eval(bool);

typedef __typeof__(sizeof(int)) size_t;
void *memset(void *dest, int ch, size_t count);

struct Point
{
    int x;
    int y;
    int z;
};

struct Circle
{
    Point origin;
    int size;
};

Point makePoint(int x, int y)
{
    Point result;
    result.x = x;
    result.y = y;
    result.z = 3;
    return result;
}

void multipleDefaultOnStack() {
    Circle testObj;
    memset(&testObj, 0, sizeof(testObj));

    clang_analyzer_eval(testObj.size == 0); //expected-warning{{TRUE}}
    testObj.origin = makePoint(1, 2);
    clang_analyzer_eval(testObj.size == 0); //expected-warning{{TRUE}}
    
    clang_analyzer_eval(testObj.origin.x == 1); //expected-warning{{TRUE}}
    clang_analyzer_eval(testObj.origin.y == 2); //expected-warning{{TRUE}}
    clang_analyzer_eval(testObj.origin.z == 3); //expected-warning{{TRUE}}
}
