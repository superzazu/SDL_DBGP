#include <stdarg.h>
#include "SDL_DBGP.h"

#define GLYPHS_PER_LINE (256 / 8)

int DBGP_OpenFont(
    DBGP_Font* font, SDL_Renderer* renderer,
    const unsigned char* const raw_data, size_t raw_data_len, Uint8 glyph_width,
    Uint8 glyph_height) {
  if (font == NULL) {
    return -1;
  }

  font->glyph_width = glyph_width;
  font->glyph_height = glyph_height;
  font->nb_glyphs = raw_data_len / font->glyph_height;
  font->tex = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      GLYPHS_PER_LINE * glyph_width,
      font->nb_glyphs / GLYPHS_PER_LINE * glyph_height);
  if (font->tex == NULL) {
    return -1;
  }
  if (SDL_SetTextureBlendMode(font->tex, SDL_BLENDMODE_BLEND) != 0) {
    SDL_Log("Error while setting blend mode: %s", SDL_GetError());
  }

  SDL_Texture* target = SDL_GetRenderTarget(renderer);
  Uint8 r = 0, g = 0, b = 0, a = 0;
  if (SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a) != 0) {
    SDL_Log("Error while getting renderer draw color: %s", SDL_GetError());
  }

  // draw all glyphs on texture once
  SDL_SetRenderTarget(renderer, font->tex);
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
  for (unsigned int i = 0; i < font->nb_glyphs; i++) {
    const int y = i / GLYPHS_PER_LINE;
    const int x = i % GLYPHS_PER_LINE;

    const unsigned char* ptr =
        &raw_data[i * font->glyph_width * font->glyph_height / 8];
    for (int gy = 0; gy < font->glyph_height; gy++) {
      for (int gx = 0; gx < font->glyph_width; gx++) {
        SDL_bool pixel = (*ptr >> (7 - gx)) & 1;
        if (pixel) {
          SDL_RenderDrawPoint(
              renderer, x * font->glyph_width + gx,
              y * font->glyph_height + gy);
        }
      }
      ptr += 1;
    }
  }

  SDL_SetRenderTarget(renderer, target);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  return 0;
}

void DBGP_CloseFont(DBGP_Font* font) {
  if (font == NULL) {
    return;
  }
  if (font->tex != NULL) {
    SDL_DestroyTexture(font->tex);
    font->tex = NULL;
  }
}

static inline SDL_bool is_hex(char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

static inline Uint8 get_hex_value(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return 0;
}

// CGA 16-color palette
static const Uint32 color_palette[16] = {
    0x000000, 0x0000aa, 0x00aa00, 0x00aaaa, 0xaa0000, 0xaa00aa,
    0xaa5500, 0xaaaaaa, 0x555555, 0x5555ff, 0x55ff55, 0x55ffff,
    0xff5555, 0xff55ff, 0xffff55, 0xffffff,
};

int DBGP_Print(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* str) {
  if (font == NULL || font->tex == NULL || renderer == NULL) {
    return -1;
  }

  Uint8 r = 0, g = 0, b = 0, a = 0;
  if (SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a) != 0) {
    SDL_Log("Error while getting renderer draw color: %s", SDL_GetError());
  }

  for (int pass = 0; pass < 2; pass++) {
    int ix = x;
    int iy = y;
    Uint8 icolors = colors;

    const char* ptr = str;
    while (*ptr != '\0') {
      if (*ptr == DBGP_ESCAPE_CHAR && is_hex(*(ptr + 1)) &&
          is_hex(*(ptr + 2)) && DBGP_ENABLE_ESCAPING) {
        Uint8 bg = get_hex_value(*(ptr + 1));
        Uint8 fg = get_hex_value(*(ptr + 2));
        icolors = (bg & 0xf) << 4 | (fg & 0xf);
        ptr += 2;
      } else if (*ptr == '\n') {
        iy += font->glyph_height;
        ix = x;
      } else {
        SDL_Rect r = {ix, iy, font->glyph_width, font->glyph_height};

        if (pass == 0) {
          // background
          Uint32 bg_color = color_palette[icolors >> 4];
          if (bg_color != 0) {
            SDL_SetRenderDrawColor(
                renderer, (bg_color >> 16) & 0xff, (bg_color >> 8) & 0xff,
                bg_color & 0xff, 0xff);
            SDL_RenderFillRect(renderer, &r);
          }
        } else {
          // foreground
          Uint8 i = *ptr;
          SDL_Rect src = {
              i % GLYPHS_PER_LINE * font->glyph_width,
              i / GLYPHS_PER_LINE * font->glyph_height, font->glyph_width,
              font->glyph_height};
          Uint32 fg_color = color_palette[icolors & 0xf];
          SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
          SDL_SetTextureColorMod(
              font->tex, (fg_color >> 16) & 0xff, (fg_color >> 8) & 0xff,
              fg_color & 0xff);
          SDL_RenderCopy(renderer, font->tex, &src, &r);
        }

        ix += font->glyph_width;
      }

      ptr += 1;
    }
  }

  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  return 0;
}

static char printf_buffer[DBGP_MAX_STR_LEN];

int DBGP_Printf(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  SDL_vsnprintf(printf_buffer, DBGP_MAX_STR_LEN, fmt, args);
  va_end(args);

  return DBGP_Print(font, renderer, x, y, colors, printf_buffer);
}

#undef GLYPHS_PER_LINE
