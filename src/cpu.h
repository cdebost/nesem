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

  Cpu(Mmu *mmu) : mmu(mmu) {}
  Cpu(const Cpu &c) = delete;
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
  Mmu *mmu;

  // Read the address of the operand, resolving addressing modes.
  // PC is expected to currently be on the operand.
  uint16_t get_operand_addr(const Opcode &mode);

  void stack_push(uint8_t val);
  uint8_t stack_pop();
  void stack_push16(uint16_t data);
  uint16_t stack_pop16();

  void update_zero_neg_flags(uint8_t data);

  // Fetch and execute the instruction under the current program counter
  void fetch_exec();

  void adc(const Opcode &opcode);
  void and_(const Opcode &opcode);
  void asl_a();
  void asl_mem(const Opcode &opcode);
  void bit(const Opcode &opcode);
  void branch_cond(uint8_t cond);
  void brk();
  void compare_with(const Opcode &opcode, uint8_t reg);
  void dec(const Opcode &opcode);
  void eor(const Opcode &opcode);
  void inc(const Opcode &opcode);
  void jmp(const Opcode &opcode);
  void jsr();
  void lda(const Opcode &opcode);
  void ldx(const Opcode &opcode);
  void ldy(const Opcode &opcode);
  void lsr_a();
  void lsr_mem(const Opcode &opcode);
  void ora(const Opcode &opcode);
  void sta(const Opcode &opcode);
  void stx(const Opcode &opcode);
  void sty(const Opcode &opcode);
  void transfer_a_to(uint8_t *reg);
  void tax();
  void txa();
  void dex();
  void inx();
  void tay();
  void tya();
  void dey();
  void iny();
  void rol_a();
  void rol_mem(const Opcode &opcode);
  void ror_a();
  void ror_mem(const Opcode &opcode);
  void rti();
  void rts();
  void sbc(const Opcode &opcode);
  void tsx();
  void txs();
  void pha();
  void pla();
  void php();
  void plp();
  void lax(const Opcode &opcode);
  void sax(const Opcode &opcode);
  void dcp(const Opcode &opcode);

  void handle_nmi();
  void handle_irq();
};

}  // namespace nesem
