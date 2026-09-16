#include <cctype>
#include <cstddef>
#include <set>

#include "config.h"
#include "CppLexer.h"
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

inline CppParser::ClassNameTable toClassNameTable(CppLexer::TokenArray& tokens, bool& good){
  using Token = CppLexer::Token;
  CppParser::ClassNameTable classNameTable;
  size_t n = tokens.size();
  size_t i = 0;
  good = true;
  
  while (i < n) {
    while(i<n && tokens[i].type == Token::Type::Space) ++i;
    if(i >= n) break;

    if(tokens[i].type == Token::Type::Class){
      ++i;
    }else{
      good = false;
      break;
    }

    if(i < n && tokens[i].type == Token::Type::Space){
      ++i;
    }else{
      good = false;
      break;
    }

    if(i < n && tokens[i].type == Token::Type::CName){
      classNameTable.push_back(tokens[i].name);
      ++i;
    }else{
      good = false;
      break;
    }
  }

  return classNameTable;
}


CppParser::ClassNameTable CppParser::parse(const std::string &cppSourceCode) {
  clearLastError();

  bool good;
  auto tokens = CppLexer::parse(cppSourceCode, &good);

  if(!good){
    setLastError("[cpp lexer error]: parsing source code failed.");
    return {};
  }

  auto classNameTable = toClassNameTable(tokens, good);
  if(!good){
    setLastError("[cpp parser error]: parsing source code failed.");
    return {};
  }

  if(classNameTable.empty()){
    setLastError("Error: no class or struct name found.");
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
