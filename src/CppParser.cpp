#include <cctype>
#include <cstddef>
#include <regex>
#include <set>
#include <stack>

#include "config.h"
#include "CppParser.h"

#define quota(key) #key

// C++ key words.
static const char* keywords[]={
quota(alignas),
quota(alignof),
quota(and),
quota(and_eq),
quota(asm),
quota(atomic_cancel),
quota(atomic_commit),
quota(atomic_noexcept),
quota(auto),
quota(bitand),
quota(bitor),
quota(bool),
quota(break),
quota(case),
quota(catch),
quota(char),
quota(char8_t),
quota(char16_t),
quota(char32_t),
quota(class),
quota(compl),
quota(concept),
quota(const),
quota(consteval),
quota(constexpr),
quota(constinit),
quota(const_cast),
quota(continue),
quota(contract_assert),
quota(co_await),
quota(co_return),
quota(co_yield),
quota(decltype),
quota(default),
quota(delete),
quota(do),
quota(double),
quota(dynamic_cast),
quota(else),
quota(enum),
quota(explicit),
quota(export),
quota(extern),
quota(false),
quota(float),
quota(for),
quota(friend),
quota(goto),
quota(if),
quota(inline),
quota(int),
quota(long),
quota(mutable),
quota(namespace),
quota(new),
quota(noexcept),
quota(not),
quota(not_eq),
quota(nullptr),
quota(operator),
quota(or),
quota(or_eq),
quota(private),
quota(protected),
quota(public),
quota(reflexpr),
quota(register),
quota(reinterpret_cast),
quota(requires),
quota(return),
quota(short),
quota(signed),
quota(sizeof),
quota(static),
quota(static_assert),
quota(static_cast),
quota(struct),
quota(switch),
quota(synchronized),
quota(template),
quota(this),
quota(thread_local),
quota(throw),
quota(true),
quota(try),
quota(typedef),
quota(typeid),
quota(typename),
quota(union),
quota(unsigned),
quota(using),
quota(virtual),
quota(void),
quota(volatile),
quota(wchar_t),
quota(while),
quota(xor),
quota(xor_eq)
};

CppParser::ClassNameTable CppParser::parse(const std::string &cppSourceCode) {
  clearLastError();

  auto trimmedCode = trimTokens(cppSourceCode);
  if (trimmedCode.empty()) return {};

  std::regex pattern(R"(\s*(class|struct)\s+(\w+)\s*)");
  std::regex_iterator<std::string::iterator> it(trimmedCode.begin(), trimmedCode.end(), pattern);
  std::regex_iterator<std::string::iterator> end;
  ClassNameTable classNameTable;
  while (it != end) {
    classNameTable.push_back(it->str(2));
    ++it;
  }

  if (classNameTable.empty()) {
    setLastError("Error: no component name found.");
    return {};
  }

  if (!isValid(classNameTable)) {
    return {};
  }

  return classNameTable;
}

std::string CppParser::getLastError() const { return mLastError; }

void CppParser::clearLastError() { mLastError.clear(); }

void CppParser::setLastError(std::string error) { mLastError = error; }

bool CppParser::isBraceBalanced(const std::string &str) {
  int depth = 0;
  for (auto &c : str) {
    if (c == '{') {
      ++depth;
    } else if (c == '}') {
      if (depth <= 0) {
        setLastError("Error: invalid C or C++ code.");
        return false;
      }
      --depth;
    }
  }
  return depth == 0;
}

std::string CppParser::trimTokens(const std::string &str) {
  auto result = trimNameSpace(str);
  return trimBracePair(!result.empty() ? result : str);
}

std::string CppParser::trimNameSpace(const std::string &str) {
  auto pos = str.find("namespace", 0);
  if (pos == std::string::npos)
    return {};
  auto first = str.find("{", pos);
  if (first == std::string::npos)
    return {};

  int depth = 1;
  size_t size = str.size();
  size_t last;
  for (last = first + 1; last < size; ++last) {
    if (str[last] == '{') {
      ++depth;
    } else if (str[last] == '}') {
      --depth;
      if (depth == 0)
        break;
    }
  }

  if (depth == 0) {
    std::string newStr = str;
    newStr[first] = ' ';
    newStr[last] = ' ';
    return newStr;
  }

  return {};
}

// Trim tokens in the brace "{...}"
std::string CppParser::trimBracePair(const std::string &str) {
  if (str.empty() || !isBraceBalanced(str)) {
    return {};
  }
  std::stack<size_t> stack;
  const size_t sz = str.size();
  std::vector<bool> keep(sz, 1);
  size_t top = 0;
  for (size_t i = 0; i < sz; ++i) {
    char c = str[i];
    if (c == '{') {
      stack.push(i);
      ++top;
    } else if (c == '}') {
      auto begin = stack.top();
      stack.pop();
      --top;
      if (top == 0) {
        for (std::size_t k = begin; k <= i; ++k) {
          keep[k] = 0;
        }
      }
    }
  }
  std::string out;
  for (size_t i = 0; i < sz; ++i) {
    if (!keep[i])
      continue;
    out.push_back(str[i]);
  }
  return out;
}

bool CppParser::isValid(const ClassNameTable &classNameTable) {
  std::set<std::string> classNameSet;

  auto dumpComponents = [&](){
    std::string error{"Dump Components:\n"};
    size_t number = 1;
    for(auto& name : classNameTable){
      error += "[" + std::to_string(number) + "] " + name + "\n";
      ++number;
    }
    return error;
  };

  for (auto &className : classNameTable) {
    auto result = classNameSet.insert(className);
    if (!result.second) {
      std::string error = "Error: the component name [" + className + "] is duplicated. " + dumpComponents();
      setLastError(error);
      return false;
    }
    if (std::isdigit(className[0])) {
      std::string error = "Error: the component name [" + className + "] cannot begin with digit. " + dumpComponents();
      setLastError(error);
      return false;
    }
    for (auto &key : keywords) {
      if (className == key) {
        std::string error = "Error: the component name [" + className + "] is a C++ keyword. " + dumpComponents();
        setLastError(error);
        return false;
      }
    }
  }

  int tableSize = (int)classNameTable.size();
  if(tableSize > config::MAX_NUMBER_OF_COMPONENTS){
    std::string error = "Error: Input size of components is ";
    error += std::to_string(tableSize) + ";\n";
    error += "Error: but supported max size of components is ";
    error +=  std::to_string(config::MAX_NUMBER_OF_COMPONENTS) + "; ";
    error += dumpComponents();
    setLastError(error);
    return false;
  }

  return true;
}
