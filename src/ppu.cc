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
      in_vblank = false;
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

void Ppu::oam_dma(uint8_t *data) {
  for (int i = 0; i < oam.size(); ++i) {
    oam[oam_addr] = data[i];
    ++oam_addr;
  }
}

void Ppu::tick(size_t cycles) {
  cycle += cycles;
  while (cycle >= 341) {
    cycle -= 341;
    if (scanline <= 239) draw_scanline();
    if (scanline == 240) draw_sprites();
    ++scanline;
    if (scanline == 241) {
      if (ctrl & (1 << 7)) nmi_pending = true;
      in_vblank = true;
    } else if (scanline >= 262) {
      in_vblank = false;
      scanline = 0;
    }
  }
}

void Ppu::frame_set(size_t x, size_t y, uint8_t color) {
  size_t idx = x + y * kDisplayWidth;
  if (idx < frame.size()) frame[idx] = color;
}

void Ppu::draw_scanline() {
  uint16_t bank_start = ((ctrl >> 4) & 0b1) * 0x1000;
  uint16_t tile_row = scanline / 8;

  for (int tile_col = 0; tile_col < kTilesPerScanline; ++tile_col) {
    uint16_t nametable_index = tile_row * kTilesPerScanline + tile_col;
    uint8_t pattern_index = vram[nametable_index];

    uint16_t begin = bank_start + pattern_index * 16;
    uint8_t upper = chr[begin + (scanline % kTileHeight)];
    uint8_t lower = chr[begin + (scanline % kTileHeight) + kTileHeight];

    for (int8_t x = 7; x >= 0; --x) {
      uint8_t value = (1 & lower) << 1 | (1 & upper);
      assert(value <= 3);
      upper >>= 1;
      lower >>= 1;
      uint8_t color;
      if (value == 0)
        color = palettes[0];
      else
        color = palettes[bg_palette_start_idx(tile_row, tile_col) + value];
      frame_set(tile_col * 8 + x, scanline, color);
    }
  }
}

uint8_t Ppu::bg_palette_start_idx(uint16_t tile_row, uint16_t tile_col) const {
  uint16_t attr_table_offset = tile_row / 4 * 8 + tile_col / 4;
  uint8_t attr = vram[0x3C0 + attr_table_offset];
  if ((tile_col % 4 / 2) >= 1) attr >>= 2;
  if ((tile_row % 4 / 2) >= 1) attr >>= 4;
  attr &= 0b11;
  return attr * 4;
}

void Ppu::draw_sprites() {
  uint16_t bank_start = ((ctrl >> 3) & 0b1) * 0x1000;

  for (int i = oam.size() - 4; i >= 0; i -= 4) {
    uint8_t tile_y = oam[i];
    uint8_t pattern_index = oam[i + 1];
    uint8_t data = oam[i + 2];
    uint8_t tile_x = oam[i + 3];

    bool flip_h = ((data >> 6) & 1) == 1;
    bool flip_v = ((data >> 7) & 1) == 1;

    uint16_t begin = bank_start + pattern_index * 16;

    for (int8_t y = 0; y <= 7; ++y) {
      uint8_t upper = chr[begin + y];
      uint8_t lower = chr[begin + y + kTileHeight];
      for (int8_t x = 7; x >= 0; --x) {
        uint8_t value = (1 & lower) << 1 | (1 & upper);
        assert(value <= 3);
        upper >>= 1;
        lower >>= 1;
        uint8_t color;
        if (value > 0) {
          color = palettes[sprite_palette_start_idx(data) + value];
          uint8_t frame_x = tile_x + (flip_h ? 7 - x : x);
          uint8_t frame_y = tile_y + (flip_v ? 7 - y : y);
          frame_set(frame_x, frame_y, color);
        }
      }
    }
  }
}

uint8_t Ppu::sprite_palette_start_idx(uint8_t data_byte) const {
  uint8_t offset = data_byte & 0b11;
  return 0x10 + offset * 4;
}

};  // namespace nesem
