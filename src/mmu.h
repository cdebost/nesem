#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "cartridge.h"
#include "ppu.h"

namespace nesem {

class Mmu {
 public:
  virtual ~Mmu() {}

  // Read a single byte at the specified address
  virtual uint8_t read(uint16_t addr) const = 0;

  // Read a single byte at the specified address
  virtual uint8_t read(uint16_t addr) {
    return const_cast<const Mmu *>(this)->read(addr);
  }

  // Write a single byte at the specified address
  virtual void write(uint16_t addr, uint8_t data) = 0;
};

// Dummy MMU that permits reads and writes to its entire address space
// with no validation and no side-effects
class RamOnlyMmu : public Mmu {
 public:
  uint8_t read(uint16_t addr) const override;

  void write(uint16_t addr, uint8_t data) override;

 private:
  std::array<uint8_t, 0xFFFF + 1> ram = {0};
};

// Actual MMU used in a NES.
//
// The CPU has a memory map of 64KB addressable space.
// Most of that space is reserved - the CPU is only able to access 2KB of RAM.
//
// The memory map is as follows:
// 0xFFFF -----------------
//        |    PRG ROM    |
// 0x8000 -----------------
//        |   Save RAM    |
// 0x6000 -----------------
//        | Expansion ROM |
// 0x4020 -----------------
//        |     APU &     |
//        |  IO Registers |
// 0x4000 -----------------
//        |    Mirrors    |
// 0x2008 -----------------
//        | PPU Registers |
// 0x2000 -----------------
//        |    Mirrors    |
// 0x0800 -----------------
//        |    CPU RAM    |
// 0x0000 -----------------
class NesMmu : public Mmu {
 public:
  NesMmu() {}
  NesMmu(const Cartridge &c);

  uint8_t read(uint16_t addr) const override;
  uint8_t read(uint16_t addr) override;

  void write(uint16_t addr, uint8_t data) override;

  std::array<uint8_t, 0x800> wram = {0};  // CPU RAM ("working ram")
  Ppu ppu;
  std::array<uint8_t, 18> apu_registers;  // TODO: dummy APU registers
  std::vector<uint8_t> prg;               // program code
};

}  // namespace nesem
