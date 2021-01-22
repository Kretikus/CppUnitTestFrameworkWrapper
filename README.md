# CppUnitTestFrameworkWrapper

A simple wrapper that allows Tests written with the CppUnitTestFramework
from Microsoft to compile and be run under linux.

This wrapper does not support all features from the MS Unit Test Framework.

Instead of template assert functions, as MS is using, this wrapper uses macrcos that forwards the file and line information to the actual test functions.

Prerequesites:
  * boost-1.67 and higher
  * C++17 (because the boost shared library lib needs it)

The testrunner is called *testhost* and need to be called like this:
`testhost <path-to-shared-lib> <optional-test-filter>`
