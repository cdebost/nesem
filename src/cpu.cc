#include "cpu.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace nesem {

void Cpu::step() {
  if (nmi_pending) {
    nmi_pending = false;
    handle_nmi();
  } else if (irq_pending && !flags.interrupt_disable) {
    irq_pending = false;
    handle_irq();
  } else {
    fetch_exec();
  }
}

void Cpu::reset() {
  sp = 0xFD;
  flags.interrupt_disable = true;
  pc = read16(kResetVector);
  cycles += 7;
}

uint8_t Cpu::read(uint16_t addr) const { return mmu->read(addr); }

void Cpu::write(uint16_t addr, uint8_t data) { mmu->write(addr, data); }

uint16_t Cpu::read16(uint16_t addr) const {
  uint16_t lo = read(addr);
  uint16_t hi = read(addr + 1);
  return (hi << 8) | lo;
}

void Cpu::write16(uint16_t addr, uint16_t data) {
  uint8_t lo = data & 0xFF;
  uint8_t hi = data >> 8;
  write(addr, lo);
  write(addr + 1, hi);
}

uint16_t Cpu::get_operand_addr(const Opcode &opcode) {
  switch (opcode.mode) {
    case AddressingMode::Immediate:
      return pc;
    case AddressingMode::Zeropage:
      return read(pc);
    case AddressingMode::ZeropageX: {
      uint8_t addr = read(pc) + x;
      return addr;
    }
    case AddressingMode::ZeropageY: {
      uint8_t addr = read(pc) + y;
      return addr;
    }
    case AddressingMode::Absolute:
      return read16(pc);
    case AddressingMode::AbsoluteX: {
      uint16_t base_addr = read16(pc);
      uint16_t addr = base_addr + x;
      if (opcode.does_add_cycle_if_page_boundary_crossed() &&
          (addr & 0xFF00) != (base_addr & 0xFF00)) {
        // crossed page boundary
        ++cycles;
      }
      return addr;
    }
    case AddressingMode::AbsoluteY: {
      uint16_t base_addr = read16(pc);
      uint16_t addr = base_addr + y;
      if (opcode.does_add_cycle_if_page_boundary_crossed() &&
          (addr & 0xFF00) != (base_addr & 0xFF00)) {
        // crossed page boundary
        ++cycles;
      }
      return addr;
    }
    case AddressingMode::Relative: {
      uint8_t addr = read(pc) + pc + 1;
      return addr;
    }
    case AddressingMode::Indirect: {
      uint16_t ref = read16(pc);
      uint8_t addr_lo = read(ref);
      uint16_t addr_hi;
      if ((ref & 0xFF) == 0xFF)
        // Unintuitively, indirect read wraps around the page
        addr_hi = read(ref & 0xFF00);
      else
        addr_hi = read(ref + 1);
      uint16_t addr = (addr_hi << 8) | addr_lo;
      return addr;
    }
    case AddressingMode::IndirectX: {
      // LDA ($02,X)
      //      ---    @ pc
      //      -----  ref
      //     ------- addr
      uint8_t ref = (read(pc) + x);
      uint8_t addr_lo = read(ref);
      uint16_t addr_hi = read((uint8_t)(ref + 1));
      return (addr_hi << 8) | addr_lo;
    }
    case AddressingMode::IndirectY: {
      // LDA ($02),Y
      //      ---    ref
      //     -----   base addr
      //     ------- addr
      uint8_t ref = read(pc);
      uint8_t base_addr_lo = read(ref);
      uint16_t base_addr_hi = read((uint8_t)(ref + 1));
      uint16_t base_addr = (base_addr_hi << 8) | base_addr_lo;
      uint16_t addr = base_addr + y;
      if (opcode.does_add_cycle_if_page_boundary_crossed() &&
          (addr & 0xFF00) != (base_addr & 0xFF00)) {
        // page boundary crossed
        ++cycles;
      }
      return addr;
    }
    default:
      assert(!"illegal mode");
  }
}

void Cpu::stack_push(uint8_t val) {
  write(0x0100 + sp, val);
  --sp;
}

uint8_t Cpu::stack_pop() {
  ++sp;
  uint8_t data = read(0x0100 + sp);
  return data;
}

void Cpu::stack_push16(uint16_t data) {
  uint8_t hi = data >> 8;
  uint8_t lo = data & 0xFF;
  stack_push(hi);
  stack_push(lo);
}

uint16_t Cpu::stack_pop16() {
  uint8_t lo = stack_pop();
  uint8_t hi = stack_pop();
  uint16_t data = ((uint16_t)hi << 8) | lo;
  return data;
}

void Cpu::update_zero_neg_flags(uint8_t val) {
  flags.zero = (val == 0);
  flags.negative = ((val & 0b10000000) != 0);
}

void Cpu::fetch_exec() {
  uint8_t opc = read(pc++);
  const Opcode &opcode = opcodes[opc];
  uint16_t operand_addr;
  if (opcode.mode != AddressingMode::Implied)
    operand_addr = get_operand_addr(opcode);
  uint16_t prev_pc = pc;

  switch (opc) {
      /*
       * Transfer instructions
       */

    case 0xA9:
    case 0xA5:
    case 0xB5:
    case 0xAD:
    case 0xBD:
    case 0xB9:
    case 0xA1:
    case 0xB1:
      lda(operand_addr);
      break;

    case 0xA2:
    case 0xA6:
    case 0xB6:
    case 0xAE:
    case 0xBE:
      ldx(operand_addr);
      break;

    case 0xA0:
    case 0xA4:
    case 0xB4:
    case 0xAC:
    case 0xBC:
      ldy(operand_addr);
      break;

    case 0x85:
    case 0x95:
    case 0x8D:
    case 0x9D:
    case 0x99:
    case 0x81:
    case 0x91:
      sta(operand_addr);
      break;

    case 0x86:
    case 0x96:
    case 0x8E:
      stx(operand_addr);
      break;

    case 0x84:
    case 0x94:
    case 0x8C:
      sty(operand_addr);
      break;

    case 0xAA:
      tax();
      break;
    case 0xA8:
      tay();
      break;
    case 0xBA:
      tsx();
      break;
    case 0x8A:
      txa();
      break;
    case 0x98:
      tya();
      break;
    case 0x9A:
      txs();
      break;

      /*
       * Stack instructions
       */

    case 0x48:
      pha();
      break;
    case 0x08:
      php();
      break;
    case 0x68:
      pla();
      break;
    case 0x28:
      plp();
      break;

      /*
       * Decrements & increments
       */

    case 0xC6:
    case 0xD6:
    case 0xCE:
    case 0xDE:
      dec(operand_addr);
      break;

    case 0xCA:
      dex();
      break;
    case 0x88:
      dey();
      break;

    case 0xE6:
    case 0xF6:
    case 0xEE:
    case 0xFE:
      inc(operand_addr);
      break;
    case 0xE8:
      inx();
      break;
    case 0xC8:
      iny();
      break;

      /*
       * Arithmetic operations
       */

    case 0x69:
    case 0x65:
    case 0x75:
    case 0x6D:
    case 0x7D:
    case 0x79:
    case 0x61:
    case 0x71:
      adc(operand_addr);
      break;

    case 0xE9:
    case 0xE5:
    case 0xF5:
    case 0xED:
    case 0xFD:
    case 0xF9:
    case 0xE1:
    case 0xF1:
      sbc(operand_addr);
      break;

      /*
       * Logical operations
       */

    case 0x29:
    case 0x25:
    case 0x35:
    case 0x2D:
    case 0x3D:
    case 0x39:
    case 0x21:
    case 0x31:
      and_(operand_addr);
      break;

    case 0x49:
    case 0x45:
    case 0x55:
    case 0x4D:
    case 0x5D:
    case 0x59:
    case 0x41:
    case 0x51:
      eor(operand_addr);
      break;

    case 0x09:
    case 0x05:
    case 0x15:
    case 0x0D:
    case 0x1D:
    case 0x19:
    case 0x01:
    case 0x11:
      ora(operand_addr);
      break;

      /*
       * Shift & rotate instructions
       */

    case 0x0A:
      asl_a();
      break;
    case 0x06:
    case 0x16:
    case 0x0E:
    case 0x1E:
      asl_mem(operand_addr);
      break;

    case 0x4A:
      lsr_a();
      break;
    case 0x46:
    case 0x56:
    case 0x4E:
    case 0x5E:
      lsr_mem(operand_addr);
      break;

    case 0x2A:
      rol_a();
      break;
    case 0x26:
    case 0x36:
    case 0x2E:
    case 0x3E:
      rol_mem(operand_addr);
      break;

    case 0x6A:
      ror_a();
      break;
    case 0x66:
    case 0x76:
    case 0x6E:
    case 0x7E:
      ror_mem(operand_addr);
      break;

      /*
       * Flag instructions
       */

    case 0x18:  // CLC
      flags.carry = false;
      break;
    case 0xD8:  // CLD
      flags.decimal = false;
      break;
    case 0x58:  // CLI
      flags.interrupt_disable = false;
      break;
    case 0xB8:  // CLV
      flags.overflow = false;
      break;
    case 0x38:  // SEC
      flags.carry = true;
      break;
    case 0xF8:  // SED
      flags.decimal = true;
      break;
    case 0x78:  // SEI
      flags.interrupt_disable = true;
      break;

      /*
       * Comparisons
       */

    case 0xC9:
    case 0xC5:
    case 0xD5:
    case 0xCD:
    case 0xDD:
    case 0xD9:
    case 0xC1:
    case 0xD1:  // CMP
      compare_with(operand_addr, a);
      break;

    case 0xE0:
    case 0xE4:
    case 0xEC:  // CPX
      compare_with(operand_addr, x);
      break;

    case 0xC0:
    case 0xC4:
    case 0xCC:  // CPY
      compare_with(operand_addr, y);
      break;

      /*
       * Condition branch instructions
       */

    case 0x90:  // BCC
      branch_cond(!flags.carry);
      break;
    case 0xB0:  // BCS
      branch_cond(flags.carry);
      break;
    case 0xF0:  // BEQ
      branch_cond(flags.zero);
      break;
    case 0x30:  // BMI
      branch_cond(flags.negative);
      break;
    case 0xD0:  // BNE
      branch_cond(!flags.zero);
      break;
    case 0x10:  // BPL
      branch_cond(!flags.negative);
      break;
    case 0x50:  // BVC
      branch_cond(!flags.overflow);
      break;
    case 0x70:  // BVS
      branch_cond(flags.overflow);
      break;

      /*
       * Jumps & subroutines
       */

    case 0x4C:
    case 0x6C:
      jmp(operand_addr);
      break;

    case 0x20:
      jsr();
      break;

    case 0x60:
      rts();
      break;

      /*
       * Interrupts
       */

    case 0x00:  // BRK
      brk();
      break;
    case 0x40:
      rti();
      break;

      /*
       * Other
       */

    case 0x24:
    case 0x2C:
      bit(operand_addr);
      break;

    case 0xEA:  // NOP
      break;

      /*
       * "Illegal" opcodes
       */

    case 0xC7:
    case 0xD7:
    case 0xCF:
    case 0xDF:
    case 0xDB:
    case 0xC3:
    case 0xD3:
      dcp(operand_addr);
      break;

    case 0xE7:
    case 0xF7:
    case 0xEF:
    case 0xFF:
    case 0xFB:
    case 0xE3:
    case 0xF3:  // ISC
      inc(operand_addr);
      sbc(operand_addr);
      break;

    case 0xA7:
    case 0xB7:
    case 0xAF:
    case 0xBF:
    case 0xA3:
    case 0xB3:
      lax(operand_addr);
      break;

    case 0x27:
    case 0x37:
    case 0x2F:
    case 0x3F:
    case 0x3B:
    case 0x23:
    case 0x33:  // RLA
      rol_mem(operand_addr);
      and_(operand_addr);
      break;

    case 0x67:
    case 0x77:
    case 0x6F:
    case 0x7F:
    case 0x7B:
    case 0x63:
    case 0x73:  // RRA
      ror_mem(operand_addr);
      adc(operand_addr);
      break;

    case 0x87:
    case 0x97:
    case 0x8F:
    case 0x83:
      sax(operand_addr);
      break;

    case 0x07:
    case 0x17:
    case 0x0F:
    case 0x1F:
    case 0x1B:
    case 0x03:
    case 0x13:  // SLO
      asl_mem(operand_addr);
      ora(operand_addr);
      break;

    case 0x47:
    case 0x57:
    case 0x4F:
    case 0x5F:
    case 0x5B:
    case 0x43:
    case 0x53:  // SRE
      lsr_mem(operand_addr);
      eor(operand_addr);
      break;

    case 0xEB:  // USBC (SBC + NOP)
      sbc(operand_addr);
      break;

    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA:
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xE2:
    case 0x04:
    case 0x44:
    case 0x64:
    case 0x14:
    case 0x34:
    case 0x54:
    case 0x74:
    case 0xD4:
    case 0xF4:
    case 0x0C:
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:  // NOPs (including DOP, TOP)
      break;

    default:
      printf("Unimplemented opc %02X\n", opc);
      exit(1);
  }

  // Don't increment pc if the instruction modified it (e.g. jmp)
  if (pc == prev_pc) pc += (opcode.len - 1);

  cycles += opcode.cycles;
}

void Cpu::adc(uint16_t addr) {
  uint8_t data = read(addr);
  uint16_t sum = a + data + (flags.carry ? 1 : 0);

  flags.carry = (sum > 0xFF);

  uint8_t result = sum;

  flags.overflow = ((data ^ result) & (result ^ a) & 0b10000000);

  a = result;
  update_zero_neg_flags(a);
}

void Cpu::and_(uint16_t addr) {
  uint8_t val = read(addr);
  a &= val;
  update_zero_neg_flags(a);
}

void Cpu::asl_a() {
  uint16_t data = a;

  data <<= 1;

  flags.carry = (data > 0xFF);

  a = data;
  update_zero_neg_flags(a);
}

void Cpu::asl_mem(uint16_t addr) {
  uint16_t data = read(addr);

  data <<= 1;

  flags.carry = (data > 0xFF);

  write(addr, (uint8_t)data);
  update_zero_neg_flags(data);
}

void Cpu::bit(uint16_t addr) {
  uint16_t data = read(addr);

  flags.negative = data & 0b10000000;
  flags.overflow = data & 0b01000000;
  flags.zero = !(data & a);
}

void Cpu::branch_cond(uint8_t cond) {
  if (cond) {
    int8_t rel = read(pc);
    ++pc;  // skip the argument
    int16_t new_pc = pc + rel;
    ++cycles;
    if ((new_pc & 0xFF00) != (pc & 0xFF00)) ++cycles;
    pc = new_pc;
  }
}

void Cpu::brk() {
  stack_push16(pc + 1);
  stack_push(flags.bits() | (0b00010000));
  flags.interrupt_disable = true;
  pc = read16(0xFFFE);
}

void Cpu::compare_with(uint16_t addr, uint8_t reg) {
  uint8_t data = read(addr);

  flags.carry = (data <= reg);

  uint8_t sub = reg - data;
  update_zero_neg_flags(sub);
}

void Cpu::dec(uint16_t addr) {
  uint8_t data = read(addr);
  --data;
  write(addr, data);
  update_zero_neg_flags(data);
}

void Cpu::eor(uint16_t addr) {
  uint8_t data = read(addr);
  a ^= data;
  update_zero_neg_flags(a);
}

void Cpu::inc(uint16_t addr) {
  uint8_t data = read(addr);
  ++data;
  write(addr, data);
  update_zero_neg_flags(data);
}

void Cpu::jmp(uint16_t addr) { pc = addr; }

void Cpu::jsr() {
  uint16_t addr = read16(pc);
  stack_push16(pc + 1);
  pc = addr;
}

void Cpu::lda(uint16_t addr) {
  uint8_t val = read(addr);
  a = val;
  update_zero_neg_flags(a);
}

void Cpu::ldx(uint16_t addr) {
  uint8_t val = read(addr);
  x = val;
  update_zero_neg_flags(x);
}

void Cpu::ldy(uint16_t addr) {
  uint8_t val = read(addr);
  y = val;
  update_zero_neg_flags(y);
}

void Cpu::lsr_a() {
  flags.carry = a & 0b1;

  a >>= 1;

  update_zero_neg_flags(a);
}

void Cpu::lsr_mem(uint16_t addr) {
  uint8_t data = read(addr);

  flags.carry = data & 0b1;

  data >>= 1;
  write(addr, data);

  update_zero_neg_flags(data);
}

void Cpu::ora(uint16_t addr) {
  uint8_t data = read(addr);
  a |= data;
  update_zero_neg_flags(a);
}

void Cpu::sta(uint16_t addr) { write(addr, a); }

void Cpu::stx(uint16_t addr) { write(addr, x); }

void Cpu::sty(uint16_t addr) { write(addr, y); }

void Cpu::transfer_a_to(uint8_t *reg) {
  *reg = a;
  update_zero_neg_flags(*reg);
}

void Cpu::tax() {
  x = a;
  update_zero_neg_flags(x);
}

void Cpu::txa() {
  a = x;
  update_zero_neg_flags(a);
}

void Cpu::dex() {
  --x;
  update_zero_neg_flags(x);
}

void Cpu::inx() {
  ++x;
  update_zero_neg_flags(x);
}

void Cpu::tay() {
  y = a;
  update_zero_neg_flags(y);
}

void Cpu::tya() {
  a = y;
  update_zero_neg_flags(a);
}

void Cpu::dey() {
  --y;
  update_zero_neg_flags(y);
}

void Cpu::iny() {
  ++y;
  update_zero_neg_flags(y);
}

void Cpu::rol_a() {
  uint8_t c = a & 0b10000000;
  a <<= 1;
  a |= (flags.carry ? 1 : 0);
  update_zero_neg_flags(a);
  flags.carry = (c != 0);
}

void Cpu::rol_mem(uint16_t addr) {
  uint8_t data = read(addr);
  uint8_t c = data & 0b10000000;
  data <<= 1;
  data |= (flags.carry ? 1 : 0);
  write(addr, data);
  update_zero_neg_flags(data);
  flags.carry = (c != 0);
}

void Cpu::ror_a() {
  uint8_t c = a & 0b1;
  a >>= 1;
  a |= ((flags.carry ? 1 : 0) << 7);
  update_zero_neg_flags(a);
  flags.carry = (c != 0);
}

void Cpu::ror_mem(uint16_t addr) {
  uint8_t data = read(addr);
  uint8_t c = data & 0b1;
  data >>= 1;
  data |= ((flags.carry ? 1 : 0) << 7);
  write(addr, data);
  update_zero_neg_flags(data);
  flags.carry = (c != 0);
}

void Cpu::rti() {
  uint8_t saved_flag_bits = stack_pop();
  flags.negative = saved_flag_bits & 0b10000000;
  flags.overflow = saved_flag_bits & 0b01000000;
  flags.decimal = saved_flag_bits & 0b00001000;
  flags.interrupt_disable = saved_flag_bits & 0b00000100;
  flags.zero = saved_flag_bits & 0b00000010;
  flags.carry = saved_flag_bits & 0b00000001;
  pc = stack_pop16();
}

void Cpu::rts() {
  uint16_t addr = stack_pop16();
  ++addr;
  pc = addr;
}

// result is reduced by one if the srflag is **CLEAR**
void Cpu::sbc(uint16_t addr) {
  uint8_t data = read(addr);
  // turn into 1s complement (subtract 1 if no carry)
  data = ~data;
  uint16_t sum = a + data + (flags.carry ? 1 : 0);

  flags.carry = (sum > 0xFF);

  uint8_t result = sum;

  flags.overflow = ((data ^ result) & (result ^ a) & 0b10000000);

  a = result;
  update_zero_neg_flags(a);
}

void Cpu::tsx() {
  x = sp;
  update_zero_neg_flags(x);
}

void Cpu::txs() { sp = x; }

void Cpu::pha() { stack_push(a); }

void Cpu::pla() {
  a = stack_pop();
  update_zero_neg_flags(a);
}

void Cpu::php() {
  uint8_t bits = flags.bits();
  // B flag is not a "real" flag, but is always set when pushed onto the stack
  // with php. R flag is always set.
  bits |= 0b00110000;
  stack_push(bits);
}

void Cpu::plp() {
  uint8_t saved_flag_bits = stack_pop();
  flags.negative = saved_flag_bits & 0b10000000;
  flags.overflow = saved_flag_bits & 0b01000000;
  flags.decimal = saved_flag_bits & 0b00001000;
  flags.interrupt_disable = saved_flag_bits & 0b00000100;
  flags.zero = saved_flag_bits & 0b00000010;
  flags.carry = saved_flag_bits & 0b00000001;
}

void Cpu::lax(uint16_t addr) {
  uint8_t data = read(addr);
  a = data;
  x = data;
  update_zero_neg_flags(a);
}

void Cpu::sax(uint16_t addr) {
  uint8_t data = a & x;
  write(addr, data);
}

void Cpu::dcp(uint16_t addr) {
  uint8_t data = read(addr);
  --data;
  write(addr, data);
  compare_with(addr, a);
}

void Cpu::handle_nmi() {
  stack_push16(pc);
  stack_push(flags.bits());
  flags.interrupt_disable = true;
  pc = read16(0xFFFA);
}

void Cpu::handle_irq() {
  stack_push16(pc);
  stack_push(flags.bits());
  flags.interrupt_disable = true;
  pc = read16(0xFFFE);
}

}  // namespace nesem
