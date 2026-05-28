// ts-highlight-print.h by WolodiaM
// License: GPL-3.0-or-later
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

#ifndef TSHL_PRINT_IMPLEMENTATION
#define TSHL_PRINT_IMPLEMENTATION
//! Markdown->ANSI converter using tree-sitter
//!
//! License: GPL-3.0-or-later
// Includes
// #include "ts-highlight-print.h"
static inline bool __tshl_compare_color(struct tshl_color_t c1, struct tshl_color_t c2) {
	return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}
void __tshl_apply_theme_entry(struct tshl_theme_entry_t* curr, struct tshl_theme_entry_t t) {
	if (t.reverse != TSHL_THEME_UNDEFINED) curr->reverse = t.reverse;
	if (t.bold != TSHL_THEME_UNDEFINED) curr->bold = t.bold;
	if (t.italic != TSHL_THEME_UNDEFINED) curr->italic = t.italic;
	if (t.underline != TSHL_THEME_UNDEFINED) curr->underline = t.underline;
	if (t.hasbg) curr->bg = t.bg;
	if (!t.nofg) curr->fg = t.fg;
}
bool __tshl_write_ansi(struct tshl_theme_entry_t prev, struct tshl_theme_entry_t curr,
	struct tshl_printer_t printer) {
	if (curr.bold != prev.bold) {
		if (curr.bold == TSHL_THEME_TRUE) {
			if (!printer.writecstr(CBUILD_TERM_BOLD, printer.ctx)) return false;
		} else if (curr.bold == TSHL_THEME_FALSE) {
			if (!printer.writecstr(CBUILD_TERM_NBOLD,printer.ctx)) return false;
		}
	}
	if (curr.italic != prev.italic) {
		if (curr.italic == TSHL_THEME_TRUE) {
			if (!printer.writecstr(CBUILD_TERM_ITALIC, printer.ctx)) return false;
		} else if (curr.italic == TSHL_THEME_FALSE) {
			if (!printer.writecstr(CBUILD_TERM_NITALIC,printer.ctx)) return false;
		}
	}
	if (curr.underline != prev.underline) {
		if (curr.underline == TSHL_THEME_TRUE) {
			if (!printer.writecstr(CBUILD_TERM_UNDERLINE, printer.ctx)) return false;
		} else if (curr.underline == TSHL_THEME_FALSE) {
			if (!printer.writecstr(CBUILD_TERM_NUNDERLINE,printer.ctx)) return false;
		}
	}
	if (curr.underline != prev.underline) {
		if (curr.underline == TSHL_THEME_TRUE) {
			if (!printer.writecstr(CBUILD_TERM_REVERSE, printer.ctx)) return false;
		} else if (curr.underline == TSHL_THEME_FALSE) {
			if (!printer.writecstr(CBUILD_TERM_REVERSE,printer.ctx)) return false;
		}
	}
	if (curr.strikethrough != prev.strikethrough) {
		if (curr.strikethrough == TSHL_THEME_TRUE) {
			if (!printer.writecstr(CBUILD_TERM_SGR("9"), printer.ctx)) return false;
		} else if (curr.strikethrough == TSHL_THEME_FALSE) {
			if (!printer.writecstr(CBUILD_TERM_SGR("29"),printer.ctx)) return false;
		}
	}
	if (prev.hasbg == curr.hasbg && __tshl_compare_color(prev.bg, curr.bg)) {
		// Skip this
	} else {
		if (!curr.hasbg) {
			if (!printer.writecstr(CBUILD_TERM_SGR("49"), printer.ctx)) return false;
		} else {
			const char* bg = cbuild_temp_sprintf(CBUILD_TERM_SGR("48;2;%u;%u;%u"),
				curr.bg.r, curr.bg.g, curr.bg.b);
			if (!printer.writecstr(bg, printer.ctx)) return false;
		}
	}
	if (prev.nofg == curr.nofg && __tshl_compare_color(prev.fg, curr.fg)) {
		// Skip this
	} else {
		if (curr.nofg) {
			if (!printer.writecstr(CBUILD_TERM_SGR("39"), printer.ctx)) return false;
		} else {
			const char* fg = cbuild_temp_sprintf(CBUILD_TERM_SGR("38;2;%u;%u;%u"),
				curr.fg.r, curr.fg.g, curr.fg.b);
			if (!printer.writecstr(fg, printer.ctx)) return false;
		}
	}
	return true;
}
bool tshl_meta_to_ansi(tshl_metadata_t* metadata, cbuild_sv_t text, tshl_theme_t theme,
	struct tshl_printer_t printer) {
	struct tshl_theme_entry_t prev = theme[TSHL_DEFAULT];
	tshl_metadata_t prev_meta = {0};
	struct tshl_theme_entry_t curr = {0};
	size_t checkpoint = cbuild_temp_checkpoint();
	for (size_t i = 0; i < text.size; i++) {
		curr = theme[TSHL_DEFAULT];
		tshl_metadata_t meta = metadata[i];
		// TODO: Handle 'flags' and 'conceal' when they actually become used by parser
		if (memcmp(&prev_meta, &meta, sizeof(tshl_metadata_t)) != 0) {
			for (size_t j = 0; j < 30; j++) {
				if (meta.style[j] == TSHL_DEFAULT) continue;
				struct tshl_theme_entry_t t = theme[meta.style[j]];
				__tshl_apply_theme_entry(&curr, t);
			}
			if (curr.reverse == TSHL_THEME_UNDEFINED) curr.reverse = TSHL_THEME_FALSE;
			if (curr.bold == TSHL_THEME_UNDEFINED) curr.bold = TSHL_THEME_FALSE;
			if (curr.italic == TSHL_THEME_UNDEFINED) curr.italic = TSHL_THEME_FALSE;
			if (curr.underline == TSHL_THEME_UNDEFINED) curr.underline = TSHL_THEME_FALSE;
			if (curr.strikethrough == TSHL_THEME_UNDEFINED) curr.strikethrough = TSHL_THEME_FALSE;
			if (!__tshl_write_ansi(prev, curr, printer)) {
				cbuild_temp_reset(checkpoint);
				return false;
			}
		}
		if (!printer.writechar(text.data[i], printer.ctx)) {
			cbuild_temp_reset(checkpoint);
			return false;
		}
		prev = curr;
		prev_meta = meta;
		cbuild_temp_reset(checkpoint);
	}
	if (!printer.writecstr(CBUILD_TERM_RESET, printer.ctx)) return false;
	return true;
}
#endif // TSHL_PRINT_IMPLEMENTATION