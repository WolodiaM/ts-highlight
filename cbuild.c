//! License: GPl-3.0-or-later
#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_TRACE
#define CBUILD_IMPLEMENTATION
#include "cbuild.h"
bool cmd_append_pkgconfig(cbuild_cmd_t* cmd, const char* lib) {
	cbuild_cmd_t pkg_cmd = {0};
	cbuild_cmd_append_many(&pkg_cmd, "pkg-config",
		lib, "--cflags","--libs");
	cbuild_fd_t rfd;
	cbuild_fd_t wfd;
	if(cbuild_fd_open_pipe(&rfd, &wfd)) {
		if (!cbuild_cmd_run(&pkg_cmd, .fdstdout = &wfd)) return false;
		char buff[1025] = {0};
		ssize_t len = read(rfd, buff, 1024);
		if(len < 0) {
			cbuild_log(CBUILD_LOG_ERROR, "pkg-config is broken.");
			return false;
		}
		cbuild_sv_t output = cbuild_sv_from_cstr(buff);
		while(output.size > 0) {
			cbuild_sv_t arg = cbuild_sv_chop_by_delim(&output, ' ');
			cbuild_sv_trim(&arg);
			if(arg.size != 0) {
				cbuild_cmd_append(cmd, cbuild_temp_sprintf(CBuildSVFmt, CBuildSVArg(arg)));
			}
		}
		cbuild_fd_close(rfd);
		cbuild_fd_close(wfd);
	} else {
		cbuild_log(CBUILD_LOG_ERROR,
			"Could not link required libs, expect compilation error.");
		return false;
	}
	return true;
}
int main(int argc, char** argv) {
	cbuild_selfrebuild(argc, argv);
	// Build main app
	cbuild_cmd_t cmd = {0};
	cbuild_cmd_append_many(&cmd, CC);
	cbuild_cmd_append_many(&cmd, CBUILD_CARGS_STD("gnu23"));
	cbuild_cmd_append_many(&cmd, CBUILD_CARGS_WARN);
	cbuild_cmd_append_many(&cmd, CBUILD_CARGS_DEBUG_GDB);
	if (!cmd_append_pkgconfig(&cmd, "tree-sitter")) return 1;
	if (!cmd_append_pkgconfig(&cmd, "readline")) return 1;
	cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("dl"));
	cbuild_cmd_append_many(&cmd, CBUILD_CARGS_LIBINCLUDE("m"));
	cbuild_cmd_append_many(&cmd, "-Wl,-z,origin", "-Wl,-rpath,$ORIGIN");
	cbuild_cmd_append_many(&cmd, "src/main.c");
	cbuild_cmd_append_many(&cmd, "-o", "microcode.run");
	if (!cbuild_cmd_run(&cmd)) return 1;
	cbuild_cmd_clear(&cmd);
	return 0;
}
