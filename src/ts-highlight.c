//! Markdown->ANSI converter using tree-sitter
//!
//! License: GPL-3.0-or-later
#include "ts-highlight.h"
////////////////////////////////////////////////////////////////////////////////
// Helpers for hashmaps
////////////////////////////////////////////////////////////////////////////////
typedef struct {
	cbuild_sv_t key;
	TSLanguage* lang;
} __tshl_language_map_entry_t;
typedef struct {
	cbuild_sv_t key;
	TSQuery* hl;
	TSQuery* inj;
} __tshl_query_map_entry_t;
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
////////////////////////////////////////////////////////////////////////////////
// Helpers for parser loading
////////////////////////////////////////////////////////////////////////////////
static bool __tshl_load_parser(tshl_t* self, cbuild_sv_t language) {
	TSLanguage* lang = self->load_parser(language);
	if (lang == NULL) return false;
	__tshl_language_map_entry_t* hs_elem = cbuild_map_get_or_alloc(&self->languages, language);
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
typedef cbuild_da_new(cbuild_sv_t) __tshl_da_sv_t;
cbuild_sv_t __tshl_temp_svdup(cbuild_sv_t sv) {
	return cbuild_sv_from_parts(
		cbuild_temp_memdup(sv.data, sv.size),
		sv.size);
}
bool __tshl_inherit_queries(tshl_t* self, cbuild_sb_t* file, size_t offset,
	__tshl_da_sv_t* loaded, TSLanguage* l) {
	cbuild_sv_t f = cbuild_sv_from_sb(*file);
	cbuild_sv_chop(&f, offset);
	cbuild_sv_trim(&f);
	cbuild_sv_t line = {0};
	while (f.size > 0 && f.data[0] == ';') {
		line = cbuild_sv_chop_by_delim(&f, '\n');
		cbuild_sv_chop_by_delim(&line, 'i');
		if (cbuild_sv_prefix(line, cbuild_sv_from_lit("nherits:"))) {
			cbuild_sv_chop_by_delim(&line, ':');
			cbuild_sv_trim(&line);
			while (line.size > 0) {
				cbuild_sv_t language = cbuild_sv_chop_by_delim(&line, ',');
				cbuild_sv_trim(&language);
				if (language.size > 0) {
					bool skip = false;
					cbuild_da_foreach(*loaded, lng) {
						if (cbuild_sv_cmp(*lng, language) == 0) skip = true;
					}
					if (skip) continue;
					cbuild_da_append(loaded, __tshl_temp_svdup(language));
					const char* q = self->get_query_dir(language);
					size_t prev_size = file->size;
					if (!cbuild_file_read(
							cbuild_temp_sprintf("%s/highlights.scm", q), file)) {
						return false;
					}
					if (!__tshl_inherit_queries(self, file, prev_size, loaded, l)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
static bool __tshl_load_queries(tshl_t* self, cbuild_sv_t language) {
	TSLanguage* l = ((__tshl_language_map_entry_t*)cbuild_map_get(&self->languages, language))->lang;
	const char* q = self->get_query_dir(language);
	cbuild_sb_t file = {0};
	if (!cbuild_file_read(
			cbuild_temp_sprintf("%s/highlights.scm", q), &file)) {
		cbuild_sb_clear(&file);
		return false;
	}
	__tshl_da_sv_t loaded = {0};
	cbuild_da_append(&loaded, __tshl_temp_svdup(language));
	if (!__tshl_inherit_queries(self, &file, 0, &loaded, l)) {
		cbuild_sb_clear(&file);
		return false;
	}
	cbuild_da_clear(&loaded);
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
	__tshl_query_map_entry_t* hs_elem = cbuild_map_get_or_alloc(&self->queries, language);
	hs_elem->key.size = language.size;
	hs_elem->key.data = cbuild_arena_memdup(&self->stringstore, language.data, language.size);
	hs_elem->hl = hl;
	hs_elem->inj = inj;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////////
tshl_t tshl_init(tshl_load_parser lp, tshl_get_query_dir gqd) {
	tshl_t self = {0};
	// Save callbacks
	self.load_parser = lp;
	self.get_query_dir = gqd;
	// Initialize arena
	cbuild_arena_base_malloc(&self.stringstore, CBUILD_TEMP_ARENA_SIZE);
	// Initialize languages hashmap. I do not expect more than 20 languages per session.
	self.languages.elem_size = sizeof(__tshl_language_map_entry_t);
	self.languages.key_size = sizeof(cbuild_sv_t);
	self.languages.hash_func = __tshl_map_hash;
	self.languages.keycmp_func = __tshl_map_keycmp;
	// hash, key compare and clear are default ones
	cbuild_map_init(&self.languages, 20);
	// Initialize queries hashmap. I do not expect more than 20 languages per session.
	self.queries.elem_size = sizeof(__tshl_query_map_entry_t);
	self.queries.key_size = sizeof(cbuild_sv_t);
	self.queries.hash_func = __tshl_map_hash;
	self.queries.keycmp_func = __tshl_map_keycmp;
	// hash, key compare and clear are default ones
	cbuild_map_init(&self.queries, 20);
	// Initialize parser
	self.parser = ts_parser_new();
	return self;
}
////////////////////////////////////////////////////////////////////////////////
// Helpers for parsing
////////////////////////////////////////////////////////////////////////////////
typedef struct {
	uint32_t start;
	uint32_t end;
	uint32_t idx;
	uint32_t priority;
	cbuild_sv_t val; // If data != NULL overrides '[start:end)' range
	cbuild_sv_t name;
	TSNode node;
} __tshl_capture_t;
cbuild_sv_t __tshl_capture_get_sv(__tshl_capture_t capt, cbuild_sv_t text) {
	if (capt.val.data != NULL) return capt.val;
	return cbuild_sv_from_parts(text.data + capt.start, capt.end - capt.start);
}
typedef cbuild_da_new(__tshl_capture_t) __tshl_captures_t;
////////////////////////////////////////////////////////////////////////////////
// Regex
////////////////////////////////////////////////////////////////////////////////
#define uchar(c) ((unsigned char)c)
void __tshl_luapat_to_pcre2_escape(cbuild_sb_t* regex, uint32_t cp) {
	if (cp == uchar('a')) {
		cbuild_sb_append_cstr(regex, "[[:alpha:]]");
	} else if (cp == uchar('c')) {
		cbuild_sb_append_cstr(regex, "[[:cntrl:]]");
	} else if (cp == uchar('d')) {
		cbuild_sb_append_cstr(regex, "\\d");
	} else if (cp == uchar('l')) {
		cbuild_sb_append_cstr(regex, "[[:lower:]]");
	} else if (cp == uchar('p')) {
		cbuild_sb_append_cstr(regex, "[[:punct:]]");
	} else if (cp == uchar('s')) {
		cbuild_sb_append_cstr(regex, "\\s");
	} else if (cp == uchar('u')) {
		cbuild_sb_append_cstr(regex, "[[:upper:]]");
	} else if (cp == uchar('w')) {
		cbuild_sb_append_cstr(regex, "[[:alnum:]]");
	} else if (cp == uchar('x')) {
		cbuild_sb_append_cstr(regex, "[[:xdigit:]]");
	} else if (cp == uchar('z')) {
		cbuild_sb_append_arr(regex, "[\0]", 3);
	} else if (cp == uchar('A')) {
		cbuild_sb_append_cstr(regex, "[[:^alpha:]]");
	} else if (cp == uchar('C')) {
		cbuild_sb_append_cstr(regex, "[[:^cntrl:]]");
	} else if (cp == uchar('D')) {
		cbuild_sb_append_cstr(regex, "\\D");
	} else if (cp == uchar('L')) {
		cbuild_sb_append_cstr(regex, "[[:^lower:]]");
	} else if (cp == uchar('P')) {
		cbuild_sb_append_cstr(regex, "[[:^punct:]]");
	} else if (cp == uchar('S')) {
		cbuild_sb_append_cstr(regex, "\\S");
	} else if (cp == uchar('U')) {
		cbuild_sb_append_cstr(regex, "[[:^upper:]]");
	} else if (cp == uchar('W')) {
		cbuild_sb_append_cstr(regex, "[[:^alnum:]]");
	} else if (cp == uchar('X')) {
		cbuild_sb_append_cstr(regex, "[[:^xdigit:]]");
	} else if (cp == uchar('Z')) {
		cbuild_sb_append_arr(regex, "[^\0]", 4);
	} else if (cp == uchar('(')) {
		cbuild_sb_append_cstr(regex, "\\(");
	} else if (cp == uchar(')')) {
		cbuild_sb_append_cstr(regex, "\\)");
	} else if (cp == uchar('.')) {
		cbuild_sb_append_cstr(regex, "\\.");
	} else if (cp == uchar('%')) {
		cbuild_sb_append_cstr(regex, "%");
	} else if (cp == uchar('+')) {
		cbuild_sb_append_cstr(regex, "\\+");
	} else if (cp == uchar('-')) {
		cbuild_sb_append_cstr(regex, "\\-");
	} else if (cp == uchar('*')) {
		cbuild_sb_append_cstr(regex, "\\*");
	} else if (cp == uchar('?')) {
		cbuild_sb_append_cstr(regex, "\\?");
	} else if (cp == uchar('[')) {
		cbuild_sb_append_cstr(regex, "\\[");
	} else if (cp == uchar('^')) {
		cbuild_sb_append_cstr(regex, "\\^");
	} else if (cp == uchar('$')) {
		cbuild_sb_append_cstr(regex, "\\$");
	} else if (cp == uchar('\\')) {
		cbuild_sb_append_cstr(regex, "\\\\");
	} else if(cp == uchar('b')) {
		cbuild_log_error("LUAPAT: '%%b' found!");
		cbuild_sb_append_utf8(regex, cp);
	} else {
		cbuild_sb_append_utf8(regex, cp);
	}
}
cbuild_sb_t __tshl_luapat_to_pcre2(cbuild_sv_t pat) {
	cbuild_sb_t regex = {0};
	// NOTE: No need to explicitly handle '^' and '$' anchors as they are same.
	// NOTE: No need to handle '^' or '-' in character classes as they are same.
	enum {
		DEFAULT,
		PERCENT2,
		CAPTURE,
		CAPTURE_PERCENT2,
		CLASS,
		CLASS_PERCENT2,
	} state = DEFAULT;
	while (pat.size > 0) {
		uint32_t cp = cbuild_sv_chop_utf8(&pat);
		if (state == DEFAULT) {
			if (cp == uchar('%')) {
				state = PERCENT2;
			} else if (cp == uchar('\\')) {
				cbuild_sb_append_cstr(&regex, "\\\\");
			} else if (cp == uchar('[')) {
				cbuild_sb_append(&regex, '[');
				state = CLASS;
			} else if (cp == uchar('(')) {
				cbuild_sb_append(&regex, '(');
				state = CAPTURE;
			} else if (cp == uchar('.')) {
				cbuild_sb_append(&regex, '.');
			} else if (cp == uchar('+')) {
				cbuild_sb_append(&regex, '+');
			} else if (cp == uchar('*')) {
				cbuild_sb_append(&regex, '*');
			} else if (cp == uchar('-')) {
				cbuild_sb_append_cstr(&regex, "*?");
			} else if (cp == uchar('?')) {
				cbuild_sb_append(&regex, '?');
			} else {
				cbuild_sb_append_utf8(&regex, cp);
			}
		} else if (state == PERCENT2) {
			__tshl_luapat_to_pcre2_escape(&regex, cp);
			state = DEFAULT;
		} else if (state == CLASS) {
			if (cp == uchar(']')) {
				cbuild_sb_append(&regex, ']');
				state = DEFAULT;
			} else if (cp == uchar('%')) {
				state = CLASS_PERCENT2;
			} else {
				cbuild_sb_append_utf8(&regex, cp);
			}
		} else if (state == CLASS_PERCENT2) {
			__tshl_luapat_to_pcre2_escape(&regex, cp);
			state = CLASS;
		} else if (state == CAPTURE) {
			if (cp == uchar(')')) {
				cbuild_sb_append(&regex, ')');
				state = DEFAULT;
			} else if (cp == uchar('%')) {
				state = CLASS_PERCENT2;
			} else {
				cbuild_sb_append_utf8(&regex, cp);
			}
		} else if (state == CAPTURE_PERCENT2) {
			__tshl_luapat_to_pcre2_escape(&regex, cp);
			state = CAPTURE;
		} else {
			cbuild_sb_append_utf8(&regex, cp);
		}
	}
	return regex;
}
cbuild_sb_t __tshl_vimregex_to_pcre2(cbuild_sv_t pat, uint32_t *pcre2_flags) {
	cbuild_sb_t ret = {0};
	if (cbuild_sv_prefix(pat, cbuild_sv_from_lit("\\\\c"))) {
		*pcre2_flags = PCRE2_CASELESS;
		cbuild_sv_chop(&pat, 3);
	}
	for (size_t i = 0; i < pat.size; i++) {
		if (pat.data[i] == '\\') {
			if ((i + 2) >= pat.size) {
				cbuild_sb_append(&ret, pat.data[i]);
			} else {
				if (pat.data[i + 1] == '\\') {
					char c = pat.data[i + 2];
					i += 2;
					if (c == '@' || c == '=') {
						cbuild_sb_append(&ret, c);
					} else {
						cbuild_sb_appendf(&ret, "\\\\%c", c);
					}
				} else {
					cbuild_sb_append(&ret, pat.data[i]);
				}
			}
		} else {
			cbuild_sb_append(&ret, pat.data[i]);
		}
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
// Predicate evaluation
////////////////////////////////////////////////////////////////////////////////
cbuild_sv_t __tshl_predicate_get_string(TSQuery* q, TSQueryPredicateStep st,
	const char* cmd, int idx) {
	if (st.type != TSQueryPredicateStepTypeString) {
		cbuild_log_error("Invalid argument %d to command '%s' - string expected.", idx, cmd);
		return (cbuild_sv_t){0};
	}
	uint32_t len = 0;
	const char* data = ts_query_string_value_for_id(q, st.value_id, &len);
	return cbuild_sv_from_parts(data, len);
}
cbuild_sv_t __tshl_predicate_get_capture_name(TSQuery* q, TSQueryPredicateStep st,
	const char* cmd, int idx) {
	if (st.type != TSQueryPredicateStepTypeCapture) {
		cbuild_log_error("Invalid argument %d to command '%s' - capture expected.", idx, cmd);
		return (cbuild_sv_t){0};
	}
	uint32_t len = 0;
	const char* data = ts_query_capture_name_for_id(q, st.value_id, &len);
	return cbuild_sv_from_parts(data, len);
}
__tshl_capture_t* __tshl_predicate_get_capture(TSQuery* q, TSQueryPredicateStep st,
	__tshl_captures_t* captures, const char* cmd, int idx) {
	cbuild_sv_t sv = __tshl_predicate_get_capture_name(q, st, cmd, idx);
	if (sv.data == NULL) return NULL;
	cbuild_da_foreach(*captures, c) {
		if (cbuild_sv_cmp(c->name, sv) == 0) return c;
	}
	return NULL;
}
// Signature: 'set! [capture]? [string] [string]'
bool __tshl_predicate_set(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(text);
	int argid = 1;
	__tshl_capture_t* c = NULL; // This is used only with 'url' and 'bo.commentstring'
	if (steps[*ip].type == TSQueryPredicateStepTypeCapture) {
		c = __tshl_predicate_get_capture(q, steps[(*ip)++], captures, "set!", argid++);
	}
	CBUILD_UNUSED(c);
	cbuild_sv_t var = __tshl_predicate_get_string(q, steps[(*ip)++], "set!", argid++);
	if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.language")) == 0) {
		cbuild_sv_t val = __tshl_predicate_get_string(q, steps[(*ip)++], "set!", argid++);
		bool found = false;
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
				capt->val = val;
				found = true;
			}
		}
		if (!found) {
			__tshl_capture_t capt = {
				.val = val,
				.name = var,
				.priority = 100,
			};
			cbuild_da_append(captures, capt);
		}
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("priority")) == 0) {
		cbuild_sv_t val = __tshl_predicate_get_string(q, steps[(*ip)++], "set!", argid++);
		unsigned int priority = (unsigned int)atoi(cbuild_sv_to_temp_cstr(val));
		cbuild_da_foreach(*captures, capt) {
			capt->priority = priority;
		}
	} else if	(cbuild_sv_cmp(var, cbuild_sv_from_lit("url")) == 0) {
		// TODO: Implement this. And it can use capture as a value...
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("bo.commentstring")) == 0) {
		// Nothing to do here. This is just renderer
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("conceal")) == 0) {
		// TODO: Implement this.
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("conceal_lines")) == 0) {
		// TODO: Implement this.
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.include-children")) == 0) {
		// TODO: Implement this.
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.self")) == 0) {
		bool found = false;
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
				capt->val = lang;
				found = true;
			}
		}
		if (!found) {
			__tshl_capture_t capt = {
				.val = lang,
				.name = var,
				.priority = 100,
			};
			cbuild_da_append(captures, capt);
		}
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.parent")) == 0) {
		bool found = false;
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
				capt->val = plang;
				found = true;
			}
		}
		if (!found) {
			__tshl_capture_t capt = {
				.val = plang,
				.name = var,
				.priority = 100,
			};
			cbuild_da_append(captures, capt);
		}
	} else if (cbuild_sv_cmp(var, cbuild_sv_from_lit("injection.combined")) == 0) {
		// TODO: Implement this.
	} else {
		cbuild_log_error("Invalid variables for 'set!' - '"CBuildSVFmt"'.", CBuildSVArg(var));
		return false;
	}
	return true;
}
// Signature: 'offset! [capture] [string] [string] [string] [string]'
bool __tshl_predicate_offset(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t c = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "offset!", 1);
	int st_row = atoi(cbuild_sv_to_temp_cstr(
			__tshl_predicate_get_string(q, steps[(*ip)++], "offset!", 2)));
	int st_col = atoi(cbuild_sv_to_temp_cstr(
			__tshl_predicate_get_string(q, steps[(*ip)++], "offset!", 3)));
	int end_row = atoi(cbuild_sv_to_temp_cstr(
			__tshl_predicate_get_string(q, steps[(*ip)++], "offset!", 4)));
	int end_col = atoi(cbuild_sv_to_temp_cstr(
			__tshl_predicate_get_string(q, steps[(*ip)++], "offset!", 5)));
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(c, capt->name) == 0) {
			cbuild_sv_t t = cbuild_sv_from_parts(&text.data[capt->start], capt->end - capt->start);
			int st_accum = 0;
			for (; st_row > 0; st_row--) {
				st_accum += ((int)cbuild_sv_chop_by_delim(&t, '\n').size + 1);
			}
			for (; st_col > 0; st_col--) {
				st_accum += cbuild_sv_utf8cp_len(t);
				cbuild_sv_chop_utf8(&t);
			}
			// NOTE: end_col and end_row are always negative in real queries
			int end_accum = 0;
			if (t.data[t.size - 1] == '\n') t.size--;
			for (; end_row < 0; end_row++) {
				end_accum -= ((int)cbuild_sv_chop_right_by_delim(&t, '\n').size + 1);
			}
			cbuild_sv_t last_line = cbuild_sv_chop_right_by_delim(&t, '\n');
			size_t len = cbuild_sv_utf8len(last_line);
			for (size_t i = 0; i < (size_t)((ssize_t)len + end_row); i++) {
				cbuild_sv_chop_utf8(&last_line);
			}
			for (; end_col < 0; end_col++) {
				end_accum -= cbuild_sv_utf8cp_len(last_line);
				cbuild_sv_chop_utf8(&last_line);
			}
			capt->start = (uint32_t)((int)capt->start + st_accum);
			capt->end = (uint32_t)((int)capt->end + end_accum);
		}
	}
	return true;
}
// Signature: 'eq? [string|capture] [string|capture]'
bool __tshl_predicate_eq(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t c1 = {0};
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "eq?", 1);
		c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "eq?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_cmp(base, __tshl_capture_get_sv(*capt, text)) != 0) {
					return false;
				}
			}
		}
		return true;
	}
	c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "eq?", 1);
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "eq?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_cmp(base, __tshl_capture_get_sv(*capt, text)) != 0) {
					return false;
				}
			}
		}
		return true;
	}
	cbuild_sv_t c2 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "eq?", 2);
	cbuild_da_foreach(*captures, capt1) {
		if (cbuild_sv_cmp(c1, capt1->name) == 0) {
			cbuild_da_foreach(*captures, capt2) {
				if (cbuild_sv_cmp(c2, capt2->name) == 0) {
					if (cbuild_sv_cmp(__tshl_capture_get_sv(*capt1, text),
							__tshl_capture_get_sv(*capt2, text)) != 0) {
						return false;
					}
				}
			}
		}
		return true;
	}
	return true;
}
// Signature: 'any-eq? [string|capture] [string|capture]'
bool __tshl_predicate_any_eq(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t c1 = {0};
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "any-eq?", 1);
		c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-eq?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_cmp(base, __tshl_capture_get_sv(*capt, text)) == 0) {
					return true;
				}
			}
		}
		return false;
	}
	c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-eq?", 1);
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "any-eq?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_cmp(base, __tshl_capture_get_sv(*capt, text)) == 0) {
					return true;
				}
			}
		}
		return false;
	}
	cbuild_sv_t c2 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-eq?", 2);
	cbuild_da_foreach(*captures, capt1) {
		if (cbuild_sv_cmp(c1, capt1->name) == 0) {
			cbuild_da_foreach(*captures, capt2) {
				if (cbuild_sv_cmp(c2, capt2->name) == 0) {
					if (cbuild_sv_cmp(__tshl_capture_get_sv(*capt1, text),
							__tshl_capture_get_sv(*capt2, text)) == 0) {
						return true;
					}
				}
			}
		}
		return false;
	}
	return false;
}
// Signature: 'contains? [string|capture] [string|capture]'
bool __tshl_predicate_contains(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t c1 = {0};
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "contains?", 1);
		c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "contains?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (!cbuild_sv_contains_sv(base, __tshl_capture_get_sv(*capt, text))) {
					return false;
				}
			}
		}
		return true;
	}
	c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "contains?", 1);
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "contains?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (!cbuild_sv_contains_sv(base, __tshl_capture_get_sv(*capt, text))) {
					return false;
				}
			}
		}
		return true;
	}
	cbuild_sv_t c2 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "contains?", 2);
	cbuild_da_foreach(*captures, capt1) {
		if (cbuild_sv_cmp(c1, capt1->name) == 0) {
			cbuild_da_foreach(*captures, capt2) {
				if (cbuild_sv_cmp(c2, capt2->name) == 0) {
					if (!cbuild_sv_contains_sv(__tshl_capture_get_sv(*capt1, text),
							__tshl_capture_get_sv(*capt2, text))) {
						return false;
					}
				}
			}
		}
		return true;
	}
	return true;
}
// Signature: 'any-contains? [string|capture] [string|capture]'
bool __tshl_predicate_any_contains(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t c1 = {0};
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "any-contains?", 1);
		c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-contains?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_contains_sv(base, __tshl_capture_get_sv(*capt, text))) {
					return true;
				}
			}
		}
		return false;
	}
	c1 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-contains?", 1);
	if (steps[*ip].type == TSQueryPredicateStepTypeString) {
		cbuild_sv_t base = __tshl_predicate_get_string(q, steps[(*ip)++], "any-contains?", 2);
		cbuild_da_foreach(*captures, capt) {
			if (cbuild_sv_cmp(c1, capt->name) == 0) {
				if (cbuild_sv_contains_sv(base, __tshl_capture_get_sv(*capt, text))) {
					return true;
				}
			}
		}
		return false;
	}
	cbuild_sv_t c2 = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-contains?", 2);
	cbuild_da_foreach(*captures, capt1) {
		if (cbuild_sv_cmp(c1, capt1->name) == 0) {
			cbuild_da_foreach(*captures, capt2) {
				if (cbuild_sv_cmp(c2, capt2->name) == 0) {
					if (cbuild_sv_contains_sv(__tshl_capture_get_sv(*capt1, text),
							__tshl_capture_get_sv(*capt2, text))) {
						return true;
					}
				}
			}
		}
		return false;
	}
	return false;
}
// Signature 'any-of? [capture] [string...]'
bool __tshl_predicate_any_of(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	int argidx = 1;
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-of?", argidx++);
	__tshl_da_sv_t strings = {0};
	while (steps[*ip].type != TSQueryPredicateStepTypeDone) {
		cbuild_sv_t str = __tshl_predicate_get_string(q, steps[(*ip)++], "any-of?", argidx++);
		cbuild_da_append(&strings, str);
	}
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_da_foreach(strings, str) {
				if (cbuild_sv_cmp(__tshl_capture_get_sv(*capt, text), *str) == 0) {
					cbuild_da_clear(&strings);
					return true;
				}
			}
		}
	}
	cbuild_da_clear(&strings);
	return false;
}
// 'has-parent?' [capture] [string...]
bool __tshl_predicate_has_parent(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	CBUILD_UNUSED(text);
	int argidx = 1;
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "has-parent?", argidx++);
	__tshl_da_sv_t strings = {0};
	while (steps[*ip].type != TSQueryPredicateStepTypeDone) {
		cbuild_sv_t str = __tshl_predicate_get_string(q, steps[(*ip)++], "has-parent?", argidx++);
		cbuild_da_append(&strings, str);
	}
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			TSNode pnode = ts_node_parent(capt->node);
			if (!ts_node_is_null(pnode)) {
				const char* cparent = ts_node_type(pnode);
				cbuild_sv_t parent = cbuild_sv_from_cstr(cparent);
				cbuild_da_foreach(strings, str) {
					if (cbuild_sv_cmp(parent, *str) == 0) {
						cbuild_da_clear(&strings);
						return true;
					}
				}
			}
		}
	}
	cbuild_da_clear(&strings);
	return false;
}
// 'has-ancestor?' [capture] [string...]
bool __tshl_predicate_has_ancestor(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	CBUILD_UNUSED(text);
	int argidx = 1;
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "has-ancestor?", argidx++);
	__tshl_da_sv_t strings = {0};
	while (steps[*ip].type != TSQueryPredicateStepTypeDone) {
		cbuild_sv_t str = __tshl_predicate_get_string(q, steps[(*ip)++], "has-ancestor?", argidx++);
		cbuild_da_append(&strings, str);
	}
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			TSNode pnode = ts_tree_root_node(capt->node.tree);
			while (pnode.id != capt->node.id && !ts_node_is_null(pnode)) {
				const char* cparent = ts_node_type(pnode);
				cbuild_sv_t parent = cbuild_sv_from_cstr(cparent);
				cbuild_da_foreach(strings, str) {
					if (cbuild_sv_cmp(parent, *str) == 0) {
						cbuild_da_clear(&strings);
						return true;
					}
				}
				pnode = ts_node_child_with_descendant(pnode, capt->node);
			}
		}
	}
	cbuild_da_clear(&strings);
	return false;
}
bool __tshl_predicate_lua_match(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "lua-match?", 1);
	cbuild_sv_t pat_arg = __tshl_predicate_get_string(q, steps[(*ip)++], "lua-match?", 2);
	cbuild_sb_t pat = __tshl_luapat_to_pcre2(pat_arg);
	int err = 0;
	PCRE2_SIZE erroffset = 0;
	pcre2_code* reg = pcre2_compile((PCRE2_SPTR)pat.data, pat.size,
		PCRE2_UTF | PCRE2_UCP, &err, &erroffset, NULL);
	if (reg == NULL) {
		cbuild_sb_clear(&pat);
		return false;
	}
	pcre2_match_data *md = pcre2_match_data_create_from_pattern(reg, NULL);
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_sv_t ntext = __tshl_capture_get_sv(*capt, text);
			int res = pcre2_match(reg, (PCRE2_SPTR)ntext.data, ntext.size, 0, 0, md, NULL);
			if (res < 0) {
				pcre2_match_data_free(md);
				pcre2_code_free(reg);
				cbuild_sb_clear(&pat);
				return false;
			}
		}
	}
	pcre2_match_data_free(md);
	pcre2_code_free(reg);
	cbuild_sb_clear(&pat);
	return true;
}
bool __tshl_predicate_any_lua_match(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-lua-match?", 1);
	cbuild_sv_t pat_arg = __tshl_predicate_get_string(q, steps[(*ip)++], "any-lua-match?", 2);
	cbuild_sb_t pat = __tshl_luapat_to_pcre2(pat_arg);
	int err = 0;
	PCRE2_SIZE erroffset = 0;
	pcre2_code* reg = pcre2_compile((PCRE2_SPTR)pat.data, pat.size,
		PCRE2_UTF | PCRE2_UCP, &err, &erroffset, NULL);
	if (reg == NULL) {
		cbuild_sb_clear(&pat);
		return false;
	}
	pcre2_match_data *md = pcre2_match_data_create_from_pattern(reg, NULL);
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_sv_t ntext = __tshl_capture_get_sv(*capt, text);
			int res = pcre2_match(reg, (PCRE2_SPTR)ntext.data, ntext.size, 0, 0, md, NULL);
			if (res >= 0) {
				pcre2_match_data_free(md);
				pcre2_code_free(reg);
				cbuild_sb_clear(&pat);
				return true;
			}
		}
	}
	pcre2_match_data_free(md);
	pcre2_code_free(reg);
	cbuild_sb_clear(&pat);
	return false;
}
bool __tshl_predicate_gsub(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "gsub!", 1);
	cbuild_sv_t pat_arg = __tshl_predicate_get_string(q, steps[(*ip)++], "gsub!", 2);
	cbuild_sv_t repl = __tshl_predicate_get_string(q, steps[(*ip)++], "gsub!", 3);
	cbuild_sb_t pat = __tshl_luapat_to_pcre2(pat_arg);
	int err = 0;
	PCRE2_SIZE erroffset = 0;
	pcre2_code* reg = pcre2_compile((PCRE2_SPTR)pat.data, pat.size,
		PCRE2_UTF | PCRE2_UCP, &err, &erroffset, NULL);
	if (reg == NULL) {
		cbuild_sb_clear(&pat);
		return false;
	}
	pcre2_match_data *md = pcre2_match_data_create_from_pattern(reg, NULL);
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_sv_t ntext = __tshl_capture_get_sv(*capt, text);
			int res = pcre2_match(reg, (PCRE2_SPTR)ntext.data, ntext.size, 0, 0, md, NULL);
			if (res >= 0) {
				cbuild_sb_t new = {0};
				for (size_t i = 0; i < repl.size; i++) {
					if (repl.data[i] == '%' && ++i <= repl.size) {
						// NOTE: This support up to 9 capture groups and group 0 as full match.
						char digit = repl.data[i];
						if ('0' <= digit && digit <= '9') {
							int capt_id = digit - '0';
							uint32_t ovector_len = pcre2_get_ovector_count(md);
							PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(md);
							if ((uint32_t)capt_id >= ovector_len) {
								cbuild_sb_append(&new, '%');
								cbuild_sb_append(&new, digit);
							} else {
								size_t start = ovector[2*capt_id];
								size_t end = ovector[2*capt_id+1];
								cbuild_sv_t capt_sv = cbuild_sv_from_parts(&ntext.data[start], end - start);
								cbuild_sb_append_sv(&new, capt_sv);
							}
						} else {
							cbuild_sb_append(&new, digit);
						}
					} else {
						cbuild_sb_append(&new, repl.data[i]);
					}
				}
				capt->val = __tshl_temp_svdup(cbuild_sv_from_sb(new));
				cbuild_sb_clear(&new);
			}
			
		}
	}
	pcre2_match_data_free(md);
	pcre2_code_free(reg);
	cbuild_sb_clear(&pat);
	return true;
}
bool __tshl_predicate_vim_match(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "match?", 1);
	cbuild_sv_t pat_arg = __tshl_predicate_get_string(q, steps[(*ip)++], "match?", 2);
	uint32_t pcre2_flags = 0;
	cbuild_sb_t pat = __tshl_vimregex_to_pcre2(pat_arg, &pcre2_flags);
	int err = 0;
	PCRE2_SIZE erroffset = 0;
	pcre2_code* reg = pcre2_compile((PCRE2_SPTR)pat.data, pat.size,
		pcre2_flags | PCRE2_UTF | PCRE2_UCP, &err, &erroffset, NULL);
	if (reg == NULL) {
		cbuild_sb_clear(&pat);
		return false;
	}
	pcre2_match_data *md = pcre2_match_data_create_from_pattern(reg, NULL);
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_sv_t ntext = __tshl_capture_get_sv(*capt, text);
			int res = pcre2_match(reg, (PCRE2_SPTR)ntext.data, ntext.size, 0, 0, md, NULL);
			if (res < 0) {
				pcre2_match_data_free(md);
				pcre2_code_free(reg);
				cbuild_sb_clear(&pat);
				return false;
			}
		}
	}
	pcre2_match_data_free(md);
	pcre2_code_free(reg);
	cbuild_sb_clear(&pat);
	return true;
}
bool __tshl_predicate_any_vim_match(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	CBUILD_UNUSED(lang);
	CBUILD_UNUSED(plang);
	cbuild_sv_t node = __tshl_predicate_get_capture_name(q, steps[(*ip)++], "any-match?", 1);
	cbuild_sv_t pat_arg = __tshl_predicate_get_string(q, steps[(*ip)++], "any-match?", 2);
	uint32_t pcre2_flags = 0;
	cbuild_sb_t pat = __tshl_vimregex_to_pcre2(pat_arg, &pcre2_flags);
	int err = 0;
	PCRE2_SIZE erroffset = 0;
	pcre2_code* reg = pcre2_compile((PCRE2_SPTR)pat.data, pat.size,
		pcre2_flags | PCRE2_UTF | PCRE2_UCP, &err, &erroffset, NULL);
	if (reg == NULL) {
		cbuild_sb_clear(&pat);
		return false;
	}
	pcre2_match_data *md = pcre2_match_data_create_from_pattern(reg, NULL);
	cbuild_da_foreach(*captures, capt) {
		if (cbuild_sv_cmp(node, capt->name) == 0) {
			cbuild_sv_t ntext = __tshl_capture_get_sv(*capt, text);
			int res = pcre2_match(reg, (PCRE2_SPTR)ntext.data, ntext.size, 0, 0, md, NULL);
			if (res >= 0) {
				pcre2_match_data_free(md);
				pcre2_code_free(reg);
				cbuild_sb_clear(&pat);
				return true;
			}
		}
	}
	pcre2_match_data_free(md);
	pcre2_code_free(reg);
	cbuild_sb_clear(&pat);
	return false;
}
struct {
	cbuild_sv_t name;
	bool (*eval)(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang);
} __TSHL_PREDICATES[] = {
	{
		.name = cbuild_sv_from_lit("set!"),
		.eval = __tshl_predicate_set,
	},
	{
		.name = cbuild_sv_from_lit("offset!"),
		.eval = __tshl_predicate_offset,
	},
	{
		.name = cbuild_sv_from_lit("gsub!"),
		.eval = __tshl_predicate_gsub,
	},
	{
		.name = cbuild_sv_from_lit("eq?"),
		.eval = __tshl_predicate_eq,
	},
	{
		.name = cbuild_sv_from_lit("any-eq?"),
		.eval = __tshl_predicate_any_eq,
	},
	{
		.name = cbuild_sv_from_lit("contains?"),
		.eval = __tshl_predicate_contains,
	},
	{
		.name = cbuild_sv_from_lit("any-contains?"),
		.eval = __tshl_predicate_any_contains,
	},
	{
		.name = cbuild_sv_from_lit("any-of?"),
		.eval = __tshl_predicate_any_of,
	},
	{
		.name = cbuild_sv_from_lit("has-parent?"),
		.eval = __tshl_predicate_has_parent,
	},
	{
		.name = cbuild_sv_from_lit("has-ancestor?"),
		.eval = __tshl_predicate_has_ancestor,
	},
	{
		.name = cbuild_sv_from_lit("lua-match?"),
		.eval = __tshl_predicate_lua_match,
	},
	{
		.name = cbuild_sv_from_lit("any-lua-match?"),
		.eval = __tshl_predicate_any_lua_match,
	},
	{
		.name = cbuild_sv_from_lit("match?"),
		.eval = __tshl_predicate_vim_match,
	},
	{
		.name = cbuild_sv_from_lit("any-match?"),
		.eval = __tshl_predicate_any_vim_match,
	},
	{
		.name = cbuild_sv_from_lit("vim-match?"),
		.eval = __tshl_predicate_vim_match,
	},
	{
		.name = cbuild_sv_from_lit("any-vim-match?"),
		.eval = __tshl_predicate_any_vim_match,
	},
};
bool __tshl_eval_predicate(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t* ip, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	bool ret = true;
	cbuild_sv_t func = __tshl_predicate_get_string(q, steps[(*ip)++], "???", 0);
	if (func.data == NULL) {
		cbuild_log_error("Predicate does not start from function name.");
		ret = false;
	} else {
		bool invert = false;
		if (cbuild_sv_prefix(func, cbuild_sv_from_lit("not-")) &&
			cbuild_sv_suffix(func, cbuild_sv_from_lit("?"))) {
			invert = true;
			cbuild_sv_chop(&func, 4); // Strip 'not-'
		}
		for (size_t i = 0; i < cbuild_arr_len(__TSHL_PREDICATES); i++) {
			if (cbuild_sv_cmp(func, __TSHL_PREDICATES[i].name) == 0) {
				if (!__TSHL_PREDICATES[i].eval(q, steps, ip, captures, text, lang, plang)) {
					ret = false;
				}
				break;
			}
		}
		if (invert) ret = !ret;
	}
	while (steps[(*ip)++].type != TSQueryPredicateStepTypeDone);
	return ret;
}
bool __tshl_eval_predicates(TSQuery* q, const TSQueryPredicateStep* steps, uint32_t step_count, __tshl_captures_t* captures, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang) {
	uint32_t ip = 0;
	bool ret = true;
	while (ip < step_count) {
		ret &= __tshl_eval_predicate(q, steps, &ip, captures, text, lang, plang);	
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
// Parsing + highlight
////////////////////////////////////////////////////////////////////////////////
int __tshl_capture_cmp(const void* e1, const void* e2) {
	const __tshl_capture_t* a = e1;
	const __tshl_capture_t* b = e2;
	if (a->start < b->start) {
		return -1;
	} else if (a->start > b->start) {
		return 1;
	} else {
		if (a->end < b->end) {
			return 1;
		} else if (a->end > b->end) {
			return -1;
		} else {
			if (a->idx < b->idx) {
				return -1;
			} else if (a->idx > b->idx) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}
void __tshl_highlight(tshl_t* self, cbuild_sv_t text, cbuild_sv_t lang, cbuild_sv_t plang, uint32_t offset, tshl_metadata_t* meta) {
	// Setup
	size_t checkpoint = cbuild_temp_checkpoint();
	__tshl_language_map_entry_t* p = cbuild_map_get(&self->languages, lang);
	if (p == NULL) {
		if (!__tshl_load_parser(self, lang)) {
			cbuild_log_error("Could not load "CBuildSVFmt" parser.", CBuildSVArg(lang));
			cbuild_temp_reset(checkpoint);
			return;
		}
		if (!__tshl_load_queries(self, lang)) {
			cbuild_log_error("Could not load "CBuildSVFmt" queries.", CBuildSVArg(lang));
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
	if (tree == NULL) {
		cbuild_log_error("Failed to parse '"CBuildSVFmt"'.", CBuildSVArg(lang));
		cbuild_temp_reset(checkpoint);
		return;
	}
	TSNode root = ts_tree_root_node(tree);
	TSQueryCursor* c = ts_query_cursor_new();
	// Highlight queries
	__tshl_query_map_entry_t* q = cbuild_map_get(&self->queries, lang);
	if (q == NULL) {
		ts_query_cursor_delete(c);
		ts_tree_delete(tree);
		cbuild_temp_reset(checkpoint);
		return;
	}
	TSQueryMatch match = {0};
	__tshl_captures_t captures = {0};
	ts_query_cursor_exec(c, q->hl, root);
	while(ts_query_cursor_next_match(c, &match)) {
		for (uint16_t i = 0; i < match.capture_count; i++) {
			TSQueryCapture capt = match.captures[i];
			uint32_t name_len = 0;
			const char* name = ts_query_capture_name_for_id(q->hl, capt.index, &name_len);
			uint32_t start = ts_node_start_byte(capt.node);
			uint32_t end = ts_node_end_byte(capt.node);
			cbuild_sv_t st_name = cbuild_sv_from_parts(name, name_len);
			__tshl_capture_t capture = {
				.start = start, 
				.end = end, 
				.idx = match.pattern_index,
				.priority = 100,
				.name = st_name,
				.node = capt.node,
			};
			cbuild_da_append(&captures, capture);
		}
		bool valid = true;
		const TSQueryPredicateStep* predicate = NULL;
		uint32_t step_count = 0;
		predicate = ts_query_predicates_for_pattern(q->hl, match.pattern_index, &step_count);
		if (predicate != NULL) {
			valid = __tshl_eval_predicates(q->hl, predicate, step_count, &captures, text, lang, plang);
		}
		if (!valid) captures.size -= match.capture_count;
	}
	qsort(captures.data, captures.size, sizeof(__tshl_capture_t), __tshl_capture_cmp);
	cbuild_da_foreach(captures, capt) {
		if (cbuild_sv_cmp(lang, cbuild_sv_from_lit("yaml")) == 0) printf(CBuildSVFmt"\n", CBuildSVArg(capt->name));
		if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("spell")) == 0) continue;
		if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("nospell")) == 0) continue;
		if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("conceal")) == 0) continue;
		if (capt->name.data[0] == '_') continue;
		enum tshl_style_t style = tshl_name_to_style(capt->name);
		for (uint32_t i = capt->start; i < capt->end; i++) {
			meta[offset+i].style[capt->priority - 90] = style;
		}
	}
	captures.size = 0;
	// Injection queries
	if (q->inj != NULL) {
		TSQuery* inj = q->inj; // Map may realloc somehow.
		ts_query_cursor_exec(c, inj, root);
		while(ts_query_cursor_next_match(c, &match)) {
			captures.size = 0;
			for (uint16_t i = 0; i < match.capture_count; i++) {
				TSQueryCapture capt = match.captures[i];
				uint32_t name_len = 0;
				const char* name = ts_query_capture_name_for_id(inj, capt.index, &name_len);
				uint32_t start = ts_node_start_byte(capt.node);
				uint32_t end = ts_node_end_byte(capt.node);
				cbuild_sv_t st_name = cbuild_sv_from_parts(name, name_len);
				__tshl_capture_t capture = {
					.start = start, 
					.end = end, 
					.name = st_name,
				};
				cbuild_da_append(&captures, capture);
			}
			bool valid = true;
			const TSQueryPredicateStep* predicate = NULL;
			uint32_t step_count = 0;
			predicate = ts_query_predicates_for_pattern(inj, match.pattern_index, &step_count);
			if (predicate != NULL) {
				valid = __tshl_eval_predicates(inj, predicate, step_count, &captures, text, lang, plang);
			}
			cbuild_sv_t inj_lang = {0};
			cbuild_sv_t inj_text = {0};
			uint32_t inj_offset = offset;
			// TODO: In general to implement injection.combined and properly handle this I need
			// to create "injections buffer" and then collect all injections there, as list of
			// non-overlapping ranges (but multiple ranges for entry is possible) and then
			// create new strings if needed and run highlight on these new ranges.
			// TODO: Need to do same sorting as highlights
			cbuild_da_foreach(captures, capt) {
				if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.language")) == 0) {
					inj_lang = __tshl_capture_get_sv(*capt, text);
				} else if (cbuild_sv_cmp(capt->name, cbuild_sv_from_lit("injection.content")) == 0) {
					// TODO: There can be multiple of this and they may need to be combined.
					inj_offset += capt->start;
					inj_text = __tshl_capture_get_sv(*capt, text);
				}
			}
			if (valid) {
				cbuild_sv_trim(&inj_lang);
				if (cbuild_sv_cmp(inj_lang, cbuild_sv_from_lit("text")) == 0) continue;
				if (inj_lang.size == 0) continue;
				__tshl_highlight(self, inj_text, inj_lang, lang, inj_offset, meta);
			}
		}
	}
	cbuild_da_clear(&captures);
	ts_query_cursor_delete(c);
	ts_tree_delete(tree);
	cbuild_temp_reset(checkpoint);
}
tshl_metadata_t* tshl_highlight(tshl_t* self, cbuild_sv_t text, cbuild_sv_t lang) {
	tshl_metadata_t* meta = malloc(sizeof(tshl_metadata_t)*text.size);
	memset(meta, 0, sizeof(tshl_metadata_t)*text.size);
	cbuild_assert(meta!= NULL, "Allocation failed.");
	__tshl_highlight(self, text, lang, lang, 0, meta);
	return meta;
}
////////////////////////////////////////////////////////////////////////////////
// Conversions between style as enum and style as string
////////////////////////////////////////////////////////////////////////////////
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
	case TSHL_NUM_STYLES: CBUILD_UNREACHABLE("Invalid style."); break;
	}
	CBUILD_UNREACHABLE("Invalid style.");
}
#pragma endregion
#pragma region tshl_name_to_style
enum tshl_style_t tshl_name_to_style(cbuild_sv_t name) {
	if (cbuild_sv_cmp(name, cbuild_sv_from_lit("default")) == 0) {
		return TSHL_DEFAULT;
	} else if (cbuild_sv_cmp(name, cbuild_sv_from_lit("none")) == 0) {
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
