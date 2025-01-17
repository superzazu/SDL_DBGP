/** \file SDL_DBGP.h */

/**
 * \mainpage SDL_DBGP
 *
 * SDL_DBGP (DeBuG Print) is a C99 library to display UTF-8 text in SDL3
 * programs (in a VGA-like text mode), greatly inspired by
 * [bgfx](https://github.com/bkaradzic/bgfx) debug text API. To install, copy
 * DBGP.c/.h and a font file (SDL_DBGP_unscii8.h or SDL_DBGP_unscii16.h) in your
 * project.
 *
 * Two fonts are provided for convenience:
 * [UNSCII-8](https://github.com/viznut/unscii) (8x8px) and UNSCII-16 (8x16px).
 * They both are in the public domain ; and include glyphs for the first 256
 * Unicode codepoints. The Python script used to generate C header files from
 * UNSCII `.hex` files is also available (unscii2raw.py). Checkout the file
 * "example.c" for a full example, or jump right in:
 *
 * \sa DBGP_CreateFont
 * \sa DBGP_DestroyFont
 * \sa DBGP_Print
 * \sa DBGP_Printf
 * \sa DBGP_ColorPrint
 * \sa DBGP_ColorPrintf
 *
 */

#ifndef DBGP_DBGP_H
#define DBGP_DBGP_H

#include <stdbool.h>
#include <SDL3/SDL.h>

/** The size of the internal text formatting buffer used by DBGP_Printf */
#ifndef DBGP_MAX_STR_LEN
#define DBGP_MAX_STR_LEN 4096
#endif

/** For "DBGP_Color*" functions only. Whether color escapes codes such as "$F0"
 * should be parsed. */
#ifndef DBGP_ENABLE_ESCAPING
#define DBGP_ENABLE_ESCAPING 1
#endif

/** For "DBGP_Color*" functions only. The escape character used for changing
 * colors in the middle of a string. By default, it's "$", meaning that "$0F"
 * will print in white on a transparent background */
#ifndef DBGP_ESCAPE_CHAR
#define DBGP_ESCAPE_CHAR '$'
#endif

/** For "DBGP_Color*" functions only. The default color to draw in (white on
 * transparent background) */
#define DBGP_DEFAULT_COLORS 0x0f

/**
 * \struct DBGP_Font
 * \brief Represents a font that will be used to display glyphes on screen.
 *
 * This struct should be considered read-only.
 *
 * \sa DBGP_CreateFont
 */
struct DBGP_Font {
  Uint8 glyph_width; /**< the width in pixels of each glyph. Always 8. */
  Uint8 glyph_height; /**< the height in pixels of each glyph */
  unsigned int nb_glyphs; /**< the number of glyphs in font */
  SDL_Texture* tex; /**< texture used when drawing text */
};
typedef struct DBGP_Font DBGP_Font; /**< Convenience typedef */

/**
 * \fn bool DBGP_CreateFont(DBGP_Font* font, SDL_Renderer* renderer,
 * const unsigned char* const raw_data, size_t raw_data_len, Uint8 glyph_height)
 * \brief Creates a font object from raw data.
 *
 * For convenience, DBGP provides two fonts: UNSCII-8 (8x8px) and UNSCII-16
 * (8x16px). To use the 8x16px one, use:
 *
 * \code
 * DBGP_Font font;
 * DBGP_CreateFont(&font, renderer, DBGP_UNSCII16, sizeof(DBGP_UNSCII16),
 *   DBGP_UNSCII16_HEIGHT)
 * \endcode
 *
 * \param font The font to draw with
 * \param renderer The rendering context
 * \param raw_data A pointer to the font raw data
 * \param raw_data_len The size in bytes of raw_data
 * \param glyph_height the height in pixels of one glyph (character)
 * \return true on success or false on failure; call SDL_GetError() for more
 * information.
 *
 * \sa DBGP_DestroyFont
 */
bool DBGP_CreateFont(
    DBGP_Font* font, SDL_Renderer* renderer,
    const unsigned char* const raw_data, size_t raw_data_len,
    Uint8 glyph_height);

/**
 * \fn void DBGP_DestroyFont(DBGP_Font* font)
 * \brief Frees all memory allocated during DBGP_CreateFont.
 *
 * \sa DBGP_CreateFont
 */
void DBGP_DestroyFont(DBGP_Font* font);

/**
 * \fn bool DBGP_ColorPrint(DBGP_Font* font, SDL_Renderer* renderer, int x,
 * int y, Uint8 colors, const char* str)
 * \brief Draws some text on a renderer. String must be UTF-8 encoded and NULL
 * terminated.
 *
 * The colors parameter is an unsigned byte, where the four least significant
 * bits represent the foreground code, and the four most significant bits
 * represent the background color.
 * Note that background color of 0 will result in a transparent background.
 *
 * For example, to display "Hello world" in white at (10, 50):
 * \code
 * DBGP_ColorPrint(font, renderer, 10, 50, 0xF, "Hello world")
 * \endcode
 *
 * This function supports color escape codes to change colors in the middle of
 * a string. These should be in the form `$f0`,
 * where "f" is the background color, and "0" is the foreground color. This
 * feature can be disabled by setting DBGP_ENABLE_ESCAPING to 0.
 *
 * \param font The font to draw with
 * \param renderer The rendering context
 * \param x The X coordinate of the text
 * \param y The Y coordinate of the text
 * \param colors The colors that will be used to draw (4 most significant
 * bits correspond to the background color, 4 least significant bits correspond
 * to the foreground color)
 * \param str The text to draw. Must be UTF-8 encoded and NULL terminated.
 * \return true on success or false on failure; call SDL_GetError() for more
 * information.
 *
 * \sa DBGP_ColorPrintf
 */
bool DBGP_ColorPrint(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* str);

/**
 * \fn bool DBGP_ColorPrintf(DBGP_Font* font, SDL_Renderer* renderer, int x,
 * int y, Uint8 colors, const char* fmt, ...)
 * \brief Formats and draws some text on a renderer. String must be UTF-8
 * encoded and NULL terminated.
 *
 * Same as DBGP_ColorPrint, but formats the string `fmt` with variable arguments
 * first. The maximum output size is fixed and can be changed by defining
 * `DBGP_MAX_STR_LEN` before including DBGP.h. See DBGP_ColorPrint documentation
 * for more information about the other parameters.
 *
 * \param font The font to draw with
 * \param renderer The rendering context
 * \param x The X coordinate of the text
 * \param y The Y coordinate of the text
 * \param colors The colors that will be used to draw (4 upper bits
 correspond to the background color, 4 lower bits correspond to the foreground
 color)
 * \param fmt The string to format. Must be UTF-8 encoded and NULL terminated.
 * \param ... Variable arguments to format the string with
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \sa DBGP_ColorPrint
 */
bool DBGP_ColorPrintf(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, Uint8 colors,
    const char* fmt, ...);

/**
 * \fn bool DBGP_Print(DBGP_Font* font, SDL_Renderer* renderer, int x,
 * int y, SDL_Color bg_color, SDL_Color fg_color, const char* str)
 * \brief Draws some text on a renderer. String must be UTF-8 encoded and NULL
 * terminated.
 *
 * \param font The font to draw with
 * \param renderer The rendering context
 * \param x The X coordinate of the text
 * \param y The Y coordinate of the text
 * \param bg_color Background color
 * \param fg_color Foreground (text) color
 * \param str The text to draw. Must be UTF-8 encoded and NULL terminated.
 * \return true on success or false on failure; call SDL_GetError() for more
 * information.
 *
 * \sa DBGP_Printf
 */
bool DBGP_Print(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, SDL_Color bg_color,
    SDL_Color fg_color, const char* str);

/**
 * \fn bool DBGP_Printf(DBGP_Font* font, SDL_Renderer* renderer, int x,
 * int y, SDL_Color bg_color, SDL_Color fg_color, const char* fmt, ...)
 * \brief Formats and draws some text on a renderer. String must be UTF-8
 * encoded and NULL terminated.
 *
 * Same as DBGP_Print, but formats the string `fmt` with variable arguments
 * first. The maximum output size is fixed and can be changed by defining
 * `DBGP_MAX_STR_LEN` before including DBGP.h. See DBGP_Print documentation
 * for more information about the other parameters.
 *
 * \param font The font to draw with
 * \param renderer The rendering context
 * \param x The X coordinate of the text
 * \param y The Y coordinate of the text
 * \param bg_color Background color
 * \param fg_color Foreground (text) color
 * \param fmt The string to format. Must be UTF-8 encoded and NULL terminated.
 * \param ... Variable arguments to format the string with
 * \return true on success or false on failure; call SDL_GetError() for more
 * information.
 *
 * \sa DBGP_Print
 */
bool DBGP_Printf(
    DBGP_Font* font, SDL_Renderer* renderer, int x, int y, SDL_Color bg_color,
    SDL_Color fg_color, const char* fmt, ...);

#endif // DBGP_DBGP_H
