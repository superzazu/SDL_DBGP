# DBGP

A C99 library to display debug text in SDL2 programs (in a VGA-like text mode). Greatly inspired by [bgfx](https://github.com/bkaradzic/bgfx) debug text API (the provided fonts are from the bgfx project). To install, just copy the three files `DBGP.c`, `DBGP.h` and `charset.h` in your project.

![screenshot](screenshot.png)

Example use:

```c
#include <SDL.h>
#include "DBGP.h"

int main(void) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);

  if (DBGP_Init(renderer, DBGP_8X16, sizeof(DBGP_8X16), 8, 16) != 0) {
    SDL_Log("Unable to initialise DBGP: %s", SDL_GetError());
    return 1;
  }

  int should_quit = 0;
  SDL_Event event;
  while (!should_quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: should_quit = 1; break;
      }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0xff);
    SDL_RenderClear(renderer);

    DBGP_Print(renderer, 0, 0, DBGP_DEFAULT_COLORS, "Hello world!");
    DBGP_Printf(renderer, 32, 32, 0x3f, "Hello %s", "Bobby");

    SDL_RenderPresent(renderer);
  }

  DBGP_Quit();
  SDL_Quit();
  return 0;
}
```

Checkout the example program `example.c`, and build the docs by running `doxygen Doxyfile` in this directory.
