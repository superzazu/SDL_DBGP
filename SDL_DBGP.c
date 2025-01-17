#include <stdarg.h>
#include "SDL_DBGP.h"

#define GLYPHS_PER_LINE (256 / 8)
#define GLYPH_WIDTH 8

bool DBGP_CreateFont(
    DBGP_Font* font, SDL_Renderer* renderer,
    const unsigned char* const raw_data, size_t raw_data_len,
    Uint8 glyph_height) {
  if (font == NULL) {
    return false;
  }

  font->glyph_width = GLYPH_WIDTH;
  font->glyph_height = glyph_height;
  font->nb_glyphs = raw_data_len / font->glyph_height;

  SDL_Surface* surface = SDL_CreateSurface(
      GLYPHS_PER_LINE * GLYPH_WIDTH,
      font->nb_glyphs / GLYPHS_PER_LINE * font->glyph_height,
      SDL_PIXELFORMAT_INDEX1LSB);
  if (!surface) {
    return false;
  }
  SDL_Palette* palette = SDL_CreatePalette(2);
  if (!palette) {
    return false;
  }
  SDL_Color colors[2] = {{0, 0, 0, 0}, {255, 255, 255, 255}};
  if (!SDL_SetPaletteColors(palette, colors, 0, 2)) {
    return false;
  }
  if (!SDL_SetSurfacePalette(surface, palette)) {
    return false;
  }

  const int bytes_per_glyph = 1 * font->glyph_height;
  for (size_t i = 0; i < raw_data_len; i++) {
    const unsigned char* ptr = &raw_data[i];
    int glyph_no = i / bytes_per_glyph;
    int glyph_y = (glyph_no / GLYPHS_PER_LINE) * font->glyph_height;
    int glyph_x = (glyph_no % GLYPHS_PER_LINE) * font->glyph_width;
    int byte_no_in_glyph = i % bytes_per_glyph;

    for (int bit = 0; bit < GLYPH_WIDTH; bit++) {
      bool value = (*ptr >> ((GLYPH_WIDTH - 1) - bit)) & 1;

      int x = glyph_x + bit;
      int y = glyph_y + byte_no_in_glyph;

      int position = y * GLYPHS_PER_LINE * GLYPH_WIDTH + x;
      int byte_position = position / 8;
      int byte_bit_no = position % 8;

      Uint8* const target = (Uint8*) surface->pixels + byte_position;
      *target |= value << byte_bit_no;
    }
  }

  font->tex = SDL_CreateTextureFromSurface(renderer, surface);
  if (font->tex == NULL) {
    return false;
  }
  if (!SDL_SetTextureScaleMode(font->tex, SDL_SCALEMODE_NEAREST)) {
    SDL_Log("Error while setting scale mode: %s", SDL_GetError());
  }
  if (!SDL_SetTextureBlendMode(font->tex, SDL_BLENDMODE_BLEND)) {
    SDL_Log("Error while setting blend mode: %s", SDL_GetError());
  }

  SDL_DestroySurface(surface);
  SDL_DestroyPalette(palette);

  return true;
}

void DBGP_DestroyFont(DBGP_Font* font) {
  if (font == NULL) {
    return;
  }
  if (font->tex != NULL) {
    SDL_DestroyTexture(font->tex);
    font->tex = NULL;
  }
  font->glyph_width = 0;
  font->glyph_height = 0;
  font->nb_glyphs = 0;
}

bool DBGP_Print(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, SDL_Color bg_color,
    SDL_Color fg_color, const char* str) {
  if (font == NULL || font->tex == NULL || renderer == NULL) {
    return false;
  }

  Uint8 r = 0, g = 0, b = 0, a = 0;
  if (!SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a)) {
    SDL_Log("Error while getting renderer draw color: %s", SDL_GetError());
  }

  for (int pass = 0; pass < 2; pass++) {
    const char* ptr = str;
    int ix = x;
    int iy = y;

    if (pass == 0) {
      SDL_SetRenderDrawColor(
          renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    } else {
      // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
      SDL_SetTextureAlphaMod(font->tex, fg_color.a);
      SDL_SetTextureColorMod(font->tex, fg_color.r, fg_color.g, fg_color.b);
    }

    Uint32 cp = 0;
    while ((cp = SDL_StepUTF8(&ptr, NULL)) != 0) {
      if (cp >= 256 && cp == SDL_INVALID_UNICODE_CODEPOINT) {
        continue;
      }

      if (cp == '\n') {
        iy += font->glyph_height;
        ix = x;
        continue;
      }
      SDL_FRect r = {ix, iy, font->glyph_width, font->glyph_height};

      if (pass == 0) {
        // background
        SDL_RenderFillRect(renderer, &r);
      } else {
        // foreground
        SDL_FRect src = {
            cp % GLYPHS_PER_LINE * font->glyph_width,
            cp / GLYPHS_PER_LINE * font->glyph_height, font->glyph_width,
            font->glyph_height};
        SDL_RenderTexture(renderer, font->tex, &src, &r);
      }

      ix += font->glyph_width;
    }
  }

  SDL_SetTextureAlphaMod(font->tex, 255);
  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  return true;
}

static char printf_buffer[DBGP_MAX_STR_LEN];

bool DBGP_Printf(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, SDL_Color bg_color,
    SDL_Color fg_color, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  SDL_vsnprintf(printf_buffer, DBGP_MAX_STR_LEN, fmt, args);
  va_end(args);

  return DBGP_Print(font, renderer, x, y, bg_color, fg_color, printf_buffer);
}

static inline bool is_hex(char c) {
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

bool DBGP_ColorPrint(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* str) {
  if (font == NULL || font->tex == NULL || renderer == NULL) {
    return false;
  }

  Uint8 r = 0, g = 0, b = 0, a = 0;
  if (!SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a)) {
    SDL_Log("Error while getting renderer draw color: %s", SDL_GetError());
  }

  for (int pass = 0; pass < 2; pass++) {
    const char* ptr = str;
    int ix = x;
    int iy = y;
    Uint8 icolors = colors;

    Uint32 cp = 0;
    while ((cp = SDL_StepUTF8(&ptr, NULL)) != 0) {
      if (cp >= 256 && cp == SDL_INVALID_UNICODE_CODEPOINT) {
        continue;
      }

      if (cp == DBGP_ESCAPE_CHAR && DBGP_ENABLE_ESCAPING) {
        const char* ptr_start_seq = ptr;
        Uint32 cp1 = SDL_StepUTF8(&ptr, NULL);
        Uint32 cp2 = SDL_StepUTF8(&ptr, NULL);

        if (is_hex(cp1) && is_hex(cp2)) {
          Uint8 bg = get_hex_value(cp1);
          Uint8 fg = get_hex_value(cp2);
          icolors = (bg & 0xf) << 4 | (fg & 0xf);
          continue;
        } else {
          ptr = ptr_start_seq;
        }
      }

      if (cp == '\n') {
        iy += font->glyph_height;
        ix = x;
      } else {
        SDL_FRect r = {ix, iy, font->glyph_width, font->glyph_height};

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
          SDL_FRect src = {
              cp % GLYPHS_PER_LINE * font->glyph_width,
              cp / GLYPHS_PER_LINE * font->glyph_height, font->glyph_width,
              font->glyph_height};
          Uint32 fg_color = color_palette[icolors & 0xf];
          // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
          SDL_SetTextureColorMod(
              font->tex, (fg_color >> 16) & 0xff, (fg_color >> 8) & 0xff,
              fg_color & 0xff);
          SDL_RenderTexture(renderer, font->tex, &src, &r);
        }

        ix += font->glyph_width;
      }
    }
  }

  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  return true;
}

bool DBGP_ColorPrintf(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  SDL_vsnprintf(printf_buffer, DBGP_MAX_STR_LEN, fmt, args);
  va_end(args);

  return DBGP_ColorPrint(font, renderer, x, y, colors, printf_buffer);
}

#undef GLYPH_WIDTH
#undef GLYPHS_PER_LINE
