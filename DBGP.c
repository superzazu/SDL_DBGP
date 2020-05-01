#include "DBGP.h"

#define GLYPHS_PER_LINE (256 / 8)

// CGA 16-color palette
static const Uint32 color_palette[16] = {
    0x000000, 0x0000aa, 0x00aa00, 0x00aaaa, 0xaa0000, 0xaa00aa,
    0xaa5500, 0xaaaaaa, 0x555555, 0x5555ff, 0x55ff55, 0x55ffff,
    0xff5555, 0xff55ff, 0xffff55, 0xffffff,
};
static struct DBGP_Font font = {0};
static char printf_buffer[DBGP_MAX_STR_LEN];

int DBGP_Init(
    SDL_Renderer* renderer, const Uint8* const raw_data, size_t raw_data_len,
    Uint8 glyph_width, Uint8 glyph_height) {
  if (font.tex != NULL) {
    DBGP_Quit();
  }

  font.glyph_width = glyph_width;
  font.glyph_height = glyph_height;
  font.nb_glyphs = raw_data_len / font.glyph_height;
  font.tex = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
      GLYPHS_PER_LINE * glyph_width,
      font.nb_glyphs / GLYPHS_PER_LINE * glyph_height);
  if (font.tex == NULL) {
    return -1;
  }
  if (SDL_SetTextureBlendMode(font.tex, SDL_BLENDMODE_BLEND) != 0) {
    SDL_Log("Error while setting blend mode: %s", SDL_GetError());
  }

  // draw all glyphs on texture once
  SDL_SetRenderTarget(renderer, font.tex);
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
  for (Uint16 i = 0; i < font.nb_glyphs; i++) {
    const int y = i / GLYPHS_PER_LINE;
    const int x = i % GLYPHS_PER_LINE;

    const Uint8* ptr = &raw_data[i * font.glyph_width * font.glyph_height / 8];
    for (int gy = 0; gy < font.glyph_height; gy++) {
      for (int gx = 0; gx < font.glyph_width; gx++) {
        SDL_bool pixel = (*ptr >> (7 - gx)) & 1;
        if (pixel) {
          SDL_RenderDrawPoint(
              renderer, x * font.glyph_width + gx, y * font.glyph_height + gy);
        }
      }
      ptr += 1;
    }
  }

  return 0;
}

void DBGP_Quit(void) {
  if (font.tex != NULL) {
    SDL_DestroyTexture(font.tex);
  }
  SDL_memset(&font, 0, sizeof(font));
}

static inline Uint8 parse_escape_code(const char** ptr) {
  *ptr += 2;

  // reset command
  if (**ptr == '0' && *(*ptr + 1) == 'm') {
    *ptr += 1;
    return DBGP_DEFAULT_COLORS;
  }

  char* mid = SDL_strchr(*ptr, ';');
  char* end = SDL_strchr(*ptr, 'm');

  if (mid == NULL || end == NULL) {
    return DBGP_DEFAULT_COLORS;
  }

  long fg = SDL_strtol(*ptr, &mid, 10);
  long bg = SDL_strtol((mid + 1), &end, 10);

  *ptr = end;
  return (bg & 0xf) << 4 | (fg & 0xf);
}

int DBGP_Printf(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* fmt,
    ...) {
  va_list ap;
  va_start(ap, fmt);
  SDL_vsnprintf(printf_buffer, DBGP_MAX_STR_LEN, fmt, ap);
  va_end(ap);

  int ret = DBGP_Print(renderer, _x, _y, _colors, printf_buffer);
  return ret;
}

int DBGP_Print(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* str) {
  if (font.tex == NULL) {
    return -1;
  }

  for (int pass = 0; pass < 2; pass++) {
    int x = _x;
    int y = _y;
    Uint8 colors = _colors;

    const char* ptr = str;
    while (*ptr != '\0') {
      if (*ptr == '\x1b' && *(ptr + 1) == '[') {
        colors = parse_escape_code(&ptr);
      } else if (*ptr == '\n') {
        y += font.glyph_height;
        x = _x;
      } else {
        SDL_Rect r = {x, y, font.glyph_width, font.glyph_height};

        if (pass == 0) {
          // background
          const Uint32 bg_color = color_palette[colors >> 4];
          if (bg_color != 0) {
            SDL_SetRenderDrawColor(
                renderer, (bg_color >> 16) & 0xff, (bg_color >> 8) & 0xff,
                bg_color & 0xff, 0xff);
            SDL_RenderFillRect(renderer, &r);
          }
        } else {
          // foreground
          Uint8 i = *ptr;
          SDL_Rect src = {i % GLYPHS_PER_LINE * font.glyph_width,
                          i / GLYPHS_PER_LINE * font.glyph_height,
                          font.glyph_width, font.glyph_height};
          const Uint32 fg_color = color_palette[colors & 0xf];
          SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
          SDL_SetTextureColorMod(
              font.tex, (fg_color >> 16) & 0xff, (fg_color >> 8) & 0xff,
              fg_color & 0xff);
          SDL_RenderCopy(renderer, font.tex, &src, &r);
        }

        x += font.glyph_width;
      }

      ptr += 1;
    }
  }

  return 0;
}

#undef GLYPHS_PER_LINE
