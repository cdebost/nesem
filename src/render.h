#pragma once

#include "SDL.h"
#include "ppu.h"

namespace nesem {

struct RenderContext {
  SDL_Renderer *renderer;
  SDL_Texture *texture;

  ~RenderContext();
};

// Initialize a fresh render context.
// Returns 0 on success and -1 on an SDL error.
int init_render_context(RenderContext *ctx, SDL_Window *window);

void render(RenderContext *ctx, const Ppu &ppu);

}  // namespace nesem
