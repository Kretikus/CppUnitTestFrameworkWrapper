#pragma once

#include <exception>
#include <functional>
#include <map>
#include <iostream>
#include <string.h>
#include <sstream>

#undef NDEBUG
#include <cassert>

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

#define RETURN_WIDE_STRING(val)         \
        std::wstringstream valasstr_;	\
        valasstr_ << val;               \
        return valasstr_.str()

namespace MyTest
{
  class TestClassImpl
  {
  public:
    virtual ~TestClassImpl() {}

    typedef TestClassImpl* (*__CreateTestClass)();
    typedef void ( *__DestroyTestClass)(TestClassImpl *);
    typedef void (TestClassImpl::*__voidFunc)();

    virtual void InitClass() {}
    virtual void DeInitClass() {}
    virtual void InitMethod() {}
    virtual void CleanupMethod() {}
  };

  template <typename T>
  class TestClass : public TestClassImpl
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

  struct MethodAttributeInfo {
     bool ignore = false;
     bool ignoreOnLinux = false;
     std::map<std::wstring, std::wstring> key2Val;
  };

  struct MemberMethodInfo
  {
     TestClassImpl::__voidFunc pVoidMethod;
     TestClassImpl::__CreateTestClass pCreateMethod;
     TestClassImpl::__DestroyTestClass pDestroyMethod;
  };
}

#define TEST_CLASS(name) class name : public MyTest::TestClass<name>

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

// Call cleanup method over virtual method.
#define TEST_METHOD_CLEANUP(mname) void CleanupMethod() override { \
  mname(); \
} \
void mname()

#define BEGIN_TEST_METHOD_ATTRIBUTE(methodName) [[gnu::used]] static MyTest::MethodAttributeInfo CATNAME(__GetMethodAttributeInfo_, methodName)() \
{ \
  MyTest::MethodAttributeInfo info;

#ifdef TEST_LNX_IGNORE
#undef TEST_LNX_IGNORE
#endif
#define TEST_LNX_IGNORE() info.ignoreOnLinux = true;
#define TEST_IGNORE() info.ignore = true;

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
  [[gnu::used]] static const ::MyTest::MemberMethodInfo* CATNAME(__GetTestMethodInfo_, methodName)()\
  {\
   static ::MyTest::MemberMethodInfo s_Info = {nullptr}; \
   s_Info.pVoidMethod = static_cast<::MyTest::TestClassImpl::__voidFunc>(&ThisClass::methodName); \
   s_Info.pCreateMethod = static_cast<::MyTest::TestClassImpl::__CreateTestClass>(&ThisClass::CreateTestClass); \
   s_Info.pDestroyMethod = static_cast<::MyTest::TestClassImpl::__DestroyTestClass>(&ThisClass::DestroyTestClass); \
   return &s_Info;\
  }\
  [[gnu::used]] void methodName()


#include <sstream>

namespace PrintHelper
{
  template<typename S, typename T, typename = void>
  struct is_to_stream_writable: std::false_type {};

  template<typename S, typename T>
  struct is_to_stream_writable<S, T,
          std::void_t<  decltype( std::declval<S&>()<<std::declval<T>() )  > >
  : std::true_type {};


  template<typename T>
  struct Printer
  {
    template<typename Q = T>
    typename std::enable_if<is_to_stream_writable<std::ostream,Q>::value, bool>::type print(std::stringstream& ss, const Q& q)
    {
      ss << q;
      return true;
    }
    template<typename Q = T>
    typename std::enable_if<!is_to_stream_writable<std::ostream,Q>::value, bool>::type print(std::stringstream& ss, const Q&)
    {
      ss << "<UNPRINTABLE>";
      return false;
    }
  };

  // This prints the value if there is a streaming operator available.
  // If not, <UNPRINTABLE> is printed.
  template<typename Expected, typename Actual>
  void getActualAndExpected(std::stringstream& ss, const Expected& expected, const Actual& actual)
  {
    Printer<Expected> expectedPrinter;
    Printer<Actual> actualPrinter;
    ss << "Expected: <";
    expectedPrinter.print(ss, expected);
    ss << "> Actual: <";
    actualPrinter.print(ss, actual);
    ss << ">\n";
  }
} // End of namespace


namespace Logger {
  inline void WriteMessage(const char* str)
  {
    std::cout << str << std::endl;
  }

  inline void WriteMessage(const wchar_t* str)
  {
    std::wcout << str << std::endl;
  }
}

namespace Assert
{

inline std::string toStdString(const std::wstring& s)
{
  std::vector<char> buf(1024);
  auto l = wcstombs(&buf[0], s.c_str(), buf.size());
  if (l == (size_t)-1) {
    // could not be converted
    return std::string();
  }
  if (l > buf.size()) {
    buf.resize(l+1);
    wcstombs(&buf[0], s.c_str(), buf.size());
  }
  return std::string(&buf[0]);
}

class AssertFailed : public std::runtime_error
{
public:
  AssertFailed(const std::string & s)
  : std::runtime_error(s)
  {}
};

template<typename Expected, typename Actual>
inline bool compareEqual(const Expected& expected, const Actual& actual)
{
  return expected == actual;
}

inline bool compareEqual(const char* expected, const char* actual)
{
  return strcmp(expected, actual) == 0;
}

inline bool compareEqual(const wchar_t* expected, const wchar_t* actual)
{
  return wcscmp(expected, actual) == 0;
}

template<typename Expected, typename Actual>
inline void __AreEqual(const Expected& expected, const Actual& actual,
                       const char* file, int line, const std::string& remark = std::string())
{
  if (!compareEqual(expected, actual)) {
    std::stringstream ss;
    ss << "Expected and Actual not equal" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << remark;
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expected, actual);
    std::cerr << ss.str();
    throw AssertFailed(ss.str());
  }
}

template<typename Expected, typename Actual>
inline void __AreEqual(const Expected& expected, const Actual& actual,
                       const char* file, int line, const std::wstring& remark)
{
  if (!compareEqual(expected, actual)) {
    std::stringstream ss;
    ss << "Expected and Actual not equal" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << toStdString(remark);
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expected, actual);
    std::cerr << ss.str();
    throw AssertFailed(ss.str());
  }
}

#define AreEqual(expected, actual, ...) __AreEqual(expected, actual, __FILE__, __LINE__, ##__VA_ARGS__)
#define VERIFY_ARE_EQUAL(expected, actual, ...) Assert::__AreEqual(expected, actual, __FILE__, __LINE__, ##__VA_ARGS__)


template<typename Expected, typename Actual>
inline void __AreNotEqual(const Expected& expected, const Actual& actual,
                          const char* file, int line, const std::wstring& remark)
{
  if (compareEqual(expected, actual)) {
    std::stringstream ss;
    ss << "Expected and Actual equal" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << toStdString(remark);
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expected, actual);
    throw AssertFailed(ss.str());
  }
}

template<typename Expected, typename Actual>
inline void __AreNotEqual(const Expected& expected, const Actual& actual,
                          const char* file, int line, const std::string& remark = std::string())
{
  if (compareEqual(expected, actual)) {
    std::stringstream ss;
    ss << "Expected and Actual equal" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << remark;
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expected, actual);
    throw AssertFailed(ss.str());
  }
}

#define AreNotEqual(expected, actual, ...) __AreNotEqual(expected, actual, __FILE__, __LINE__, ##__VA_ARGS__)


inline void __IsFalse(const bool& val, const char* expr,
                      const char* file, int line, const std::wstring& remark)
{
  if (val) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") not false" << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}

inline void __IsFalse(const bool& val, const char* expr,
                      const char* file, int line, const std::string& remark = std::string())
{
  if (val) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") not false" << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}


#define IsFalse(expr, ...) __IsFalse(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)
#define VERIFY_IS_FALSE(expr, ...) Assert::__IsFalse(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)

inline void __IsTrue(const bool& val, const char* expr,
                     const char* file, int line, const std::string& remark = std::string())
{
  if (!val) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") not true at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}

inline void __IsTrue(const bool& val, const char* expr,
                     const char* file, int line, const std::wstring& remark)
{
  if (!val) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") not true at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}
#define IsTrue(expr, ...) __IsTrue(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)
#define VERIFY_IS_TRUE(expr, ...) Assert::__IsTrue(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)


template<typename Expected, typename Actual>
inline void __IsLessThan(const Expected& expectedLess, const Actual& expectedGreater,
                         const char* file, int line, const std::string& remark = std::string())
{
  if (!(expectedLess < expectedGreater)) {
    std::stringstream ss;
    ss << "expectedLess is not less than expectedGreater" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << remark;
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expectedLess, expectedGreater);
    std::cerr << ss.str();
    throw AssertFailed(ss.str());
  }
}

template<typename Expected, typename Actual>
inline void __IsLessThan(const Expected& expectedLess, const Actual& expectedGreater,
                         const char* file, int line, const std::wstring& remark)
{
  if (!(expectedLess < expectedGreater)) {
    std::stringstream ss;
    ss << "expectedLess is not less than expectedGreater" << " " << file << " " << line;
    if (!remark.empty()) ss << " Remark: " << toStdString(remark);
    ss << "\n";
    PrintHelper::getActualAndExpected(ss, expectedLess, expectedGreater);
    std::cerr << ss.str();
    throw AssertFailed(ss.str());
  }
}

#define IsLessThan(expected, actual, ...) __IsLessThan(expected, actual, __FILE__, __LINE__, ##__VA_ARGS__)
#define VERIFY_IS_LESS_THAN(expectedLess, expectedGreater, ...) Assert::__IsLessThan(expectedLess, expectedGreater, __FILE__, __LINE__, ##__VA_ARGS__)

inline void __IsNull(const void* ptr, const char* expr,
                     const char* file, int line, const std::string& remark = std::string())
{
  if (ptr != nullptr) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") is not null at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}

inline void __IsNull(const void* ptr, const char* expr,
                     const char* file, int line, const std::wstring& remark)
{
  if (ptr != nullptr) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") is not null at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}
#define IsNull(expr, ...) __IsNull(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)


inline void __IsNotNull(const void* ptr, const char* expr,
                        const char* file, int line, const std::wstring& remark)
{
  if (ptr == nullptr) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") is null at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}

inline void __IsNotNull(const void* ptr, const char* expr,
                        const char* file, int line, const std::string& remark = std::string())
{
  if (ptr == nullptr) {
    std::stringstream ss;
    ss << "Expression (" << expr << ") is null at " << expr << " " << file << " " << line << "\n";
    throw AssertFailed(ss.str());
  }
}
#define IsNotNull(expr, ...) __IsNotNull(expr, #expr, __FILE__, __LINE__, ##__VA_ARGS__)


template <typename ExceptionType>
inline void ExpectException(std::function<void()> fnc)
{
  bool exceptionCaught = false;
  bool wrongExceptionCaught = false;
  try {
    fnc();
  } catch(const ExceptionType&) {
   exceptionCaught = true;
  } catch(...) {
    wrongExceptionCaught = true;
  }

  if (!exceptionCaught) {
    std::stringstream ss;
    if (wrongExceptionCaught) {
      ss << "Wrong exception caught. Expected: " << typeid(ExceptionType).name() << "\n";
    } else {
      ss << "No exception caught. Expected: " << typeid(ExceptionType).name() << "\n";
    }
    throw AssertFailed(ss.str());
  }
}

inline void Fail(const std::wstring& str)
{
  const size_t max = str.length()+1;
  std::vector<char> buff(max, '\0');
  wcstombs(buff.data(), str.c_str(), max);
  std::stringstream ss;
  ss << buff.data() << "\n";
  throw AssertFailed(ss.str());
}

inline void Fail()
{
  std::string ss{"Failed"};
  throw AssertFailed(ss);
}

}
