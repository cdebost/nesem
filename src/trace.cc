#include "trace.h"

#include <fmt/core.h>

#include "cpu.h"

namespace nesem {

// ex: 20 76 F9
static std::string trace_opcode_operands(const Cpu &cpu) {
  assert(opcodes.size() == 0x100);
  const Opcode &info = opcodes[cpu.read(cpu.pc)];
  switch (info.mode) {
    case AddressingMode::Implied:
      return fmt::format("{:02X}", cpu.read(cpu.pc));
    case AddressingMode::Immediate:
    case AddressingMode::Relative:
    case AddressingMode::Zeropage:
    case AddressingMode::ZeropageX:
    case AddressingMode::ZeropageY:
    case AddressingMode::IndirectX:
    case AddressingMode::IndirectY:
      return fmt::format("{:02X} {:02X}", cpu.read(cpu.pc),
                         cpu.read(cpu.pc + 1));
    case AddressingMode::Indirect:
    case AddressingMode::Absolute:
    case AddressingMode::AbsoluteX:
    case AddressingMode::AbsoluteY:
      return fmt::format("{:02X} {:02X} {:02X}", cpu.read(cpu.pc),
                         cpu.read(cpu.pc + 1), cpu.read(cpu.pc + 2));
  }
}

// ex:  LDA ($89),Y = 0300 @ 0300 = 89
// max len: 32
static std::string trace_assembly(const Cpu &cpu) {
  std::string str{};

  assert(opcodes.size() == 0x100);
  const Opcode &info = opcodes[cpu.read(cpu.pc)];

  str = info.is_illegal() ? fmt::format("*{} ", info.mnemonic)
                          : fmt::format(" {} ", info.mnemonic);

  if (info.mode == AddressingMode::Implied) {
    switch (info.code) {
      case 0x0a:
      case 0x4a:
      case 0x2a:
      case 0x6a:
        str += "A";
    }
    return str;
  }

  if (info.mode == AddressingMode::Immediate) {
    str += fmt::format("#${:02X}", cpu.read(cpu.pc + 1));
    return str;
  }

  uint16_t addr;
  uint16_t absaddr;
  uint16_t value;

  assert(info.len > 1);
  if (info.len == 2) {
    uint8_t operand = cpu.read(cpu.pc + 1);

    switch (info.mode) {
      case AddressingMode::Zeropage: {
        uint8_t value = cpu.read(operand);
        str += fmt::format("${:02X} = {:02X}", operand, value);
        break;
      }
      case AddressingMode::ZeropageX: {
        uint8_t addr = operand + cpu.x;
        uint8_t value = cpu.read(addr);
        str += fmt::format("${:02X},X @ {:02X} = {:02X}", operand, addr, value);
        break;
      }
      case AddressingMode::ZeropageY: {
        uint8_t addr = operand + cpu.y;
        uint8_t value = cpu.read(addr);
        str += fmt::format("${:02X},Y @ {:02X} = {:02X}", operand, addr, value);
        break;
      }
      case AddressingMode::Relative: {
        // Add 2 to account for the length of the instruction
        uint16_t addr = cpu.pc + (int8_t)operand + 2;
        str += fmt::format("${:04X}", addr);
        break;
      }
      case AddressingMode::IndirectX: {
        uint8_t ref = operand + cpu.x;
        uint8_t addr_lo = cpu.read(ref);
        uint16_t addr_hi = cpu.read((uint8_t)(ref + 1));
        uint16_t addr = (addr_hi << 8) | addr_lo;
        uint8_t value = cpu.read(addr);
        str += fmt::format("(${:02X},X) @ {:02X} = {:04X} = {:02X}", operand,
                           ref, addr, value);
        break;
      }
      case AddressingMode::IndirectY: {
        uint8_t ref_lo = cpu.read(operand);
        uint16_t ref_hi = cpu.read((uint8_t)(operand + 1));
        uint16_t ref = (ref_hi << 8) | ref_lo;
        uint16_t addr = ref + cpu.y;
        uint8_t value = cpu.read(addr);
        str += fmt::format("(${:02X}),Y = {:04X} @ {:04X} = {:02X}", operand,
                           ref, addr, value);
        break;
      }
      default:
        assert(!"should never reach here");
    }

  } else {
    uint16_t operand = cpu.read16(cpu.pc + 1);

    switch (info.mode) {
      case AddressingMode::Indirect: {
        uint8_t addr_lo = cpu.read(operand);
        uint16_t addr_hi;
        if ((operand & 0xFF) == 0xFF)
          addr_hi = cpu.read(operand & 0xFF00);
        else
          addr_hi = cpu.read(operand + 1);
        uint16_t addr = (addr_hi << 8) | addr_lo;
        str += fmt::format("(${:04X}) = {:04X}", operand, addr);
        break;
      }
      case AddressingMode::Absolute: {
        // special case for absolute jump instructions (JMP, JSR)
        if (info.code == 0x4C || info.code == 0x20) {
          str += fmt::format("${:04X}", operand);
        } else {
          uint8_t value = cpu.read(operand);
          str += fmt::format("${:04X} = {:02X}", operand, value);
        }
        break;
      }
      case AddressingMode::AbsoluteX: {
        uint16_t addr = operand + cpu.x;
        uint8_t value = cpu.read(addr);
        str += fmt::format("${:04X},X @ {:04X} = {:02X}", operand, addr, value);
        break;
      }
      case AddressingMode::AbsoluteY: {
        uint16_t addr = operand + cpu.y;
        uint8_t value = cpu.read(addr);
        str += fmt::format("${:04X},Y @ {:04X} = {:02X}", operand, addr, value);
        break;
      }
      default:
        assert(!"should never reach here");
    }
  }

  return str;
}

std::string trace_explain_state(const Cpu &cpu) {
  std::string opcode_operands = trace_opcode_operands(cpu);
  std::string assembly = trace_assembly(cpu);
  return fmt::format(
      "{:04X}  {:<8s} {:<32s} A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X} "
      "CYC:{}",
      cpu.pc, opcode_operands, assembly, cpu.a, cpu.x, cpu.y, cpu.flags.bits(),
      cpu.sp, cpu.cycles);
}

}  // namespace nesem
