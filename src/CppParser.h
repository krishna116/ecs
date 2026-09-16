#pragma once

#include <string>
#include <vector>

class CppParser {
public:
  using ClassNameTable = std::vector<std::string>;

  /**
   * Extracting component names from C++ source code.
   * 
   * @param cppSourceCode     C++ source code.
   * @return ClassNameTable   Component names.
   */
  ClassNameTable parse(const std::string &cppSourceCode);

  /**
   * Get parser last error.
   */
  std::string getLastError() const;

private:
  std::string mLastError;

  void clearLastError();
  void setLastError(std::string error);
  bool isValid(ClassNameTable& classNameTable);
};
