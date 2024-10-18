# SDL_DBGP

SDL_DBGP (DeBuG Print) is a C99 library to display UTF-8 text in SDL3 programs (in a VGA-like text mode), greatly inspired by [bgfx](https://github.com/bkaradzic/bgfx) debug text API. To install, copy DBGP.c/.h and a font file (SDL_DBGP_unscii8.h or SDL_DBGP_unscii16.h) in your project.

Two fonts are provided for convenience: [UNSCII-8](https://github.com/viznut/unscii) (8x8px) and UNSCII-16 (8x16px). They both are in the public domain ; and include glyphs for the first 256 Unicode codepoints. The Python script used to generate C header files from UNSCII `.hex` files is also available (unscii2raw.py).

![screenshot](screenshot.png)

Example:

```c
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "SDL_DBGP.h"
#include "SDL_DBGP_unscii16.h"

int main(void) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  SDL_CreateWindowAndRenderer("SDL_DBGP", 800, 600, 0, &window, &renderer);

  DBGP_Font font;
  if (!DBGP_CreateFont(
          &font, renderer, DBGP_UNSCII16, sizeof(DBGP_UNSCII16),
          DBGP_UNSCII16_HEIGHT)) {
    SDL_Log("Unable to initialise DBGP_UNSCII16: %s", SDL_GetError());
    return 1;
  }

  int should_quit = 0;
  SDL_Event event;
  while (!should_quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT: should_quit = 1; break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0xff);
    SDL_RenderClear(renderer);

    DBGP_ColorPrint(&font, renderer, 0, 0, DBGP_DEFAULT_COLORS, "Hello world!");
    DBGP_ColorPrintf(&font, renderer, 32, 32, 0x3f, "Hello %s", "Bobby");
    DBGP_ColorPrint(
        &font, renderer, 0, 64, 0x0f,
        "Color can be changed with\n"
        "$09e$0As$0Bc$0Ca$0Dp$0Ee$0F"
        " codes too.");

    SDL_RenderPresent(renderer);
  }

  DBGP_DestroyFont(&font);
  SDL_Quit();
  return 0;
}
```

Checkout the example program `example.c`, and build the docs by running `doxygen Doxyfile` in this directory.
