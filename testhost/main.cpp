#include <boost/core/demangle.hpp>
#include <boost/dll.hpp>
#include <boost/dll/import_mangled.hpp>

#include <mstest.h>

#include <iostream>

inline bool endsWith(const std::string& value, const std::string& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Pass path to lib to test" << std::endl;
    return -1;
  }

  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(argv[1]);

  const std::string filter = argc > 2 ? std::string(argv[2]) : std::string();

  // Getting exported symbols
  std::vector<std::string> exports = inf.symbols();

  std::vector<std::string> testfunctions;
  for (auto sym : exports) {
    if (sym.find("GetTestMethodInfo_") != std::string::npos)
    {
      std::string demangled = boost::core::demangle(sym.c_str());
      if (endsWith(demangled, "()")) {
        testfunctions.push_back(demangled.substr(0, demangled.length()-2));
      }
    }
  }

  std::sort(testfunctions.begin(), testfunctions.end());

  if (!filter.empty()) {
    std::cout << "Applying filter " << filter << std::endl;
    testfunctions.erase(
      std::remove_if(testfunctions.begin(),
                     testfunctions.end(),
                     [&filter](const std::string& elem) -> bool {
                        return elem.find(filter) == std::string::npos;
                      }),
    testfunctions.end());
  }

  using namespace boost::dll::experimental;
  if (testfunctions.size() > 0) {
    boost::dll::shared_library lib;
    try {
      lib.load(argv[1]);
    } catch (const std::exception& ex) {
      std::cerr << "Exception loading lib at " << ex.what() << std::endl;
    }

    int totalTests = 0;
    int ignoredOnLinux = 0;
    std::vector<std::pair<std::string, Assert::AssertFailed>> failedTests;
    const std::string needle = "__GetTestMethodInfo_";
    const std::string attributeInfo = "__GetMethodAttributeInfo_";
    for (auto sym : testfunctions) {
      std::string testname = sym;
      testname.replace(testname.find(needle), needle.length(), "");
      std::string fncGetAttribInfo= sym;
      fncGetAttribInfo.replace(fncGetAttribInfo.find(needle), needle.length(), attributeInfo);

      MyTest::MethodAttributeInfo info;
      try {
        // Not every function has AttribInfos. This lookup is allowed to fail.
        auto fnc = import_mangled<MyTest::MethodAttributeInfo()>(argv[1], fncGetAttribInfo);
        info = fnc();
      }
      catch(...)
      {}

      if (info.ignoreOnLinux) ++ignoredOnLinux;

      if (!info.ignore && !info.ignoreOnLinux)
      {
        std::cout << "Calling Test " << testname << "\n";
        try {
          // Get Function to fetch entry points
          auto fnc = import_mangled<MyTest::MemberMethodInfo*()>(argv[1], sym);
          auto* mInfo = fnc();

          // Run actual test
          MyTest::TestClassImpl* classUnderTest = mInfo->pCreateMethod();
          classUnderTest->InitClass();
          classUnderTest->InitMethod();

          try {
            // Call the test
            (classUnderTest->*mInfo->pVoidMethod)();
          } catch(const Assert::AssertFailed& ex) {
            failedTests.push_back(std::make_pair(testname, ex));
          }
          catch(...) {
            std::cerr << "Unexpected exception" << std::endl;
            failedTests.push_back(std::make_pair(testname, Assert::AssertFailed("Unexpected exception")));
          }

          classUnderTest->DeInitClass();
          mInfo->pDestroyMethod(classUnderTest);
          ++totalTests;
        } catch (const std::exception& ex) {
          std::cerr << "Exception at " << ex.what() << std::endl;
        }
      } else {
        std::cout << "Ignoring Test " << testname << "\n";
      }
    }
    if (failedTests.size() > 0) {
      std::cout << failedTests.size() << " of " << totalTests << " Tests failed.\n";
      std::cout << "Summary:\n";
      for(const auto& ft : failedTests) {
        std::cerr << ft.first << ": " << ft.second.what();
      }
    } else {
      std::cout << "All tests passed.";
      if (ignoredOnLinux > 0) {
        std::cout << " " << ignoredOnLinux << " fuer Linux ignoriert.";
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
