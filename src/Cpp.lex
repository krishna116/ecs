// This is a modified cpp lex version.
// The source is here: https://github.com/skvadrik/re2c/blob/master/examples/c/real_world/cxx98.re
// The source license: re2c is in the public domain.
// re2c <input> -o CppLexer.cpp

#include <cfloat>
#include <climits>
#include <iostream>

#include "CppLexer.h"

using Token = CppLexer::Token;
using TokenArray = CppLexer::TokenArray;

struct LexContext {
  const char *curser;
  const char *limit;
  const char *marker;
  const char *tok;
  LexContext(const char *str, size_t size) : curser(str), limit(str + size) {}
  TokenArray tokenArray;
};

/*!re2c re2c:YYCTYPE = "char"; */

template <int base> static bool adddgt(unsigned long &u, unsigned long d) {
  if (u > (ULONG_MAX - d) / base) {
    return false;
  }
  u = u * base + d;
  return true;
}

static bool lex_oct(const char *s, const char *e, unsigned long &u) {
  for (u = 0, ++s; s < e; ++s) {
    if (!adddgt<8>(u, *s - 0x30u)) {
      return false;
    }
  }
  return true;
}

static bool lex_dec(const char *s, const char *e, unsigned long &u) {
  for (u = 0; s < e; ++s) {
    if (!adddgt<10>(u, *s - 0x30u)) {
      return false;
    }
  }
  return true;
}

static bool lex_hex(const char *s, const char *e, unsigned long &u) {
  for (u = 0, s += 2; s < e;) {
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:YYCURSOR = s;
    *     { if (!adddgt<16>(u, s[-1] - 0x30u))      return false; continue; } 
    [a-f] { if (!adddgt<16>(u, s[-1] - 0x61u + 10)) return false; continue; } 
    [A-F] { if (!adddgt<16>(u, s[-1] - 0x41u + 10)) return false; continue; }
    */
  }
  return true;
}

static bool lex_str(LexContext &ctx, char q){
  for (unsigned long u = q;;) {
    ctx.tok = ctx.curser;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:YYCURSOR = ctx.curser;
    re2c:YYMARKER = ctx.marker;
    re2c:YYLIMIT = ctx.limit;
    *                    { return false; }
    [^\n\\]              { u = ctx.tok[0]; if (u == q) break; continue; }
    "\\a"                { u = '\a'; continue; }
    "\\b"                { u = '\b'; continue; }
    "\\f"                { u = '\f'; continue; }
    "\\n"                { u = '\n'; continue; }
    "\\r"                { u = '\r'; continue; }
    "\\t"                { u = '\t'; continue; }
    "\\v"                { u = '\v'; continue; }
    "\\\\"               { u = '\\'; continue; }
    "\\'"                { u = '\''; continue; }
    "\\\""               { u = '"';  continue; }
    "\\?"                { u = '?';  continue; }
    "\\" [0-7]{1,3}      { lex_oct(ctx.tok, ctx.curser, u); continue; }
    "\\u" [0-9a-fA-F]{4} { lex_hex(ctx.tok, ctx.curser, u); continue; }
    "\\U" [0-9a-fA-F]{8} { lex_hex(ctx.tok, ctx.curser, u); continue; }
    "\\x" [0-9a-fA-F]+   { if (!lex_hex(ctx.tok, ctx.curser, u)) return false; continue; }
    */
  }
  return true;
}

static bool lex_float_number(const char *s){
  double d = 0;
  double x = 1;
  int e = 0;
  /*!re2c
  re2c:yyfill:enable = 0;
  re2c:YYCURSOR = s;
  */

mant_int:
  /*!re2c
  "."   { goto mant_frac; }
  [eE]  { goto exp_sign; }
  *     { d = (d * 10) + (s[-1] - '0'); goto mant_int; }
  */
mant_frac:
  /*!re2c
  ""    { goto sfx; }
  [eE]  { goto exp_sign; }
  [0-9] { d += (x /= 10) * (s[-1] - '0'); goto mant_frac; }
  */
exp_sign:
  /*!re2c
  "+"?  { x = 1e+1; goto exp; }
  "-"   { x = 1e-1; goto exp; }
  */
exp:
  /*!re2c
  ""    { for (; e > 0; --e) d *= x;    goto sfx; }
  [0-9] { e = (e * 10) + (s[-1] - '0'); goto exp; }
  */
sfx:
  /*!re2c
  *     { goto end; }
  [fF]  { if (d > FLT_MAX) return false; goto end; }
  */
end:
  return true;
}

static bool lex(LexContext &ctx){
  unsigned long u;
  for (;;) {
    ctx.tok = ctx.curser;
    /*!re2c
    re2c:yyfill:enable = 0;
    re2c:YYCURSOR = ctx.curser;
    re2c:YYMARKER = ctx.marker;
    re2c:YYLIMIT = ctx.limit;
    re2c:sentinel = 0;

    *      { ctx.tokenArray.emplace_back(Token{Token::Type::Eof, "Failed"}); return 0; }
    [\x00] { ctx.tokenArray.emplace_back(Token{Token::Type::Eof, "Success"}); return 1; }

    // macros
    macro = ("#" | "%:") ([^\n\x00] | "\\\n")* "\n";
    macro { continue; }

    // whitespaces
    mcm = "/*" ([^*\x00] | ("*" [^/\x00]))* "*""/";
    scm = "//" [^\n\x00]* "\n";
    wsp = ([ \t\v\n\r] | scm | mcm)+;
    wsp { continue; }

    // character and string literals
    "L"? ['"] { if (!lex_str(ctx, ctx.curser[-1])) return false; continue; }
    "L"? "''" { return false; }

    // integer literals
    oct = "0" [0-7]*;
    dec = [1-9][0-9]*;
    hex = '0x' [0-9a-fA-F]+;
    oct { if (!lex_oct(ctx.tok, ctx.curser, u)) return false; goto sfx; }
    dec { if (!lex_dec(ctx.tok, ctx.curser, u)) return false; goto sfx; }
    hex { if (!lex_hex(ctx.tok, ctx.curser, u)) return false; goto sfx; }

    // floating literals
    frc = [0-9]* "." [0-9]+ | [0-9]+ ".";
    exp = 'e' [+-]? [0-9]+;
    flt = (frc exp? | [0-9]+ exp) [fFlL]?;
    flt { if (lex_float_number(ctx.tok)) continue; return false; }

    // boolean literals
    "false" { continue; }
    "true"  { continue; }

    // keywords
    "asm"              { continue; }
    "auto"             { continue; }
    "bool"             { continue; }
    "break"            { continue; }
    "case"             { continue; }
    "catch"            { continue; }
    "char"             { continue; }
    "class"            { ctx.tokenArray.emplace_back(Token{Token::Type::Class, "class"}); continue; }
    "const"            { continue; }
    "const_cast"       { continue; }
    "continue"         { continue; }
    "default"          { continue; }
    "do"               { continue; }
    "double"           { continue; }
    "dynamic_cast"     { continue; }
    "else"             { continue; }
    "enum"             { continue; }
    "explicit"         { continue; }
    "export"           { continue; }
    "extern"           { continue; }
    "float"            { continue; }
    "for"              { continue; }
    "friend"           { continue; }
    "goto"             { continue; }
    "if"               { continue; }
    "inline"           { continue; }
    "int"              { continue; }
    "long"             { continue; }
    "mutable"          { continue; }
    "namespace"        { ctx.tokenArray.emplace_back(Token{Token::Type::Namespace, "struct"}); continue; }
    "operator"         { continue; }
    "private"          { continue; }
    "protected"        { continue; }
    "public"           { continue; }
    "register"         { continue; }
    "reinterpret_cast" { continue; }
    "return"           { continue; }
    "short"            { continue; }
    "signed"           { continue; }
    "sizeof"           { continue; }
    "static"           { continue; }
    "static_cast"      { continue; }
    "struct"           { ctx.tokenArray.emplace_back(Token{Token::Type::Class, "struct"}); continue; }
    "switch"           { continue; }
    "template"         { continue; }
    "this"             { continue; }
    "throw"            { continue; }
    "try"              { continue; }
    "typedef"          { continue; }
    "typeid"           { continue; }
    "typename"         { continue; }
    "union"            { continue; }
    "unsigned"         { continue; }
    "using"            { continue; }
    "virtual"          { continue; }
    "void"             { continue; }
    "volatile"         { continue; }
    "wchar_t"          { continue; }
    "while"            { continue; }

    // operators and punctuation (including preprocessor)
    ("{" | "<%")      { ctx.tokenArray.emplace_back(Token{Token::Type::LeftBrace, "{"}); continue; }
    ("}" | "%>")      { ctx.tokenArray.emplace_back(Token{Token::Type::RightBrace, "}"}); continue; }
    ("[" | "<:")      { continue; }
    ("]" | ":>")      { continue; }
    "("               { continue; }
    ")"               { continue; }
    ";"               { continue; }
    ":"               { continue; }
    "..."             { continue; }
    "new"             { continue; }
    "delete"          { continue; }
    "?"               { continue; }
    "::"              { continue; }
    "."               { continue; }
    ".*"              { continue; }
    "+"               { continue; }
    "-"               { continue; }
    "*"               { continue; }
    "/"               { continue; }
    "%"               { continue; }
    ("^" | "xor")     { continue; }
    ("&" | "bitand")  { continue; }
    ("|" | "bitor")   { continue; }
    ("~" | "compl")   { continue; }
    ("!" | "not")     { continue; }
    "="               { continue; }
    "<"               { continue; }
    ">"               { continue; }
    "+="              { continue; }
    "-="              { continue; }
    "*="              { continue; }
    "/="              { continue; }
    "%="              { continue; }
    ("^=" | "xor_eq") { continue; }
    ("&=" | "and_eq") { continue; }
    ("|=" | "or_eq")  { continue; }
    "<<"              { continue; }
    ">>"              { continue; }
    ">>="             { continue; }
    "<<="             { continue; }
    "=="              { continue; }
    ("!=" | "not_eq") { continue; }
    "<="              { continue; }
    ">="              { continue; }
    ("&&" | "and")    { continue; }
    ("||" | "or")     { continue; }
    "++"              { continue; }
    "--"              { continue; }
    ","               { continue; }
    "->*"             { continue; }
    "->"              { continue; }

    // identifiers
    id = [a-zA-Z_][a-zA-Z_0-9]*;
    id { ctx.tokenArray.emplace_back(Token{Token::Type::Identifier, std::string(ctx.tok, ctx.curser - ctx.tok)}); continue; }
*/
sfx:
  /*!re2c
    ""          { if (u > INT_MAX)  return false; fprintf(stderr, "%d",  static_cast<int>(u));      continue; }
    'u'         { if (u > UINT_MAX) return false; fprintf(stderr, "%u",  static_cast<unsigned>(u)); continue; }
    'l'         { if (u > LONG_MAX) return false; fprintf(stderr, "%ld", static_cast<long>(u));     continue; }
    'ul' | 'lu' { fprintf(stderr, "%lu", u); continue; }
  */
  }
}

TokenArray CppLexer::parse(const std::string& source, bool* good) {
  LexContext ctx(source.data(), source.size());

  auto result = lex(ctx);
  if(good) *good = result;

  return ctx.tokenArray;
}
