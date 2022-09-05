#include <SDL.h>
#include <fmt/core.h>

#include <cstdio>
#include <fstream>

#include "cartridge.h"
#include "cpu.h"
#include "mmu.h"
#include "render.h"
#include "trace.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    fmt::print("One argument required: name of .nes file to run\n");
  }
  std::string nes_file_path = argv[1];
  std::fstream fs{nes_file_path};
  nesem::Cartridge cartridge;
  try {
    cartridge = nesem::load_ines_rom_dump(&fs);
  } catch (const std::exception &e) {
    fmt::print(stderr, "Failed to load ines file {}: {}\n", nes_file_path,
               e.what());
    return 1;
  }
  nesem::Nes nes{cartridge};
  nes.cpu.reset();

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("nesem",                  // title
                                        SDL_WINDOWPOS_UNDEFINED,  // x
                                        SDL_WINDOWPOS_UNDEFINED,  // y
                                        256 * 2,                  // w
                                        256 * 2,                  // h
                                        SDL_WINDOW_SHOWN          // flags
  );
  if (window == NULL) {
    const char *err = SDL_GetError();
    fmt::print("Create window failed: {}\n", err);
    return 1;
  }

  nesem::RenderContext render_ctx;
  if (nesem::init_render_context(&render_ctx, window)) {
    const char *err = SDL_GetError();
    fmt::print("Init render context failed: {}\n", err);
    return 1;
  }

  uint8_t scanline = nes.mmu.ppu.scanline;

  for (;;) {
    // fmt::print("{}\n", nesem::trace_explain_state(nes));
    nes.step();
    if (nes.mmu.ppu.scanline < scanline) {
      nesem::render(&render_ctx, nes.mmu.ppu);

      SDL_Event e;
      if (SDL_PollEvent(&e)) {
        switch (e.type) {
          case SDL_QUIT:
            exit(0);
          case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    exit(0);
                case SDLK_a:
                    nes.mmu.gamepad.btn_a = true;
                    break;
                case SDLK_b:
                    nes.mmu.gamepad.btn_b = true;
                    break;
                case SDLK_RETURN:
                    nes.mmu.gamepad.btn_start = true;
                    break;
                case SDLK_UP:
                    nes.mmu.gamepad.btn_up = true;
                    break;
                case SDLK_DOWN:
                    nes.mmu.gamepad.btn_down = true;
                    break;
                case SDLK_LEFT:
                    nes.mmu.gamepad.btn_left = true;
                    break;
                case SDLK_RIGHT:
                    nes.mmu.gamepad.btn_right = true;
                    break;
            }
            break;
          case SDL_KEYUP:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    exit(0);
                case SDLK_a:
                    nes.mmu.gamepad.btn_a = false;
                    break;
                case SDLK_b:
                    nes.mmu.gamepad.btn_b = false;
                    break;
                case SDLK_RETURN:
                    nes.mmu.gamepad.btn_start = false;
                    break;
                case SDLK_UP:
                    nes.mmu.gamepad.btn_up = false;
                    break;
                case SDLK_DOWN:
                    nes.mmu.gamepad.btn_down = false;
                    break;
                case SDLK_LEFT:
                    nes.mmu.gamepad.btn_left = false;
                    break;
                case SDLK_RIGHT:
                    nes.mmu.gamepad.btn_right = false;
                    break;
            }
            break;
        }
      }
    }
    scanline = nes.mmu.ppu.scanline;
  }
}
