#include <SDL.h>
#include "SDL_DBGP.h"
#include "SDL_DBGP_unscii16.h"
#include "SDL_DBGP_unscii8.h"

#define WIN_WIDTH 512
#define WIN_HEIGHT 342

void screenshot(SDL_Renderer* renderer, const char* filename) {
  int width = 0;
  int height = 0;
  SDL_GetRendererOutputSize(renderer, &width, &height);

  SDL_Surface* screenshot = SDL_CreateRGBSurface(
      0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  SDL_RenderReadPixels(
      renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot->pixels,
      screenshot->pitch);
  SDL_SaveBMP(screenshot, filename);
  SDL_FreeSurface(screenshot);
}

int main(void) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Unable to initialise SDL2: %s", SDL_GetError());
    return 1;
  }
  SDL_SetHint(SDL_HINT_BMP_SAVE_LEGACY_FORMAT, "1");

  SDL_Window* window = SDL_CreateWindow(
      "SDL_DBGP-test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

  DBGP_Font unscii16;
  if (DBGP_OpenFont(
          &unscii16, renderer, DBGP_UNSCII16, sizeof(DBGP_UNSCII16),
          DBGP_UNSCII16_WIDTH, DBGP_UNSCII16_HEIGHT) != 0) {
    SDL_Log("Unable to initialise DBGP_UNSCII16: %s", SDL_GetError());
    return 1;
  }

  DBGP_Font unscii8;
  if (DBGP_OpenFont(
          &unscii8, renderer, DBGP_UNSCII8, sizeof(DBGP_UNSCII8),
          DBGP_UNSCII8_WIDTH, DBGP_UNSCII8_HEIGHT) != 0) {
    SDL_Log("Unable to initialise DBGP_UNSCII8: %s", SDL_GetError());
    return 1;
  }

  char* iso_string =
      SDL_iconv_string("ISO-8859-1", "UTF-8", "Ébène", sizeof("Ébène"));

  int should_quit = 0;
  SDL_Event event;
  while (!should_quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: should_quit = 1; break;

      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_s) {
          screenshot(renderer, "screenshot.bmp");
        }
        break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0x30, 0x30, 0x30, 0xff);
    SDL_RenderClear(renderer);

    DBGP_Print(
        &unscii16, renderer, 40 * 8, 0 * 16, 0x0f,
        "$00    $10    $20    $30    $40    $50    $60    $70");
    DBGP_Print(
        &unscii16, renderer, 40 * 8, 1 * 16, 0x0f,
        "$80    $90    $A0    $B0    $C0    $D0    $E0    $F0");

    DBGP_Print(&unscii16, renderer, 0, 0, 0x5f, "@ SDL_DBGP! @");
    DBGP_Print(
        &unscii16, renderer, 0, 1 * 16, 0x0f,
        "Color can be changed with\n"
        "$09e$0As$0Bc$0Ca$0Dp$0Ee$0F"
        " codes too.");
    DBGP_Print(
        &unscii16, renderer, 0 * 8, 3 * 16, 0x1f, "abcdefghijkl0123456789");
    DBGP_Print(
        &unscii16, renderer, 0 * 8, 4 * 16, 0x3f, "Something $F3somethinG");

    DBGP_Printf(
        &unscii16, renderer, 0, 5 * 16, DBGP_DEFAULT_COLORS,
        "A string with accents: $74%s", iso_string);

    DBGP_Printf(
        &unscii8, renderer, 0, 7 * 16, DBGP_DEFAULT_COLORS,
        "Default fonts include the entire ISO-8859-1 charset:");
    for (int i = 0; i < 256; i++) {
      int x = (8 * 1) + (i % 32) * DBGP_UNSCII8_WIDTH;
      int y = (8 * 16) + (i / 32) * DBGP_UNSCII8_HEIGHT;
      DBGP_Printf(&unscii8, renderer, x, y, DBGP_DEFAULT_COLORS, "%c", i);
    }

    SDL_RenderPresent(renderer);
  }

  SDL_free(iso_string);
  DBGP_CloseFont(&unscii8);
  DBGP_CloseFont(&unscii16);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
