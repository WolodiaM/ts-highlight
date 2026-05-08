//! Markdown->ANSI converter using tree-sitter
//!
//! License: GPL-3.0-or-later

#ifndef __TSHL_H__
#define __TSHL_H__

#include "../../cbuild.h"
#include "tree_sitter/api.h"

typedef TSLanguage* (*tshl_load_parser)(cbuild_sv_t language);
typedef const char* (*tshl_get_query_dir)(cbuild_sv_t language);

typedef struct tshl_t {
	// Parser
	TSParser* parser;
	// Tree-sitter data
	cbuild_map_t languages; // const char* -> TSLanguage*
	cbuild_map_t queries;   // const char* -> (TSQuery*, TSQuery*)
	// For internal string
	cbuild_arena_t stringstore;
	// Callbacsk
	tshl_load_parser load_parser;
	tshl_get_query_dir get_query_dir;
} tshl_t;

typedef struct tshl_span_t {
	cbuild_sv_t text;
	enum tshl_style_t : uint8_t { // Taken from 'https://neovim.io/doc/user/treesitter/#treesitter-highlight-groups'
		TSHL_DEFAULT = 0,                 // Default, empty style (plain text)
		TSHL_VARIABLE,                    // various variable names
		TSHL_VARIABLE_BUILTIN,            // built-in variable names (e.g. this, self)
		TSHL_VARIABLE_PARAMETER,          // parameters of a function
		TSHL_VARIABLE_PARAMETER_BUILTIN,  // special parameters (e.g. _, it)
		TSHL_VARIABLE_MEMBER,             // object and struct fields
		TSHL_CONSTANT,                    // constant identifiers
		TSHL_CONSTANT_BUILTIN,            // built-in constant values
		TSHL_CONSTANT_MACRO,              // constants defined by the preprocessor
		TSHL_MODULE,                      // modules or namespaces
		TSHL_MODULE_BUILTIN,              // built-in modules or namespaces
		TSHL_LABEL,                       // GOTO and other labels (e.g. label: in C), including heredoc labels
		TSHL_STRING,                      // string literals
		TSHL_STRING_DOCUMENTATION,        // string documenting code (e.g. Python docstrings)
		TSHL_STRING_REGEXP,               // regular expressions
		TSHL_STRING_ESCAPE,               // escape sequences
		TSHL_STRING_SPECIAL,              // other special strings (e.g. dates)
		TSHL_STRING_SPECIAL_SYMBOL,       // symbols or atoms
		TSHL_STRING_SPECIAL_PATH,         // filenames
		TSHL_STRING_SPECIAL_URL,          // URIs (e.g. hyperlinks)
		TSHL_CHARACTER,                   // character literals
		TSHL_CHARACTER_SPECIAL,           // special characters (e.g. wildcards)
		TSHL_BOOLEAN,                     // boolean literals
		TSHL_NUMBER,                      // numeric literals
		TSHL_NUMBER_FLOAT,                // floating-point number literals
		TSHL_TYPE,                        // type or class definitions and annotations
		TSHL_TYPE_BUILTIN,                // built-in types
		TSHL_TYPE_DEFINITION,             // identifiers in type definitions (e.g. typedef <type> <identifier> in C)
		TSHL_ATTRIBUTE,                   // attribute annotations (e.g. Python decorators, Rust lifetimes)
		TSHL_ATTRIBUTE_BUILTIN,           // builtin annotations (e.g. @property in Python)
		TSHL_PROPERTY,                    // the key in key/value pairs
		TSHL_FUNCTION,                    // function definitions
		TSHL_FUNCTION_BUILTIN,            // built-in functions
		TSHL_FUNCTION_CALL,               // function calls
		TSHL_FUNCTION_MACRO,              // preprocessor macros
		TSHL_FUNCTION_METHOD,             // method definitions
		TSHL_FUNCTION_METHOD_CALL,        // method calls
		TSHL_CONSTRUCTOR,                 // constructor calls and definitions
		TSHL_OPERATOR,                    // symbolic operators (e.g. +, *)
		TSHL_KEYWORD,                     // keywords not fitting into specific categories
		TSHL_KEYWORD_COROUTINE,           // keywords related to coroutines (e.g. go in Go, async/await in Python)
		TSHL_KEYWORD_FUNCTION,            // keywords that define a function (e.g. func in Go, def in Python)
		TSHL_KEYWORD_OPERATOR,            // operators that are English words (e.g. and, or)
		TSHL_KEYWORD_IMPORT,              // keywords for including or exporting modules (e.g. import, from in Python)
		TSHL_KEYWORD_TYPE,                // keywords describing namespaces and composite types (e.g. struct, enum)
		TSHL_KEYWORD_MODIFIER,            // keywords modifying other constructs (e.g. const, static, public)
		TSHL_KEYWORD_REPEAT,              // keywords related to loops (e.g. for, while)
		TSHL_KEYWORD_RETURN,              // keywords like return and yield
		TSHL_KEYWORD_DEBUG,               // keywords related to debugging
		TSHL_KEYWORD_EXCEPTION,           // keywords related to exceptions (e.g. throw, catch)
		TSHL_KEYWORD_CONDITIONAL,         // keywords related to conditionals (e.g. if, else)
		TSHL_KEYWORD_CONDITIONAL_TERNARY, // ternary operator (e.g. ?, :)
		TSHL_KEYWORD_DIRECTIVE,           // various preprocessor directives and shebangs
		TSHL_KEYWORD_DIRECTIVE_DEFINE,    // preprocessor definition directives
		TSHL_PUNCTUATION_DELIMITER,       // delimiters (e.g. ;, ., ,)
		TSHL_PUNCTUATION_BRACKET,         // brackets (e.g. (), {}, [])
		TSHL_PUNCTUATION_SPECIAL,         // special symbols (e.g. {} in string interpolation)
		TSHL_COMMENT,                     // line and block comments
		TSHL_COMMENT_DOCUMENTATION,       // comments documenting code
		TSHL_COMMENT_ERROR,               // error-type comments (e.g. ERROR, FIXME, DEPRECATED)
		TSHL_COMMENT_WARNING,             // warning-type comments (e.g. WARNING, FIX, HACK)
		TSHL_COMMENT_TODO,                // todo-type comments (e.g. TODO, WIP)
		TSHL_COMMENT_NOTE,                // note-type comments (e.g. NOTE, INFO, XXX)
		TSHL_MARKUP_STRONG,               // bold text
		TSHL_MARKUP_ITALIC,               // italic text
		TSHL_MARKUP_STRIKETHROUGH,        // struck-through text
		TSHL_MARKUP_UNDERLINE,            // underlined text (only for literal underline markup!)
		TSHL_MARKUP_HEADING,              // headings, titles (including markers)
		TSHL_MARKUP_HEADING_1,            // top-level heading
		TSHL_MARKUP_HEADING_2,            // section heading
		TSHL_MARKUP_HEADING_3,            // subsection heading
		TSHL_MARKUP_HEADING_4,            // and so on
		TSHL_MARKUP_HEADING_5,            // and so forth
		TSHL_MARKUP_HEADING_6,            // six levels ought to be enough for anybody
		TSHL_MARKUP_QUOTE,                // block quotes
		TSHL_MARKUP_MATH,                 // math environments (e.g. $ ... $ in LaTeX)
		TSHL_MARKUP_LINK,                 // text references, footnotes, citations, etc.
		TSHL_MARKUP_LINK_LABEL,           // link, reference descriptions
		TSHL_MARKUP_LINK_URL,             // URL-style links
		TSHL_MARKUP_RAW,                  // literal or verbatim text (e.g. inline code)
		TSHL_MARKUP_RAW_BLOCK,            // literal or verbatim text as a stand-alone block
		TSHL_MARKUP_LIST,                 // list markers
		TSHL_MARKUP_LIST_CHECKED,         // checked todo-style list markers
		TSHL_MARKUP_LIST_UNCHECKED,       // unchecked todo-style list markers
		TSHL_DIFF_PLUS,                   // added text (for diff files)
		TSHL_DIFF_MINUS,                  // deleted text (for diff files)
		TSHL_DIFF_DELTA,                  // changed text (for diff files)
		TSHL_TAG,                         // XML-style tag names (e.g. in XML, HTML, etc.)
		TSHL_TAG_BUILTIN,                 // builtin tag names (e.g. HTML5 tags)
		TSHL_TAG_ATTRIBUTE,               // XML-style tag attributes
		TSHL_TAG_DELIMITER,               // XML-style tag delimiters
	} style;
} tshl_span_t;

typedef cbuild_da_new(tshl_span_t) tshl_spans_t;

tshl_t tshl_init(tshl_load_parser load_parser, tshl_get_query_dir get_query_dir);
tshl_spans_t tshl_highlight(tshl_t* self, cbuild_sv_t text, cbuild_sv_t lang);
const char* tshl_style_to_name(enum tshl_style_t style);
enum tshl_style_t tshl_name_to_style(cbuild_sv_t name);

#endif // __TSHL_H__
