#include "mmu.h"

#include <fmt/core.h>

namespace nesem {

uint8_t RamOnlyMmu::read(uint16_t addr) const { return ram[addr]; }

void RamOnlyMmu::write(uint16_t addr, uint8_t data) { ram[addr] = data; }

// The addressable ram space goes up to 0x1FFF, which requires 13 bits to
// address. However, the bus only decodes 11 bits, which leads to mirroring in
// the WRAM address space.
static inline uint16_t mirror_wram_addr(uint16_t addr) {
  addr &= 0b0000011111111111;
  return addr;
}

NesMmu::NesMmu(const Cartridge &c) : prg(c.prg), ppu(c) {}

uint8_t NesMmu::read(uint16_t addr) const {
  uint8_t data;
  if (addr <= 0x1FFF) {
    // WRAM
    addr = mirror_wram_addr(addr);
    data = wram[addr];
  } else if ((addr >= 0x4000 && addr <= 0x4013) || (addr == 0x4015) ||
             (addr == 0x4017)) {
    // APU registers
    data = apu_registers[addr - 0x4000];
  } else if (addr >= 0x8000 && addr <= 0xFFFF) {
    // PRG ROM
    addr -= 0x8000;
    if (addr >= prg.size()) addr %= prg.size();
    data = prg[addr];
  } else {
    data = 0;
  }
  return data;
}

uint8_t NesMmu::read(uint16_t addr) {
  uint8_t data;
  if (addr <= 0x1FFF) {
    // WRAM
    addr = mirror_wram_addr(addr);
    data = wram[addr];
  } else if (addr <= 0x3FFF) {
    addr &= 0x2007;
    data = ppu.read(addr);
  } else if ((addr >= 0x4000 && addr <= 0x4013) || (addr == 0x4015) ||
             (addr == 0x4017)) {
    // APU registers
    data = apu_registers[addr - 0x4000];
  } else if (addr >= 0x8000 && addr <= 0xFFFF) {
    // PRG ROM
    addr -= 0x8000;
    if (addr >= prg.size()) addr %= prg.size();
    data = prg[addr];
  } else {
    fmt::print(stderr, "Invalid read at addr {:X}\n", addr);
    data = 0;
  }
  return data;
}

void NesMmu::write(uint16_t addr, uint8_t data) {
  if (addr <= 0x1FFF) {
    // WRAM
    addr = mirror_wram_addr(addr);
    wram[addr] = data;
  } else if ((addr >= 0x4000 && addr <= 0x4013) || (addr == 0x4015) ||
             (addr == 0x4017)) {
    // APU registers
    apu_registers[addr - 0x4000] = data;
  } else if (addr >= 0x8000 && addr <= 0xFFFF) {
    // PRG ROM
    throw std::runtime_error(
        fmt::format("Attempted to write to ROM at {:04X}", addr));
  } else {
    fmt::print(stderr, "Invalid write at addr {:04X}\n", addr);
  }
}

}  // namespace nesem
