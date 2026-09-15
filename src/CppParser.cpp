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

  auto commentErased = eraseComment(cppSourceCode);
  auto defineErased = eraseDefine(commentErased);
  auto trimmedCode = trimTokens(defineErased);
  
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

std::string CppParser::eraseDefine(const std::string &source) {
  auto trimFrontSpace = [](const std::string &s)->std::string {
    size_t start = s.find_first_not_of(" \t\r\f\v");
    if (start == std::string::npos)
      return {};
    return s.substr(start);
  };

  auto trimBackSpace = [](const std::string &s)->std::string {
    size_t end = s.find_last_not_of(" \t\r\f\v");
    if (end == std::string::npos)
      return {};
    return s.substr(0, end + 1);
  };

  auto trimSpace = [&](const std::string &s) {
    return trimFrontSpace(trimBackSpace(s));
  };

  auto isDefineStart = [](const std::string &trimmedLine) {
    if(trimmedLine[0] != '#') return false;

    size_t size = trimmedLine.size();
    size_t i = 1;
    while(i < size){
      if(std::isspace(static_cast<unsigned char>(trimmedLine[i]))){
        ++i;
        continue;
      }else{
        break;
      }
    }

    if(i >= size || trimmedLine[i] != 'd') return false;
    if(++i >= size || trimmedLine[i] != 'e') return false;
    if(++i >= size || trimmedLine[i] != 'f') return false;
    if(++i >= size || trimmedLine[i] != 'i') return false;
    if(++i >= size || trimmedLine[i] != 'n') return false;
    if(++i >= size || trimmedLine[i] != 'e') return false;
    if(++i >= size || !std::isspace(static_cast<unsigned char>(trimmedLine[i]))) return false;

    return true;
  };

  auto endWidthBackSlash = [](const std::string &trimmedLine) {
    return trimmedLine.back() == '\\';
  };

  // Start work here.
  if (source.empty())
    return {};

  std::vector<std::string> lineArray;
  std::string line;

  for (char c : source) {
    if (c == '\n') {
      std::string trimmed = trimSpace(line);
      if (!trimmed.empty()) {
        lineArray.push_back(trimmed);
      }
      line.clear();
    } else {
      line.push_back(c);
    }
  }

  std::string lastLine = trimSpace(line);
  if (!lastLine.empty()) {
    lineArray.push_back(lastLine);
  }

  std::string out;
  bool inMacro = false;
  for (const auto &line : lineArray) {
    if (inMacro) {
      if (endWidthBackSlash(line)) {
        continue;
      } else {
        inMacro = false;
        continue;
      }
    }

    if (isDefineStart(line)) {
      if (endWidthBackSlash(line)) {
        inMacro = true;
      }
      continue;
    }

    out += line + "\n";
  }

  return out;
}

std::string CppParser::eraseComment(const std::string& source) {
  std::string result;

  const size_t len = source.size();
  result.reserve(len);

  size_t i = 0;
  size_t line = 1;

  auto advance = [&](size_t n) {
    i += n;
    if(source[i] == '\n') ++line;
  };

  while (i < len) {
    if (i + 1 >= len) {
      result += source[i];
      advance(1);
      continue;
    }

    char current = source[i];
    char next = source[i+1];

    if (current == '/' && next == '/') {
      advance(2);
      while (i < len && source[i] != '\n') {
        advance(1);
      }
      if (i < len) {
        result += '\n';
        advance(1);
      }
    } else if (current == '/' && next == '*') {
      size_t startLine = line;
      bool foundEnd = false;
      advance(2);
      while (i < len) {
        if (source[i] == '*' && i + 1 < len && source[i+1] == '/') {
          advance(2);
          foundEnd = true;
          break;
        }
        if (source[i] == '\n') {
          result += '\n';
        }
        advance(1);
      }
      if (!foundEnd) {
        //error = "[file line = " + std::to_string(startLine) + "] ";
        //error += "The comment is not closed correctly.";
        return {};
      }
    } else if (current == '"') {
      result += current;
      advance(1);
      while (i < len && source[i] != '"') {
        if (source[i] == '\\' && i + 1 < len) {
          result += source[i];
          ++i;
          result += source[i+1];
          advance(2);
        } else {
          result += source[i];
          advance(1);
        }
      }
      if (i < len) {
        result += source[i];
        advance(1);
      }
    } else if (current == '\'') {
      result += current;
      advance(1);
      while (i < len && source[i] != '\'') {
        if (source[i] == '\\' && i + 1 < len) {
          result += source[i];
          advance(1);
          result += source[i+1];
          advance(2);
        } else {
          result += source[i];
          advance(1);
        }
      }
      if (i < len) {
        result += source[i];
        advance(1);
      }
    } else {
      result += current;
      advance(1);
    }
  }

  return result;
}

bool CppParser::isValid(ClassNameTable &classNameTable) {
  std::set<std::string> classNameSet;

  auto dumpComponents = [&](){
    std::string error{"Dump all the components:\n"};
    size_t number = 1;
    for(auto& name : classNameTable){
      error += "[" + std::to_string(number) + "] " + name + "\n";
      ++number;
    }
    return error;
  };

  for (auto &className : classNameTable) {
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
    classNameSet.insert(className);
  }

  classNameTable.clear();
  for(auto& className : classNameSet){
    classNameTable.push_back(className);
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
