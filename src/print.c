//! Licese: GPL-3.0-or-later

#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_INFO
#include "../cbuild.h"
#include "stdio.h"
#include "tree_sitter/api.h"
#include "ts-highlight.h"
#include "ts-highlight-print.h"
#define CBUILD_IMPLEMENTATION
#include "../cbuild.h"

TSLanguage* load_parser(cbuild_sv_t language) {
	const char* lib = cbuild_temp_sprintf("%s/.local/share/nvim/site/parser/"CBuildSVFmt".so", getenv("HOME"), CBuildSVArg(language));
	void* handle = cbuild_dlib_open(lib, false);
	if (handle == NULL) return NULL;
	TSLanguage* (*fn)(void) = dlsym(handle, cbuild_temp_sprintf("tree_sitter_"CBuildSVFmt, CBuildSVArg(language)));
	if (fn == NULL) return NULL;
	return fn();
}
const char* get_query_dir(cbuild_sv_t language) {
	return cbuild_temp_sprintf("%s/.local/share/nvim/site/queries/"CBuildSVFmt, getenv("HOME"), CBuildSVArg(language));
}
bool writechar(char c, void* ctx) {
	CBUILD_UNUSED(ctx);
	return putchar(c) != EOF;
}
bool writecstr(const char* str, void* ctx) {
	CBUILD_UNUSED(ctx);
	return printf("%s", str) >= 0;
}
#define GruvboxBG0       { .r = 0x28, .g = 0x28, .b = 0x28 }
#define GruvboxFG1       { .r = 0xEB, .g = 0xDB, .b = 0xB2 }
#define GruvboxRed       { .r = 0xCC, .g = 0x24, .b = 0x1D }
#define GruvboxGreen     { .r = 0xB8, .g = 0xBB, .b = 0x26 }
#define GruvboxYellow    { .r = 0xFA, .g = 0xBD, .b = 0x2F }
#define GruvboxBlue      { .r = 0x83, .g = 0xA5, .b = 0x98 }
#define GruvboxPurple    { .r = 0xD3, .g = 0x86, .b = 0x9B }
#define GruvboxAqua      { .r = 0x68, .g = 0x9D, .b = 0x6A }
#define GruvboxOrange    { .r = 0xFE, .g = 0x80, .b = 0x19 }
#define GruvboxDarkRed   { .r = 0x72, .g = 0x25, .b = 0x29 }
#define GruvboxDarkGreen { .r = 0x62, .g = 0x69, .b = 0x3E }
#define GruvboxDarkAqua  { .r = 0x49, .g = 0x50, .b = 0x3B }
#define GruvboxGray      { .r = 0x92, .g = 0x83, .b = 0x74 }
tshl_theme_t theme = {
	[TSHL_DEFAULT] = {
		.fg = GruvboxFG1,
	},
	[TSHL_VARIABLE] = {
		.fg = GruvboxFG1,
	},
	[TSHL_VARIABLE_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_VARIABLE_PARAMETER] = {
		.fg = GruvboxBlue,
	},
	[TSHL_VARIABLE_PARAMETER_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_VARIABLE_MEMBER] = {
		.fg = GruvboxBlue,
	},
	[TSHL_CONSTANT] = {
		.fg = GruvboxPurple,
	},
	[TSHL_CONSTANT_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_CONSTANT_MACRO] = {
		.fg = GruvboxAqua,
	},
	[TSHL_MODULE] = {
		.fg = GruvboxFG1,
	},
	[TSHL_MODULE_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_LABEL] = {
		.fg = GruvboxRed,
	},
	[TSHL_STRING] = {
		.fg = GruvboxGreen,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_STRING_DOCUMENTATION] = {
		.fg = GruvboxGreen,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_STRING_REGEXP] = {
		.fg = GruvboxGreen,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_STRING_ESCAPE] = {
		.fg = GruvboxOrange,
	},
	[TSHL_STRING_SPECIAL] = {
		.fg = GruvboxOrange,
	},
	[TSHL_STRING_SPECIAL_SYMBOL] = {
		.fg = GruvboxBlue,
	},
	[TSHL_STRING_SPECIAL_PATH] = {
		.fg = GruvboxBlue,
		.underline = TSHL_THEME_TRUE,
	},
	[TSHL_STRING_SPECIAL_URL] = {
		.fg = GruvboxBlue,
		.underline = TSHL_THEME_TRUE,
	},
	[TSHL_CHARACTER] = {
		.fg = GruvboxPurple,
	},
	[TSHL_CHARACTER_SPECIAL] = {
		.fg = GruvboxOrange,
	},
	[TSHL_BOOLEAN] = {
		.fg = GruvboxPurple,
	},
	[TSHL_NUMBER] = {
		.fg = GruvboxPurple,
	},
	[TSHL_NUMBER_FLOAT] = {
		.fg = GruvboxPurple,
	},
	[TSHL_TYPE] = {
		.fg = GruvboxYellow,
	},
	[TSHL_TYPE_BUILTIN] = {
		.fg = GruvboxYellow,
	},
	[TSHL_TYPE_DEFINITION] = {
		.fg = GruvboxYellow,
	},
	[TSHL_ATTRIBUTE] = {
		.fg = GruvboxAqua,
	},
	[TSHL_ATTRIBUTE_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_PROPERTY] = {
		.fg = GruvboxBlue,
	},
	[TSHL_FUNCTION] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_FUNCTION_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_FUNCTION_CALL] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_FUNCTION_MACRO] = {
		.fg = GruvboxAqua,
	},
	[TSHL_FUNCTION_METHOD] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_FUNCTION_METHOD_CALL] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_CONSTRUCTOR] = {
		.fg = GruvboxOrange,
	},
	[TSHL_OPERATOR] = {
		.fg = GruvboxOrange,
	},
	[TSHL_KEYWORD] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_COROUTINE] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_FUNCTION] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_OPERATOR] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_IMPORT] = {
		.fg = GruvboxAqua,
	},
	[TSHL_KEYWORD_TYPE] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_MODIFIER] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_REPEAT] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_RETURN] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_DEBUG] = {
		.fg = GruvboxOrange,
	},
	[TSHL_KEYWORD_EXCEPTION] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_CONDITIONAL] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_CONDITIONAL_TERNARY] = {
		.fg = GruvboxRed,
	},
	[TSHL_KEYWORD_DIRECTIVE] = {
		.fg = GruvboxAqua,
	},
	[TSHL_KEYWORD_DIRECTIVE_DEFINE] = {
		.fg = GruvboxAqua,
	},
	[TSHL_PUNCTUATION_DELIMITER] = {
		.fg = GruvboxOrange,
	},
	[TSHL_PUNCTUATION_BRACKET] = {
		.fg = GruvboxOrange,
	},
	[TSHL_PUNCTUATION_SPECIAL] = {
		.fg = GruvboxOrange,
	},
	[TSHL_COMMENT] = {
		.fg = GruvboxGray,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_COMMENT_DOCUMENTATION] = {
		.fg = GruvboxGray,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_COMMENT_ERROR] = {
		.fg = GruvboxBG0,
		.bg = GruvboxRed,
		.hasbg = true,
	},
	[TSHL_COMMENT_WARNING] = {
		.fg = GruvboxRed,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_COMMENT_TODO] = {
		.fg = GruvboxBG0,
		.bg = GruvboxYellow,
		.hasbg = true,
		.bold = TSHL_THEME_TRUE,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_COMMENT_NOTE] = {
		.fg = GruvboxOrange,
	},
	[TSHL_MARKUP_STRONG] = {
		.bold = TSHL_THEME_TRUE,
		.nofg = true,
	},
	[TSHL_MARKUP_ITALIC] = {
		.italic = TSHL_THEME_TRUE,
		.nofg = true,
	},
	[TSHL_MARKUP_STRIKETHROUGH] = {
		.strikethrough = TSHL_THEME_TRUE,
		.nofg = true,
	},
	[TSHL_MARKUP_UNDERLINE] = {
		.underline = TSHL_THEME_TRUE,
		.nofg = true,
	},
	[TSHL_MARKUP_HEADING] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_1] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_2] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_3] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_4] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_5] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_HEADING_6] = {
		.fg = GruvboxGreen,
		.bold = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_QUOTE] = {
		.nofg = true,
	},
	[TSHL_MARKUP_MATH] = {
		.fg = GruvboxOrange,
	},
	[TSHL_MARKUP_LINK] = {
		.fg = GruvboxBlue,
		.underline = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_LINK_LABEL] = {
		.fg = GruvboxOrange,
	},
	[TSHL_MARKUP_LINK_URL] = {
		.fg = GruvboxBlue,
		.underline = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_RAW] = {
		.fg = GruvboxGreen,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_RAW_BLOCK] = {
		.fg = GruvboxGreen,
		.italic = TSHL_THEME_TRUE,
	},
	[TSHL_MARKUP_LIST] = {
		.fg = GruvboxOrange,
	},
	[TSHL_MARKUP_LIST_CHECKED] = {
		.fg = GruvboxGreen,
	},
	[TSHL_MARKUP_LIST_UNCHECKED] = {
		.fg = GruvboxGray,
	},
	[TSHL_DIFF_PLUS] = {
		.bg = GruvboxDarkGreen,
		.hasbg = true,
		.nofg = true,
	},
	[TSHL_DIFF_MINUS] = {
		.bg = GruvboxDarkRed,
		.hasbg = true,
		.nofg = true,
	},
	[TSHL_DIFF_DELTA] = {
		.bg = GruvboxDarkAqua,
		.hasbg = true,
		.nofg = true,
	},
	[TSHL_TAG] = {
		.fg = GruvboxOrange,
	},
	[TSHL_TAG_BUILTIN] = {
		.fg = GruvboxOrange,
	},
	[TSHL_TAG_ATTRIBUTE] = {
		.fg = GruvboxBlue,
	},
	[TSHL_TAG_DELIMITER] = {
		.fg = GruvboxOrange,
	},
};
int main(int argc, char** argv) {
	cbuild_sb_t sb = {0};
	cbuild_shift(argv, argc);
	const char* file = cbuild_shift(argv, argc);
	const char* lang = "markdown";
	if (argc > 0) lang = cbuild_shift(argv, argc);
	if (!cbuild_file_read(file, &sb)) return 1;
	tshl_t tshl = tshl_init(load_parser, get_query_dir);
	tshl_metadata_t* meta = tshl_highlight(&tshl, cbuild_sv_from_sb(sb), cbuild_sv_from_cstr(lang));
	if (!tshl_meta_to_ansi(meta, cbuild_sv_from_sb(sb), theme, (tshl_printer_t){
		.writechar = writechar,
		.writecstr = writecstr,
	})) {
		return 1;
	}
	return 0;
}
