//! Licese: GPL-3.0-or-later

#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_INFO
#include "../cbuild.h"
#include "stdio.h"
#include "tree_sitter/api.h"
#include "ts-highlight/ts-highlight.h"

#define PREFIX "/home/wolodiam/.local/share/nvim/site"
TSLanguage* load_parser(cbuild_sv_t language) {
	const char* lib = cbuild_temp_sprintf(PREFIX"/parser/"CBuildSVFmt".so", CBuildSVArg(language));
	void* handle = cbuild_dlib_open(lib, false);
	if (handle == NULL) return NULL;
	TSLanguage* (*fn)(void) = dlsym(handle, cbuild_temp_sprintf("tree_sitter_"CBuildSVFmt, CBuildSVArg(language)));
	if (fn == NULL) return NULL;
	return fn();
}
const char* get_query_dir(cbuild_sv_t language) {
	return cbuild_temp_sprintf(PREFIX"/queries/"CBuildSVFmt, CBuildSVArg(language));
}
int main(int argc, char** argv) {
	cbuild_sb_t sb = {0};
	cbuild_shift(argv, argc);
	const char* file = cbuild_shift(argv, argc);
	if (!cbuild_file_read(file, &sb)) return 1;
	tshl_t tshl = tshl_init(load_parser, get_query_dir);
	tshl_spans_t spans = tshl_highlight(&tshl, cbuild_sv_from_sb(sb), cbuild_sv_from_lit("markdown"));
	printf("Generated %zu spans.\n", spans.size);
	cbuild_da_foreach(spans, span) {
		printf("Span '"CBuildSVFmt"' with style '%s'\n",
			CBuildSVArg(span->text), tshl_style_to_name(span->style));
	}
	return 0;
}
#include "ts-highlight/ts-highlight.c"
#define CBUILD_IMPLEMENTATION
#include "../cbuild.h"
