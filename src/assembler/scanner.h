#pragma once

#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

namespace nesem {
namespace assembler {

enum class TokenType {
  kEol,
  kMnemonic,
  kIdent,
  kLabel,
  kPound,
  kHex,
  kIndexX,
  kIndexY,
  kParenOpen,
  kParenClose,
  kIllegal
};

inline std::string token_type_str(TokenType type) {
  switch (type) {
    case TokenType::kEol:
      return "EOL";
    case TokenType::kMnemonic:
      return "mnemonic";
    case TokenType::kIdent:
      return "identifier";
    case TokenType::kLabel:
      return "label";
    case TokenType::kPound:
      return "#";
    case TokenType::kHex:
      return "hexadecimal value";
    case TokenType::kIndexX:
      return ",X";
    case TokenType::kIndexY:
      return ",Y";
    case TokenType::kParenOpen:
      return "(";
    case TokenType::kParenClose:
      return ")";
    case TokenType::kIllegal:
      return "illegal token";
  }
}

struct Token {
  TokenType type;
  std::variant<uint8_t, uint16_t, std::string> val;
};

// Tokenizes raw input
class Scanner {
 public:
  explicit Scanner(const std::string &input);

  // Return true if there is more input to scan
  operator bool() const;

  // Return the current token
  Token peek();

  // Return the current token if its type matches the specified type.
  // If it does, the current token is also consumed.
  std::optional<Token> accept(TokenType type);

  // Return the current token if its type matches any of the specified types.
  // If it does, the current token is also consumed.
  std::optional<Token> accept_any(const std::vector<TokenType> &types);

 private:
  std::istringstream iss;
  std::unordered_set<std::string> mnemonics;
  Token current_token;
  bool eof = false;

  Token next_token();
};

}  // namespace assembler
}  // namespace nesem
