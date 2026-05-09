//! License: GPl-3.0-or-later
#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_TRACE
#define CBUILD_IMPLEMENTATION
#include "cbuild.h"
int main(int argc, char** argv) {
	cbuild_selfrebuild(argc, argv);
	cbuild_shift(argv, argc);
	// TODO: Find proper way to run tests. Probably I need to dump some highlight from neovim and compare them? On same theme? But for now simple manual testing is fine. And I would need custom test-case as random third-party markdowns in repo are not the best thing. Also, text runner should not depend on my neovim install path.
	const char* op = cbuild_shift_expect(argv, argc, "Subcommand required (one of 'run', 'build').");
	cbuild_cmd_t cmd = {0};
	if (strcmp(op, "run") == 0) {
		if (cbuild_compare_mtime_many("ts-highlight.run", 
				(const char*[]){"src/main.c", "src/ts-highlight.c", "src/ts-highlight.h"},
				3) > 0) {
			cbuild_cmd_append_many(&cmd, CC);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_STD("gnu23"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_WARN);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_DEBUG_GDB);
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("tree-sitter"));
			cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("dl"));
			cbuild_cmd_append_many(&cmd, "-Wl,-z,origin", "-Wl,-rpath,$ORIGIN");
			cbuild_cmd_append_many(&cmd, "src/main.c", "src/ts-highlight.c");
			cbuild_cmd_append_many(&cmd, "-o", "ts-highlight.run");
			if (!cbuild_cmd_run(&cmd)) return 1;
			cmd.size = 0;
		}
		cbuild_cmd_append(&cmd, "./ts-highlight.run");
		cbuild_cmd_append_arr(&cmd, argv, (size_t)argc);
		if (!cbuild_cmd_run(&cmd)) return 1;
	} else if (strcmp(op, "build") == 0) {
		// TODO: Amalgamate library. It is small-enough to be used as a single-header library.
	} else {
		cbuild_log_error("Invalid subcommand. Expected one of 'run', 'build'.");
		return 1;
	}
	cbuild_cmd_clear(&cmd);
	return 0;
}
