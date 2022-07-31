#include "assembler/parser.h"

namespace nesem {
namespace assembler {

// Grammar:
//
// program = "eol"
//         | instruction program
// instruction = mnemonic [ operand ]
// operand = "#" value
//         | value [ index ]
//         | "(" value ")"
//         | "(" value index_x ")"
//         | "(" value ")" index_y
// value = hex | ident
// index = index_x | index_y

class Parser {
 public:
  Parser(Scanner *scanner) : scanner(scanner) {}

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
  auto token = expect(TokenType::kMnemonic);
  auto mnemonic = std::get<std::string>(token.val);
  if (scanner->accept(TokenType::kEol)) {
    return {mnemonic};
  } else {
    auto operand = this->operand();
    expect(TokenType::kEol);
    return {mnemonic, operand};
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

  else {
    auto token = expect(TokenType::kHex);
    if (scanner->accept(TokenType::kIndexX))
      return {OperandType::kDirectX, token.val};
    else if (scanner->accept(TokenType::kIndexY))
      return {OperandType::kDirectY, token.val};
    else
      return {OperandType::kDirect, token.val};
  }
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
