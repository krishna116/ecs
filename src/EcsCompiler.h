#pragma once
#include <cstddef>
#include <string>
#include <vector>

class EcsCompiler {
public:
  /**
   * Compile C++ source code(has component declarations) to ecs code.
   * 
   * Input C++ source code example:
   *   1, "struct Positon; class HealthPoint;"
   *   2, "struct Positon{int x; int y;}; class HealthPoint{int value;}"
   *   3, "class Cat{}; class Dog{}; class Boss{class Cat; class Dog;}"
   *   4, "#include<something> namespace MyNameSpace{ struct Positon{int x; int y;}; struct HealthPoint{int value; }, class fish; }"
   * 
   * @param cppSourceCode     C++ source code.
   * @return string           ecs code.
   */
  std::string compile(const std::string &cppSourceCode);
};