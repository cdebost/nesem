#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

#include "assembler/scanner.h"

namespace nesem {
namespace assembler {

enum class OperandType {
  kImmediate,
  kDirect,
  kDirectX,
  kDirectY,
  kIndirect,
  kIndirectX,
  kIndirectY
};

struct Operand {
  OperandType type;
  std::variant<uint8_t, uint16_t, std::string> val;
};

struct Instruction {
  std::optional<std::string> label;
  std::string mnemonic;
  std::optional<Operand> operand;
};

struct Program {
  std::vector<Instruction> instructions;
};

class ParseError : public std::exception {
 public:
  explicit ParseError(const std::string &message) : message(message) {}

  const char *what() const noexcept(true) override { return message.c_str(); }

 private:
  std::string message;
};

Program parse(Scanner *scanner);

}  // namespace assembler
}  // namespace nesem
