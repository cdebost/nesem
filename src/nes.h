#pragma once

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"

namespace nesem {

struct Nes {
  Cpu cpu;
  NesMmu mmu;

  Nes() : cpu(&mmu) {}
  explicit Nes(const Cartridge &cartridge) : mmu(cartridge), cpu(&mmu) {}

  void reset() {
    size_t before_cycles = cpu.cycles;
    cpu.reset();
    size_t elapsed_cycles = cpu.cycles - before_cycles;
    mmu.ppu.tick(elapsed_cycles * 3);
  }

  void step() {
    if (mmu.ppu.nmi_pending) {
      mmu.ppu.nmi_pending = false;
      cpu.nmi_pending = true;
    }
    size_t before_cycles = cpu.cycles;
    cpu.step();
    size_t elapsed_cycles = cpu.cycles - before_cycles;
    mmu.ppu.tick(elapsed_cycles * 3);
  }
};

};  // namespace nesem
