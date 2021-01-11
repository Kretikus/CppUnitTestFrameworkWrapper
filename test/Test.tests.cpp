#include "mstest.h"

class ToStringTestClass
{
public:
};

namespace Microsoft {
  namespace VisualStudio {
    namespace CppUnitTestFramework {
      // Do nothing. This tests, that the ToSting function in the Namespace can be overloaded
      template <> inline std::wstring ToString<ToStringTestClass>(const ToStringTestClass&)
      {
        std::wostringstream os;
        return os.str();
      }
    }
  }
}

TEST_CLASS(INDateTests)
{
  std::string stringInitOnInitClass;
  std::string stringInitOnInitMethod;

  BEGIN_TEST_CLASS_ATTRIBUTE()
  TEST_CLASS_ATTRIBUTE(L"Something", L"Something")
  END_TEST_CLASS_ATTRIBUTE()
public:

  TEST_CLASS_INITIALIZE(OnInitClass) {
    stringInitOnInitClass = "InitOnClass";
  }

  TEST_METHOD_INITIALIZE(InitBeforeEachMethod) {
    stringInitOnInitMethod = "InitOnMethod";
  }

  TEST_METHOD(InitTest1) {
    Assert::AreEqual("InitOnClass", stringInitOnInitClass);
    Assert::AreEqual("InitOnMethod", stringInitOnInitMethod);
    stringInitOnInitMethod = "";
  }

  BEGIN_TEST_METHOD_ATTRIBUTE(InitTest2)
      TEST_METHOD_ATTRIBUTE(L"Something", L"Something")
  END_TEST_METHOD_ATTRIBUTE()
  TEST_METHOD(InitTest2) {
    Assert::AreEqual("InitOnClass", stringInitOnInitClass);
    Assert::AreEqual("InitOnMethod", stringInitOnInitMethod);
    stringInitOnInitMethod = "";
  }

  BEGIN_TEST_METHOD_ATTRIBUTE(Ignore1)
      TEST_IGNORE()
  END_TEST_METHOD_ATTRIBUTE()
  TEST_METHOD(Ignore1) {
    Assert::IsFalse(true);
  }

  BEGIN_TEST_METHOD_ATTRIBUTE(Ignore2)
      TEST_LNX_IGNORE()
  END_TEST_METHOD_ATTRIBUTE()
  TEST_METHOD(Ignore2) {
    Assert::IsFalse(true);
  }

  TEST_METHOD(CatchExceptionTest) {
    Assert::ExpectException<std::bad_exception>([]() { throw std::bad_exception(); });
  }
};
