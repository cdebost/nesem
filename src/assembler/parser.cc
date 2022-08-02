#include "assembler/parser.h"

namespace nesem {
namespace assembler {

// Grammar:
//
// program = "eol"
//         | instruction program
// instruction = [label] mnemonic [ operand ]
// operand = "#" value
//         | value [ index ]
//         | "(" value ")"
//         | "(" value index_x ")"
//         | "(" value ")" index_y
// value = hex | ident
// index = index_x | index_y

class Parser {
 public:
  explicit Parser(Scanner *scanner) : scanner(scanner) {}

  Program parse();

 private:
  Scanner *scanner;

  Instruction instruction();
  Operand operand();

  Token expect(TokenType type);
  Token expect_any(const std::vector<TokenType> &types);
};

Program parse(Scanner *scanner) { return Parser{scanner}.parse(); }

Program Parser::parse() {
  std::vector<Instruction> instructions;
  while (*scanner) {
    if (scanner->accept(TokenType::kEol)) {
      // skip consecutive newlines
      continue;
    }
    instructions.push_back(instruction());
  }
  return {instructions};
}

Instruction Parser::instruction() {
  std::optional<std::string> label = std::nullopt;
  if (auto token = scanner->accept(TokenType::kLabel))
    label = std::get<std::string>(token->val);
  auto token = expect(TokenType::kMnemonic);
  auto mnemonic = std::get<std::string>(token.val);
  if (scanner->accept(TokenType::kEol)) {
    return {label, mnemonic};
  } else {
    auto operand = this->operand();
    expect(TokenType::kEol);
    return {label, mnemonic, operand};
  }
}

Operand Parser::operand() {
  if (scanner->accept(TokenType::kPound)) {
    auto token = expect(TokenType::kHex);
    return {OperandType::kImmediate, token.val};
  }

  else if (scanner->accept(TokenType::kParenOpen)) {
    auto token = expect(TokenType::kHex);
    if (scanner->accept(TokenType::kIndexX)) {
      expect(TokenType::kParenClose);
      return {OperandType::kIndirectX, token.val};
    } else {
      expect(TokenType::kParenClose);
      if (scanner->accept(TokenType::kIndexY))
        return {OperandType::kIndirectY, token.val};
      return {OperandType::kIndirect, token.val};
    }
  }

  else if (auto token = scanner->accept(TokenType::kHex)) {
    if (scanner->accept(TokenType::kIndexX))
      return {OperandType::kDirectX, token->val};
    else if (scanner->accept(TokenType::kIndexY))
      return {OperandType::kDirectY, token->val};
    else
      return {OperandType::kDirect, token->val};
  }

  else if (auto token = scanner->accept(TokenType::kIdent)) {
    return {OperandType::kDirect, token->val};
  }

  else
    throw ParseError(
        "Expected an immediate operand (#), indirect operand ('('), direct "
        "literal operand ($), or identifier");
}

Token Parser::expect(TokenType type) {
  if (auto token = scanner->accept(type))
    return *token;
  else
    throw ParseError{"Expected " + token_type_str(type)};
}

Token Parser::expect_any(const std::vector<TokenType> &types) {
  assert(types.size() > 0);
  if (auto token = scanner->accept_any(types)) {
    return *token;
  } else {
    std::string message = "Expected " + token_type_str(types[0]);
    for (int i = 1; i < types.size(); ++i) {
      message += ", ";
      message += token_type_str(types[i]);
    }
    throw ParseError{message};
  }
}

}  // namespace assembler
}  // namespace nesem
