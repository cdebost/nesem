#include "assembler/assembler.h"

#include "assembler/parser.h"
#include "assembler/scanner.h"
#include "instruction_set.h"

namespace nesem {
namespace assembler {

const Opcode& lookup_opcode(const Instruction& instruction) {
  bool found_mnemonic;
  for (const auto& opcode : opcodes) {
    if (opcode.mnemonic == instruction.mnemonic) {
      found_mnemonic = true;
      if (auto& operand = instruction.operand) {
        if (std::holds_alternative<uint8_t>(operand->val)) {
          if (operand->type == OperandType::kImmediate &&
              opcode.mode == AddressingMode::Immediate)
            return opcode;
          else if (operand->type == OperandType::kDirect &&
                   opcode.mode == AddressingMode::Zeropage)
            return opcode;
          else if (operand->type == OperandType::kDirect &&
                   opcode.mode == AddressingMode::Relative)
            return opcode;
          else if (operand->type == OperandType::kDirectX &&
                   opcode.mode == AddressingMode::ZeropageX)
            return opcode;
          else if (operand->type == OperandType::kDirectY &&
                   opcode.mode == AddressingMode::ZeropageY)
            return opcode;
          else if (operand->type == OperandType::kIndirectX &&
                   opcode.mode == AddressingMode::IndirectX)
            return opcode;
          else if (operand->type == OperandType::kIndirectY &&
                   opcode.mode == AddressingMode::IndirectY)
            return opcode;
        } else {  // uint16_t
          if (operand->type == OperandType::kDirect &&
              opcode.mode == AddressingMode::Absolute)
            return opcode;
          else if (operand->type == OperandType::kDirectX &&
                   opcode.mode == AddressingMode::AbsoluteX)
            return opcode;
          else if (operand->type == OperandType::kDirectY &&
                   opcode.mode == AddressingMode::AbsoluteY)
            return opcode;
          else if (operand->type == OperandType::kIndirect &&
                   opcode.mode == AddressingMode::Indirect)
            return opcode;
        }
      } else if (opcode.mode == AddressingMode::Implied) {
        return opcode;
      }
    }
  }
  if (found_mnemonic)
    throw ParseError("Invalid mode for instruction");
  else
    throw ParseError("Invalid mnemonic " + instruction.mnemonic);
}

std::vector<uint8_t> assemble(std::string input) {
  std::vector<uint8_t> prg_code;
  Scanner s{input};
  Program program = parse(&s);
  for (const auto& instruction : program.instructions) {
    const Opcode& opcode = lookup_opcode(instruction);
    prg_code.push_back(opcode.code);
    if (opcode.len == 2) {
      prg_code.push_back(std::get<uint8_t>(instruction.operand->val));
    } else if (opcode.len == 3) {
      uint16_t val = std::get<uint16_t>(instruction.operand->val);
      prg_code.push_back(val & 0xFF);
      prg_code.push_back(val >> 8);
    }
  }
  return prg_code;
}

}  // namespace assembler
}  // namespace nesem
