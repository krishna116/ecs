#include <iostream>
#include <cctype>

#include "template.h"
#include "EcsCompiler.h"
#include "CppParser.h"

struct ComponentInfo{
  std::string id;       // Component id.
  std::string name;     // Component name.
  std::string instName; // Component instance name.
};

using ComponentInfoTable = std::vector<ComponentInfo>;

static std::string toInstName(std::string compName){
  if(std::isupper(compName[0])){
    compName[0] = std::tolower(compName[0]);
    return compName;
  }else{
    compName += "Inst";
    return compName;
  }
}

static ComponentInfoTable toComponentInfoTable(CppParser::ClassNameTable& classNameTable){
  ComponentInfoTable compTable;
  size_t size = classNameTable.size();
  for(size_t i = 0; i < size; ++i){
    compTable.emplace_back(ComponentInfo{std::to_string(i), classNameTable[i], toInstName(classNameTable[i])});
  }
  return compTable;
}

static size_t getMaxComponentNameSize(const ComponentInfoTable& compInfoTable){
  size_t maxSize = 0;
  for(auto& compInfo : compInfoTable){
    size_t curSize = compInfo.name.size();
    if(maxSize < curSize) maxSize = curSize;
  }
  return maxSize;
}

/**
 * Search and replace tokens in the text.
 */
static void searchAndReplace(std::string& text, const std::string& toFind, const std::string& toReplace) {
  size_t pos = 0;
  while ((pos = text.find(toFind, pos)) != std::string::npos) {
    text.replace(pos, toFind.length(), toReplace);
    pos += toReplace.length();
  }
}

static std::string generateCode(const ComponentInfoTable& compInfoTable){
  const auto compTableSize = compInfoTable.size();
  const auto compTableSizeStr = std::to_string(compTableSize);
  const auto compNameSizeMax = getMaxComponentNameSize(compInfoTable);

  auto genDefineToUintPtr = [&](){
    std::string str = gDefineToUintPtrTemplate;
    auto length = (compNameSizeMax + 1) > 4 ? compNameSizeMax + 1 - 4 : 1;
    std::string spaceBetweenDefine(length, ' ');
    searchAndReplace(str, Token::SpaceBetweenDefine, spaceBetweenDefine);
    return str;
  };

  auto genDefineToComponentPtrTable=[&]{
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gDefineToComponentPtrTemplate;
      searchAndReplace(str, Token::ComponentName,  compInfoTable[i].name);
      std::string spaceBetweenDefine(compNameSizeMax + 1 - compInfoTable[i].name.size(), ' ');
      searchAndReplace(str, Token::SpaceBetweenDefine, spaceBetweenDefine);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genVisitorInterfaceTable = [&]{
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gVisitorInterfaceTemplate;
      searchAndReplace(str, Token::ComponentName,  compInfoTable[i].name);
      searchAndReplace(str, Token::ComponentInstName, compInfoTable[i].instName);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genComponentIdTable = [&]{
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gComponentIdDefineTemplate;
      searchAndReplace(str, Token::ComponentName,  compInfoTable[i].name);
      searchAndReplace(str, Token::ComponentId, compInfoTable[i].id);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genComponentIdTableSize = [&]{
    std::string str = gComponentIdTableSizeTemplate;
    searchAndReplace(str, Token::NumberOfComponents, compTableSizeStr);
    return str;
  };

  auto genComponentInfoTable = [&](){
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gComponentInfoTemplate;
      searchAndReplace(str, Token::ComponentName, compInfoTable[i].name);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genCloneSlotTable = [&](){
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gCloneSlotTemplate;
      searchAndReplace(str, Token::ComponentId, compInfoTable[i].id);
      searchAndReplace(str, Token::ComponentName, compInfoTable[i].name);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genClearSlotTable = [&](){
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gClearSlotTemplate;
      searchAndReplace(str, Token::ComponentId, compInfoTable[i].id);
      searchAndReplace(str, Token::ComponentName, compInfoTable[i].name);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  auto genVisitSlotTable = [&](){
    std::string strTable;
    for(size_t i = 0; i < compTableSize; ++i){
      std::string str = gVisitSlotTemplate;
      searchAndReplace(str, Token::ComponentId, compInfoTable[i].id);
      searchAndReplace(str, Token::ComponentName, compInfoTable[i].name);
      strTable += str;
      if(i + 1 < compTableSize) strTable += "\n";
    }
    return strTable;
  };

  std::string code = gCodeTemplate;
  searchAndReplace(code, Token::EcsCompilerVersion, config::ECS_COMIPLER_VERSION);
  searchAndReplace(code, Token::NameSpaceName, config::ECS_COMIPLER_NAME);
  searchAndReplace(code, Token::DefineUintPtr, genDefineToUintPtr());
  searchAndReplace(code, Token::DefineToComponentPtrTable, genDefineToComponentPtrTable());
  searchAndReplace(code, Token::VisitorInterfaceTable, genVisitorInterfaceTable());
  searchAndReplace(code, Token::ComponentIdTable, genComponentIdTable());
  searchAndReplace(code, Token::ComponentIdTableSize, genComponentIdTableSize());
  searchAndReplace(code, Token::ComponentInfoTable, genComponentInfoTable());
  searchAndReplace(code, Token::CloneSlotTable, genCloneSlotTable());
  searchAndReplace(code, Token::ClearSlotTable, genClearSlotTable());
  searchAndReplace(code, Token::VisitSlotTable, genVisitSlotTable());

  return code;
}

std::string EcsCompiler::compile(const std::string &cppSourceCode){
  CppParser parser;

  auto classNameTable = parser.parse(cppSourceCode);
  if(!parser.getLastError().empty()){
    std::cout<< parser.getLastError() << std::endl;
    return {};
  }
  
  auto compInfoTable = toComponentInfoTable(classNameTable);
  return generateCode(compInfoTable);
}
