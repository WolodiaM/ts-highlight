//! Licese: GPL-3.0-or-later

#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_INFO
#include "../cbuild.h"
#include "stdio.h"
#include "tree_sitter/api.h"
#include "ts-highlight.h"
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
int main(int argc, char** argv) {
	cbuild_sb_t sb = {0};
	cbuild_shift(argv, argc);
	const char* file = cbuild_shift(argv, argc);
	if (!cbuild_file_read(file, &sb)) return 1;
	tshl_t tshl = tshl_init(load_parser, get_query_dir);
	tshl_metadata_t* meta = tshl_highlight(&tshl, cbuild_sv_from_sb(sb), cbuild_sv_from_lit("c"));
	enum tshl_style_t curr = meta[0].style[10];
	printf("Span '");
	for (size_t i = 0; i < sb.size; i++) {
		if (curr != meta[i].style[10]) {
			printf("' with style %s.\n", tshl_style_to_name(curr));
			printf("Span '");
		}
		curr = meta[i].style[10];
		printf("%c", sb.data[i]);
	}
	return 0;
}
