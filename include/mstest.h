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

#pragma once

#include <exception>
#include <functional>
#include <map>
#include <iostream>
#include <string.h>
#include <sstream>
#include <source_location>
#include <cmath>
#include <locale>
#include <codecvt>

#define CPPT_ISVISIBLE __attribute__ ((visibility ("default")))

namespace Microsoft {
namespace VisualStudio {
namespace CppUnitTestFramework {
  template<typename T>
  inline std::wstring ToString(const T& v) {
    return std::wstring();
  }
}
}
}

namespace MyTest
{
  class CPPT_ISVISIBLE TestClassImpl
  {
  public:
    virtual ~TestClassImpl() {}

    typedef TestClassImpl* (*__CreateTestClass)();
    typedef void ( *__DestroyTestClass)(TestClassImpl *);
    typedef void (TestClassImpl::*__voidFunc)();

    virtual void InitClass() {}
    virtual void DeInitClass() {}
    virtual void InitMethod() {}
    virtual void DeInitMethod() {}
  };

  template <typename T>
  class CPPT_ISVISIBLE TestClass : public TestClassImpl
  {
  public:
    typedef T ThisClass;

    static TestClassImpl* CreateTestClass()
    {
      return new T();
    }

    static void DestroyTestClass(TestClassImpl* p)
    {
      delete p;
    }
  };

  struct CPPT_ISVISIBLE MethodAttributeInfo {
     bool ignore = false;
     bool ignoreOnLinux = false;
     std::map<std::wstring, std::wstring> key2Val;
  };

  struct CPPT_ISVISIBLE MemberMethodInfo
  {
     TestClassImpl::__voidFunc pVoidMethod;
     TestClassImpl::__CreateTestClass pCreateMethod;
     TestClassImpl::__DestroyTestClass pDestroyMethod;
  };
}

#define TEST_CLASS(name) class CPPT_ISVISIBLE name : public MyTest::TestClass<name>

#define BEGIN_TEST_CLASS_ATTRIBUTE()
#define TEST_CLASS_ATTRIBUTE(key, value)
#define END_TEST_CLASS_ATTRIBUTE()

#define TEST_CLASS_INITIALIZE(mname) void InitClass() override { \
  mname(); \
} \
void mname()


#define TEST_CLASS_CLEANUP(mname) void DeInitClass() override { \
  mname(); \
} \
void mname()


// Call init method over virtual method.
#define TEST_METHOD_INITIALIZE(mname) void InitMethod() override { \
  mname(); \
} \
void mname()

// Call init method over virtual method.
#define TEST_METHOD_CLEANUP(mname) void DeInitMethod() override { \
  mname(); \
} \
void mname()

#define BEGIN_TEST_METHOD_ATTRIBUTE(methodName) \
CPPT_ISVISIBLE [[gnu::used]] static MyTest::MethodAttributeInfo CATNAME(__GetMethodAttributeInfo_, methodName)() \
{ \
  MyTest::MethodAttributeInfo info;

#define TEST_METHOD_ATTRIBUTE(key, value) info.key2Val[key] = value;

#define END_TEST_METHOD_ATTRIBUTE() return info; }


#ifndef CATNAME2
#define CATNAME2(x, y) x##y
#endif

#ifndef CATNAME
#define CATNAME(x, y) CATNAME2(x, y)
#endif

//Macro for creating test methods.
#define TEST_METHOD(methodName)\
  CPPT_ISVISIBLE [[gnu::used]] static const ::MyTest::MemberMethodInfo* CATNAME(__GetTestMethodInfo_, methodName)()\
  {\
   static ::MyTest::MemberMethodInfo s_Info = {nullptr}; \
   s_Info.pVoidMethod = static_cast<::MyTest::TestClassImpl::__voidFunc>(&ThisClass::methodName); \
   s_Info.pCreateMethod = static_cast<::MyTest::TestClassImpl::__CreateTestClass>(&ThisClass::CreateTestClass); \
   s_Info.pDestroyMethod = static_cast<::MyTest::TestClassImpl::__DestroyTestClass>(&ThisClass::DestroyTestClass); \
   return &s_Info;\
  }\
  [[gnu::used]] void methodName()


namespace AssertX
{
  class CPPT_ISVISIBLE AssertFailed : public std::runtime_error
  {
  public:
    AssertFailed(const std::string & s)
    : std::runtime_error(s)
    {}
  };

  template<typename TChar>
  TChar CppUnitLower(TChar c) {return towlower(c);}  
  template<>
  inline char CppUnitLower<char>(char c) {return tolower(c);}
  template<>
  inline wchar_t CppUnitLower<wchar_t>(wchar_t c) {return towlower(c);}
  
  template<typename TChar>
  bool CppUnitStrCmp(const TChar* str1, const TChar* str2, bool ignoreCase)
  {
    if (!str1 && !str2)
      return true;    
    if (!str1 && str2)
      return false;
    if (str1 && !str2)
      return false;      

    while (*str1 && *str2)
    {
      TChar c1 = *str1++;
      TChar c2 = *str2++;

      if (ignoreCase)
      {
        c1 = CppUnitLower(c1);
        c2 = CppUnitLower(c2);
      }

      if (c1 != c2)
        return false;
    }

    return (*str1 == '\0' && *str2 == '\0');
  }

  //----------------------------------------------------------------
  inline std::string WideToUtf8(const wchar_t* wstr)
  {
    if (!wstr || wstr[0] == '\0')
      return std::string();

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
  }

  //----------------------------------------------------------------
  inline std::wstring GetAssertMessage(bool equality, const std::wstring expected, const std::wstring actual, const wchar_t* userMessage)
  {
    std::wstringstream ss;
    if (equality)
    {
      ss << "ExpectedEqual";
    }
    else
    {
      ss << "ExpecteddNotEqual";
    }

    ss << " Expected: <" << expected << "> Actual: <" <<actual << ">";
    if (userMessage && userMessage[0] != '\0')
    {
      ss << " Message: <" << userMessage << ">";
    }

    std::wstring message = ss.str();
    return message;    
  }

#define RETURN_WIDE_STRING(inputValue)  std::wstringstream _cppts;	_cppts << inputValue; return _cppts.str()  
  template<typename T> std::wstring ToString(T t){ RETURN_WIDE_STRING(t); }
	template<> inline std::wstring ToString<const char*>(const char* t){ if (nullptr == t) return std::wstring(L"(NULL)"); RETURN_WIDE_STRING(t); }
	template<> inline std::wstring ToString<const wchar_t*>(const wchar_t* t) { if (nullptr == t) return std::wstring(L"(NULL)"); RETURN_WIDE_STRING(t); }
	template<> inline std::wstring ToString<char*>(char* t){ if (nullptr == t) return std::wstring(L"(NULL)"); RETURN_WIDE_STRING(t); }
	template<> inline std::wstring ToString<wchar_t*>(wchar_t* t) { if (nullptr == t) return std::wstring(L"(NULL)"); RETURN_WIDE_STRING(t); }  
	template<> inline std::wstring ToString<const std::string>(const std::string t) { return ToString(t.c_str()); }    
  template<> inline std::wstring ToString<std::string>(std::string t) { return ToString(t.c_str()); }    
}

inline bool CppUnitStrCmpA(const char* str1, const char* str2, bool ignoreCase) {return AssertX::CppUnitStrCmp<char>(str1, str2, ignoreCase);}
inline bool CppUnitStrCmpW(const wchar_t* str1, const wchar_t* str2, bool ignoreCase) {return AssertX::CppUnitStrCmp<wchar_t>(str1, str2, ignoreCase);}

static inline void FailOnCondition(bool condition, const std::wstring message, const std::source_location location)
{
  if (!condition)
  {
    const std::string oMsg = AssertX::WideToUtf8(message.c_str());

    std::stringstream ss;
    ss 
      << oMsg
      << " at "
      << location.file_name()
      << ":"
      << location.line()
      << " in function "
      << location.function_name();

      throw AssertX::AssertFailed(ss.str());
  }
}

static inline void FailOnCondition(bool condition, const wchar_t* message, const std::source_location location)
{
  FailOnCondition(condition, message ? std::wstring(message) : std::wstring(), location);
}

class Assert
{
#define DEFAULT_IGNORECASE false
#define EQUALS_MESSAGE(expected, actual, message)           AssertX::GetAssertMessage(true, AssertX::ToString(expected), AssertX::ToString(actual), message)
#define NOT_EQUALS_MESSAGE(notExpected, actual, message)    AssertX::GetAssertMessage(false, AssertX::ToString(notExpected), AssertX::ToString(actual), message)

public:
    template<typename T> static void AreEqual(const T& expected, const T& actual, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(expected == actual, EQUALS_MESSAGE(expected, actual, message), location);
    }

    static void AreEqual(double expected, double actual, double tolerance, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        double diff = expected - actual;
        FailOnCondition(fabs(diff) <= fabs(tolerance), EQUALS_MESSAGE(expected, actual, message), location);
    }
    
    static void AreEqual(float expected, float actual, float tolerance, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        float diff = expected - actual;
        FailOnCondition(fabs(diff) <= fabs(tolerance), EQUALS_MESSAGE(expected, actual, message), location);
    }
            
    static void AreEqual(const char* expected, const char* actual, const char* message, const std::source_location location = std::source_location::current())
    {
        AreEqual(expected, actual, DEFAULT_IGNORECASE, AssertX::ToString(message).c_str(), location);
    }
    
    static void AreEqual(const char* expected, const char* actual, const wchar_t* message, const std::source_location location = std::source_location::current())
    {
        AreEqual(expected, actual, DEFAULT_IGNORECASE, message, location);
    }
    
    static void AreEqual(const char* expected, const char* actual, bool ignoreCase = false, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(CppUnitStrCmpA(expected, actual, ignoreCase), EQUALS_MESSAGE(expected, actual, message), location);
    }		
            
    static void AreEqual(const wchar_t* expected, const wchar_t* actual, const char* message, const std::source_location location = std::source_location::current())
    {
        AreEqual(expected, actual, DEFAULT_IGNORECASE, AssertX::ToString(message).c_str(), location);
    }
    
    static void AreEqual(const wchar_t* expected, const wchar_t* actual, const wchar_t* message, const std::source_location location = std::source_location::current())
    {
        AreEqual(expected, actual, DEFAULT_IGNORECASE, message, location);
    }
    
    static void AreEqual(const wchar_t* expected, const wchar_t* actual, bool ignoreCase = false, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(CppUnitStrCmpW(expected, actual, ignoreCase), EQUALS_MESSAGE(expected, actual, message), location);
    }
    
    template<typename T> static void AreSame(const T& expected, const T& actual, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(&expected == &actual, EQUALS_MESSAGE(expected, actual, message), location);
    }
    
    template<typename T> static void AreNotEqual(const T& notExpected, const T& actual, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!(notExpected == actual), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }
    
    static void AreNotEqual(double notExpected, double actual, double tolerance, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        double diff = notExpected - actual;
        FailOnCondition(fabs(diff) > fabs(tolerance), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }
    
    static void AreNotEqual(float notExpected, float actual, float tolerance, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        float diff = notExpected - actual;
        FailOnCondition(fabs(diff) > fabs(tolerance), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }
    
    static void AreNotEqual(const char* notExpected, const char* actual, const char* message, const std::source_location location = std::source_location::current())
    {
        AreNotEqual(notExpected, actual, DEFAULT_IGNORECASE, AssertX::ToString(message).c_str(), location);
    }
    
    static void AreNotEqual(const char* notExpected, const char* actual, const wchar_t* message, const std::source_location location = std::source_location::current())
    {
        AreNotEqual(notExpected, actual, DEFAULT_IGNORECASE, message, location);
    }
    
    static void AreNotEqual(const char* notExpected, const char* actual, bool ignoreCase = false, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!CppUnitStrCmpA(notExpected, actual, ignoreCase), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }

    static void AreNotEqual(const wchar_t* notExpected, const wchar_t* actual, const char* message, const std::source_location location = std::source_location::current())
    {
        AreNotEqual(notExpected, actual, DEFAULT_IGNORECASE, AssertX::ToString(message).c_str(), location);
    }
    
    static void AreNotEqual(const wchar_t* notExpected, const wchar_t* actual, const wchar_t* message, const std::source_location location = std::source_location::current())
    {
        AreNotEqual(notExpected, actual, DEFAULT_IGNORECASE, message, location);
    }		

    static void AreNotEqual(const wchar_t* notExpected, const wchar_t* actual, bool ignoreCase = false, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!CppUnitStrCmpW(notExpected, actual, ignoreCase), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }

    template<typename T> static void AreNotSame(const T& notExpected, const T& actual, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!(&notExpected == &actual), NOT_EQUALS_MESSAGE(notExpected, actual, message), location);
    }
    
    template<typename T> static void IsNull(const T* actual,const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(actual == nullptr, message, location);
    }

    template<typename T> static void IsNotNull(const T* actual, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!actual, message, location);
    }
    
    static void IsTrue(bool condition, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(condition, message, location);
    }

    static void IsFalse(bool condition, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(!condition, message, location);
    }		

    static void Fail(const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(false, message, location);
    }

    template<typename TExpectedExp, typename TFunct> static void ExpectException(TFunct functor, const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {            
        try
        {
            functor();
        }
        catch(TExpectedExp)
        {
            return;
        }
        catch(...)
        {
            throw;
        }

        Assert::Fail(message, location);
    }

    template<typename TExpectedExp, typename TReturnType> static void ExpectException(TReturnType (*func)(), const wchar_t* message = NULL, const std::source_location location = std::source_location::current())
    {
        FailOnCondition(func != nullptr, message, location);
        try
        {
            func();
        }
        catch(TExpectedExp)
        {
            return;
        }
        catch(...)
        {
            throw;
        }

        Assert::Fail(message, location);
    }    
};

#define TEST_OWNER(ownerAlias) TEST_METHOD_ATTRIBUTE(L"Owner", ownerAlias)
#define TEST_DESCRIPTION(description) TEST_METHOD_ATTRIBUTE(L"Description", description)
#define TEST_PRIORITY(priority) TEST_METHOD_ATTRIBUTE(L"Priority", L## #priority)
#define TEST_WORKITEM(workitem) TEST_METHOD_ATTRIBUTE(L"WorkItem", L## #workitem)

#ifdef TEST_LNX_IGNORE
#undef TEST_LNX_IGNORE
#endif
#define TEST_LNX_IGNORE() info.ignoreOnLinux = true;
#define TEST_IGNORE() info.ignore = true;