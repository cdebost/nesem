#include "assembler/scanner.h"

#include "instruction_set.h"

namespace nesem {
namespace assembler {

static bool ishex(char c) {
  return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static int hextoi(char c) {
  if (c >= 'a') c -= 'A' - 'a';
  if (c >= 'A')
    c = 0xA + (c - 'A');
  else
    c -= '0';
  return c;
}

Scanner::Scanner(std::string input) : iss(input) {
  for (const auto &opcode : opcodes) {
    mnemonics.insert(opcode.mnemonic);
  }
  current_token = next_token();
}

Scanner::operator bool() const { return !eof; }

Token Scanner::peek() { return current_token; }

std::optional<Token> Scanner::accept(TokenType type) {
  auto token = peek();
  if (token.type == type) {
    current_token = next_token();
    return token;
  } else {
    return std::nullopt;
  }
}

std::optional<Token> Scanner::accept_any(const std::vector<TokenType> &types) {
  std::optional<Token> token = std::nullopt;
  for (auto type : types) {
    token = accept(type);
    if (token) break;
  }
  return token;
}

Token Scanner::next_token() {
  char c = iss.get();
  if (c == EOF) eof = true;
  if (eof || c == '\n') {
    return {TokenType::kEol};
  } else if (isspace(c)) {
    return next_token();
  } else if (isalpha(c)) {
    std::string ident = {c};
    while (iss) {
      c = iss.get();
      if (!isalnum(c)) {
        iss.unget();
        break;
      }
      ident.push_back(c);
    }
    if (mnemonics.contains(ident))
      return {TokenType::kMnemonic, ident};
    else
      return {TokenType::kIdent, ident};
  } else if (c == '#') {
    return {TokenType::kPound};
  } else if (c == '$') {
    uint16_t hex = 0;
    int n = 0;
    while (iss) {
      c = iss.get();
      if (ishex(c)) {
        hex *= 16;
        hex += hextoi(c);
        ++n;
      } else {
        iss.unget();
        break;
      }
    }
    if (n == 2) {
      return {TokenType::kHex, (uint8_t)hex};
    } else if (n == 4) {
      return {TokenType::kHex, hex};
    } else {
      return {TokenType::kIllegal};
    }
  } else if (c == ',') {
    c = iss.get();
    if (c == 'X')
      return {TokenType::kIndexX};
    else if (c == 'Y')
      return {TokenType::kIndexY};
    else
      return {TokenType::kIllegal};
  } else if (c == '(') {
    return {TokenType::kParenOpen};
  } else if (c == ')') {
    return {TokenType::kParenClose};
  } else {
    return {TokenType::kIllegal};
  }
}

}  // namespace assembler
}  // namespace nesem
