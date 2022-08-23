#include "ppu.h"

#include <fmt/core.h>

namespace nesem {

Ppu::Ppu(const Cartridge &cartridge)
    : chr(cartridge.chr), mirroring(cartridge.mirroring) {}

uint8_t Ppu::status() const {
  return (in_vblank << 7) | (sprite_0_hit << 6) | (sprite_overflow << 5) |
         (io_databus & 0b11111);
}

static uint16_t translate_vram_addr(uint16_t addr, ScreenMirroring mirroring) {
  addr %= 0x2000;
  switch (mirroring) {
    case ScreenMirroring::Horizontal:
      if (addr >= 0x800) {
        addr %= 0x400;
        addr += 0x400;
      } else {
        addr %= 0x400;
      }
      break;
    case ScreenMirroring::Vertical:
      addr %= 0x800;
      break;
    default:
      break;
  }
  return addr;
}

uint8_t Ppu::read(uint16_t addr) {
  switch (addr) {
    case 0x2002:  // status
      io_databus = status();
      addr_latch.reset();
      break;
    case 0x2004:  // oam data
      io_databus = oam[oam_addr];
      break;
    case 0x2007:  // data
      addr = addr_latch.read();
      if (addr <= 0x1FFF) {  // chr
        io_databus = read_buffer;
        read_buffer = chr[addr];
      } else if (addr <= 0x3EFF) {  // vram
        io_databus = read_buffer;
        uint16_t vram_addr = translate_vram_addr(addr, mirroring);
        read_buffer = vram[vram_addr];
      } else {  // palettes
        addr &= 0x3F1F;
        io_databus = palettes[addr - 0x3F00];
        // read buffer still gets updated, to the mirrored nametable data
        // that would be beneath the palette
        uint16_t vram_addr = translate_vram_addr(addr & 0x2FFF, mirroring);
        read_buffer = vram[vram_addr];
      }
      addr_latch.increment((ctrl & 0b100) ? 32 : 1);
      break;
  }
  return io_databus;
}

void Ppu::write(uint16_t addr, uint8_t data) {
  switch (addr) {
    case 0x2000:  // control
      ctrl = data;
      break;
    case 0x2001:  // mask
      mask = data;
      break;
    case 0x2003:  // oam address
      oam_addr = data;
      break;
    case 0x2004:  // oam data
      oam[oam_addr] = data;
      ++oam_addr;
      break;
    case 0x2005:  // scroll
      addr_latch.write(data);
      break;
    case 0x2006:  // address
      addr_latch.write(data);
      break;
    case 0x2007:  // data
      addr = addr_latch.read();
      if (addr <= 0x1FFF) {  // chr
        // ignore writes to chr
      } else if (addr <= 0x3EFF) {  // vram
        uint16_t vram_addr = translate_vram_addr(addr, mirroring);
        vram[vram_addr] = data;
      } else {  // palettes
        addr &= 0x3F1F;
        palettes[addr - 0x3F00] = data;
      }
      addr_latch.increment((ctrl & 0b100) ? 32 : 1);
      break;
  }
  io_databus = data;
}

void Ppu::tick(size_t cycles) {
  cycle += cycles;
  while (cycle >= 341) {
    cycle -= 341;
    ++scanline;
    if (scanline == 241) {
      in_vblank = true;
    } else if (scanline >= 262) {
      in_vblank = false;
      scanline = 0;
    }
  }
}

};  // namespace nesem
