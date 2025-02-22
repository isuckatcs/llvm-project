.. title:: clang-tidy - bugprone-true-macro

bugprone-true-macro
===================

In C++, ``true`` is considered a keyword by the preprocessor so an ``#if true`` enters the true branch,
while in C, ``true`` is not treated as a special keyword by the preprocessor, so the false branch is entered.

The following snippet returns ``1`` in C++, but ``0`` in C.

.. code-block:: c++
    int main() {
    #if true
      return 1;
    #else
      return 0;
    #endif
    }

The check also offers fix-it hints.
