#include <SDL.h>
#include "DBGP.h"

#define WIN_WIDTH 512
#define WIN_HEIGHT 342

int main(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialise SDL2: %s", SDL_GetError());
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow(
      "DBGP-test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_WIDTH,
      WIN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if (window == NULL) {
    SDL_Log("Unable to create window: %s", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    SDL_Log("unable to create renderer: %s", SDL_GetError());
    return 1;
  }
  SDL_RenderSetLogicalSize(renderer, WIN_WIDTH, WIN_HEIGHT);

  if (DBGP_Init(renderer, DBGP_8X16, sizeof(DBGP_8X16), 8, 16) != 0) {
    SDL_Log("Unable to initialise DBGP: %s", SDL_GetError());
    return 1;
  }

  char* cp437_string =
      SDL_iconv_string("CP437", "UTF-8", "Ébène", sizeof("Ébène"));

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

    DBGP_Print(
        renderer, 40 * 8, 0 * 16, 0x0f,
        "\x1b[;0m    \x1b[;1m    \x1b[;2m    \x1b[;3m    \x1b[;4m    \x1b[;"
        "5m    \x1b[;6m    \x1b[;7m    \x1b[0m");
    DBGP_Print(
        renderer, 40 * 8, 1 * 16, 0x0f,
        "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    "
        "\x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

    DBGP_Print(renderer, 0, 0, 0x5f, "\x01 DBGP for SDL2! \x02");
    DBGP_Print(
        renderer, 0, 1 * 16, 0x0f,
        "Color can be changed with ANSI-like\n"
        "\x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m"
        " codes too.");
    DBGP_Print(renderer, 0 * 8, 3 * 16, 0x1f, "abcdefghijkl0123456789");
    DBGP_Print(
        renderer, 0 * 8, 4 * 16, 0x3f, "Something \x1b[3;15msomethinG\x1b[0m");

    DBGP_Printf(
        renderer, 0, 5 * 16, DBGP_DEFAULT_COLORS,
        "A string with accents: \x1b[4;7m%s\x1b[0m", cp437_string);

    SDL_RenderPresent(renderer);
  }

  SDL_free(cp437_string);
  DBGP_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
