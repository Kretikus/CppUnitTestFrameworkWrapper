//------------------------------------------------------------------------
// Initial code by Kretikus Roman, released on MIT license
// Copyright (c) 2025
//    https://github.com/Kretikus/CppUnitTestFrameworkWrapper
//
// This file is under the MIT License (MIT).
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// ==========================================================================
// Additional work by Julien Vary for Genetec, Inc.
//------------------------------------------------------------------------

#include <boost/core/demangle.hpp>
#include <boost/dll.hpp>
#include <boost/dll/import_mangled.hpp>

#include <mstest.h>

#include <iostream>
#include <set>
#include <filesystem>

#include "guid.h"
#include "timehelpers.h"
#include "test.h"
#include "trxoutput.h"

std::string testSo;
std::string testFilter;
std::string testTrx;

SomeTime testEntry = Now();
SomeTime testCompletion;

struct LocalCounter
{
  int totalTests = 0;
  int ignoredOnLinux = 0;
  int ignored = 0;    
};

// --------------------------------------------------------------------------
//   forwards
// --------------------------------------------------------------------------
int parseArgs(int argc, char* argv[]);

void ProcessMethod( const std::string &rMethodInfoName
                  , const std::set<std::string>& testfunctionsAttributes              
                  , std::vector<Test>& rAllTests
                  , std::vector<std::pair<std::string, AssertX::AssertFailed>>& rFailedTests
                  , LocalCounter& rLocalCounters
                  );

// --------------------------------------------------------------------------
inline bool endsWith(const std::string& value, const std::string& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// --------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Fill arguments" << std::endl;
    return -1;
  }

  if (parseArgs(argc-1, argv+1) < 0)
  {
    std::cerr << "Error parsing arguments" << std::endl;
    return -1;
  }

  std::cout << "Running within " << std::filesystem::current_path() << std::endl;
  std::cout << "Testing dll " << testSo << std::endl;

  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(testSo);

  const std::string filter = testFilter;

  // Getting exported symbols
  std::vector<std::string> exports = inf.symbols();

  std::vector<std::string> testfunctions;
  std::set<std::string> testfunctionsAttributes;
  for (auto sym : exports) {
    if (sym.find("__GetTestMethodInfo_") != std::string::npos)
    {
      std::string demangled = boost::core::demangle(sym.c_str());
      if (endsWith(demangled, "()")) {
        testfunctions.push_back(demangled.substr(0, demangled.length()-2));        
      }
    }
    else if (sym.find("__GetMethodAttributeInfo_") != std::string::npos)
    {
      std::string demangled = boost::core::demangle(sym.c_str());
      if (endsWith(demangled, "()")) {
        testfunctionsAttributes.insert(demangled.substr(0, demangled.length()-2));        
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

  std::vector<std::pair<std::string, AssertX::AssertFailed>> failedTests;
  std::vector<Test> allTests;
  LocalCounter olocalCounters;

  using namespace boost::dll::experimental;
  if (testfunctions.size() > 0) {
    boost::dll::shared_library lib;
    try {
      lib.load(testSo);
    } catch (const std::exception& ex) {
      std::cerr << "Exception loading lib at " << ex.what() << std::endl;
      return -1;
    }

    for (auto sym : testfunctions) {
        ProcessMethod(sym, testfunctionsAttributes, allTests, failedTests, olocalCounters);
    }

    if (failedTests.size() > 0)
    {
      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
      std::cout << failedTests.size() << " of " << olocalCounters.totalTests << " Tests failed.\n";
      std::cout << "Summary:\n";
      for(const auto& ft : failedTests)
      {
        std::wcerr << converter.from_bytes(ft.first) << ": " << converter.from_bytes(ft.second.what()) << std::endl;
      }
    } 
    else 
    {
      std::cout << "All tests passed.";
      if (olocalCounters.ignoredOnLinux > 0) 
      {
        std::cout << " " << olocalCounters.ignoredOnLinux << " ignored for Linux; ";
        std::cout << " " << olocalCounters.ignored << " ignored total; ";        
      }
      std::cout << std::endl;
    }

    testCompletion = Now();

    TrxOutput::OutputToFile(testTrx, testSo, testEntry, testCompletion, allTests);
  }
  else 
  {
    std::cerr << "No tests found in " << testSo << std::endl;
    TrxOutput::OutputToFile(testTrx, testSo, testEntry, testCompletion, allTests);
    return -1;
  }  

  return 0;
}

static const std::string needle = "__GetTestMethodInfo_";
static const std::string attributeInfo = "__GetMethodAttributeInfo_";
void ProcessMethod( const std::string &rMethodInfoName
                  , const std::set<std::string>& testfunctionsAttributes              
                  , std::vector<Test>& rAllTests
                  , std::vector<std::pair<std::string, AssertX::AssertFailed>>& rFailedTests
                  , LocalCounter& rLocalCounters
                  )
{
    std::string testname = rMethodInfoName;
    testname.replace(testname.find(needle), needle.length(), "");
    std::string fncGetAttribInfo= rMethodInfoName;
    fncGetAttribInfo.replace(fncGetAttribInfo.find(needle), needle.length(), attributeInfo);

    rAllTests.push_back(Test()); 
    Test& rCurrentTest = rAllTests.back();
    rCurrentTest.functionName = testname;
    rCurrentTest.testId = Guid::New();
    rCurrentTest.executionId = Guid::New();

    using namespace boost::dll::experimental;

    MyTest::MethodAttributeInfo info;
    try
    {
        if (testfunctionsAttributes.find(fncGetAttribInfo) != testfunctionsAttributes.end())
        {
            auto fnc = import_mangled<MyTest::MethodAttributeInfo()>(testSo, fncGetAttribInfo);
            info = fnc();
        }
    } 
    catch(std::exception &e)
    {
        std::cerr << "Exception at " << e.what() << " for " << fncGetAttribInfo << std::endl;
        rCurrentTest.error = true;
        return;
    }

    if (info.ignoreOnLinux)
        ++rLocalCounters.ignoredOnLinux;

    if (!info.ignore && !info.ignoreOnLinux)
    {
        std::cout << "Calling Test " << testname << "\n";
        try
        {
            // Get Function to fetch entry points
            auto fnc = import_mangled<MyTest::MemberMethodInfo*()>(testSo, rMethodInfoName);
            auto* mInfo = fnc();

            // Run actual test
            MyTest::TestClassImpl* classUnderTest = mInfo->pCreateMethod();
            classUnderTest->InitClass();
            classUnderTest->InitMethod();

            try
            {
                // Call the test
                rCurrentTest.executed = true;
                rCurrentTest.entryTime = Now();
                (classUnderTest->*mInfo->pVoidMethod)();
            }
            catch(const AssertX::AssertFailed& ex)
            {
                rFailedTests.push_back(std::make_pair(testname, ex));
                rCurrentTest.pFailure= std::make_shared<AssertX::AssertFailed>(ex);      
            }
            catch(...)
            {
                std::cerr << "Unexpected exception" << std::endl;
                auto ex = AssertX::AssertFailed("Unexpected exception");
                rFailedTests.push_back(std::make_pair(testname, ex ));
                rCurrentTest.pFailure= std::make_shared<AssertX::AssertFailed>(ex);
            }
            rCurrentTest.endTime= Now();

            classUnderTest->DeInitMethod();
            classUnderTest->DeInitClass();
            mInfo->pDestroyMethod(classUnderTest);
            ++rLocalCounters.totalTests;
        } 
        catch (const std::exception& ex)
        {
            std::cerr << "Exception at " << ex.what() << std::endl;
            rCurrentTest.error = true;
        }
    }
    else
    {
        std::cout << "Ignoring Test " << testname << "\n";
        rLocalCounters.ignored++;
        rCurrentTest.ignored = true;
    }
}

// --------------------------------------------------------------------------
//starting 1 pass the exec name
int parseArgs(int argc, char* argv[])
{
    if (argc %2 != 0)
    {
        std::cerr << "Invalid number of arguments" << std::endl;
        return -1;
    }

    for (int i = 0; i < argc; i += 2)
    {
        if (strcmp(argv[i], "--so") == 0)
        {
            testSo = argv[i + 1];
        }
        else if (strcmp(argv[i], "--filter") == 0)
        {
            testFilter = argv[i + 1];
        }
        else if (strcmp(argv[i], "--trx") == 0)
        {
            testTrx = argv[i + 1];
        }
        else
        {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            return -1;
        }
    }
    if (testSo.empty())
    {
        std::cerr << "No --so argument provided" << std::endl;
        return -1;
    }

    if (testFilter.empty())    
        std::cout  << "No filter provided" << std::endl;
    else
        std::cout  << "Filter is : " << testFilter << std::endl;

    if (testTrx.empty())    
        std::cout  << "No trx sink" << std::endl;
    else
        std::cout  << "Trx sink is : " << testTrx << std::endl;

    return 0;
}