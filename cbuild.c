//! License: GPl-3.0-or-later
#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_TRACE
#define CBUILD_IMPLEMENTATION
#include "cbuild.h"
int main(int argc, char** argv) {
	cbuild_selfrebuild(argc, argv);
	cbuild_shift(argv, argc);
	// TODO: Find proper way to run tests. Probably I need to dump some highlight from neovim and compare them? On same theme? But for now simple manual testing is fine. And I would need custom test-case as random third-party markdowns in repo are not the best thing. Also, text runner should not depend on my neovim install path.
	const char* op = cbuild_shift_expect(argv, argc, "Subcommand required (one of 'test', 'print', 'build').\n");
	cbuild_cmd_t cmd = {0};
	if (strcmp(op, "test") == 0) {
		if (cbuild_compare_mtime_many("ts-highlight-test.run", 
				(const char*[]){"src/test.c", "src/ts-highlight.c", "src/ts-highlight.h"},
				3) > 0) {
			cbuild_cmd_append_many(&cmd, CC);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_STD("gnu23"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_WARN);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_DEBUG_GDB);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("tree-sitter"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("pcre2-8"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("dl"));
			cbuild_cmd_append_many(&cmd, "-Wl,-z,origin", "-Wl,-rpath,$ORIGIN");
			cbuild_cmd_append_many(&cmd, "src/test.c", "src/ts-highlight.c");
			cbuild_cmd_append_many(&cmd, "-o", "ts-highlight-test.run");
			if (!cbuild_cmd_run(&cmd)) return 1;
			cmd.size = 0;
		}
		cbuild_cmd_append(&cmd, "./ts-highlight-test.run");
		cbuild_cmd_append_arr(&cmd, argv, (size_t)argc);
		if (!cbuild_cmd_run(&cmd)) return 1;
	} else if (strcmp(op, "print") == 0) {
		if (cbuild_compare_mtime_many("ts-highlight-print.run", 
				(const char*[]){"src/print.c", "src/ts-highlight.c", "src/ts-highlight.h", "src/ts-highlight-print.c", "src/ts-highlight-print.h"},
				5) > 0) {
			cbuild_cmd_append_many(&cmd, CC);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_STD("gnu23"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_WARN);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_DEBUG_GDB);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("tree-sitter"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("pcre2-8"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("dl"));
			cbuild_cmd_append_many(&cmd, "-Wl,-z,origin", "-Wl,-rpath,$ORIGIN");
			cbuild_cmd_append_many(&cmd, "src/print.c", "src/ts-highlight.c", "src/ts-highlight-print.c");
			cbuild_cmd_append_many(&cmd, "-o", "ts-highlight-print.run");
			if (!cbuild_cmd_run(&cmd)) return 1;
			cmd.size = 0;
		}
		cbuild_cmd_append(&cmd, "./ts-highlight-print.run");
		cbuild_cmd_append_arr(&cmd, argv, (size_t)argc);
		if (!cbuild_cmd_run(&cmd)) return 1;
	} else if (strcmp(op, "build") == 0) {
		cbuild_sb_t output = {0};
		cbuild_sb_append_cstr(&output, "// ts-highlight.h by WolodiaM\n");
		cbuild_sb_append_cstr(&output, "// License: GPL-3.0-or-later\n");
		if (!cbuild_file_read("src/ts-highlight.h", &output)) return 1;
		cbuild_sb_append_cstr(&output, "\n");
		cbuild_sb_append_cstr(&output, "#ifndef TSHL_IMPLEMENTATION\n");
		cbuild_sb_append_cstr(&output, "#define TSHL_IMPLEMENTATION\n");
		cbuild_sb_t src = {0};
		if (!cbuild_file_read("src/ts-highlight.c", &src)) return 1;
		cbuild_sv_t content = cbuild_sv_from_sb(src);
		while (content.size > 0) {
			cbuild_sv_t line = cbuild_sv_chop_by_delim(&content, '\n');
			if (cbuild_sv_prefix(line, cbuild_sv_from_lit("#include"))) {
				cbuild_sb_append_cstr(&output, "// ");
			}
			cbuild_sb_append_sv(&output, line);
			cbuild_sb_append_cstr(&output, "\n");
		}
		cbuild_sb_append_cstr(&output, "#endif // TSHL_IMPLEMENTATION");
		if (!cbuild_file_write("ts-highlight.h", &output)) return 1;
		output.size = 0;
		cbuild_sb_append_cstr(&output, "// ts-highlight-print.h by WolodiaM\n");
		cbuild_sb_append_cstr(&output, "// License: GPL-3.0-or-later\n");
		if (!cbuild_file_read("src/ts-highlight-print.h", &output)) return 1;
		cbuild_sb_append_cstr(&output, "\n");
		cbuild_sb_append_cstr(&output, "#ifndef TSHL_PRINT_IMPLEMENTATION\n");
		cbuild_sb_append_cstr(&output, "#define TSHL_PRINT_IMPLEMENTATION\n");
		src.size = 0;
		if (!cbuild_file_read("src/ts-highlight-print.c", &src)) return 1;
		content = cbuild_sv_from_sb(src);
		while (content.size > 0) {
			cbuild_sv_t line = cbuild_sv_chop_by_delim(&content, '\n');
			if (cbuild_sv_prefix(line, cbuild_sv_from_lit("#include"))) {
				cbuild_sb_append_cstr(&output, "// ");
			}
			cbuild_sb_append_sv(&output, line);
			cbuild_sb_append_cstr(&output, "\n");
		}
		cbuild_sb_append_cstr(&output, "#endif // TSHL_PRINT_IMPLEMENTATION");
		if (!cbuild_file_write("ts-highlight-print.h", &output)) return 1;
	} else {
		cbuild_log_error("Invalid subcommand. Expected one of 'run', 'build'.");
		return 1;
	}
	cbuild_cmd_clear(&cmd);
	return 0;
}
