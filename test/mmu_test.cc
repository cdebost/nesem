#include "mmu.h"

#include <gtest/gtest.h>

namespace nesem {

class MmuTest : public ::testing::Test {
 protected:
  Cartridge cartridge;
  NesMmu mmu{cartridge};
};

TEST_F(MmuTest, wram) {
  mmu.write(0x05, 0x06);
  ASSERT_EQ(mmu.read(0x05), 0x06);
}

TEST_F(MmuTest, wram_mirror) {
  mmu.write(0x05, 0x06);
  ASSERT_EQ(mmu.read(0b0000000000101), 0x06);
  // 0x05 + ignored bits
  ASSERT_EQ(mmu.read(0b1100000000101), 0x06);
}

TEST_F(MmuTest, read_prg_small) {
  cartridge.prg.insert(cartridge.prg.begin(), 0x4000, 0);
  cartridge.prg[0x05] = 0x06;
  mmu = {cartridge};
  ASSERT_EQ(mmu.read(0xC005), 0x06);
}

TEST_F(MmuTest, read_prg_large) {
  cartridge.prg.insert(cartridge.prg.begin(), 0x8000, 0);
  cartridge.prg[0x4005] = 0x06;
  mmu = {cartridge};
  ASSERT_EQ(mmu.read(0xC005), 0x06);
}

}  // namespace nesem
