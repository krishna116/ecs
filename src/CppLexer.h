#pragma once

#include <string>
#include <vector>
#include <cstddef>

class CppLexer {
public:
  struct Token {
    enum class Type {
      Eof,
      LeftBrace,
      RightBrace,
      Namespace,
      Class,
      Identifier,
    };
    Type type;
    std::string name;
    Token(Type type_ = Type::Eof, std::string name_ = {}) : type(type_), name(name_) {}
  };
  using TokenArray = std::vector<Token>;

  /**
   * Get key and value token pairs from C++ source file.
   * The key and value token pair for example: "struct cat class dog".
   * 
   * @param source  C++ source file;
   * @param good    Used to get parser result.
   * 
   * @return TokenArray   All the key and value token pairs.
   */
  static TokenArray parse(const std::string& source, bool* good = nullptr);
};
