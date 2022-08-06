// Instruction set for the 6502 processor.
//
// The instruction set includes the following instructions:
//      ADC     ADd with carry
//      AND     Bitwise AND with accumulator
//      ASL     Arithmetic Shift Left
//      BIT     test BITs
//      BPL     Branch on PLus
//      BMI     Branch on MInus
//      BVC     Branch on oVerflow Clear
//      BVS     Branch on oVerflow Set
//      BCC     Branch on Carry Clear
//      BCS     Branch on Carry Set
//      BNE     Branch on Not Equal
//      BEQ     Branch on EQual
//      BRK     BReaK
//      CMP     CoMPare accumulator
//      CPX     ComPare X register
//      CPY     ComPare Y register
//      DEC     DECrement memory
//      EOR     bitwise Exclusive OR
//      CLC     CLear Carry
//      SEC     SEt Carry
//      CLI     CLear Interrupt
//      SEI     SEt Interrupt
//      CLV     CLear oVerflow
//      CLD     CLear Decimal
//      SED     SEt Decimal
//      INC     INCrement memory
//      JMP     JuMP
//      JSR     Jump to SubRoutine
//      LDA     LoaD Accumulator
//      LDX     LoaD X register
//      LDY     LoaD Y register
//      LSR     Logical Shift Right
//      NOP     NO oPeration
//      ORA     bitwise OR with Accumulator
//      TAX     Transfer A to X
//      TXA     Transfer X to A
//      DEX     DEcrement X
//      INX     INcrement X
//      TAY     Transfer X to Y
//      TYA     Transfer Y to A
//      DEY     DEcrement Y
//      INY     INcrement Y
//      ROL     ROtate Left
//      ROR     ROtate Right
//      RTI     ReTurn from Interrupt
//      RTS     ReTurn from Subroutine
//      SBC     SuBtrace with Carry
//      STA     STore Accumulator
//      TXS     Transfer X to Stack ptr
//      TSX     Transfer Stack ptr to X
//      PHA     PusH Accumulator
//      PLA     PuLl Accumulator
//      PHP     PusH Processor status on stack
//      PLP     PuLl Processor status
//      STX     STore X register
//      STY     STore Y register
//
// Each instruction may have multiple opcodes which corresponds to different
// addressing modes of the same logical instruction.
//
// Full instruction reference: http://www.6502.org/tutorials/6502opcodes.html

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nesem {

// The 6502 processor uses a 16-bit address bus, giving 65536 bytes of available
// memory. Each memory location is represented as two bytes.
//
// There are various ways to refer to these memory locations, which determines
// how many operands each opcode uses. Each logical operation therefore has
// multiple opcode variants that perform the same logic, but with a different
// addressing mode.
enum class AddressingMode {
  // No operand.
  Implied,

  // No memory address; the operand is an actual value.
  Immediate,

  // Take a single-byte address. This means the op only takes one operand, but
  // it also means only the first page (256 bytes) is addressable.
  Zeropage,

  // A zero-page address is given and added with the X register.
  ZeropageX,

  // A zero-page address is given and added with the Y register.
  ZeropageY,

  // The full memory location is given, which means the op takes two operands.
  Absolute,

  // The absolute addressing version of zero-page-x.
  AbsoluteX,

  // The absolute addressing version of zero-page-y.
  AbsoluteY,

  // The operand is a (signed) offset from the current PC.
  Relative,

  // The operand is a memory address that will be dereferenced to get the value.
  Indirect,

  // Take the zero-page address, add the X register, and use that to look up an
  // absolute address.
  IndirectX,

  // Take the zero-page address, use that to look up an absolute address, and
  // add the Y register.
  IndirectY
};

enum OpcodeFlag {
  kIllegalOpcode = 1,
  kUnstableOpcode = 2,
  kHighlyUnstableOpcode = 4,
  kAddCycleIfPageBoundaryCrossed = 8
};

struct Opcode {
  uint8_t code;                // actual machine code
  const std::string mnemonic;  // human-readable name
  AddressingMode mode;
  uint8_t len;     // number of operands + 1
  uint8_t cycles;  // number of cycles to execute
  uint8_t flags;   // illegal, unstable, highly unstable

  inline bool is_illegal() const { return flags & kIllegalOpcode; }

  inline bool does_add_cycle_if_page_boundary_crossed() const {
    return flags & kAddCycleIfPageBoundaryCrossed;
  }
};

extern const std::vector<Opcode> opcodes;

}  // namespace nesem
