#include "cpu.h"

#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>

#include <functional>
#include <memory>

#include "assembler/assembler.h"

namespace nesem {

class CpuTest : public ::testing::Test {
 protected:
  RamOnlyMmu mmu;
  Cpu cpu = {&mmu};
  uint16_t prg_end = -1;

  void load(const std::string& code, uint16_t start_pc = 0x8000) {
    std::vector<uint8_t> prg = assembler::assemble(code);
    for (int i = 0; i < prg.size(); ++i) cpu.write(start_pc + i, prg[i]);
    cpu.write16(Cpu::kResetVector, start_pc);
    prg_end = start_pc + prg.size();
    cpu.reset();
  }

  void run() {
    while (cpu.pc < prg_end) cpu.step();
  }

  size_t count_cycles(std::function<void()>&& foo) {
    size_t start_cycles = cpu.cycles;
    foo();
    return cpu.cycles - start_cycles;
  }
};

// Load, store and interregister transfer instructions

TEST_F(CpuTest, lda_immediate_load_data) {
  load("LDA #$05");
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_negative_flag) {
  load("LDA #$FF");
  run();

  EXPECT_TRUE(cpu.flags.negative);
}

TEST_F(CpuTest, lda_zero_flag) {
  load("LDA #$00");
  run();

  EXPECT_TRUE(cpu.flags.zero);
}

TEST_F(CpuTest, lda_immediate_timing) {
  load("LDA #$00");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 2);
}

TEST_F(CpuTest, lda_zeropage_timing) {
  load("LDA $00");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 3);
}

TEST_F(CpuTest, lda_zeropage_index_timing) {
  load("LDA $00,X");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 4);
}

TEST_F(CpuTest, lda_absolute_timing) {
  load("LDA $0000");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 4);
}

TEST_F(CpuTest, lda_absolute_indexed_timing_same_page) {
  load("LDA $0000,X");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 4);
}

TEST_F(CpuTest, lda_absolute_indexed_timing_cross_page) {
  load("LDA $00FF,X");
  cpu.x = 0x01;
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 5);
}

TEST_F(CpuTest, lda_indirect_x_timing) {
  load("LDA ($00,X)");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 6);
}

TEST_F(CpuTest, lda_indirect_y_timing_same_page) {
  load("LDA ($00),Y");
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 5);
}

TEST_F(CpuTest, lda_indirect_y_timing_cross_page) {
  load("LDA ($00),Y");
  cpu.y = 0x01;
  cpu.write(0x00, 0xFF);
  size_t cycles = count_cycles([this] { run(); });

  ASSERT_EQ(cycles, 6);
}

TEST_F(CpuTest, ldx) {
  load("LDX #$05");
  run();

  EXPECT_EQ(cpu.x, 0x05);
}

TEST_F(CpuTest, ldy) {
  load("LDY #$05");
  run();

  EXPECT_EQ(cpu.y, 0x05);
}

TEST_F(CpuTest, sta) {
  load("STA $01");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.read(0x01), 0x05);
}

TEST_F(CpuTest, stx) {
  load("STX $01");
  cpu.x = 0x05;
  run();

  EXPECT_EQ(cpu.read(0x01), 0x05);
}

TEST_F(CpuTest, sty) {
  load("STY $01");
  cpu.y = 0x05;
  run();

  EXPECT_EQ(cpu.read(0x01), 0x05);
}

TEST_F(CpuTest, tax) {
  load("TAX");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.x, 0x05);
}

TEST_F(CpuTest, tay) {
  load("TAY");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.y, 0x05);
}

TEST_F(CpuTest, tsx) {
  load("TSX");
  run();

  EXPECT_EQ(cpu.x, 0xFD);
}

TEST_F(CpuTest, txa) {
  load("TXA");
  cpu.x = 0x05;
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, txs) {
  load("TXS");
  cpu.x = 0x05;
  run();

  EXPECT_EQ(cpu.sp, 0x05);
}

TEST_F(CpuTest, tya) {
  load("TYA");
  cpu.y = 0x05;
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

// addressing modes

TEST_F(CpuTest, lda_zeropage) {
  load("LDA $10");
  cpu.write(0x10, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_zeropage_index) {
  load("LDA $10,X");
  cpu.x = 0x50;
  cpu.write(0x60, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_zeropage_wraparound) {
  load("LDA $FF,X");
  cpu.x = 0x01;
  cpu.write(0x00, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_absolute) {
  load("LDA $0101");
  cpu.write(0x0101, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_absolute_x) {
  load("LDA $0101,X");
  cpu.x = 0x01;
  cpu.write(0x0102, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_absolute_y) {
  load("LDA $0101,Y");
  cpu.y = 0x01;
  cpu.write(0x0102, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_indirect_x) {
  load("LDA ($10,X)");
  cpu.x = 0x01;
  cpu.write16(0x0011, 0x0050);
  cpu.write(0x0050, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_indirect_x_wraparound) {
  load("LDA ($FF,X)");
  // write $0150, wrapped around the page
  cpu.write(0x00FF, 0x50);
  cpu.write(0x0000, 0x01);
  // write data at $0150
  cpu.write(0x0150, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_indirect_y) {
  load("LDA ($10),Y");
  cpu.y = 0x01;
  cpu.write16(0x0010, 0x0050);
  cpu.write(0x0051, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, lda_indirect_y_wraparound) {
  load("LDA ($FF),Y");
  // write $0150, wrapped around the page
  cpu.write(0x00FF, 0x50);
  cpu.write(0x0000, 0x01);
  // write data at $0150
  cpu.write(0x0150, 0x05);
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

// Stack instructions

TEST_F(CpuTest, pha) {
  load("PHA");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.sp, 0xFC);
  EXPECT_EQ(cpu.read(0x01FD), 0x05);
}

TEST_F(CpuTest, pla) {
  load("PLA");
  cpu.write(0x01FD, 0x05);
  --cpu.sp;
  run();

  EXPECT_EQ(cpu.a, 0x05);
}

TEST_F(CpuTest, php) {
  assert(cpu.flags.bits() == 0b00100100);
  load("PHP");
  run();

  EXPECT_EQ(cpu.sp, 0xFC);
  EXPECT_EQ(cpu.read(0x01FD), 0b00110100);  // break flag is set
}

TEST_F(CpuTest, plp) {
  assert(cpu.flags.bits() == 0b00100100);
  load("PLP");
  cpu.write(0xFD, 0x00);
  --cpu.sp;
  run();

  EXPECT_EQ(cpu.flags.bits(), 0b00100000);
}

// Decrements & increments

TEST_F(CpuTest, dec) {
  load("DEC $00");
  run();

  EXPECT_EQ(cpu.read(0x00), 0xFF);
  EXPECT_TRUE(cpu.flags.negative);
}

TEST_F(CpuTest, dex) {
  load("DEX");
  run();

  EXPECT_EQ(cpu.x, 0xFF);
  EXPECT_TRUE(cpu.flags.negative);
}

TEST_F(CpuTest, dey) {
  load("DEY");
  run();

  EXPECT_EQ(cpu.y, 0xFF);
  EXPECT_TRUE(cpu.flags.negative);
}

TEST_F(CpuTest, inc) {
  load("INC $00");
  run();

  EXPECT_EQ(cpu.read(0x00), 0x01);
}

TEST_F(CpuTest, inx) {
  load("INX");
  run();

  EXPECT_EQ(cpu.x, 0x01);
}

TEST_F(CpuTest, iny) {
  load("INY");
  run();

  EXPECT_EQ(cpu.y, 0x01);
}

// Arithmetic operations

TEST_F(CpuTest, adc) {
  load("ADC #$06");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.a, 0x0B);
}

TEST_F(CpuTest, adc_carry_in) {
  load("ADC #$01");
  cpu.a = 0x00;
  cpu.flags.carry = true;
  run();

  EXPECT_EQ(cpu.a, 0x02);
}

TEST_F(CpuTest, adc_carry_out) {
  load("ADC #$01");
  cpu.a = 0xFF;
  run();

  EXPECT_TRUE(cpu.flags.carry);
}

TEST_F(CpuTest, adc_overflow) {
  load("ADC #$01");
  cpu.a = 0x7F;
  run();

  EXPECT_TRUE(cpu.flags.overflow);
}

TEST_F(CpuTest, sbc) {
  load("SBC #$03");
  cpu.a = 0x05;
  run();

  EXPECT_EQ(cpu.a, 0x01);
}

TEST_F(CpuTest, sbc_carry_in) {
  load("SBC #$03");
  cpu.a = 0x05;
  cpu.flags.carry = true;
  run();

  EXPECT_EQ(cpu.a, 0x02);
}

TEST_F(CpuTest, sbc_overflow) {
  load("SBC #$00");
  cpu.a = 0x80;
  run();

  EXPECT_TRUE(cpu.flags.overflow);
}

// Logical operations

TEST_F(CpuTest, and) {
  load("AND #$06");  // 0b110
  cpu.a = 0b101;
  run();

  EXPECT_EQ(cpu.a, 0b100);
}

TEST_F(CpuTest, eor) {
  load("EOR #$06");  // 0b110
  cpu.a = 0b101;
  run();

  EXPECT_EQ(cpu.a, 0b011);
}

TEST_F(CpuTest, ora) {
  load("ORA #$06");  // 0b110
  cpu.a = 0b101;
  run();

  EXPECT_EQ(cpu.a, 0b111);  // 0b111
}

// Shift & rotate instructions

TEST_F(CpuTest, asl) {
  load("ASL");
  cpu.a = 0b1;
  run();

  EXPECT_EQ(cpu.a, 0b10);
}

TEST_F(CpuTest, asl_carry_out) {
  load("ASL");
  cpu.a = 0b10000000;
  run();

  EXPECT_TRUE(cpu.flags.carry);
}

TEST_F(CpuTest, lsr) {
  load("LSR");
  cpu.a = 0b10;
  run();

  EXPECT_EQ(cpu.a, 0b01);
}

TEST_F(CpuTest, lsr_carry_out) {
  load("LSR");
  cpu.a = 0b1;
  run();

  EXPECT_TRUE(cpu.flags.carry);
}

TEST_F(CpuTest, rol_carry_in) {
  load("ROL");
  cpu.a = 0b1;
  cpu.flags.carry = true;

  run();

  EXPECT_EQ(cpu.a, 0b11);
}

TEST_F(CpuTest, ror_carry_in) {
  load("ROR");
  cpu.a = 0b10;
  cpu.flags.carry = true;
  run();

  EXPECT_EQ(cpu.a, 0b10000001);
}

// Comparisons

TEST_F(CpuTest, cmp_eq) {
  load("CMP $00");
  cpu.a = 0;
  cpu.write(0x00, 0x00);
  run();

  EXPECT_TRUE(cpu.flags.zero);
  EXPECT_TRUE(cpu.flags.carry);
}

TEST_F(CpuTest, cmp_lt) {
  load("CMP $00");
  cpu.a = 0x01;
  cpu.write(0x00, 0x00);

  run();

  EXPECT_FALSE(cpu.flags.zero);
  EXPECT_TRUE(cpu.flags.carry);
}

TEST_F(CpuTest, cmp_gt) {
  load("LDA #$01 \n STA $00 \n LDA #$00 \n CMP $00");
  cpu.a = 0x00;
  cpu.write(0x00, 0x01);
  run();

  EXPECT_FALSE(cpu.flags.zero);
  EXPECT_FALSE(cpu.flags.carry);
}

// Conditional branch instructions

TEST_F(CpuTest, bcc_carry_set) {
  load("BCC $01");
  cpu.flags.carry = true;
  run();

  EXPECT_EQ(cpu.pc, 0x8002);
}

TEST_F(CpuTest, bcc_carry_clr) {
  load("BCC $01");
  cpu.flags.carry = false;
  run();

  EXPECT_EQ(cpu.pc, 0x8003);
}

TEST_F(CpuTest, bcs_carry_set) {
  load("BCS $01");
  cpu.flags.carry = true;
  run();

  EXPECT_EQ(cpu.pc, 0x8003);
}

TEST_F(CpuTest, bcs_carry_clr) {
  load("BCS $01");
  cpu.flags.carry = false;
  run();

  EXPECT_EQ(cpu.pc, 0x8002);
}

TEST_F(CpuTest, bcs_timing_no_branch) {
  load("BCS $01");
  cpu.flags.carry = false;
  size_t cycles = count_cycles([this] { run(); });

  EXPECT_EQ(cycles, 2);
}

TEST_F(CpuTest, bcs_branch_same_page) {
  // same page because the next PC without branch would be 0x8010
  load("BCS $01", 0x80FE);
  cpu.flags.carry = true;
  size_t cycles = count_cycles([this] { run(); });

  EXPECT_EQ(cycles, 3);
}

TEST_F(CpuTest, bcs_branch_different_page) {
  load("BCS $01", 0x80FD);
  cpu.flags.carry = true;
  size_t cycles = count_cycles([this] { run(); });

  EXPECT_EQ(cycles, 4);
}

// Jumps & subroutines

TEST_F(CpuTest, jmp) {
  load("JMP $9000");
  run();

  EXPECT_EQ(cpu.pc, 0x9000);
}

TEST_F(CpuTest, jmp_indirect) {
  load("JMP ($0101)");
  cpu.write16(0x0101, 0x9000);
  run();

  EXPECT_EQ(cpu.pc, 0x9000);
}

TEST_F(CpuTest, jmp_indirect_wraparound) {
  load("JMP ($00FF)");
  // store $9000 in memory at $00FF, wrapping around the page
  // if the cpu emulates JMP indirect correctly,
  // it will read the address from ($00FF, $0000),
  // instead of ($00FF, $0100) as might be expected
  cpu.write(0x00FF, 0x00);
  cpu.write(0x0000, 0x90);
  run();

  EXPECT_EQ(cpu.pc, 0x9000);
}

TEST_F(CpuTest, jsr_rts) {
  load(
      "      JSR mysr \n"
      "mysr: INX \n"
      "      RTS");
  cpu.step();
  cpu.step();
  cpu.step();
  cpu.step();

  EXPECT_EQ(cpu.x, 2);
}

// Interrupts

TEST_F(CpuTest, brk_rti) {
  load(
      "         BRK \n"
      "         NOP \n"
      "handler: INX \n"
      "         RTI");
  cpu.write16(Cpu::kIrqVector, 0x8002);

  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8002);
  EXPECT_EQ(cpu.x, 0);

  cpu.step();
  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8002);
  EXPECT_EQ(cpu.x, 1);
}

TEST_F(CpuTest, brk_set_flag) {
  load("BRK");

  cpu.step();
  EXPECT_TRUE(cpu.read(0x01FB) & 0b00010000);
}

TEST_F(CpuTest, irq) {
  load(
      "         NOP \n"
      "         NOP \n"
      "handler: INX \n"
      "         RTI");
  cpu.write16(Cpu::kIrqVector, 0x8002);
  cpu.flags.interrupt_disable = false;
  cpu.irq_pending = true;

  cpu.step();
  EXPECT_FALSE(cpu.irq_pending);
  EXPECT_EQ(cpu.pc, 0x8002);
}

TEST_F(CpuTest, irq_ignore) {
  load(
      "         NOP \n"
      "         CLI \n"
      "         NOP \n"
      "handler: INX \n"
      "         RTI");
  cpu.write16(Cpu::kIrqVector, 0x8003);
  cpu.irq_pending = true;

  cpu.step();
  EXPECT_TRUE(cpu.irq_pending);
  EXPECT_EQ(cpu.pc, 0x8001);

  cpu.step();
  cpu.step();
  EXPECT_FALSE(cpu.irq_pending);
  EXPECT_EQ(cpu.pc, 0x8003);
}

TEST_F(CpuTest, irq_nested) {
  load(
      "         NOP \n"
      "         NOP \n"
      "handler: INX \n"
      "         CLI \n"
      "         RTI");
  cpu.write16(Cpu::kIrqVector, 0x8002);
  cpu.flags.interrupt_disable = false;
  cpu.irq_pending = true;

  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8002);
  cpu.irq_pending = true;

  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8003);
  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8004);
  cpu.step();
  EXPECT_EQ(cpu.pc, 0x8002);
  cpu.step();
  EXPECT_EQ(cpu.x, 2);
}

TEST_F(CpuTest, nmi) {
  load(
      "         NOP \n"
      "         NOP \n"
      "handler: INX \n"
      "         RTI");
  cpu.write16(Cpu::kNmiVector, 0x8002);
  cpu.flags.interrupt_disable = true;
  cpu.nmi_pending = true;

  cpu.step();
  EXPECT_FALSE(cpu.nmi_pending);
  EXPECT_EQ(cpu.pc, 0x8002);
}

}  // namespace nesem
