/**
 * An ecs compiler to generate C++ ECS(Entity Component System).
 */
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>

#include "config.h"
#include "EcsCompiler.h"

static int printUsage(){
  printf("An ecs compiler to generate C++ ECS(Entity Component System).\n");
  printf("Usage:\n");
  printf("  ecs --str <string>      // Read input from string.\n");
  printf("  ecs --input <file>      // Read input from file.\n");
  printf("Options:\n");
  printf("  -s,--str <string>       // Read input from string.\n");
  printf("  -i,--input <file>       // Read input from file.\n");
  printf("  -o,--output <file>      // Specify output file name.\n");
  printf("  -h,--help               // Show this help.\n");
  printf("  -v,--version            // Show version.\n");
  return 0;
}

static int printVersion(){
  std::cout << config::ECS_COMIPLER_NAME <<" version " << config::ECS_COMIPLER_VERSION << std::endl;
  return 0;
}

static int printErrorArg(const char* arg){
  std::cout <<"Error: Unknown parameter [" << arg << "]" <<std::endl;
  return 1;
}

static std::string toLower(const char* str){
  std::string out;
  for(const char* p = str; *p; ++p){
    out.push_back(std::tolower(*p));
  }
  return out;
}

static std::string readStringFromFile(const std::string& inputFileName){
  std::ifstream ifs(inputFileName);
  if(!ifs.is_open()){
    std::cout << "Error: Cannot open input file [" << inputFileName << "]" << std::endl;
    return {};
  }
  std::string str;
  ifs >> str;
  if(str.empty()){
    std::cout << "Error: Input file [" << inputFileName << "] is empty." << std::endl;
  }
  return str;
};

static bool writeCodeFile(const std::string& fileName, const std::string& code){
  std::ofstream ofs(fileName);
  if(ofs.is_open()){
    ofs << code;
    ofs.close();
    return true;
  }else{
    std::cout << "Error: Cannot write file [" << fileName << "]" << std::endl;
  }
  return false;
}

int main(int argc, char* argv[]){
  std::string inputString;
  std::string inputFileName;
  std::string outputFileName;

  for(int i = 1; i < argc; ++i){
    auto const arg = toLower(argv[i]);
    if(arg == "-h" || arg == "--help" || arg == "/h" || arg == "/help" || arg == "/?" ){
      return printUsage();
    }else if(arg == "-v" || arg == "--version"){
      return printVersion();
    }else if(arg == "-s" || arg == "--str"){
      if(i + 1 < argc){
        inputString = argv[++i];
      }
    }else if(arg == "-i" || arg == "--input"){
      if(i + 1 < argc){
        inputFileName = argv[++i];
      }
    }else if(arg == "-o" || arg == "--output"){
      if(i + 1 < argc){
        outputFileName = argv[++i];
      }
    }else{
      return printErrorArg(argv[i]);
    }
  }

  if(inputFileName.empty() && inputString.empty()){
    return printUsage();
  }

  if(!inputFileName.empty() && inputFileName == outputFileName){
    printf("Error: Input file name cannot be same as output file name.\n");
    return 1;
  }

  if(inputString.empty()){
    inputString = readStringFromFile(inputFileName);
  }

  /**
   * Input string example:
   *   1, "struct Positon; class HealthPoint;"
   *   2, "struct A, struct B, struct C{struct D{}, struct E{}}"
   *   3, "struct Positon{int x; int y;}; class HealthPoint{int value;}"
   *   4, "#include<something> namespace ns{struct Positon{int x; int y;}; struct HealthPoint{int value; }, class fish;}"
   */
  if(!inputString.empty()){
    EcsCompiler compiler;
    auto code = compiler.compile(inputString);
    if(!code.empty()){
      if(!outputFileName.empty()){
        if(writeCodeFile(outputFileName,code)){
          return 0;
        }
      }else{
        std::cout << code << std::endl;
        return 0;
      }
    }
  }

  return 1;
}