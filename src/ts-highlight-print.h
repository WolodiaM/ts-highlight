//! Markdown->ANSI converter using tree-sitter
//!
//! License: GPL-3.0-or-later
#ifndef __TSHL_PRINT_H__
#define __TSHL_PRINT_H__
// Includes
#include "../cbuild.h"
#include "ts-highlight.h"
/// Color definition, plain RGB888
struct tshl_color_t {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
// Theme entry, contain all metadata to render character with specific style.
#define TSHL_THEME_UNDEFINED 0b00
#define TSHL_THEME_TRUE 0b11
#define TSHL_THEME_FALSE 0b01
struct tshl_theme_entry_t {
	struct tshl_color_t bg;
	struct tshl_color_t fg;
	union {
		struct {
			uint16_t reverse       : 2;
			uint16_t bold          : 2;
			uint16_t italic        : 2;
			uint16_t underline     : 2;
			uint16_t strikethrough : 2;
			uint16_t hasbg         : 1;
			uint16_t nofg          : 1;
			uint16_t               : 4;
		};
		uint16_t flags;
	};
};
// Theme. Each element correspond to style from `enum tshl_style_t`.
typedef struct tshl_theme_entry_t tshl_theme_t[TSHL_NUM_STYLES];
/// "Printer" interface.
typedef struct tshl_printer_t {
	void* ctx;
	bool (*writechar)(char c, void* ctx);
	bool (*writecstr)(const char* str, void* ctx);
} tshl_printer_t;
/// Convert tshl metadata to ANSI codes.
bool tshl_meta_to_ansi(tshl_metadata_t* meta, cbuild_sv_t text, tshl_theme_t theme,
	struct tshl_printer_t printer);
#endif // __TSHL_PRINT_H__
