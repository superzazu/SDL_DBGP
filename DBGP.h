/** \file DBGP.h */

/**
 * \mainpage DBGP
 *
 * DBGP is a C99 library to display debug text in
 * [SDL2](https://www.libsdl.org/) programs (in a VGA-like text mode).
 * Just copy DBGP.c/.h and charset.h in your project, and you're good to go.
 *
 * Checkout the file "example.c" for a full example, or jump right in:
 *
 * \sa DBGP_Init
 * \sa DBGP_Print
 * \sa DBGP_Printf
 * \sa DBGP_Quit
 *
 */

#ifndef DBGP_DBGP_H
#define DBGP_DBGP_H

#include <stdarg.h>
#include <SDL.h>
#include "charset.h"

/** The max string size when using DBGP_Printf */
#ifndef DBGP_MAX_STR_LEN
#define DBGP_MAX_STR_LEN SDL_MAX_LOG_MESSAGE
#endif

/** DBGP 8x8 pixel font (CP437-encoded), can be used as second argument of
 * DBGP_Init */
#define DBGP_8X8 vga8x8
/** DBGP 8x16 pixel font (CP437-encoded), can be used as second argument of
 * DBGP_Init */
#define DBGP_8X16 vga8x16
/** The default color to draw in (white on transparent background) */
#define DBGP_DEFAULT_COLORS (0x0 << 4 | 0xf)

/**
 * \struct DBGP_Font
 * \brief Internal, represents a font that will be used to display characters on
 * screen
 *
 * As a user you shouldn't use this struct, use DBGP_Init directly.
 *
 * \sa DBGP_Init
 */
struct DBGP_Font {
  Uint8 glyph_width; /** the width in pixels of each glyph (character) */
  Uint8 glyph_height; /** the height in pixels of each glyph (character) */
  int nb_glyphs; /** the number of glyphs in font */
  SDL_Texture* tex; /** texture used when drawing text */
};

/**
 * \fn int DBGP_Init(
    SDL_Renderer* renderer, const Uint8* const raw_data, size_t raw_data_len,
    Uint8 glyph_width, Uint8 glyph_height)
 * \brief Initialises DBGP by loading a font.
 *
 * This should be called once before any other DBGP call. DBGP provides two
 * CP437-encoded fonts (8x8 and 8x16). To use the 8x16 one, use:
 *
 * \code
 * DBGP_Init(renderer, DBGP_8X16, sizeof(DBGP_8X16), 8, 16)
 * \endcode
 *
 *    \param renderer The SDL2 renderer object that will be used to render
 *    \param raw_data A pointer to the font raw data
 *    \param raw_data_len The size in bytes of raw_data
 *    \param glyph_width the width in pixels of one glyph (character)
 *    \param glyph_height the height in pixels of one glyph (character)
 *   \return zero on success, -1 on error. You can retrieve the error message
 *           with a call to SDL_GetError()
 *
 * \sa DBGP_Quit
 */
int DBGP_Init(
    SDL_Renderer* renderer, const Uint8* const raw_data, size_t raw_data_len,
    Uint8 glyph_width, Uint8 glyph_height);

/**
 * \fn void DBGP_Quit(void)
 * \brief Frees all memory allocated during DBGP_Init.
 *
 * This should be called whenever you want to free memory allocated during
 * DBGP_Init (for example, at the end of your program).
 *
 * \sa DBGP_Init
 */
void DBGP_Quit(void);

/**
 * \fn int DBGP_Print(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* str)
 * \brief Draws some text on a renderer
 *
 * Draws a string `str` on a renderer at coordinates (`_x`,`_y`) with colors
 * `_colors`.
 * The colors parameter is an unsigned byte, where the four low bits
 * represent the foreground code, and the four high bits represent the
 * background
 * color. Note that background color of 0 will result in a transparent
 * background.
 *
 * For example, to display "Hello world" in white at (10, 50):
 * \code
 * DBGP_Print(renderer, 10, 50, 0xF, "Hello world")
 * \endcode
 *
 * This function supports ANSI-like escape code in the form: `\x1b[14;1m`,
 * where "14" (decimal, between 0 and 15) is the foreground color, and "1" is
 * the background color. You can
 * Also reset the colors with `\x1b[0m`.
 *
 *    \param renderer The SDL2 renderer object that will be used to render
 *    \param _x The X coordinate of the text
 *    \param _y The Y coordinate of the text
 *    \param _colors The colors that will be used to draw (4 upper bits
 correspond to the background color, 4 lower bits correspond to the foreground
 color)
 *    \param str The string to display
 *   \return zero on success, -1 on error. You can retrieve the error message
 *           with a call to SDL_GetError()
 *
 * \sa DBGP_Printf
 */
int DBGP_Print(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* str);

/**
 * \fn int DBGP_Printf(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* fmt,
    ...)
 * \brief Formats and draws a string onto the string
 *
 * Same as DBGP_Print, but formats the string `fmt` with variable arguments
 * first. The maximum output size is fixed and can be changed by defining
 * `DBGP_MAX_STR_LEN` before including DBGP.h. See DBGP_Print docs for more info
 * about the arguments to send.
 *
 *    \param renderer The SDL2 renderer object that will be used to render
 *    \param _x The X coordinate of the text
 *    \param _y The Y coordinate of the text
 *    \param _colors The colors that will be used to draw (4 upper bits
 correspond to the background color, 4 lower bits correspond to the foreground
 color)
 *    \param fmt The string to display
 *    \param ... Variable arguments to format the string with
 *   \return zero on success, -1 on error. You can retrieve the error message
 *           with a call to SDL_GetError()
 *
 * \sa DBGP_Print
 */
int DBGP_Printf(
    SDL_Renderer* renderer, int _x, int _y, Uint8 _colors, const char* fmt,
    ...);

#endif // DBGP_DBGP_H
