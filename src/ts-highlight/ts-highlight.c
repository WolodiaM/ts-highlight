//! Markdown->ANSI converter using tree-sitter
//!
//! License: GPL-3.0-or-later

#include "../../cbuild.h"
#include "ts-highlight.h"
#include "tree_sitter/api.h"

typedef struct {
	cbuild_sv_t key;
	TSLanguage* lang;
} tshl_language_map_entry_t;
typedef struct {
	cbuild_sv_t key;
	TSQuery* hl;
	TSQuery* inj;
} tshl_query_map_entry_t;
size_t __tshl_map_hash(const void* map, const void* key) {
	CBUILD_UNUSED(map);
	cbuild_sv_t* k = (cbuild_sv_t*)key;
	return __cbuild_map_hash_func(k->data, k->size);
}
bool __tshl_map_keycmp(const void* map, const void* k1, const void* k2) {
	CBUILD_UNUSED(map);
	cbuild_sv_t* key1 = (cbuild_sv_t*)k1;
	cbuild_sv_t* key2 = (cbuild_sv_t*)k2;
	return cbuild_sv_cmp(*key1, *key2) == 0;
}

static bool __tshl_load_parser(tshl_t* self, cbuild_sv_t language) {
	TSLanguage* lang = self->load_parser(language);
	if (lang == NULL) return false;
	tshl_language_map_entry_t* hs_elem = cbuild_map_get_or_alloc(&self->languages, language);
	hs_elem->key.size = language.size;
	hs_elem->key.data = cbuild_arena_memdup(&self->stringstore, language.data, language.size);
	hs_elem->lang = lang;
	return true;
}
#pragma region tshl_query_err_to_str
static const char* tshl_query_err_to_str(TSQueryError err) {
	switch (err) {
	case TSQueryErrorNone:
		return "no errors";
		break;
	case TSQueryErrorSyntax:
		return "syntax error";
		break;
	case TSQueryErrorNodeType:
		return "invalid node";
		break;
	case TSQueryErrorField:
		return "invalid field";
		break;
	case TSQueryErrorCapture:
		return "invalid capture";
		break;
	case TSQueryErrorStructure:
		return "invalid structure";
		break;
	case TSQueryErrorLanguage:
		return "wrong language";
		break;
	}
	return NULL;
}
#pragma endregion
static bool __tshl_load_queries(tshl_t* self, cbuild_sv_t language) {
	TSLanguage* l = ((tshl_language_map_entry_t*)cbuild_map_get(&self->languages, language))->lang;
	const char* q = self->get_query_dir(language);
	cbuild_sb_t file = {0};
	if (!cbuild_file_read(
			cbuild_temp_sprintf("%s/highlights.scm", q), &file)) {
		cbuild_sb_clear(&file);
		return false;
	}
	uint32_t err_offset = 0;
	TSQueryError err = TSQueryErrorNone;
	TSQuery* hl = ts_query_new(l, 
		file.data, (uint32_t)file.size,
		&err_offset, &err);
	if (hl == NULL) {
		cbuild_log_error("Failed to load '%s/highlights.scm': %s at %u.",
			q, tshl_query_err_to_str(err), err_offset);
		cbuild_sb_clear(&file);
		return false;
	}
	file.size = 0;
	TSQuery* inj = NULL;
	// NOTE: This one is optional
	if (cbuild_file_check(cbuild_temp_sprintf("%s/injections.scm", q))) {
		if (!cbuild_file_read(
				cbuild_temp_sprintf("%s/injections.scm", q), &file)) {
			ts_query_delete(hl);
			cbuild_sb_clear(&file);
			return false;
		}
		inj = ts_query_new(l, 
			file.data, (uint32_t)file.size,
			&err_offset, &err);
		if (inj == NULL) {
			cbuild_log_error("Failed to load '%s/injections.scm': %s at %u.",
				q, tshl_query_err_to_str(err), err_offset);
			cbuild_sb_clear(&file);
			return false;
		}
	}
	cbuild_sb_clear(&file);
	tshl_query_map_entry_t* hs_elem = cbuild_map_get_or_alloc(&self->queries, language);
	hs_elem->key.size = language.size;
	hs_elem->key.data = cbuild_arena_memdup(&self->stringstore, language.data, language.size);
	hs_elem->hl = hl;
	hs_elem->inj = inj;
	return true;
}
tshl_t tshl_init(tshl_load_parser lp, tshl_get_query_dir gqd) {
	tshl_t self = {0};
	// Save callbacks
	self.load_parser = lp;
	self.get_query_dir = gqd;
	// Initialize arena
	cbuild_arena_base_malloc(&self.stringstore, CBUILD_TEMP_ARENA_SIZE);
	// Initialize languages hashmap. I do not expect more than 20 languages per session.
	self.languages.elem_size = sizeof(tshl_language_map_entry_t);
	self.languages.key_size = sizeof(cbuild_sv_t);
	self.languages.hash_func = __tshl_map_hash;
	self.languages.keycmp_func = __tshl_map_keycmp;
	// hash, key compare and clear are default ones
	cbuild_map_init(&self.languages, 20);
	// Initialize queries hashmap. I do not expect more than 20 languages per session.
	self.queries.elem_size = sizeof(tshl_query_map_entry_t);
	self.queries.key_size = sizeof(cbuild_sv_t);
	self.queries.hash_func = __tshl_map_hash;
	self.queries.keycmp_func = __tshl_map_keycmp;
	// hash, key compare and clear are default ones
	cbuild_map_init(&self.queries, 20);
	// Initialize parser
	self.parser = ts_parser_new();
	return self;
}
tshl_spans_t __tshl_buffers_to_spans(cbuild_sv_t text, enum tshl_style_t* atrib) {
	tshl_spans_t spans = {0};
	enum tshl_style_t curr = atrib[0];
	tshl_span_t span = {0};
	span.text.data = text.data;
	span.style = atrib[0];
	for (size_t i = 0; i < text.size; i++) {
		if (curr != atrib[i]) {
			cbuild_da_append(&spans, span);
			span.text.data = &text.data[i];
			span.text.size = 0;
			span.style = atrib[i];
		}
		curr = atrib[i];
		span.text.size++;
	}
	cbuild_da_append(&spans, span);
	return spans;
}
typedef struct {
	uint32_t start;
	uint32_t end;
	cbuild_sv_t val; // If data != NULL overrides '[start:end)' range
	cbuild_sv_t name;
} tshl_capture_t;
cbuild_sv_t __tshl_capture_get_sv(tshl_capture_t capt, cbuild_sv_t text) {
	if (capt.val.data != NULL) return capt.val;
	return cbuild_sv_from_parts(text.data + capt.start, capt.end - capt.start);
}
typedef cbuild_da_new(tshl_capture_t) tshl_captures_t;
cbuild_sv_t __tshl_predicate_get_string(TSQuery* q, TSQueryPredicateStep st) {
	if (st.type != TSQueryPredicateStepTypeString) {
		return (cbuild_sv_t){0};
	}
	uint32_t len = 0;
	const char* data = ts_query_string_value_for_id(q, st.value_id, &len);
	return cbuild_sv_from_parts(data, len);
}
// NOTE: Assumes that steps end with DONE
bool __tshl_eval_predicate(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, tshl_captures_t* captures) {
	bool ret = true;
	cbuild_sv_t func = __tshl_predicate_get_string(q, steps[(*ip)++]);
	if (func.data == NULL) {
		cbuild_log_error("Predicate does not start from function name.");
		ret = false;
		goto defer;
	}
	if (cbuild_sv_cmp(func, cbuild_sv_from_lit("set!")) == 0) {
		TSQueryPredicateStep st = steps[(*ip)++];
		if (st.type == TSQueryPredicateStepTypeString) {
			cbuild_sv_t var = __tshl_predicate_get_string(q, st);
			if (var.data == NULL) {
				cbuild_log_error("Predicate '#set!' is not followed by variable name.");
				ret = false;
				goto defer;
			}
			if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.language")) == 0) {
				cbuild_sv_t val = __tshl_predicate_get_string(q, steps[(*ip)++]);
				if (val.data == NULL) {
					cbuild_log_error("Predicate '#set! injection.language' is not followed by value"); 
					ret = false;
					goto defer;
				}
				bool found = false;
				cbuild_da_foreach(*captures, capt) {
					if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
						capt->val = val;
						found = true;
						break;
					}
				}
				if (!found) {
					tshl_capture_t capt = {0};
					capt.name = cbuild_sv_from_lit("injection.language");
					capt.val = val;
					cbuild_da_append(captures, capt);
				}
			} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("conceal")) == 0) {
				// Nothing to do here
			} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("conceal_lines")) == 0) {
				// Nothing to do here
			} else {
				cbuild_log_warn("Unimplemented variable for '#set!': '"CBuildSVFmt"'.\n", CBuildSVArg(var));
			}
		} else if (st.type == TSQueryPredicateStepTypeCapture) {
			// TODO: This is used for changing commentstring and marking things as url:
			// '(#set! @_hyperlink url @markup.link.url)'.
			// First argument is a capture, second marks it as url and thirrd "renames" capture.
		} else {
			cbuild_log_warn("Invalid '#set!' directive.");
		}
	} else {
		cbuild_log_warn("Unimplemented predicate: '"CBuildSVFmt"'", CBuildSVArg(func));
	}
defer:
	while (steps[(*ip)++].type != TSQueryPredicateStepTypeDone);
	return ret;
}
bool __tshl_eval_predicates(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t step_count, tshl_captures_t* captures) {
	uint32_t ip = 0;
	bool ret = true;
	while (ip < step_count) ret &= __tshl_eval_predicate(q, steps, &ip, captures);	
	return ret;
}
void __tshl_highlight(tshl_t* self, cbuild_sv_t text, cbuild_sv_t lang, uint32_t offset, enum tshl_style_t* atrib) {
	// Setup
	size_t checkpoint = cbuild_temp_checkpoint();
	tshl_language_map_entry_t* p = cbuild_map_get(&self->languages, lang);
	if (p == NULL) {
		if (!__tshl_load_parser(self, lang)) {
			cbuild_log_error("Could not load "CBuildSVFmt" parser.", CBuildSVArg(lang));
			cbuild_temp_reset(checkpoint);
			return;
		}
		if (!__tshl_load_queries(self, lang)) {
			cbuild_log_error("Could not load "CBuildSVFmt" queries.", CBuildSVArg(lang));
			exit(1);
			cbuild_temp_reset(checkpoint);
			return;
		}
		p = cbuild_map_get(&self->languages, lang);
		cbuild_assert(p != NULL, "Parser not found in hashmap.");
	}
	if (!ts_parser_set_language(self->parser, p->lang)) {
		cbuild_log_error("Could not set "CBuildSVFmt" parser.", CBuildSVArg(lang));
		cbuild_temp_reset(checkpoint);
		return;
	}
	TSTree* tree = ts_parser_parse_string(
		self->parser,
		NULL,
		text.data,
		(uint32_t)text.size);
	TSNode root = ts_tree_root_node(tree);
	TSQueryCursor* c = ts_query_cursor_new();
	// Highlight queries
	tshl_query_map_entry_t* q = cbuild_map_get(&self->queries, lang);
	if (q == NULL) {
		ts_query_cursor_delete(c);
		ts_tree_delete(tree);
		cbuild_temp_reset(checkpoint);
		return;
	}
	TSQueryMatch match = {0};
	tshl_captures_t captures = {0};
	ts_query_cursor_exec(c, q->hl, root);
	while(ts_query_cursor_next_match(c, &match)) {
		captures.size = 0;
		for (uint16_t i = 0; i < match.capture_count; i++) {
			TSQueryCapture capt = match.captures[i];
			uint32_t name_len = 0;
			const char* name = ts_query_capture_name_for_id(q->hl, capt.index, &name_len);
			uint32_t start = ts_node_start_byte(capt.node);
			uint32_t end = ts_node_end_byte(capt.node);
			cbuild_sv_t st_name = cbuild_sv_from_parts(name, name_len);
			tshl_capture_t capture = {
				.start = start, 
				.end = end, 
				.name = st_name,
			};
			cbuild_da_append(&captures, capture);
		}
		bool valid = true;
		const TSQueryPredicateStep* predicate = NULL;
		uint32_t step_count = 0;
		predicate = ts_query_predicates_for_pattern(q->hl, match.pattern_index, &step_count);
		if (predicate != NULL) {
			valid = __tshl_eval_predicates(q->hl, predicate, step_count, &captures);
		}
		if (valid) {
			cbuild_da_foreach(captures, capt) {
				if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("spell")) == 0) continue;
				if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("nospell")) == 0) continue;
				if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("conceal")) == 0) continue;
				if (capt->name.data[0] == '_') continue;
				enum tshl_style_t style = tshl_name_to_style(capt->name);
				for (uint32_t i = capt->start; i < capt->end; i++) {
					atrib[offset+i] = style;
				}
			}
		}
	}
	// Injection queries
	if (q->inj != NULL) {
		ts_query_cursor_exec(c, q->inj, root);
		while(ts_query_cursor_next_match(c, &match)) {
			captures.size = 0;
			for (uint16_t i = 0; i < match.capture_count; i++) {
				TSQueryCapture capt = match.captures[i];
				uint32_t name_len = 0;
				const char* name = ts_query_capture_name_for_id(q->inj, capt.index, &name_len);
				uint32_t start = ts_node_start_byte(capt.node);
				uint32_t end = ts_node_end_byte(capt.node);
				cbuild_sv_t st_name = cbuild_sv_from_parts(name, name_len);
				tshl_capture_t capture = {
					.start = start, 
					.end = end, 
					.name = st_name,
				};
				cbuild_da_append(&captures, capture);
			}
			bool valid = true;
			const TSQueryPredicateStep* predicate = NULL;
			uint32_t step_count = 0;
			predicate = ts_query_predicates_for_pattern(q->inj, match.pattern_index, &step_count);
			if (predicate != NULL) {
				valid = __tshl_eval_predicates(q->inj, predicate, step_count, &captures);
			}
			if (valid) {
				cbuild_sv_t inj_lang = {0};
				cbuild_sv_t inj_text = {0};
				uint32_t inj_offset = offset;
				cbuild_da_foreach(captures, capt) {
					if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
						inj_lang = __tshl_capture_get_sv(*capt, text);
					} else if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.content")) == 0) {
						inj_offset += capt->start;
						inj_text = __tshl_capture_get_sv(*capt, text);
					}
				}
				__tshl_highlight(self, inj_text, inj_lang, inj_offset, atrib);
			}
		}
	}
	ts_query_cursor_delete(c);
	ts_tree_delete(tree);
	cbuild_temp_reset(checkpoint);
}
tshl_spans_t tshl_highlight(tshl_t* self, cbuild_sv_t text, cbuild_sv_t lang) {
	enum tshl_style_t* atrib = malloc(sizeof(enum tshl_style_t)*text.size);
	memset(atrib, 0, sizeof(enum tshl_style_t)*text.size);
	cbuild_assert(atrib != NULL, "Allocation failed.");
	__tshl_highlight(self, text, lang, 0, atrib);
	return __tshl_buffers_to_spans(text, atrib);
}
#pragma region tshl_style_to_name
const char* tshl_style_to_name(enum tshl_style_t style) {
	switch (style) {	
	case TSHL_DEFAULT:                     return "default";                     break;
	case TSHL_VARIABLE:                    return "variable";                    break;
	case TSHL_VARIABLE_BUILTIN:            return "variable.builtin";            break;
	case TSHL_VARIABLE_PARAMETER:          return "variable.parameter";          break;
	case TSHL_VARIABLE_PARAMETER_BUILTIN:  return "variable.parameter.builtin";  break;
	case TSHL_VARIABLE_MEMBER:             return "variable.member";             break;
	case TSHL_CONSTANT:                    return "constant";                    break;
	case TSHL_CONSTANT_BUILTIN:            return "constant.builtin";            break;
	case TSHL_CONSTANT_MACRO:              return "constant.macro";              break;
	case TSHL_MODULE:                      return "module";                      break;
	case TSHL_MODULE_BUILTIN:              return "module.builtin";              break;
	case TSHL_LABEL:                       return "label";                       break;
	case TSHL_STRING:                      return "string";                      break;
	case TSHL_STRING_DOCUMENTATION:        return "string.documentation";        break;
	case TSHL_STRING_REGEXP:               return "string.regexp";               break;
	case TSHL_STRING_ESCAPE:               return "string.escape";               break;
	case TSHL_STRING_SPECIAL:              return "string.special";              break;
	case TSHL_STRING_SPECIAL_SYMBOL:       return "string.special.symbol";       break;
	case TSHL_STRING_SPECIAL_PATH:         return "string.special.path";         break;
	case TSHL_STRING_SPECIAL_URL:          return "string.special.url";          break;
	case TSHL_CHARACTER:                   return "character";                   break;
	case TSHL_CHARACTER_SPECIAL:           return "character.special";           break;
	case TSHL_BOOLEAN:                     return "boolean";                     break;
	case TSHL_NUMBER:                      return "number";                      break;
	case TSHL_NUMBER_FLOAT:                return "number.float";                break;
	case TSHL_TYPE:                        return "type";                        break;
	case TSHL_TYPE_BUILTIN:                return "type.builtin";                break;
	case TSHL_TYPE_DEFINITION:             return "type.definition";             break;
	case TSHL_ATTRIBUTE:                   return "attribute";                   break;
	case TSHL_ATTRIBUTE_BUILTIN:           return "attribute.builtin";           break;
	case TSHL_PROPERTY:                    return "property";                    break;
	case TSHL_FUNCTION:                    return "function";                    break;
	case TSHL_FUNCTION_BUILTIN:            return "function.builtin";            break;
	case TSHL_FUNCTION_CALL:               return "function.call";               break;
	case TSHL_FUNCTION_MACRO:              return "function.macro";              break;
	case TSHL_FUNCTION_METHOD:             return "function.method";             break;
	case TSHL_FUNCTION_METHOD_CALL:        return "function.method.call";        break;
	case TSHL_CONSTRUCTOR:                 return "constructor";                 break;
	case TSHL_OPERATOR:                    return "operator";                    break;
	case TSHL_KEYWORD:                     return "keyword";                     break;
	case TSHL_KEYWORD_COROUTINE:           return "keyword.coroutine";           break;
	case TSHL_KEYWORD_FUNCTION:            return "keyword.function";            break;
	case TSHL_KEYWORD_OPERATOR:            return "keyword.operator";            break;
	case TSHL_KEYWORD_IMPORT:              return "keyword.import";              break;
	case TSHL_KEYWORD_TYPE:                return "keyword.type";                break;
	case TSHL_KEYWORD_MODIFIER:            return "keyword.modifier";            break;
	case TSHL_KEYWORD_REPEAT:              return "keyword.repeat";              break;
	case TSHL_KEYWORD_RETURN:              return "keyword.return";              break;
	case TSHL_KEYWORD_DEBUG:               return "keyword.debug";               break;
	case TSHL_KEYWORD_EXCEPTION:           return "keyword.exception";           break;
	case TSHL_KEYWORD_CONDITIONAL:         return "keyword.conditional";         break;
	case TSHL_KEYWORD_CONDITIONAL_TERNARY: return "keyword.conditional.ternary"; break;
	case TSHL_KEYWORD_DIRECTIVE:           return "keyword.directive";           break;
	case TSHL_KEYWORD_DIRECTIVE_DEFINE:    return "keyword.directive.define";    break;
	case TSHL_PUNCTUATION_DELIMITER:       return "punctuation.delimiter";       break;
	case TSHL_PUNCTUATION_BRACKET:         return "punctuation.bracket";         break;
	case TSHL_PUNCTUATION_SPECIAL:         return "punctuation.special";         break;
	case TSHL_COMMENT:                     return "comment";                     break;
	case TSHL_COMMENT_DOCUMENTATION:       return "comment.documentation";       break;
	case TSHL_COMMENT_ERROR:               return "comment.error";               break;
	case TSHL_COMMENT_WARNING:             return "comment.warning";             break;
	case TSHL_COMMENT_TODO:                return "comment.todo";                break;
	case TSHL_COMMENT_NOTE:                return "comment.note";                break;
	case TSHL_MARKUP_STRONG:               return "markup.strong";               break;
	case TSHL_MARKUP_ITALIC:               return "markup.italic";               break;
	case TSHL_MARKUP_STRIKETHROUGH:        return "markup.strikethrough";        break;
	case TSHL_MARKUP_UNDERLINE:            return "markup.underline";            break;
	case TSHL_MARKUP_HEADING:              return "markup.heading";              break;
	case TSHL_MARKUP_HEADING_1:            return "markup.heading.1";            break;
	case TSHL_MARKUP_HEADING_2:            return "markup.heading.2";            break;
	case TSHL_MARKUP_HEADING_3:            return "markup.heading.3";            break;
	case TSHL_MARKUP_HEADING_4:            return "markup.heading.4";            break;
	case TSHL_MARKUP_HEADING_5:            return "markup.heading.5";            break;
	case TSHL_MARKUP_HEADING_6:            return "markup.heading.6";            break;
	case TSHL_MARKUP_QUOTE:                return "markup.quote";                break;
	case TSHL_MARKUP_MATH:                 return "markup.math";                 break;
	case TSHL_MARKUP_LINK:                 return "markup.link";                 break;
	case TSHL_MARKUP_LINK_LABEL:           return "markup.link.label";           break;
	case TSHL_MARKUP_LINK_URL:             return "markup.link.url";             break;
	case TSHL_MARKUP_RAW:                  return "markup.raw";                  break;
	case TSHL_MARKUP_RAW_BLOCK:            return "markup.raw.block";            break;
	case TSHL_MARKUP_LIST:                 return "markup.list";                 break;
	case TSHL_MARKUP_LIST_CHECKED:         return "markup.list.checked";         break;
	case TSHL_MARKUP_LIST_UNCHECKED:       return "markup.list.unchecked";       break;
	case TSHL_DIFF_PLUS:                   return "diff.plus";                   break;
	case TSHL_DIFF_MINUS:                  return "diff.minus";                  break;
	case TSHL_DIFF_DELTA:                  return "diff.delta";                  break;
	case TSHL_TAG:                         return "tag";                         break;
	case TSHL_TAG_BUILTIN:                 return "tag.builtin";                 break;
	case TSHL_TAG_ATTRIBUTE:               return "tag.attribute";               break;
	case TSHL_TAG_DELIMITER:               return "tag.delimiter";               break;
	}
	CBUILD_UNREACHABLE("Invalid style.");
}
#pragma endregion
#pragma region tshl_name_to_style
enum tshl_style_t tshl_name_to_style(cbuild_sv_t name) {
	if (cbuild_sv_cmp(name, cbuild_sv_from_lit("default")) == 0) {
		return TSHL_DEFAULT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("variable")) == 0) {
		return TSHL_VARIABLE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("variable.builtin")) == 0) {
		return TSHL_VARIABLE_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("variable.parameter")) == 0) {
		return TSHL_VARIABLE_PARAMETER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("variable.parameter.builtin")) == 0) {
		return TSHL_VARIABLE_PARAMETER_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("variable.member")) == 0) {
		return TSHL_VARIABLE_MEMBER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("constant")) == 0) {
		return TSHL_CONSTANT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("constant.builtin")) == 0) {
		return TSHL_CONSTANT_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("constant.macro")) == 0) {
		return TSHL_CONSTANT_MACRO;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("module")) == 0) {
		return TSHL_MODULE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("module.builtin")) == 0) {
		return TSHL_MODULE_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("label")) == 0) {
		return TSHL_LABEL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string")) == 0) {
		return TSHL_STRING;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.documentation")) == 0) {
		return TSHL_STRING_DOCUMENTATION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.regexp")) == 0) {
		return TSHL_STRING_REGEXP;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.escape")) == 0) {
		return TSHL_STRING_ESCAPE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.special")) == 0) {
		return TSHL_STRING_SPECIAL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.special.symbol")) == 0) {
		return TSHL_STRING_SPECIAL_SYMBOL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.special.path")) == 0) {
		return TSHL_STRING_SPECIAL_PATH;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("string.special.url")) == 0) {
		return TSHL_STRING_SPECIAL_URL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("character")) == 0) {
		return TSHL_CHARACTER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("character.special")) == 0) {
		return TSHL_CHARACTER_SPECIAL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("boolean")) == 0) {
		return TSHL_BOOLEAN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("number")) == 0) {
		return TSHL_NUMBER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("number.float")) == 0) {
		return TSHL_NUMBER_FLOAT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("type")) == 0) {
		return TSHL_TYPE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("type.builtin")) == 0) {
		return TSHL_TYPE_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("type.definition")) == 0) {
		return TSHL_TYPE_DEFINITION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("attribute")) == 0) {
		return TSHL_ATTRIBUTE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("attribute.builtin")) == 0) {
		return TSHL_ATTRIBUTE_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("property")) == 0) {
		return TSHL_PROPERTY;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function")) == 0) {
		return TSHL_FUNCTION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function.builtin")) == 0) {
		return TSHL_FUNCTION_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function.call")) == 0) {
		return TSHL_FUNCTION_CALL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function.macro")) == 0) {
		return TSHL_FUNCTION_MACRO;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function.method")) == 0) {
		return TSHL_FUNCTION_METHOD;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("function.method.call")) == 0) {
		return TSHL_FUNCTION_METHOD_CALL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("constructor")) == 0) {
		return TSHL_CONSTRUCTOR;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("operator")) == 0) {
		return TSHL_OPERATOR;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword")) == 0) {
		return TSHL_KEYWORD;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.coroutine")) == 0) {
		return TSHL_KEYWORD_COROUTINE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.function")) == 0) {
		return TSHL_KEYWORD_FUNCTION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.operator")) == 0) {
		return TSHL_KEYWORD_OPERATOR;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.import")) == 0) {
		return TSHL_KEYWORD_IMPORT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.type")) == 0) {
		return TSHL_KEYWORD_TYPE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.modifier")) == 0) {
		return TSHL_KEYWORD_MODIFIER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.repeat")) == 0) {
		return TSHL_KEYWORD_REPEAT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.return")) == 0) {
		return TSHL_KEYWORD_RETURN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.debug")) == 0) {
		return TSHL_KEYWORD_DEBUG;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.exception")) == 0) {
		return TSHL_KEYWORD_EXCEPTION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.conditional")) == 0) {
		return TSHL_KEYWORD_CONDITIONAL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.conditional.ternary")) == 0) {
		return TSHL_KEYWORD_CONDITIONAL_TERNARY;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.directive")) == 0) {
		return TSHL_KEYWORD_DIRECTIVE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("keyword.directive.define")) == 0) {
		return TSHL_KEYWORD_DIRECTIVE_DEFINE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("punctuation.delimiter")) == 0) {
		return TSHL_PUNCTUATION_DELIMITER;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("punctuation.bracket")) == 0) {
		return TSHL_PUNCTUATION_BRACKET;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("punctuation.special")) == 0) {
		return TSHL_PUNCTUATION_SPECIAL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment")) == 0) {
		return TSHL_COMMENT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment.documentation")) == 0) {
		return TSHL_COMMENT_DOCUMENTATION;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment.error")) == 0) {
		return TSHL_COMMENT_ERROR;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment.warning")) == 0) {
		return TSHL_COMMENT_WARNING;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment.todo")) == 0) {
		return TSHL_COMMENT_TODO;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("comment.note")) == 0) {
		return TSHL_COMMENT_NOTE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.strong")) == 0) {
		return TSHL_MARKUP_STRONG;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.italic")) == 0) {
		return TSHL_MARKUP_ITALIC;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.strikethrough")) == 0) {
		return TSHL_MARKUP_STRIKETHROUGH;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.underline")) == 0) {
		return TSHL_MARKUP_UNDERLINE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading")) == 0) {
		return TSHL_MARKUP_HEADING;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.1")) == 0) {
		return TSHL_MARKUP_HEADING_1;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.2")) == 0) {
		return TSHL_MARKUP_HEADING_2;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.3")) == 0) {
		return TSHL_MARKUP_HEADING_3;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.4")) == 0) {
		return TSHL_MARKUP_HEADING_4;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.5")) == 0) {
		return TSHL_MARKUP_HEADING_5;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.heading.6")) == 0) {
		return TSHL_MARKUP_HEADING_6;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.quote")) == 0) {
		return TSHL_MARKUP_QUOTE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.math")) == 0) {
		return TSHL_MARKUP_MATH;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.link")) == 0) {
		return TSHL_MARKUP_LINK;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.link.label")) == 0) {
		return TSHL_MARKUP_LINK_LABEL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.link.url")) == 0) {
		return TSHL_MARKUP_LINK_URL;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.raw")) == 0) {
		return TSHL_MARKUP_RAW;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.raw.block")) == 0) {
		return TSHL_MARKUP_RAW_BLOCK;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.list")) == 0) {
		return TSHL_MARKUP_LIST;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.list.checked")) == 0) {
		return TSHL_MARKUP_LIST_CHECKED;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("markup.list.unchecked")) == 0) {
		return TSHL_MARKUP_LIST_UNCHECKED;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("diff.plus")) == 0) {
		return TSHL_DIFF_PLUS;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("diff.minus")) == 0) {
		return TSHL_DIFF_MINUS;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("diff.delta")) == 0) {
		return TSHL_DIFF_DELTA;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("tag")) == 0) {
		return TSHL_TAG;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("tag.builtin")) == 0) {
		return TSHL_TAG_BUILTIN;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("tag.attribute")) == 0) {
		return TSHL_TAG_ATTRIBUTE;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("tag.delimiter")) == 0) {
		return TSHL_TAG_DELIMITER;
	} else {
		cbuild_log_error("Invalid style name: '"CBuildSVFmt"'", CBuildSVArg(name));
		return TSHL_DEFAULT;
	}
}
#pragma endregion
/* vim: set foldmethod=marker foldmarker=#pragma\ region,#pragma\ endregion : */
