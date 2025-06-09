# CppUnitTestFrameworkWrapper

A simple wrapper that allows tests written with the CppUnitTestFramework
from Microsoft to compile and run under linux. Usesful to help you port windows only source code to linux.

This wrapper consists of a header mstest.h, so that the unit tests can be compiled as a shared library, and an execution application which tries to detect all existing tests inside the shared library that needs to be executed.

This wrapper does not support all features from the MS Unit Test Framework yet. Contributions are welcome.

Instead of template assert functions, as MS is using, this wrapper uses macrcos that forwards the file and line information to the actual test functions.

Prerequesites:
  * boost-1.67 and higher
  * C++17 (because the boost shared library lib needs it)

The test runner is called *testhost*.
You needs to call it like this:
`testhost <path-to-shared-lib> <optional-test-filter>`
