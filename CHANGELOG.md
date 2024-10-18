# CHANGELOG

## v5

- SDL3 support
- Print functions now expect strings encoded in UTF-8 (instead of latin-1)
- improved `DBGP_CreateFont()` performance
- `DBGP_CloseFont()` renamed to `DBGP_DestroyFont()`
- `DBGP_OpenFont()` renamed to `DBGP_CreateFont()`
- `DBGP_CloseFont()` and `DBGP_OpenFont()` now return booleans
- `DBGP_DestroyFont()` also resets to "0" `glyph_width`, `glyph_height` and `nb_glyphs`
- Update docs & API to make it clear that only fonts with 8px width are currently supported (eg `DBGP_CreateFont()` does not have `glyph_width` as parameter anymore)
