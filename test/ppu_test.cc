#include "ppu.h"

#include <gtest/gtest.h>

namespace nesem {

class PpuTest : public ::testing::Test {
 protected:
  Ppu ppu;
};

TEST_F(PpuTest, write_ctrl) {
  ppu.write(0x2000, 0xFF);
  EXPECT_EQ(ppu.ctrl, 0xFF);
}

TEST_F(PpuTest, write_ctrl_fills_databus) {
  ppu.write(0x2000, 0xFF);
  EXPECT_EQ(ppu.read(0x2001), 0xFF);
}

TEST_F(PpuTest, write_mask) {
  ppu.write(0x2001, 0xFF);
  EXPECT_EQ(ppu.mask, 0xFF);
}

TEST_F(PpuTest, write_mask_fills_databus) {
  ppu.write(0x2001, 0xFF);
  EXPECT_EQ(ppu.read(0x2000), 0xFF);
}

TEST_F(PpuTest, read_status_open_bus) {
  ppu.write(0x2000, 0xFF);  // fill io databus
  EXPECT_EQ(ppu.read(0x2002) & 0b11111, 0b11111);
}

TEST_F(PpuTest, read_oam) {
  ppu.oam[0x10] = 0x05;

  ppu.write(0x2003, 0x10);
  EXPECT_EQ(ppu.read(0x2003), 0x10);
  EXPECT_EQ(ppu.read(0x2004), 0x05);
}

TEST_F(PpuTest, write_oam) {
  ppu.write(0x2003, 0x10);
  ppu.write(0x2004, 0x05);

  EXPECT_EQ(ppu.oam_addr, 0x11);
  EXPECT_EQ(ppu.oam[0x10], 0x05);
}

TEST_F(PpuTest, read_vram) {
  ppu.vram[0x111] = 0x05;

  ppu.write(0x2006, 0x21);
  ppu.write(0x2006, 0x11);
  ppu.read(0x2007);
  EXPECT_EQ(ppu.read(0x2007), 0x05);
}

TEST_F(PpuTest, write_vram) {
  ppu.write(0x2006, 0x20);
  ppu.write(0x2006, 0x11);
  ppu.write(0x2007, 0x05);

  EXPECT_EQ(ppu.vram[0x11], 0x05);
}

TEST_F(PpuTest, write_vram_incr_across) {
  ppu.ctrl &= ~0b100;

  ppu.write(0x2006, 0x20);
  ppu.write(0x2006, 0x11);
  ppu.write(0x2007, 0x05);
  ppu.write(0x2007, 0x06);

  EXPECT_EQ(ppu.vram[0x11], 0x05);
  EXPECT_EQ(ppu.vram[0x12], 0x06);
}

TEST_F(PpuTest, write_vram_incr_down) {
  ppu.ctrl |= 0b100;

  ppu.write(0x2006, 0x20);
  ppu.write(0x2006, 0x11);
  ppu.write(0x2007, 0x05);
  ppu.write(0x2007, 0x06);

  EXPECT_EQ(ppu.vram[0x11], 0x05);
  EXPECT_EQ(ppu.vram[0x31], 0x06);
}

TEST_F(PpuTest, read_palette) {
  ppu.palettes[0x01] = 0x05;

  ppu.write(0x2006, 0x3F);
  ppu.write(0x2006, 0x01);
  EXPECT_EQ(ppu.read(0x2007), 0x05);
}

TEST_F(PpuTest, write_palette) {
  ppu.vram[0x111] = 0x05;

  ppu.write(0x2006, 0x01);
  ppu.write(0x2006, 0x11);
  ppu.write(0x2007, 0x05);

  EXPECT_EQ(ppu.vram[0x111], 0x05);
}

TEST_F(PpuTest, vblank) {
  EXPECT_FALSE(ppu.status() & (1 << 7));
  ppu.tick(242 * 341);
  EXPECT_TRUE(ppu.status() & (1 << 7));
  ppu.tick(21 * 341);
  EXPECT_FALSE(ppu.status() & (1 << 7));
}

}  // namespace nesem
