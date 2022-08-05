// Modified version of the 6502 processor
// https://en.wikipedia.org/wiki/MOS_Technology_6502

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <array>

#include "instruction_set.h"
#include "mmu.h"

namespace nesem {

struct CpuFlags {
  bool negative = false;
  bool overflow = false;
  bool brk = false;
  bool decimal = false;
  bool interrupt_disable = true;
  bool zero = false;
  bool carry = false;

  // The status register (aka processor flags) is laid out as follows:
  // NV-B DIZC
  // |||| ||||
  // |||| |||+- carry
  // |||| ||+-- zero
  // |||| |+--- interrupt disable
  // |||| +---- decimal
  // ||||
  // |||+------ break (not a "real" register, only observable on the stack)
  // ||+------- reserved (always on)
  // |+-------- overflow
  // +--------- negative
  uint8_t bits() const {
    return (negative << 7) | (overflow << 6) | (1 << 5) | (brk << 4) |
           (decimal << 3) | (interrupt_disable << 2) | (zero << 1) |
           (carry << 0);
  }
};

struct Cpu {
  uint8_t a = 0;      // accumulator
  uint8_t x = 0;      // index register
  uint8_t y = 0;      // index register
  uint8_t sp = 0xFD;  // stack pointer

  union {
    uint16_t pc;  // program counter
    struct {
      uint8_t pch;  // program counter (high 8 bits)
      uint8_t pcl;  // program counter (low 8 bits)
    };
  };

  CpuFlags flags;  // processor flags

  /* interrupts */
  bool irq_pending = false;
  bool nmi_pending = false;

  /* interrupt vectors */
  static constexpr uint16_t kNmiVector = 0xFFFA;
  static constexpr uint16_t kResetVector = 0xFFFC;
  static constexpr uint16_t kIrqVector = 0xFFFE;

  size_t cycles = 0;

  Cpu(Mmu* mmu) : mmu(mmu) {}
  Cpu(const Cpu& c) = delete;
  ~Cpu() {}

  // Read a single byte at the specified address
  uint8_t read(uint16_t addr) const;
  // Write a single byte at the specified address
  void write(uint16_t addr, uint8_t data);

  // Read two bytes in little-endian order at the specified address
  uint16_t read16(uint16_t addr) const;
  // Write two bytes in little-endian order at the specified address
  void write16(uint16_t addr, uint16_t data);

  // Handle a pending interrupt if applicable, or execute the next instruction
  void step();

  // Handle the reset signal
  void reset();

 private:
  Mmu* mmu;

  // Convert the given address into an absolute address. The behavior differs
  // based on the specified addressing mode.
  uint16_t abs_addr(AddressingMode mode, uint16_t addr) const;

  uint16_t get_operand_addr(AddressingMode mode);

  void stack_push(uint8_t val);
  uint8_t stack_pop();
  void stack_push16(uint16_t data);
  uint16_t stack_pop16();

  void update_zero_neg_flags(uint8_t data);

  // Fetch and execute the instruction under the current program counter
  void fetch_exec();

  void adc(AddressingMode mode);
  void and_(AddressingMode mode);
  void asl_a();
  void asl_mem(AddressingMode mode);
  void bit(AddressingMode mode);
  void branch_cond(uint8_t cond);
  void brk();
  void compare_with(AddressingMode mode, uint8_t reg);
  void dec(AddressingMode mode);
  void eor(AddressingMode mode);
  void inc(AddressingMode mode);
  void jmp(AddressingMode mode);
  void jsr();
  void lda(AddressingMode mode);
  void ldx(AddressingMode mode);
  void ldy(AddressingMode mode);
  void lsr_a();
  void lsr_mem(AddressingMode mode);
  void ora(AddressingMode mode);
  void sta(AddressingMode mode);
  void stx(AddressingMode mode);
  void sty(AddressingMode mode);
  void transfer_a_to(uint8_t* reg);
  void tax();
  void txa();
  void dex();
  void inx();
  void tay();
  void tya();
  void dey();
  void iny();
  void rol_a();
  void rol_mem(AddressingMode mode);
  void ror_a();
  void ror_mem(AddressingMode mode);
  void rti();
  void rts();
  void sbc(AddressingMode mode);
  void tsx();
  void txs();
  void pha();
  void pla();
  void php();
  void plp();
  void lax(AddressingMode mode);
  void sax(AddressingMode mode);
  void dcp(AddressingMode mode);

  void handle_nmi();
  void handle_irq();
};

}  // namespace nesem
