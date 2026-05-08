// cbuild.h by WolodiaM
// License: GPL-3.0-or-later
/* CHANGELOG
 * --------------------------------------------
 * 15.12.2024  v1.0    Initial release
 *   Initial release [new]
 * --------------------------------------------
 * 21.12.2024  v1.1    Few small additions
 *   Proc.h [feature]
 *     - Added 'cbuild_proc_start' function.
 * --------------------------------------------
 * 27.12.2024  v1.2    Bugfix release
 *   Compile.h [bugfix]
 *     - Fixed typo ('CBUILD_CARG_WARN' -> 'CBUILD_CARGS_WARN').
 *   Log.h [bugfix]
 *     - Logger always use one output stream for its messages now.
 *   General [bugfix]
 *     - Added 'CBUILD_IMPL' macro to allow multiple includes in same
 *       translation unit.
 * --------------------------------------------
 * 10.01.2025  v1.3    New API features and bugfixes
 *   common.h [bugfix]
 *     - Fixed bug in macro (wrong variable name used).
 *     - Switched 'CBuildFD' typedef from '__pid_t' to 'pid_t.
 *   common.h	[feature]
 *     - Added 'cbuild_shift_expect' (allows to have error message on top of
 *       'cbuild_shift').
 *     - Added 'cbuild_assert'.
 *     - Added 'noreturn' attribute to an assert function.
 *   Command.h [bugfix]
 *     - Added 'fd' prefix to 'stdin', 'stdout' and 'stderr' for compatibility
 *       with libc that defines this names as a macro.
 *     - Fixed typo in error message.
 *   Log.h [feature]
 *     - Logger now use 16-color ANSI mode.
 *     - Added 'cbuild_vlog' that takes 'va_list' inserted of variadic
 *       arguments.
 *   General [bugfix]
 *     - Added few 'const' annotations to pointers.
 *     - Changed all asserts to 'cbuild_assert'.
 * --------------------------------------------
 * 18.02.2025  v1.4    Small bugfix
 *   common.h [bugfix]
 *     - Wrong argument passed to '__CBUILD_ERR_PRINTF' in 'CBuild_UNREACHAGLE'.
 * --------------------------------------------
 * 18.02.2025  v1.5    Bugfix release
 *   impl.c [bugfix]
 *     - Fixed buffer overflow in 'cbuild_dir_copy'.
 *     - Improved 'cbuild_shift'.
 *   General [change]
 *     - Changed macro 'CBUILD_IMPL' to 'CBUILD_IMPLEMENTATION'.
 * --------------------------------------------
 * 16.06.2025  v1.6    Full rewrite
 *   Term.h [new]
 *     - ANSI wrapper module.
 *   Log.h [feature]
 *     - Rely on 'Term.h' for colored output.
 *     - Runtime configuration for minimum log level.
 *     - Logger now support customizable formatting callbacks.
 *   DynArray.h [new]
 *     - Different implementation.
 *   common.h [feature]
 *     - Fixes for MacOS related to '__progname'.
 *     - Better OS and API abstraction.
 *     - Unified INIT_CAPACITY macro.
 *     - Better IO abstraction.
 *     - Assert now use proper IO abstractions.
 *     - Types for WinAPI.
 *   StringBuilder.h [new]
 *     - Rely on new 'DynArray.h'.
 *     - Few new operations.
 *   Proc.h [feature]
 *     - Better naming scheme.
 *     - Added ability to allocate inter-process shared memory.
 *   Command.h [feature]
 *     - More consistent names.
 *   Map.h [new]
 *     - Hash-map implementation.
 *     - Hash functions for string added.
 *   Compile.h [change]
 *     - Added consistent prefixes.
 *     - Added output arg	macro.
 *   FS.h [change]
 *     - Changes becouse of rewrite of other modules.
 *   StringView.h [new]
 *     - New implementation, better functionality.
 *     - 'find', 'rfind' and 'contains' function.
 *     - 'memrchr' is used on MacOS, *BSD and Glibc (if detected).
 *   Compile.h [feature]
 *     - Better naming.
 *     - More macro.
 *   Span.h [new]
 *     - New module - array+size.
 *   Stack.h [new]
 *     - New module - basic re-sizable stack implementation.
 *   DLload.h [new]
 *     - Small wrapper for runtime loading of dynamic libraries, first code to
 *       support 'WinAPI'.
 *   FlagParse.h [new]
 *     - CLI flag parser library.
 *     - Supports long and short options.
 *     - Support different flag arguments (but not support their types).
 *     - Pre-parses positional arguments - collects them in separate buffer.
 *     - Has build-in help and version flags and build-in help and version subs.
 *     - Has function to print proper help for all defined flags.
 *     - Allow overrides to help and version stubs using 'weak' linking.
 *   General [feature]
 *     - Better error handling.
 *     - More integrated logging.
 *     - Slight formatting change.
 *     - Proper use of asserts in place of older 'exit(0);'.
 * --------------------------------------------
 * 2025-07-01  v1.7    Fix in build script and few code fixes
 *   General [bugfix]
 *     - Add 'FlagParse.h' into a output library ;).
 *   Compile.h [bugfix]
 *     - New file now explicitly marked as executable.
 *     - Allow adding compile argument for self-rebuild.
 *     - Changed API of 'cbuild_cmd_to_sb' to match other 'cbuild_*_to_*'
 *       functions.
 *   FlagParse.h [bugfix]
 *     - Fixed short flags for help and version.
 *     - Fixed 'cbuild_flag_get_flag' to actually return NULL if flag was not
 *       specified..
 *   FS.h [change]
 *     - Different implementation for 'cbuild_dir_list'.
 *     - '.' and '..' now excluded.
 *     - Entries now sorted alphabetically.
 *     - Disable file descriptor auto inheritance on exec by default.
 * --------------------------------------------
 * 2025-07-09  v1.8    Command updates
 *   Command.h [feature]
 *     - Allow to autokill child processes on exit on Linux.
 *   Command.h [change]
 *     - Changed cbuild_cmd_t format.
 * --------------------------------------------
 * 2025-07-21  v1.9    Back to beginning
 *   General
 *     - Revert basic data types to be implemented as a raw macro.
 *       Map is not rewritten because complexity is too high.
 *   Map.h [bugfix]
 *     - Fixed memory leak.
 *   Span.h [remove]
 *     - Removed because it is useless.
 * --------------------------------------------
 * 2025-07-24  v1.10    API cleanup
 *   Map.h [change]
 *     - Rewrite with a new API. Same basic design, but now use void* and
 *       elem_size to be type-agnostic. Should be much faster (less function
 *       pointers) and have API more tailored to read-modify-write patter.
 *       Map now access only a key (and only does 'read' access).
 *   Compile.h [change]
 *     - Now '-Wall', '-Wextra', '-Wno-comments', '-Wconversion' and
 *       '-Wcast-align' is default set of compile args on selfrebuild.
 *   FlagParse.h [change]
 *     - 'cbuild_flag_flg_help' renamed to 'cbuild_flag_print_help'.
 *   StringView.h [feature]
 *     - Added compile-time constructor for string literals.
 *   Stack.h [feature]
 *     - Added 'cbuild_stack_top' to get top element in a read mode.
 *   Proc.h [feature]
 *     - Added 'cbuild_proc_is_running' to check process without blocking.
 *   FS.h [feature]
 *     - Added 'cbuild_fd_read' and 'cbuild_fd_write' to abstract POSIX 'read'
 *       and 'write'.
 *   FlagParse.h [feature]
 *     - Added 'alias' command to DSL - allow to define long option aliases to
 *       another option, used in help message and while parsing, but cannot be
 *       targeted in 'cbuild_flag_get_flag'.
 *     - Added flag groups. Added metadata key 'group' that allow to append flag
 *       to a specific group. Added command 'group' that allow to add extended
 *       description to a group. Groups will only be used during help generation
 *       and flags from same group will be grouped under same heading.
 *   Arena.h [new]
 *     - Temporary allocated. Experimental module.
 *   General [new]
 *     - New allocator hook system.
 *   DynArray.h [bugfix]
 *     - If 'cbuild_da_resize' shrinks array and size becomes invalid it is
 *       decrement.
 *   StringView.h [bugfix]
 *     - Function that can return '-1' now returns 'ssize_t' not 'size_t'.
 * --------------------------------------------
 * 2025-08-05  v1.11		Cleanup and utf8
 *   FS.h [feature]
 *     - 'cbuild_fs_move' and 'cbuild_fs_remove' now try 'rename' before
 *       copying.
 *   Log.h [change]
 *     - Renamed types.
 *   Compile.h [bugfix]
 *     - Now default warn flags include VLA warnings.
 *     - Now if compilation failed in selfrebuild, old version will be run.
 *   FlagParse.h [change]
 *     - Short flag for version option is more more conventional 'V'.
 *   StringView.h [feature]
 *     - Optional utf8 supports.
 *   StringBuilder.h [feature]
 *     - Optional utf8 support.
 *   General [bugfix]
 *     - Now functions that takes no arguments properly have 'void' as
 *       arguments.
 * --------------------------------------------
 * 2025-08-18  v0.12    Switch to zero-ver
 *   Compile.h [feature]
 *     - New 'cbuild_selfrebuild_ex' allow to check more files before rebuild.
 *   Command.h [change]
 *     - Added new API, 'cbuild_cmd_run'.
 *   Command.h [feature]
 *     - Improved output of 'cbuild_cmd_to_sb'.
 *   Proc.h [feature]
 *     - Added ability to wait for multiple processes.
 *   DynArray.h [feature]
 *     - Added 'cbuild_da_remove_unordered'.
 *   Compile.h [bugfix]
 *     - Fixed 'CBUILD_CARGS_INCLUDE'.
 *   FS.h [feature]
 *     - Now 'cbuild_dir_create' will create all needed dirs recursively.
 *     - Added 'cbuild_symlink', which forces symlink at destination.
 *     - Added 'cbuild_path_normalize' to normalize paths without FS lookup.
 *       Strips "<dir>/../: pairs, removes ".", remove trailing slashes
 *       and collapse slash sequences.
 *   FS.h [bugfix]
 *     - 'cbuild_path_filetype' now use 'lstat'.
 *   General [new]
 *     - Now test on 'musl' libc too.
 *     - Now test on strict POSIX.1-2001.
 *     - Now I use zero-ver. This will fully break all version checks.
 *       The only change is that leading '1' will be '0'.
 *       "Real" version number will be monotonically incremented as always.
 *   General [bugfix]
 *     - Better OS-detect logic.
 *     - First deprecated APIs.
 *     - Added CBDEF for all functions (user-overridable).
 *     - Fixed macro to document when they evaluate args multiple times.
 * --------------------------------------------
 * 2025-09-02  v0.13
 *   common.h [change]
 *     - Better platform detection logic.
 *   common.h [feature]
 *     - New timing function.
 *     - CBUILD_MAX and CBUILD_MIN macro.
 *   Commang.h [feature]
 *     - Added 'file_std*' as optional args that can be used to redirect
 *       directly.
 *       to a file.
 *     - Added ability to control how many async workers can be spawned.
 *   Command.h [remove]
 *     - Few small changes to an API. Removed few useless args.
 *   FS.h [feature]
 *     - Added 'cbuild_dir_current' and 'cbuild_dir_set_current'.
 *     - Now cbuild_path_normalize respect POSIX network paths stating from '//'
 *     - Added 'CBUILD_FTYPE_MISSING'
 *   FS.h [change]
 *     - Now 'cbuild_path_filetype' return 'CBUILD_FTYPE_MISSING' on invalid
 *       files.
 *   Proc.h [feature]
 *     - Ability to get amount of CPU cores.
 *     - Ability to wait for any process from a list.
 *   Log.h [feature]
 *     - Added shortcuts for built-in log levels.
 *     - Made CBUILD_LOG_PRINT print always (except if
 *       CBUILD_LOG_NO_LOGS is enabled).
 *     - Made enum for print levels not sparse. Extension of log level was not really 
 *       a useful feature.
 *     - Removed cbuild_log_fmt.
 *     - Exported few internal function to allow easier extension of logger
 *   Arena.h [feature]
 *     - Added ability to do checkpoint and rewind allocator to that checkpoint.
 *       To reset allocator you should use '0' in place of a checkpoint.
 *   StringView.h [bugfix]
 *     - Fixed bug in rfind for strict POSIX
 *     - Added early return for both find and rfind
 *   StringView.h [change]
 *     - Deprecated cbuild_sv_utf8cmp. cbuild_sv_cmp is enough.
 *   StringBuilder.h [change]
 *     - Deprecated cbuild_sb_utf8cmp. cbuild_sb_cmp is enough.
 *   Compile.h [feature]
 *     - Added 'CBUILD_CARGS_DEFINE_TEMP' and 'CBUILD_CARGS_DEFINE_VAL_TEMP'
 *      that use 'cbuild_temp_sprintf' to support runtime values or names
 *      for defines.
 *     - Added 'CBUILD_CARGS_DEBUG_GDB' that expands to '-ggdb'. It will be fine
 *       if you use gdb debugger or your debugger support all of gdb extensions.
 *     - Added __cbuild_selfrebuild_ex_argv to allow having self-rebuilding 
 *       binaries in PATH. For normal binaries can be 'argv[0]'.
 *   General [change]
 *     - Changed CBDEF to CBUILDDEF.
 *     - Remove build stage for a header.
 * --------------------------------------------
 * 2025-01-12  v0.14
 *   DynArray.h [feature]
 *     - Add macro to allow easy creation of required structures.
 *   Proc.h [bugfix]
 *     - Handle EINTR in cbuild_proc_wait_code
 *   Compile.h [bugfix]
 *     - Selfrebuild now will not abort if argv0 binary not found
 *   StringView.h [feature]
 *     - Added cbuild_sv_chop_right* functions to chop from the end of string view
 *   General [bugfix]
 *     - Added some 'const' annotations
 *     - Moved few functions to take parameters by value and not by pointer
 * --------------------------------------------
 * TODO Change on release  v0.15
 * General
 *   - Split library between multiple files with an explicit build step. (@WolodiaM)
 *   - Reverted function to take pointers back. (@WolodiaM)
 * Common.h
 *   - Some symbols renamed. (@WolodiaM)
 *     * `CBUILD_SB_QUICK_SPRINTF_SIZE to CBUILD_QUICK_SPRINTF_SIZE`.
 *     * `CBUILD_TMP_BUFF_SIZE to CBUILD_FS_TMP_SIZE`.
 *   - Removed a few internal macros. (@WolodiaM)
 *     * `__CBUILD_ERR_PRINT`.
 *     * `__CBUILD_ERR_PRINTF`.
 *   - Added better attribute wrappers. (@WolodiaM)
 *     * `__CBUILD_ATTR_DEPRECATED`.
 *     * `__CBUILD_ATTR_NORETURN`.
 *     * `__CBUILD_ATTR_FALLTROUGH`.
 *     * `__CBUILD_ATTR_PRINTF`.
 *   - Added few functions from internal API. (@WolodiaM)
 *     * `__cbuild_progname`
 *     * `__cbuild_memrchr`
 *   - `CBUILD_TODO` and `CBUILD_UNREACHANBLE` now take printf-style
 *     arguments. (@WolodiaM)
 *   - Added `CBUILD_PROFILER` to enable profiling support in CBuild. (@WolodiaM)
 * Log.h
 *   - Removed some symbols. (@WolodiaM)
 *     * `CBUILD_LOG_NO_LOGS`.
 *     * `CBUILD_LOG_PRINT`.
 *     * `cbuild_log_print`.
 *     * `__cbuild_int_vlog`.
 *     * `__cbuild_int_log`.
 *   - Added new handlers support. Log handler take log level (as integer) and
 *     logged string and should just output it. (@WolodiaM)
 *     * `cbuild_log_handler_t`.
 *     * `cbuild_log_set_handler`.
 *     * `cbuild_log_get_handler`.
 *   - Added ability to overwrite `cbuild_log_level_t` by creating enum 
 *     `cbuild_custom_log_level_t` and defining `CBUILD_LOG_CUSTOM_LEVELS`. 
 *     (@WolodiaM)
 * Map.h
 *   - Element clear function moved into structure from function arguments.
 *     (@WolodiaM)
 * Proc.h
 *   - Added new functions. (@WolodiaM)
 *     * `cbuild_proclist_append`.
 *     * `cbuild_proclist_set`.
 *     * `cbuild_proclist_get`.
 *   - `cbuild_proc_wait_any` now sleeps a little on each iteration.
 *     (@WolodiaM)
 * Command.h
 *   - Removed deprecated functions. (@WolodiaM)
 *     * `cbuild_cmd_sync`.
 *     * `cbuild_cmd_sync_redirect`.
 *     * `cbuild_cmd_async`.
 *     * `cbuild_cmd_async_redirect`.
 * FS.h
 *   - Added few new functions. (@WolodiaM)
 *     * `cbuild_pathlist_get`.
 *   - Added low-level directory API. (@WolodiaM)
 *     * `cbuild_dir_open`
 *     * `cbuild_dir_next`
 *     * `cbuild_dir_close`
 *   - Added directory walker. (@WolodiaM)
 *     * `cbuild_dir_walk`.
 *     * `cbuild_dir_walk_opt`.
 *     * `enum cbuild_dir_walk_result_t`.
 *     * `CBUILD_DIR_WALK_CONTINUE`.
 *     * `CBUILD_DIR_WALK_NO_ENTER`.
 *     * `CBUILD_DIR_WALK_STOP`.
 *     * `cbuild_dir_walk_func_args_t`.
 *     * `cbuild_dir_walk_func_t`.
 *     * `cbuild_dir_walk_opts_t`.
 * Compile.h
 *   - Removed most tool selection logic. (@WolodiaM)
 *     * `CBUILD_LD`.
 *     * `CBUILD_CXX`.
 *     * `CBUILD_AR`.
 *     * `CBUILD_AS`.
 *     * `CBUILD_CPP`.
 *     * `CBUILD_CC_OUT`.
 *   - Removed static analyzer flag. (@WolodiaM)
 *     * `CBUILD_CARGS_STATIC_ANALYZER`.
 *   - Simplified self-rebuild logic. (@WolodiaM)
 *     * `__cbuild_selfrebuild_ex`.
 *     * `__cbuild_selfrebuild_ex_argv`.
 *   - Now normal `__cbuild_selfrebuild` provides ability to add list of files
 *     and custom `argv[0]`, (@WolodiaM)
 * DLLoad.h
 *   - Make implementation use real functions. Merged two `cbuild_dlib_open*`
 *     functions to one that takes not only filename but also load type
 *     (eager or now). (@WolodiaM)
 * FlagParse.h
 *   - Removed DSL and switched to a new register API. (@WolodiaM)
 *     * `cbuild_flag_new_opt`
 *     * `cbuild_flag_new`
 *     * `enum cbuild_flag_options_t`
 *     * `cbuild_flag_print_func_t`
 *     * `cbuild_flag_set_option`
 *   - Removed aliases and group descriptions. They are too rarely useful
 *     to support them. (@WolodiaM)
 *   - Added support for controlling flag repeat mode. Currently, support
 *     two behaviours - error if flag encountered for the second time or
 *     just parse it again and append its argument to shared buffer. If
 *     flag has limits on argument count, this limit will be applied
 *     per-invocation, not for all invocations in total. (@WolodiaM)
 * Arena.h
 *   - New arena allocator. Uses heap-allocated memory pool. (@WolodiaM)
 *     * `cbuild_arena_base_realloc`
 *     * `cbuild_arena_base_malloc`
 *     * `cbuild_arena_base_free`
 *     * `cbuild_arena_malloc`
 *     * `cbuild_arena_realloc`
 *     * `cbuild_arena_checkpoint`
 *     * `cbuild_arena_reset`
 *   - Added few convenience functions for arena. (@WolodiaM)
 *     * `cbuild_arena_free`
 *     * `cbuild_arena_strdup`
 *     * `cbuild_arena_memdup`
 *     * `cbuild_arena_sprintf`
 *     * `cbuild_arena_vsprintf`
 *     * `cbuild_arena_profiler`
 * RGlob.h
 *   - New glob matcher. Based on top of POSIX ERE. (@WolodiaM)
 *     * `cbuild_glob_t`
 *     * `__cbuild_glob_res_t`
 *     * `cbuid_glob_compile`
 *     * `cbuild_glob_match`
 *     * `cbuild_glob_match_fs`
 *     * `cbuild_glob_match_files`
 *     * `cbuild_glob_free`
 * Temp.h
 *   - Now backed by `Arena.h`. (@WolodiaM)
 *   - New function: `cbuild_temp_realloc`. (@WolodiaM)
 *   - Renamed `cbuild_temp_alloc` to `cbuild_temp_malloc`. (@WolodiaM)
 *   - Added `cbuild_temp_profiler`. (@WolodiaM)
 * StringView.h
 *   - Renamed `cbuild_temp_sv_to_cstr` to `cbuild_sv_to_temp_cstr`
 */
#ifndef __CBUILD_H__
#define __CBUILD_H__

/* Common.h */

// #pragma once // For LSP 
//! This file provides OS-selection logic, includes and some general utilities.
//!
//! License: `GPL-3.0-or-later`.
//! 
//! ::: note
//! CBuild should be first header included in translation unit. On `CBUILD_API_POSIX`
//! (default for modern Unixes) it defines `_GNU_SOURCE`.
//! :::

//! # Compiler defines [line:cbuild-cc-defines]
//!
//! Valid compilers:
//!
//! - `CBUILD_CC_CLANG` - clang or its derivative.
//! - `CBUILD_CC_GCC` - gcc or its derivative.
//!
//! ::: note
//! This selection can be overriden by manually defining valid compiler and `CBUILD_CC_DEFINED`.
//! :::
//!
//! Several compiler extensions are required from supported compiler:
//!
//! - Binary literals support (`0b`) either trough c standard (c23+) or as extension.
//! - Attributes trough `__attribute__((...))` syntax, some are supported trough `[[...]]` syntax if support is detected 
//!   * deprecated.
//!   * format.
//!   * noreturn.
//!   * falltrough.
//! - Unnamed structs/enums/unions.
//! - typeof.
//! - `__VA_OPT__`.
//! - GNU statement expressions `({...})`.
//! - `__auto_type`.
//!
//! Explicetely unsupported compilers:
//!
//! - TCC - no statement expression (at least).
//! - MSVC - no extensions at all.
//!
//! # Debug
//!
//! By default CBuild provides _debug_ logging from most of its modules. All logging goes
//! trough `cbuild_log`, so if your app does no need any messages in terminal you can
//! disable CBuild's logger (which will hide all messages).

//@ cbuild-cc-defines
#if !defined(CBUILD_CC_DEFINED)
	#if defined(__clang__)
		#define CBUILD_CC_CLANG
	#elif defined(__TINYC__)
		#error "You are trying to compile CBuild with TinyC compiler. This compile is not supported. It should support 'gnu99' extensions, including binary literals; attributes; unnamed (anonymous) structs, enums, unions; typeof; __VA_OPT__; statement expressions."
	#elif defined(__GNUC__)
		#define CBUILD_CC_GCC
	#elif defined(_MSC_VER)
		#error "You are trying to compile CBuild with MSVC compiler. It does not support 'gnu99' extensions, including binary literals; attributes; unnamed (anonymous) structs, enums, unions; typeof; __VA_OPT__; statement expressions."
	#else
		#error "This compile is unsupported. If it supports 'gnu99' extensions, including binary literals; attributes; unnamed (anonymous) structs, enums, unions; typeof; __VA_OPT__; statement expressions; then you can simply add a check to this block for it."
	#endif // Compiler check
	#define CBUILD_CC_DEFINED
#endif // !CBUILD_CC_DEFINNED

//! # OS defines [line:cbuild-os-defines]
//!
//! Valid OSes:
//!
//! - `CBUILD_OS_LINUX` - Any desktop Linux (excluding Android or ChromeOS for example).
//! - `CBUILD_OS_MACOS` - MacOS.
//! - `CBUILD_OS_BSD` - Any desktop BSD.
//! - `CBUILD_OS_UNIX` - Generic Unix, should support at least POSIX.1-2001.
//! - `CBUILD_OS_WINDOWS` - Windows.
//!
//! Also, there are some auxiliary OS checks, valid values are:
//!
//! - `CBUILD_OS_WINDOWS`:
//!   * `CBUILD_OS_WINDOWS_CYGWIN `- Windows under Cygwin.
//! - `CBUILD_OS_LINUX`:
//!   * `CBUILD_OS_LINUX_GLIBC` - Linux with glibc.
//!   * `CBUILD_OS_LINUX_UCLIBC` - Linux with uclibc.
//!   * `CBUILD_OS_LINUX_MUSL` - Linux with musl (fallback, because musl provides no way of detection).

//@ cbuild-os-defines
#if !defined(CBUILD_OS_DEFINED)
	#if defined(__linux__) || defined(linux) || defined(__linux)
		#define CBUILD_OS_LINUX
	#elif defined(__APPLE__) || defined(__MACH__)
		#define CBUILD_OS_MACOS
	#elif defined(__FreeBSD__) || \
		defined(__NetBSD__) || defined(__NETBSD__) || \
		defined(__OpenBSD__) || \
		defined(__DragonFly__)
		#define CBUILD_OS_BSD
	#elif defined(__CYGWIN__) || defined(__cygwin__)
		#define CBUILD_OS_WINDOWS
		#define CBUILD_OS_WINDOWS_CYGWIN
	#elif defined(__unix__) || defined(unix) || defined(__unix)
		#define CBUILD_OS_UNIX
	#elif defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
		#define CBUILD_OS_WINDOWS
	#else
		#error "This OS is unsupported by CBuild. If it supports POSIX.1-2001 you can simply add check for it to a list of checks here. If not you should add a check for OS and a macro for its API and then implement OS/API specifc parts of CBuild."
	#endif // OS detect
	// TODO: Android and IOS
#endif // !CBUILD_OS_DEFINED

//! # API defines [line:cbuild-api-defines]
//!
//! Valid APIs:
//!
//! - `CBUILD_API_POSIX`.
//!   POSIX.1-2001 (`_POSIX_C_SOURCE == 200112L`). 
//!   Any detected extension can be used.
//!   Permorms detection for extensions on MacOS, BSD, Linux.
//!   Default for:
//!   * `CBUILD_OS_LINUX`
//!   * `CBUILD_OS_MACOS`
//!   * `CBUILD_OS_BSD`
//!   * `CBUILD_OS_WINDOWS_CYGWIN`
//! - `CBUILD_API_STRICT_POSIX`
//!   POSIX.1-2001 (`_POSIX_C_SOURCE == 200112L`). 
//!   Only three extension are used and one extension type is considered: 
//!   * `MAP_ANON/MAP_ANONYMOUS`. If none is available fallabck to `shm_open` is provided but needs `dlopen` and `dlsym` provided.
//!   * `sysconf(_SC_NPROCESSORS_ONLN)`. Literal `1` is used as fallback.
//!   * `ioctl` and `TIOCGWINSZ`.
//!   * Newer POSIX versions can be used if detected.
//! - `CBUILD_API_WINAPI`
//!   WinAPI and possibly NT API.
//!   Not implemented for now.

//@ cbuild-api-defines
#if !defined(CBUILD_API_DEFINED)
	#if defined(CBUILD_OS_LINUX) || defined(CBUILD_OS_MACOS) || defined(CBUILD_OS_BSD)
		#define CBUILD_API_POSIX
	#elif defined(CBUILD_OS_UNIX)
		#define CBUILD_API_STRICT_POSIX
	#elif defined(CBUILD_OS_WINDOWS_CYGWIN)
		#define CBUILD_API_POSIX
	#elif defined(CBUILD_OS_WINDOWS)
		#define CBUILD_API_WINAPI
		#error "WinAPI is unsupported!"
	#else
		// TODO: Android/IOS/ChromeOS/IPadOS
		#error "This OS is not known by CBuild. If it supports POSIX.1-2001 you can simply add check for it's OS to a list of OS checks and define appropriate API macro there. If not you should add a check for OS and a macro for its API and then implement OS/API specifc parts of CBuild."
	#endif // API select
#endif // !CBUILD_API_DEFINED
//! # Platform selector [line:cbuild-platform-selector]
//!
//! This selector include platform-specific headers and configures platforms's libc.

//@ cbuild-platform-selector
#if defined(CBUILD_API_POSIX)
	// Use GNU extentions if possible
	#if !defined(_GNU_SOURCE)
		#define _GNU_SOURCE
	#endif // _GNU_SOURCE
	// Platform include
	#include <dirent.h>
	#include <dlfcn.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <signal.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	#if defined(CBUILD_OS_MACOS)
		#include <crt_externs.h>
	#endif // CBUILD_OS_MACOS
	#if defined(CBUILD_OS_LINUX)
		#if defined(__GLIBC__)
			#define CBUILD_OS_LINUX_GLIBC
		#elif defined(__UCLIBC__)
			#define CBUILD_OS_LINUX_UCLIBC
		#else // Assume musl
			#define CBUILD_OS_LINUX_MUSL
		#endif // Libc select
		#include <sys/prctl.h>
	#endif // CBUILD_OS_LINUX
	/// Process handle
	typedef pid_t cbuild_proc_t;
	/// Invalid process handle
	#define CBUILD_INVALID_PROC -1
	/// File descriptor
	typedef int cbuild_fd_t;
	/// Invalid file descriptor
	#define CBUILD_INVALID_FD -1
	/// Directory descriptor
	typedef DIR* cbuild_dir_t;
	/// Invalid directory descriptor
	#define CBUILD_INVALID_DIR NULL
	/// Additional error value for pointers (when error and not found are separate cases)
	#define CBUILD_PTR_ERR (void*)((intptr_t)-1)
	/// Handle for dynamic library
	typedef void* cbuild_dlib_t;
#elif defined(CBUILD_API_STRICT_POSIX)
	#if !defined(_POSIX_C_SOURCE)
		#define _POSIX_C_SOURCE 200112L
	#endif
	// Platform includes
	#include <dirent.h>
	#include <dlfcn.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <signal.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <sys/ioctl.h>
	#include <unistd.h>
	// Process and file handles
	typedef pid_t cbuild_proc_t;
	#define CBUILD_INVALID_PROC -1
	typedef int cbuild_fd_t;
	#define CBUILD_INVALID_FD -1
	/// Directory descriptor
	typedef DIR* cbuild_dir_t;
	/// Invalid directory descriptor
	#define CBUILD_INVALID_DIR NULL
	// For pointer errors
	#define CBUILD_PTR_ERR (void*)((intptr_t)-1)
	/// Handle for dynamic library
	typedef void* cbuild_dlib_t;
#elif defined(CBUILD_API_WINAPI)
	// Platform includes
	#include <windows.h>
	#include <io.h>
	/// Process handle
	typedef HANDLE cbuild_proc_t;
	/// Invalid process handle
	#define CBUILD_INVALID_PROC ((HANDLE)(intptr_t)-1)
	/// File descriptor
	typedef HANDLE cbuild_fd_t;
	/// Invalid file descriptor
	#define CBUILD_INVALID_FD ((HANDLE)(intptr_t)-1)
	/// Additional error value for pointers (when error and not found are separate cases)
	#define CBUILD_PTR_ERR (void*)((intptr_t)-1)
	/// Handle for dynamic library
	typedef HINSTANCE cbuild_dlib_t;
#endif // CBUILD_API_*

//! # Normal platform-independent includes [lines:cbuild-includes]

//@ cbuild-includes
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h> // mingw seems to provide it too

//! # Configuration values [line:cbuild-configuration]
//!
//! All of this are defines that user can redefine.
//!
//! There are few more configuration define not listed here:
//!
//! - `CBUILD_PROFILER` - Enable profiler features in modules.
//! - `CBUILD_LOG_CUSTOM_LEVELS` - used to inject custom log levels in 
//!    CBuild's logger

//@ cbuild-configuration
#ifndef CBUILD_INIT_CAPACITY
	/// Initial capacity of cbuild dynamic arrays. Size is in elements.
	///
	/// Type: `size_t`{.c}.
	#define CBUILD_INIT_CAPACITY (size_t)256
#endif // CBUILD_INIT_CAPACITY
#ifndef CBUILD_QUICK_SPRINTF_SIZE
	/// Size of stack buffer used to speedup `sprintf` for small strings.
	///
	/// Type: `size_t`{.c}.
	#define CBUILD_QUICK_SPRINTF_SIZE (size_t)512
#endif // CBUILD_QUICK_SPRINTF_SIZE
#ifndef CBUILD_FS_TMP_SIZE
	/// Size of temporary buffer used for FS operations
	///
	/// Type: `size_t`{.c}.
	#define CBUILD_FS_TMP_SIZE ((size_t)32 * (size_t)1024 * (size_t)1024)
#endif // CBUILD_FS_TMP_SIZE
#ifndef CBUILD_TEMP_ARENA_SIZE
	/// Size of temporary allocator.
	///
	/// Type: `size_t`{.c}.
	#define CBUILD_TEMP_ARENA_SIZE ((size_t)8 * (size_t)1024 * (size_t)1024)
#endif // CBUILD_TEMP_ARENA_SIZE
#ifndef CBUILD_MAP_DEFAULT_HASH_FUNC
	/// Default hash function for CBuild map. User can override it in runtime 
	/// for specific map instance.
	///
	/// [Type](DOC:cbuild_map_hash_t): `size_t func(const void* key, size_t len)`{.c}. 
	#define CBUILD_MAP_DEFAULT_HASH_FUNC __cbuild_map_hash_func
#endif // CBUILD_MAP_DEFAULT_HASH_FUNC
#ifndef CBUILD_SELFREBUILD_ARGS
	/// Default arguments for `cbuild_selfrebuild`.
	///
	/// Type: Comma-separated list of `const char*`{.c}.
	#define CBUILD_SELFREBUILD_ARGS CBUILD_CARGS_WARN
#endif // CBUILD_SEFLREBUILD_ARGS
#ifndef CBUILD_LOG_MIN_LEVEL
	/// Minimal log level for `cbuild_log`.
	/// 
	/// [Type](DOC:cbuild_log_level_t): `cbuild_log_level_t`.
	#define CBUILD_LOG_MIN_LEVEL CBUILD_LOG_ERROR
#endif // CBUILD_LOG_MIN_LEVEL
#ifndef CBUILD_GLOB_CAPTURE_COUNT
	/// Number og "capture groups" glob supports. This is used for underlying regex engine.
	#define CBUILD_GLOB_CAPTURE_COUNT 10
#endif // CBUILD_GLOB_CAPTURE_COUNT
#ifndef CBUILDDEF
	/// This is prepended to all cbuild's functions. Can be set to eg. `static inline`
	/// for build that use only one translation unit.
	///
	/// Type: function attributes.
	#define CBUILDDEF
#endif // CBUILDDEF

//! # Preprocessor configuration [line:cbuild-misc-preproc] {#preprocessor-configuration}
//!
//! This section provides several miscellaneous preprocessor macro.
//!
//! - Print hooks. These can not be overridden by user and should be edited here.
//! - Macro helpers for '##' and '#' operators
//! - Attribute abstractions to support both new C23 attributes and older 
//!   gnu-style attributes.
//! - Allocator hooks (`__CBUILD_MALLOC`, `__CBUILD_REALLOC`, `__CBUILD_FREE`) which
//!   can be overridden by defining CBUILD_CUSTOM_ALLOC

//@ cbuild-misc-preproc
// These are used for logging only (both from assert and from logger).
#define __CBUILD_PRINT(str)                printf("%s", (str))
#define __CBUILD_PRINTF(fmt, ...)          printf((fmt), __VA_ARGS__)
#define __CBUILD_VPRINTF(fmt, va_args)     vprintf((fmt), (va_args))
#define __CBUILD_FLUSH()                   fflush(stdout)
// Macro helpers
#define __CBUILD_STRINGIFY(var)  #var
#define __CBUILD_XSTRINGIFY(var) __CBUILD_STRINGIFY(var)
#define __CBUILD_CONCAT(a, b)    a ## b
#define __CBUILD_XCONCAT(a, b)   __CBUILD_CONCAT(a, b)
#define __CBUILD_EXPAND(a)       a
// Attributes
#if defined(__has_c_attribute)
	#if __has_c_attribute(deprecated)
		#define CBUILD_ATTR_DEPRECATED(msg, decl) [[deprecated(msg)]] decl
	#else
		#define CBUILD_ATTR_DEPRECATED(msg, decl) decl __attribute__((deprecated(msg)))
	#endif //__has_c_attribute(deprecated)
#else
	#define CBUILD_ATTR_DEPRECATED(msg, decl) decl __attribute__((deprecated(msg)))
#endif // defined(__has_c_attribute)
#if defined(__has_c_attribute)
	#if __has_c_attribute(noreturn)
		#define CBUILD_ATTR_NORETURN(decl) [[noreturn]] decl
	#else
		#define CBUILD_ATTR_NORETURN(decl) decl __attribute__((noreturn))
	#endif // __has_c_attribute(noreturn)
#else
	#define CBUILD_ATTR_NORETURN(decl) decl __attribute__((noreturn))
#endif // defined(__has_c_attribute)
#if defined(__has_c_attribute)
	#if __has_c_attribute(fallthrough)
		#define CBUILD_ATTR_FALLTHROUGH() [[fallthrough]] 
	#else
		#define CBUILD_ATTR_FALLTHROUGH() __attribute__((fallthrough))
	#endif // __has_c_attribute(fallthrough)
#else
	#define CBUILD_ATTR_FALLTHROUGH() __attribute__((fallthrough))
#endif // defined(__has_c_attribute)
#define __CBUILD_ATTR_ADD1_0 1
#define __CBUILD_ATTR_ADD1_1 2
#define __CBUILD_ATTR_ADD1_2 3
#define __CBUILD_ATTR_ADD1_3 4
#define __CBUILD_ATTR_ADD1_4 5
#define __CBUILD_ATTR_ADD1_5 6
#if defined(__has_c_attribute)
	#if __has_c_attribute(gnu::format)
		#define CBUILD_ATTR_PRINTF(arg, decl) [[gnu::format(printf, arg, __CBUILD_XCONCAT(__CBUILD_ATTR_ADD1_, arg))]] decl
	#else
		#define CBUILD_ATTR_PRINTF(arg, decl) decl __attribute__((format(printf, arg, __CBUILD_XCONCAT(__CBUILD_ATTR_ADD1_, arg))))
	#endif // __has_c_attribute(gnu::format)
#else
	#define CBUILD_ATTR_PRINTF(arg, decl) decl __attribute__((format(printf, arg, __CBUILD_XCONCAT(__CBUILD_ATTR_ADD1_, arg))))
#endif // defined(__has_c_attribute)
// Allocator hooks
#ifndef CBUILD_CUSTOM_ALLOC
	#define __CBUILD_MALLOC(size)       malloc(size)
	#define __CBUILD_REALLOC(ptr, size) realloc(ptr, size)
	#define __CBUILD_FREE(ptr)          free(ptr)
#endif // CBUILD_CUSTOM_ALLOC

/// Mark some variable as unused.
///
/// * [pl:val:name] Variable name.
#define CBUILD_UNUSED(val)       (void)(val)

/// Place `TODO` marker in code. Upon reaching it execution will abort with message. 
///
/// * [pl:message:const char*] Message that will be printed before aborting. printf-style.
/// * [pl:...:...any[]] Format string arguments.
#define CBUILD_TODO(message, ...)                              \
	do {                                                         \
		__CBUILD_PRINTF("%s:%d: TODO: %s\n", __FILE__, __LINE__,   \
			cbuild_temp_sprintf(message __VA_OPT__(,) __VA_ARGS__)); \
		abort();                                                   \
	} while (0)
/// Mark some code as unreachable. This will abort with message if executed.
///
/// * [pl:message:const char*] Message that will be printed before aborting. printf-style.
/// * [pl:...:...any[]] Format string arguments.
#define CBUILD_UNREACHABLE(message, ...)                            \
	do {                                                              \
		__CBUILD_PRINTF("%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, \
			cbuild_temp_sprintf(message __VA_OPT__(,) __VA_ARGS__));      \
		abort();                                                        \
	} while (0)

/// Assert that can pretty-print messages.
///
/// * [pl:expr:() => bool] Any expression returns boolean.
/// * [pl:...:format string] Printf-style format string and its arguments.
#define cbuild_assert(expr, ...)                                      \
	(expr) ? (void)(0)                                                  \
	: __cbuild_assert(__FILE__, __LINE__, __func__, #expr, __VA_ARGS__)
/// Pretty-printer for `cbuild_assert`.
///
/// * [pl:file] File that should be reported.
/// * [pl:line] Line that should be reported.
/// * [pl:func] Function that should be reported.
/// * [pl:expr] Expression that should be reported.
/// * [pl:...:format string] Printf-style format string and its arguments.
CBUILD_ATTR_NORETURN(CBUILDDEF void __cbuild_assert(const char* file, unsigned int line,
		const char* func, const char* expr, ...));

/// Get length of array whose size is know at compile-time
///
/// * [pl:array:any*] Array, not just a pointer.
///
/// [r:size_t] Length of an array in elements.
#define cbuild_arr_len(array) (sizeof(array) / sizeof((array)[0]))
/// Get element from array with bound checking (array size should be know at compile-time)
/// Abort execution if index is out of bounds.
///
/// * [pl:array:any*] Array, not just a pointer.
/// * [pl:index:size_t] 
///
/// [r:any] Element from an array.
#define cbuild_arr_get(array, index)                      \
	(cbuild_assert((size_t)(index) < cbuild_arr_len(array), \
			"Index %zu is out of bounds.", (size_t)index),      \
		(array)[(size_t)(index)])

/// Shift arguments from "standard" C array view - pointer and size as separate variables.
/// Size will be decremented and pointer will be incremented.
///
/// * [pl:argv:any*] Array.
/// * [pl:argc:size_t] Length of an array.
///
/// [r:any] Shifted First element of an array.
#define cbuild_shift(argv, argc)                                        \
	(cbuild_assert((argc) > 0, "More arguments are required."), (argc)--, \
		*(argv)++)
/// Shift arguments from "standard" C array view - pointer and size as separate variables.
/// Size will be decremented and pointer will be incremented.
/// Allows for a used-defined message.
///
/// * [pl:argv:any*] Array.
/// * [pl:argc:size_t] Length of an array.
/// * [pl:...:format string] Message that will be printed if array does not have enough elements.
///
/// [r:any] Shifted First element of an array.
#define cbuild_shift_expect(argv, argc, ...)                    \
	(cbuild_assert((argc) > 0, __VA_ARGS__), (argc)--, *(argv)++)

/// Get time in nanoseconds.
///
/// [r:] Nanoseconds from unspecified epoch.
CBUILDDEF uint64_t cbuild_time_nanos(void);
/// Number of nanoseconds in one second
#define CBUILD_NANOS_PER_SEC (long long)(1000*1000*1000)

/// Return bigger number from 2 numbers
#define CBUILD_MAX(a, b) a > b ? a : b
/// Return smaller number from 2 numbers
#define CBUILD_MIN(a, b) a < b ? a : b

//! # Internal API [line:cbuild-api-internal]
//! These are internal function, so their API can break at any time.
//! But they still can be considered mostly stable (eg. I does not
//! break their API that often).

/// Get name of program (`argv[0]`).
CBUILDDEF const char* __cbuild_progname(void);
/// `memrchr` wrapper.
CBUILDDEF void* __cbuild_memrchr(const void* s, int c, size_t n);
/// `memmem` wrapper
CBUILDDEF void* __cbuild_memmem(const void* haystack, size_t hsize,
	const void* needle, size_t nsize);

/* Version.h */

#define CBUILD_VERSION "v0.15"
#define CBUILD_VERSION_MAJOR 0
#define CBUILD_VERSION_MINOR 15

/* Term.h */

// #pragma once // For LSP 
//! This file provides utilities to construct ANSII escape codes.
//!
//! License: `GPL-3.0-or-later`.
//!
//! ::: note
//! This only provides strings for escape codes and some macro to build that strings
//! :::

/// ANSII escape starter.
#define CBUILD_TERM_ESCAPE "\x1b["
/// CSI - Control Sequence Introducer.
#define CBUILD_TERM_CSI    CBUILD_TERM_ESCAPE
/// SGR - Select Graphic Rendition, meta-function to generate proper code.
#define CBUILD_TERM_SGR(n) CBUILD_TERM_ESCAPE n "m"

/// Set cursor to specific coordinates.
#define CBUILD_TERM_CURSOR_SET(x, y)                                           \
	CBUILD_TERM_CSI __CBUILD_STRINGIFY(x) ";" __CBUILD_STRINGIFY(y) "H"
/// Set cursor to specific column in current line
#define CBUILD_TERM_CURSOR_SETH(x) CBUILD_TERM_CSI __CBUILD_STRINGIFY(x) "G"
/// Move cursor in specific direction for specified number of steps.
///
/// * [pl:direction:direction] One of CBUILD_TERM_[UP|DOWN|RIGHT|LEFT].
/// * [pl:num_steps:unsigned int] Number of steps, should be either literal or define.
#define CBUILD_TERM_CURSOR_MOVE(direction, num_steps)                          \
	CBUILD_TERM_CSI __CBUILD_XSTRINGIFY(num_steps) direction
///
#define CBUILD_TERM_UP        "A"
///
#define CBUILD_TERM_DOWN      "B"
///
#define CBUILD_TERM_RIGHT     "C"
///
#define CBUILD_TERM_LEFT      "D"
///
#define CBUILD_TERM_NEXT_LINE "E"
///
#define CBUILD_TERM_PREV_LINE "F"
/// Get current position of a cursor.
#define CBUILD_TERM_CURSOR_GET CBUILD_TERM_CSI "6n"

/// Clean specific part of terminal
///
/// * [pl:mode:clear mode] One of CBUILD_TERM_[CURSOR_TO_END|CURSOR_TO_BEGIN|FULL|FULL_SCROLLBACK].
#define CBUILD_TERM_ERASE_SCR(mode) CBUILD_TERM_CSI __CBUILD_STRINGIFY(mode) "J"
/// Clean specific part of terminal
///
/// * [pl:mode:clear mode] One of CBUILD_TERM_[CURSOR_TO_END|CURSOR_TO_BEGIN|FULL].
#define CBUILD_TERM_ERASE_LINE(mode)                                           \
	CBUILD_TERM_CSI __CBUILD_STRINGIFY(mode) "K"
///
#define CBUILD_TERM_CURSOR_TO_END    0
///
#define CBUILD_TERM_CURSOR_TO_BEGIN  1
///
#define CBUILD_TERM_FULL             2
///
#define CBUILD_TERM_FULL_SCROLLBACK  3

/// Reset all styling
#define CBUILD_TERM_RESET            CBUILD_TERM_SGR("0")
///
#define CBUILD_TERM_BOLD             CBUILD_TERM_SGR("1")
///
#define CBUILD_TERM_FAINT            CBUILD_TERM_SGR("2")
///
#define CBUILD_TERM_ITALIC           CBUILD_TERM_SGR("3")
/// Disable italic
#define CBUILD_TERM_NITALIC          CBUILD_TERM_SGR("23")
///
#define CBUILD_TERM_UNDERLINE        CBUILD_TERM_SGR("4")
/// Disable underline
#define CBUILD_TERM_NUNDERLINE       CBUILD_TERM_SGR("24")
/// Enable blinking text
#define CBUILD_TERM_BLINK            CBUILD_TERM_SGR("5")
/// Disable blinking text
#define CBUILD_TERM_NBLINK           CBUILD_TERM_SGR("25")
/// Enable swapping of background and foreground color
#define CBUILD_TERM_REVERSE          CBUILD_TERM_SGR("7")
/// Disable reverse mode
#define CBUILD_TERM_NREVERSE         CBUILD_TERM_SGR("27")
/// Enable crossed-out text rendering
#define CBUILD_TERM_CROSS_OUT        CBUILD_TERM_SGR("9")
/// Disable crossed-out rendering
#define CBUILD_TERM_NCROSS_OUT       CBUILD_TERM_SGR("29")
///
#define CBUILD_TERM_DOUBLE_UNDERLINE CBUILD_TERM_SGR("21")
///
#define CBUILD_TERM_OVERLINE         CBUILD_TERM_SGR("53")
/// Disable overline
#define CBUILD_TERM_NOVERLINE        CBUILD_TERM_SGR("55")

/// Set foreground color based on 16 color palette.
///
/// * [pl:color:Color] One of CBUILD_TERM_<COLOR> defines.
#define CBUILD_TERM_FG(color) CBUILD_TERM_SGR(__CBUILD_XSTRINGIFY(color))
/// Set background color based on 16 color palette.
///
/// * [pl:color:Color] One of CBUILD_TERM_<COLOR> defines.
#define CBUILD_TERM_BG(color) CBUILD_TERM_SGR(__CBUILD_XSTRINGIFY(color##addten))
/// Set foreground color in terms of [8-bit palette](https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
#define CBUILD_TERM_FG8(color)                                                 \
	CBUILD_TERM_SGR("38;5;" __CBUILD_STRINGIFY(color))
/// Set background color in terms of [8-bit palette](https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
#define CBUILD_TERM_BG8(color)                                                 \
	CBUILD_TERM_SGR("48;5;" __CBUILD_STRINGIFY(color))
/// Set underline color in terms of [8-bit palette](https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
#define CBUILD_TERM_UNDERLINE8(color)                                          \
	CBUILD_TERM_SGR("58;5;" __CBUILD_STRINGIFY(color))
/// Set foreground color to to some RGB888 color
#define CBUILD_TERM_FG24(r, g, b)                                              \
	CBUILD_TERM_SGR("38;2;" __CBUILD_STRINGIFY(r) ";" __CBUILD_STRINGIFY(g) ";"  \
		__CBUILD_STRINGIFY(b))
/// Set background color to to some RGB888 color
#define CBUILD_TERM_BG24(r, g, b)                                              \
	CBUILD_TERM_SGR("48;2;" __CBUILD_STRINGIFY(r) ";" __CBUILD_STRINGIFY(g) ";"  \
		__CBUILD_STRINGIFY(b))
/// Set underline color to to some RGB888 color
#define CBUILD_TERM_UNDERLINE24(r, g, b)                                       \
	CBUILD_TERM_SGR("58;2;" __CBUILD_STRINGIFY(r) ";" __CBUILD_STRINGIFY(g) ";"  \
		__CBUILD_STRINGIFY(b))

//! # Colors

///
#define CBUILD_TERM_BLACK           30
///
#define CBUILD_TERM_RED             31
///
#define CBUILD_TERM_GREEN           32
///
#define CBUILD_TERM_YELLOW          33
///
#define CBUILD_TERM_BLUE            34
///
#define CBUILD_TERM_MAGENTA         35
///
#define CBUILD_TERM_CYAN            36
///
#define CBUILD_TERM_WHITE           37
///
#define CBUILD_TERM_BRBLACK         90
///
#define CBUILD_TERM_BRRED           91
///
#define CBUILD_TERM_BRGREEN         92
///
#define CBUILD_TERM_BRYELLOW        93
///
#define CBUILD_TERM_BRBLUE          94
///
#define CBUILD_TERM_BRMAGENTA       95
///
#define CBUILD_TERM_BRCYAN          96
///
#define CBUILD_TERM_BRWHITE         97
#define CBUILD_TERM_BLACKaddten     40
#define CBUILD_TERM_REDaddten       41
#define CBUILD_TERM_GREENaddten     42
#define CBUILD_TERM_YELLOWaddten    43
#define CBUILD_TERM_BLUEaddten      44
#define CBUILD_TERM_MAGENTAaddten   45
#define CBUILD_TERM_CYANaddten      46
#define CBUILD_TERM_WHITEaddten     47
#define CBUILD_TERM_BRBLACKaddten   100
#define CBUILD_TERM_BRREDaddten     101
#define CBUILD_TERM_BRGREENaddten   102
#define CBUILD_TERM_BRYELLOWaddten  103
#define CBUILD_TERM_BRBLUEaddten    104
#define CBUILD_TERM_BRMAGENTAaddten 105
#define CBUILD_TERM_BRCYANaddten    106
#define CBUILD_TERM_BRWHITEaddten   107

/* Log.h */

// #pragma once // For LSP 
//! This file provides small logger utility.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

#ifndef CBUILD_LOG_CUSTOM_LEVELS
	/// Supported log levels. Compared arithmetically.
	///
	/// This can be overridden by defining `CBUILD_LOG_CUSTOM_LEVELS` and then providing
	/// enumeration `cbuild_custom_log_level_t` (without typedef).
	/// This 4 levels should be present in you implementation of this enum, because
	/// they are used by CBuild itself.
	///
	/// You would also need to provide `const char* __cbuild_log_level_names[]`.
	/// This array is used by default handler to format names of log levels.
	/// Level is used as index and value is treated as a c-string.
	typedef enum {
		CBUILD_LOG_ERROR   = 1,
		CBUILD_LOG_WARN    = 2,
		CBUILD_LOG_INFO    = 3,
		CBUILD_LOG_TRACE   = 4,
	} cbuild_log_level_t;
#else
	typedef enum cbuild_custom_log_level_t cbuild_log_level_t;
#endif // CBUILD_LOG_CUSTOM_LEVELS

/// Main dispatcher for logger. Support printf-style formatting.
///
/// * [pl:level] Log level. Will be used both for printing and for disabling logs based on minimum level. 
/// * [pl:fmt] Format string for printf.
/// * [pl:...] Arguments for format string.
CBUILD_ATTR_PRINTF(2,
	CBUILDDEF void cbuild_log(cbuild_log_level_t level, const char* fmt, ...));

/// Wrapper for [`cbuild_log`](DOC:cbuild_log) that sets level to `CBUILD_LOG_ERROR`.
#define cbuild_log_error(msg, ...)                            \
	cbuild_log(CBUILD_LOG_ERROR, msg __VA_OPT__(,) __VA_ARGS__)
/// Wrapper for [`cbuild_log`](DOC:cbuild_log) that sets level to `CBUILD_LOG_WARN`.
#define cbuild_log_warn(msg, ...)                             \
	cbuild_log(CBUILD_LOG_WARN, msg __VA_OPT__(,) __VA_ARGS__)
/// Wrapper for [`cbuild_log`](DOC:cbuild_log) that sets level to `CBUILD_LOG_INFO`.
#define cbuild_log_info(msg, ...)                             \
	cbuild_log(CBUILD_LOG_INFO, msg __VA_OPT__(,) __VA_ARGS__)
/// Wrapper for [`cbuild_log`](DOC:cbuild_log) that sets level to `CBUILD_LOG_TRACE`.
#define cbuild_log_trace(msg, ...)                            \
	cbuild_log(CBUILD_LOG_TRACE, msg __VA_OPT__(,) __VA_ARGS__)

/// Same as [`cbuild_log`](DOC:cbuild_log) but takes `va_list`{.c} instead of variadics. 
///
/// * [pl:level] Log level. Will be used both for printing and for disabling logs based on minimum level. 
/// * [pl:fmt] Format string for printf.
/// * [pl:args] Arguments for format string.
CBUILDDEF void cbuild_vlog(cbuild_log_level_t level, const char* fmt, va_list args);

/// Set minimum level of logs that will be printed.
///
/// Name of this functions is rather misleading, because it disable all levels that are
/// arithmetically higher than specified here. So technically it set maximum level. But
/// for logs levels lower technically means  less severe, so name is like this.
CBUILDDEF void cbuild_log_set_min_level(cbuild_log_level_t level);
/// Get minimum level of logs that would be printed.
CBUILDDEF cbuild_log_level_t cbuild_log_get_min_level(void);

/// This function will be called only for messages that should be printed.
/// It can be treated as a formatter for logged messages.
///
/// * [pl:level] Log level name. 
/// * [pl:fmt] Format string for printf.
/// * [pl:args] Arguments for format string.
typedef void (*cbuild_log_handler_t)(const char* level, const char* fmt, va_list args);
/// Setup handler for logger. Passing NULL will disable logs.
/// Default log handler will be setup by default.
CBUILDDEF void cbuild_log_set_handler(cbuild_log_handler_t handler);
/// Get current log handler.
CBUILDDEF cbuild_log_handler_t cbuild_log_get_handler(void);
/// Default log handler for CBuild.
CBUILDDEF void __cbuild_default_log_handler(const char* level,
	const char* fmt, va_list args);

/* Arena.h */

// #pragma once // For LSP 
//! Arena allocator.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

/// Arena allocator state.
///
/// ::: note
/// Arena automatically pad things to `2 * sizeof(void*)`{.c}. This is done to
/// be a little more cache-friendly. Padding is done after allocation so it is
/// always placed after the data.
/// :::
///
/// * [fl:base] Base pointer for allocated memory.
/// * [fl:capacity] Capacity of this arena.
/// * [fl:pointer] Pointer to first unallocated byte.
typedef struct cbuild_arena_t {
	void* base;
	size_t capacity;
	size_t pointer;
	#ifdef CBUILD_PROFILER
		size_t max_pointer;
	#endif // CBUILD_PROFILER
} cbuild_arena_t;

/// Reallocate arena base to new size.
///
/// If old base is `NULL` then new memory will be allocated.
///
/// ::: note
/// This is just a direct call to `realloc` plus some metadata management.
/// :::
CBUILDDEF void cbuild_arena_base_realloc(cbuild_arena_t* arena, size_t new_capacity);
/// Allocate new arena base.
///
/// * [pl:arena:cbuild_arena_t*] Arena object.
/// * [pl:capacity:size_t] Capacity of arena.
#define cbuild_arena_base_malloc(arena, capacity) cbuild_arena_base_realloc(arena, capacity)
/// Free arena base.
CBUILDDEF void cbuild_arena_base_free(cbuild_arena_t* arena);
/// Allocate inside of arena.
///
/// * [pl:arena] Arena object.
/// * [pl:size] Number of bytes to allocate.
///
/// [r:] Pointer into arena. NULL if out-of-memory.
CBUILDDEF void* cbuild_arena_malloc(cbuild_arena_t* arena, size_t size);
/// `realloc` for arena. Will not free old memory.
///
/// It [p:size] is bigger than old size then memory from adjacent allocations
/// will be used to initialize new memory block. This is safe access (no
/// segmentation fault possible), but can expose unrelated memory trough
/// new allocation.
CBUILDDEF void* cbuild_arena_realloc(cbuild_arena_t* arena, void* ptr, size_t size);
/// Save checkpoint into arena.
CBUILDDEF size_t cbuild_arena_checkpoint(cbuild_arena_t* arena);
/// Reset arena to some checkpoint.
CBUILDDEF void cbuild_arena_reset(cbuild_arena_t* arena, size_t checkpoint);
/// Free arena. Set `pointer` to 0.
///
/// * [pl:arena:cbuild_arena_t*] Arena object.
#define cbuild_arena_free(arena) cbuild_arena_reset(arena, 0)
/// `strdup` that uses arena as allocator.
CBUILDDEF char* cbuild_arena_strdup(cbuild_arena_t* arena, const char* src);
/// `memdup` that uses arena as its allocator.
CBUILDDEF void* cbuild_arena_memdup(cbuild_arena_t* arena, const void* src, size_t n);
/// `sprintf` that uses arena as its allocator.
CBUILDDEF char* cbuild_arena_sprintf(cbuild_arena_t* arena, const char* fmt, ...);
/// `vsprintf` that uses arena as its allocator.
CBUILDDEF char* cbuild_arena_vsprintf(cbuild_arena_t* arena, const char* fmt,
	va_list args);

//! # Arena allocator profiles
//!
//! This adds one size_t field to each arena that track maximum allocated size.
//! This makes each allocation a little slower.
//!
//! `cbuild_arena_profiler` Just prints usage as `TRACE` log.

#ifdef CBUILD_PROFILER
	CBUILDDEF void cbuild_arena_profiler(cbuild_arena_t* arena, const char* arena_id);
#endif // CBUILD_PROFILER

/* Temp.h */

// #pragma once // For LSP 
//! Global arena for CBuild. Uses bump allocator under the hood.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

/// Same as `malloc`. Can return NULL.
CBUILDDEF void* cbuild_temp_malloc(size_t size);
/// Same as `realloc`. Does not free old memory. Can return NULL.
CBUILDDEF void* cbuild_temp_realloc(void* ptr, size_t size);
/// `sprintf` that allocates using `cbuild_temp_malloc`.
CBUILD_ATTR_PRINTF(1, 
	CBUILDDEF char* cbuild_temp_sprintf(const char* fmt, ...));
/// `vsprintf` that allocates using `cbuild_temp_malloc`.
CBUILDDEF char* cbuild_temp_vsprintf(const char* fmt, va_list ap);
/// `strdup` that allocates using `cbuild_temp_malloc`.
CBUILDDEF char* cbuild_temp_strdup(const char* str);
/// Same as `strdup` but copies raw memory. Allocates using `cbuild_temp_malloc`.
CBUILDDEF void* cbuild_temp_memdup(const void* mem, size_t size);
/// Reset temporary allocator back to [p:checkpoint]. 
CBUILDDEF void cbuild_temp_reset(size_t checkpoint);
/// Create new allocator checkpoint.
CBUILDDEF size_t cbuild_temp_checkpoint(void);
#ifdef CBUILD_PROFILER
	/// Profile temp allocator. Same as `cbuild_arena_profile`
	CBUILDDEF void cbuild_temp_profiler(void);
#endif // CBUILD_PROFILER

/* DynArray.h */

// #pragma once // For LSP 
//! Implements dynamic array datatype. It works over structures with
//! specific "form", where `T` is a generic type:
//!
//! ```c
//! typedef struct cbuild_da_t
//!     T* data;
//!     size_t size;
//!     size_t capacity;
//! }
//! ```
//!
//! License: `GPL-3.0-or-later`.

/// Create type for a new dynamic array.
///
/// * [pl:type:type] Type of elements for a new dynamic array.
#define cbuild_da_new(type) \
	struct {                  \
		type* data;             \
		size_t size;            \
		size_t capacity;        \
	}
/// Append new element to dynamic array.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:elem:*typeof(da->data)] Element to append.
#define cbuild_da_append(da, elem)                                             \
	do {                                                                         \
		if (((da)->size + 1) > (da)->capacity) {                                   \
			cbuild_da_resize((da), 0);                                               \
		}                                                                          \
		(da)->data[(da)->size++] = elem;                                           \
	} while(0)
/// Append new elements to dynamic array.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:arr:typeof(da->data)] Elements to append.
/// * [pl:arr_size:size_t] Number of elements to append.
#define cbuild_da_append_arr(da, arr, arr_size)                                \
	do {                                                                         \
		if (((da)->size + (arr_size)) > (da)->capacity) {                          \
			cbuild_da_resize((da), (da)->capacity + (arr_size));                     \
		}                                                                          \
		memcpy((da)->data + (da)->size, (arr),                                     \
			(arr_size) * sizeof(typeof(*(da)->data)));                               \
		(da)->size += (arr_size);                                                  \
	} while(0)
/// Append new elements to dynamic array.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:...:*typeof(da->data)] Elements to append.
#define cbuild_da_append_many(da, ...)                                         \
	do {                                                                         \
		typeof(*((da)->data))   __cbuild_da_tmp_arr[] = { __VA_ARGS__ };           \
		size_t __cbuild_da_tmp_count =                                             \
			sizeof(__cbuild_da_tmp_arr) / sizeof(__cbuild_da_tmp_arr[0]);            \
		cbuild_da_append_arr((da), __cbuild_da_tmp_arr, __cbuild_da_tmp_count);    \
	} while (0)
/// Set element in dynamic array using its index.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:idx:size_t] Element index.
/// * [pl:elem:*typeof(da->data)] New element's value.
/// 
/// [r:bool] False on overflow.
#define cbuild_da_set(da, idx, elem)                                           \
	({                                                                           \
		bool __cbuild__ret = false;                                                \
		if ((idx) >= (da)->size) {                                                 \
			cbuild_log(CBUILD_LOG_ERROR, "Index overflow in set.");                  \
			__cbuild__ret = false;                                                   \
		} else {                                                                   \
			(da)->data[(idx)] = elem;                                                \
			__cbuild__ret = true;                                                    \
		}                                                                          \
		__cbuild__ret;                                                             \
	})
/// Get element from dynamic array using its index.
///
/// * [pl:da:const cbuild_da_t*] Dynamic array object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:typeof(da->data)] Pointer to an element or `NULL`{.c} if index out of bounds.
#define cbuild_da_get(da, idx)                                                 \
	({                                                                           \
		typeof(*(da)->data)* __cbuild__ret = NULL;                                 \
		if ((idx) >= (da)->size) {                                                 \
			cbuild_log(CBUILD_LOG_ERROR, "Index overflow in get.");                  \
			__cbuild__ret = NULL;                                                    \
		} else {                                                                   \
			__cbuild__ret = &((da)->data[(idx)]);                                    \
		}                                                                          \
		__cbuild__ret;                                                             \
	})
/// Remove element dynamic array using its index.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:bool] False on overflow.
#define cbuild_da_remove(da, idx)                                              \
	({                                                                           \
		bool __cbuild__ret = false;                                                \
		if ((idx) >= (da)->size) {                                                 \
			cbuild_log(CBUILD_LOG_ERROR, "Index overflow in remove.");               \
			__cbuild__ret = false;                                                   \
		} else {                                                                   \
			memmove((da)->data + (idx), ((da)->data + ((idx) + 1)),                  \
				((da)->size - ((idx) + 1)) * sizeof(typeof(*(da)->data)));             \
			(da)->size--;                                                            \
			__cbuild__ret = true;                                                    \
		}                                                                          \
		__cbuild__ret;                                                             \
	})
/// Remove element dynamic array using its index.
///
/// This function has O(1) complexity but changes ordering of elements in 
/// an array. It copies last element into same index as removed one, overriding
/// it and then just decrease size by 1.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:bool] False on overflow.
#define cbuild_da_remove_unordered(da, idx)                                    \
	({                                                                           \
		bool __cbuild__ret = false;                                                \
		if ((idx) >= (da)->size) {                                                 \
			cbuild_log(CBUILD_LOG_ERROR, "Index overflow in remove.");               \
			__cbuild__ret = false;                                                   \
		} else {                                                                   \
			(da)->data[(idx)] = (da)->data[--(da)->size];                            \
			__cbuild__ret = true;                                                    \
		}                                                                          \
		__cbuild__ret;                                                             \
	})
/// Resize dynamic array.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
/// * [pl:new_size:size_t] Resize dynamic array. If new_size equal 0 default growth strategy will be used.
#define cbuild_da_resize(da, new_size)                                         \
	do {                                                                         \
		if ((da)->data == NULL) {                                                  \
			if ((new_size) == 0) {                                                   \
				(da)->capacity = CBUILD_INIT_CAPACITY;                                 \
			} else {                                                                 \
				(da)->capacity = (new_size);                                           \
			}                                                                        \
			(da)->data = __CBUILD_MALLOC(                                            \
				(da)->capacity * sizeof(typeof(*(da)->data)));                         \
		} else {                                                                   \
			if ((new_size) == 0) {                                                   \
				(da)->capacity *= 2;                                                   \
			} else {                                                                 \
				(da)->capacity = (new_size);                                           \
			}                                                                        \
			(da)->data = __CBUILD_REALLOC((da)->data,                                \
				(da)->capacity * sizeof(typeof(*(da)->data)));                         \
		}                                                                          \
		cbuild_assert((da)->data != NULL, "Allocation failed.\n");                 \
		((da)->size > (da)->capacity) ? ((da)->size = (da)->capacity) : 0;         \
	} while(0)
/// Free dynamic array.
///
/// This should be used only if you want to free underlying memory, if you just
/// want to clear array and reuse it, setting `size` to `0` would be much faster.
///
/// * [pl:da:cbuild_da_t*] Dynamic array object.
#define cbuild_da_clear(da)                                                    \
	do {                                                                         \
		__CBUILD_FREE((da)->data);                                                 \
		(da)->data     = NULL;                                                     \
		(da)->size     = 0;                                                        \
		(da)->capacity = 0;                                                        \
	} while(0)
/// Foreach implementation for dynamic array. Same semantics as 'for' loop.
///
/// * [pl:da:cbuild_da_t] Dynamic array object.
/// * [pl:iter:name] Name if variable that will be used as iterator. Will have type `typeof(da->data)`{.c}.
#define cbuild_da_foreach(da, iter)                                            \
	for (typeof(*((da).data))* iter = (da).data;                                 \
		iter < ((da).data + (da).size); iter++)

/* StringView.h */

// #pragma once // For LSP 
//! Implementation of string view in C.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

/// String view datatype
///
/// * [fl:data] Pointer into data. This pointer does not "own" its data.
/// * [fl:size] Length of string view. Data in range from `data`{.c} to `data+size`{.c} is guaranteed to be valid.
typedef struct cbuild_sv_t {
	char*  data;
	size_t size;
} cbuild_sv_t;
/// Format description for string view for printf-style formatters.
#define CBuildSVFmt     "%.*s"
/// Format argument for string view for printf-style formatters.
#define CBuildSVArg(sv) (int)(sv).size, (sv).data
/// Create new string view from raw parts
#define cbuild_sv_from_parts(_data, _size)                   \
	((cbuild_sv_t){ .data = (char*)(_data), .size = (_size) })
/// Create new string view from C-string. 
#define cbuild_sv_from_cstr(cstr)                                \
((cbuild_sv_t){ .data = (char*)(cstr), .size = strlen((cstr)) })
/// Create new string view from C-string literal (compile-time). 
#define cbuild_sv_from_lit(lit)                                  \
((cbuild_sv_t){ .data = (char*)(lit), .size = sizeof(lit) - 1 })
/// Trim whitespaces from a string view starting from the left.
///
/// [r:] Number of characters removed.
CBUILDDEF size_t cbuild_sv_trim_left(cbuild_sv_t* sv);
/// Trim whitespaces from a string view starting from the right.
///
/// [r:] Number of characters removed.
CBUILDDEF size_t cbuild_sv_trim_right(cbuild_sv_t* sv);
/// Trim whitespaces from a string view starting from both sides.
///
/// [r:] Number of characters removed.
CBUILDDEF size_t cbuild_sv_trim(cbuild_sv_t* sv);
/// Chop [p:size] characters from string view starting from the left.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:size] Number of character that will be chopped.
///
/// [r:] New string view containing chopped characters.
CBUILDDEF cbuild_sv_t cbuild_sv_chop(cbuild_sv_t* sv, size_t size);
/// Chop [p:size] characters from string view starting from the right.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:size] Number of character that will be chopped.
///
/// [r:] New string view containing chopped characters.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_right(cbuild_sv_t* sv, size_t size);
/// Chop characters from string view starting from the left until [p:delim] is found.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter character.
///
/// [r:] New string view containing chopped characters. [p:delim] will not be included.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_by_delim(cbuild_sv_t* sv, char delim);
/// Chop characters from string view starting from the right until [p:delim] is found.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter character.
///
/// [r:] New string view containing chopped characters. [p:delim] will not be included.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_right_by_delim(cbuild_sv_t* sv, char delim);
/// Chop characters from string view starting from the left until [p:delim] is found.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter string view.
///
/// [r:] New string view containing chopped characters. [p:delim] will not be included.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_by_sv(cbuild_sv_t* sv, cbuild_sv_t delim);
/// Chop characters from string view starting from the right until [p:delim] is found.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter string view.
///
/// [r:] New string view containing chopped characters. [p:delim] will not be included.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_right_by_sv(cbuild_sv_t* sv, cbuild_sv_t delim);
/// Delimiter function. Used as comparator in [`cbuild_sv_chop_by_func`](DOC:cbuild_sv_chop_by_func)
/// and [`cbuild_sv_chop_right_by_func`](DOC:cbuild_sv_chop_right_by_func)
///
/// * [pl:sv] String view passed to function.
/// * [pl:idx] Current character.
/// * [pl:args] Arguments passed by caller.
///
/// [r:] If true is returned this character will be considered a delimiter
typedef bool (*cbuild_sv_delim_func)(const cbuild_sv_t* sv, size_t idx, void* args);
/// chop characters from string view starting from the left until [p:delim] returns `true`.
///
/// * [pl:sv] String view from which data will be chopped. its size will be reduced.
/// * [pl:delim] Delimiter function.
/// * [pl:args] Arguments to a function.
///
/// [r:] new string view containing chopped characters. delimeter character will not be in any string view.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_by_func(cbuild_sv_t* sv,
	cbuild_sv_delim_func delim, void* args);
/// chop characters from string view starting from the right until [p:delim] returns `true`.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter function.
/// * [pl:args] Arguments to a function.
///
/// [r:] new string view containing chopped characters. Delimiter character will not be in any string view.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_right_by_func(cbuild_sv_t* sv,
	cbuild_sv_delim_func delim, void* args);
/// `strcmp` for string view. Fully compatible but extends API. 
///
/// * [r:-2] If size of first string view is smaller.
/// * [r:-1] If first different character in first string view is smaller.
/// * [r:0] If two string views are equal.
/// * [r:1] If first different character in first string view is larger.
/// [r:2] If size of first string view is larger.
CBUILDDEF int cbuild_sv_cmp(cbuild_sv_t a, cbuild_sv_t b);
/// `strcmp` for string view. Fully compatible but extends API. 
/// Performs case folding for ASCII.
///
/// * [r:-2] If size of first string view is smaller.
/// * [r:-1] If first different character in first string view is smaller.
/// * [r:0] If two string views are equal.
/// * [r:1] If first different character in first string view is larger.
/// * [r:2] If size of first string view is larger.
CBUILDDEF int cbuild_sv_cmp_icase(cbuild_sv_t a, cbuild_sv_t b);
/// Check if string view has specific prefix.
CBUILDDEF bool cbuild_sv_prefix(cbuild_sv_t sv, cbuild_sv_t prefix);
/// Check if string view has specific suffix.
CBUILDDEF bool cbuild_sv_suffix(cbuild_sv_t sv, cbuild_sv_t suffix);
/// Find specific character in string view.
///
/// [r:] Index of character or `-1`{.c} if not found
CBUILDDEF ssize_t cbuild_sv_find(cbuild_sv_t sv, char c);
/// Find specific substring (provided as string view) in another string view.
///
/// [r:] Index of first character of [p:needle] or `-1`{.c} if not found
CBUILDDEF ssize_t cbuild_sv_find_sv(cbuild_sv_t sv, cbuild_sv_t needle);
/// Find specific character in string view. Searches from the end.
///
/// [r:] Index of character or `-1`{.c} if not found
CBUILDDEF ssize_t cbuild_sv_rfind(cbuild_sv_t sv, char c);
/// Check if string view contains some character.
CBUILDDEF bool cbuild_sv_contains(cbuild_sv_t sv, char c);
/// Check substring (provided as string view) is contained in another string view.
CBUILDDEF bool cbuild_sv_contains_sv(cbuild_sv_t sv, cbuild_sv_t needle);
/// Get length of first utf8 codepoint in string view.
///
/// ::: note
/// Invalid codepoints assumed to have length of 1
/// :::
///
/// [r:] Length of codepoint in bytes.
CBUILDDEF int cbuild_sv_utf8cp_len(cbuild_sv_t sv);
/// `strchr` implementation for string view.
///
/// * [pl:sv] String view to work with
/// * [pl:c] Character that will be searched.
///
/// [r:] Pointer to character or NULL if not found.
CBUILDDEF char* cbuild_sv_strchr(cbuild_sv_t sv, char c);
/// `strchr` implementation for string view.
/// Operates on utf8 codepoints instead of ASCII characters.
///
/// * [pl:sv] String view to work with
/// * [pl:c] Codepoint that will be searched.
///
/// [r:] Pointer to character or NULL if not found.
CBUILDDEF char* cbuild_sv_utf8chr(cbuild_sv_t sv, uint32_t c);
/// Chop first utf8 codepoint out of string view.
///
/// [r:] Unicode codepoint. `UINT32_MAX`{.c} means error.
CBUILDDEF uint32_t cbuild_sv_chop_utf8(cbuild_sv_t* sv);
/// Chop characters from string view starting from the left until [p:delim] is found.
/// Operates on utf8 codepoints instead of ASCII characters.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter character.
///
/// [r:] New string view containing chopped characters. [p:delim] will not be included.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_by_utf8(cbuild_sv_t* sv, uint32_t delim);
/// Delimiter function. Used as comparator in 
/// [`cbuild_sv_chop_by_func_utf8`](DOC:cbuild_sv_chop_by_func_utf8)
///
/// * [pl:sv] String view, prechopped, so first character is possible delimiter and should be checked.
/// * [pl:args] Arguments passed by caller.
///
/// [r:] If true is returned this character will be considered a delimiter
typedef bool (*cbuild_sv_utf8delim_func)(const cbuild_sv_t* sv, void* args);
/// chop characters from string view starting from the right until [p:delim] returns `true`.
/// Operates on utf8 codepoints instead of ASCII characters.
///
/// * [pl:sv] String view from which data will be chopped. Its size will be reduced.
/// * [pl:delim] Delimiter function.
/// * [pl:args] Arguments to a function.
///
/// [r:] new string view containing chopped characters. Delimiter character will not be in any string view.
CBUILDDEF cbuild_sv_t cbuild_sv_chop_by_func_utf8(cbuild_sv_t* sv,
	cbuild_sv_utf8delim_func delim, void* args);
/// Get length of a string view in utf8 codepoints.
///
/// ::: note
/// Invalid codepoints assumed to have length of 1
/// :::
CBUILDDEF size_t cbuild_sv_utf8len(cbuild_sv_t sv);
/// Check if string view is a valid utf8 data.
///
/// * [pl:sv] String view to work with.
/// * [pl:idx] If validation failed this will store index of first invalid codepoint. Can be `NULL`{.c}.
///
/// [r:] `false` if validation failed.
CBUILDDEF bool cbuild_sv_utf8valid(cbuild_sv_t sv, size_t* idx);
/// Convert string view to c-string. Allocate memory via `malloc`.
CBUILDDEF char* cbuild_sv_to_cstr(cbuild_sv_t sv);
/// Convert string view to c-string. Allocate memory via `malloc`.
#define cbuild_cstr_from_sv(sv) cbuild_sv_to_cstr(sv)
/// Convert string view to c-string. Allocate memory via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
CBUILDDEF char* cbuild_sv_to_temp_cstr(cbuild_sv_t sv);
/// Convert string view to c-string. Allocate memory via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
#define cbuild_temp_cstr_from_sv(sv) cbuild_temp_sv_to_cstr(sv)

/* StringBuilder.h */

// #pragma once // For LSP 
//! Dynamic string - string builder. Specialization of DynArray.h for `char`{.c} datatype.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"
// #include "StringView.h"

/// String builder string builder.
///
/// * [fl:data] Pointer to data. This pointer "owns" the data.
/// * [fl:size] Length of string builder. Data in range from `data`{.c} to `data+size`{.c} is guaranteed to be valid.
/// * [fl:capacity] Length of allocated data. Data is guaranteed to be allocated (but may be not initialized) from `data`{.c} to `data+size`{.c}.
typedef struct cbuild_sb_t {
	char* data;
	size_t size;
	size_t capacity;
} cbuild_sb_t;
/// Format description for string view for printf-style formatters.
#define CBuildSBFmt     "%.*s"
/// Format argument for string view for printf-style formatters.
#define CBuildSBArg(sb) (int)(sb).size, (sb).data
/// Append new element to string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:elem:char] Element to append.
#define cbuild_sb_append(sb, elem) cbuild_da_append((sb), (elem))
/// Append new elements to string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:arr:char*] Elements to append.
/// * [pl:arr_size:size_t] Number of elements to append.
#define cbuild_sb_append_arr(sb, arr, size) cbuild_da_append_arr((sb), (arr), (size))
/// Append new elements to string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:...:char] Elements to append.
#define cbuild_sb_append_many(sb, ...) cbuild_da_append_many((sb), __VA_ARGS__)
/// Append c-string to string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:cstr:char*] C-string (NULL-terminated).
#define cbuild_sb_append_cstr(sb, cstr)	cbuild_da_append_arr((sb), (cstr), strlen(cstr))
/// Append NULL-terminator to string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
#define cbuild_sb_append_null(sb) cbuild_da_append((sb), '\0')
/// Set element in string builder using its index.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:idx:size_t] Element index.
/// * [pl:elem:char] New element's value.
/// 
/// [r:bool] False on overflow.
#define cbuild_sb_set(sb, idx, elem) cbuild_da_set((sb), (idx), (elem))
/// Get element from string builder using its index.
///
/// * [pl:sb:const cbuild_sb_t*] String builder object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:char] Pointer to an element or `NULL`{.c} if index out of bounds.
#define cbuild_sb_get(sb, idx) cbuild_da_get((sb), (idx))
/// Remove element string builder using its index.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:bool] False on overflow.
#define cbuild_sb_remove(sb, idx) cbuild_da_remove((sb), (idx))
/// Resize string builder.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:new_size:size_t] Resize string builder. If new_size equal 0 default growth strategy will be used.
#define cbuild_sb_resize(sb, new_capacity) cbuild_da_resize((sb), (new_capacity))
/// Free string builder.
///
/// This should be used only if you want to free underlying memory, if you just
/// want to clear array and reuse it, setting `size` to `0` would be much faster.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
#define cbuild_sb_clear(sb) cbuild_da_clear(sb)
/// Foreach implementation for string builder. Same semantics as 'for' loop.
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:iter:name] Name if variable that will be used as iterator. Will have type `char`{.c}.
#define cbuild_sb_foreach(sb, iter) cbuild_da_foreach(sb, iter)
/// `strcmp` for string builder. Fully compatible but extends API. 
///
/// * [r:-2] If size of first string builder is smaller.
/// * [r:-1] If first different character in first string builder is smaller.
/// * [r:0] If two string builders are equal.
/// * [r:1] If first different character in first string builder is larger.
/// * [r:2] If size of first string builder is larger.
CBUILDDEF int cbuild_sb_cmp(cbuild_sb_t a, cbuild_sb_t b);
/// `strcmp` for string builder. Fully compatible but extends API. 
/// Performs case folding for ASCII.
///
/// * [r:-2] If size of first string builder is smaller.
/// * [r:-1] If first different character in first string builder is smaller.
/// * [r:0] If two string builders are equal.
/// * [r:1] If first different character in first string builder is larger.
/// * [r:2] If size of first string builder is larger.
CBUILDDEF int cbuild_sb_cmp_icase(cbuild_sb_t a, cbuild_sb_t b);
/// Append utf8 character to a string builder.
///
/// * [pl:sb] String builder object.
/// * [pl:cp] UTF8 codepoint.
CBUILDDEF void cbuild_sb_append_utf8(cbuild_sb_t* sb, uint32_t cp);
/// Get length of a string builder in utf8 codepoints.
///
/// ::: note
/// Invalid codepoints assumed to have length of 1
/// :::
CBUILDDEF size_t cbuild_sb_utf8len(cbuild_sb_t sb);
/// Convert string builder to a string view.
CBUILDDEF cbuild_sv_t cbuild_sb_to_sv(cbuild_sb_t sb);
/// Convert string builder to a string view.
#define	cbuild_sv_from_sb(sb) cbuild_sb_to_sv(sb)
/// Convert string view to a string builder (allocated new data).
CBUILDDEF cbuild_sb_t cbuild_sv_to_sb(cbuild_sv_t sv);
/// Convert string view to a string builder (allocated new data).
#define cbuild_sb_from_sv(sv) cbuild_sv_to_sb(sv)
/// Apppend string view to a string builder
///
/// * [pl:sb:cbuild_sb_t*] String builder object.
/// * [pl:cstr:cbuild_sv_t] String view.
#define cbuild_sb_append_sv(sb, sv) cbuild_sb_append_arr((sb), (sv).data, (sv).size)
/// Append data to a string builder using printf-style format string.
CBUILD_ATTR_PRINTF(2,
	CBUILDDEF int cbuild_sb_appendf(cbuild_sb_t* sb, const char* fmt, ...));
/// Append data to a string builder using printf-style format string.
CBUILDDEF int cbuild_sb_vappendf(cbuild_sb_t* sb, const char* fmt, va_list args);

/* Stack.h */

// #pragma once // For LSP 
//! Stack datatype implementation. This implements same overall datastructure
//! as dynamic array but with simplified API to only allow `push`, `pop` and
//! `top` (just read top element without poping it) operations. It works over
//! structures with specific "form", where `T` is a generic type:
//!
//! ```c
//! typedef struct cbuild_da_t
//!     T* data;
//!     size_t ptr;
//!     size_t capacity;
//! }
//! ```
//!
//! License: `GPL-3.0-or-later`.

/// Push a new element on top of a stack.
///
/// * [pl:stack:cbuild_stack_t*] Stack object.
/// * [pl:elem:*typeof(stack->data)] New element.
#define cbuild_stack_push(stack, elem)                                         \
	do {                                                                         \
		if ((stack)->ptr >= (stack)->capacity) {                                   \
			cbuild_stack_resize((stack), 0);                                         \
		}                                                                          \
		(stack)->data[(stack)->ptr++] = elem;                                      \
	} while(0)
/// Pop element from top of a stack.
///
/// * [pl:stack:cbuild_stack_t*] Stack object.
/// 
/// [r:*typeof(stack->data)] Popped element.
#define cbuild_stack_pop(stack)                                                \
	({                                                                           \
		cbuild_assert((stack)->ptr != 0, "Stack underfloor.\n");                   \
		(stack)->data[--(stack)->ptr];                                             \
	})
/// Get element from top of a stack.
///
/// * [pl:stack:cbuild_stack_t*] Stack object.
/// 
/// [r:*typeof(stack->data)] Element on top of a stack.
#define cbuild_stack_top(stack)                                                \
	({                                                                           \
		cbuild_assert((stack)->ptr != 0, "Stack underfloor.\n");                   \
		(stack)->data[(stack)->ptr - 1];                                           \
	})
/// Resize dynamic array.
///
/// * [pl:stack:cbuild_stack_t*] Stack object.
/// * [pl:new_size:size_t] Resize stack If new_size equal 0 default growth strategy will be used.
#define cbuild_stack_resize(stack, size)                                       \
	do {                                                                         \
		if ((stack)->data == NULL) {                                               \
			if (size == 0) {                                                         \
				(stack)->capacity = CBUILD_INIT_CAPACITY;                              \
			} else {                                                                 \
				(stack)->capacity = size;                                              \
			}                                                                        \
			(stack)->data = __CBUILD_MALLOC(                                         \
				(stack)->capacity * sizeof(typeof(*(stack)->data)));                   \
		} else {                                                                   \
			if (size == 0) {                                                         \
				(stack)->capacity *= 2;                                                \
			} else {                                                                 \
				(stack)->capacity = size;                                              \
			}                                                                        \
			(stack)->data = __CBUILD_REALLOC((stack)->data,                          \
				(stack)->capacity * sizeof(typeof(*(stack)->data)));                   \
		}                                                                          \
		cbuild_assert((stack)->data != NULL, "Allocation failed.\n");              \
	} while (0)
/// Free stack.
///
/// This should be used only if you want to free underlying memory, if you just
/// want to clear stack and reuse it, setting `ptr` to `0` would be much faster.
///
/// * [pl:stack:cbuild_stack_t*] Stack object.
#define cbuild_stack_clear(stack)                                              \
	do {                                                                         \
		__CBUILD_FREE((stack)->data);                                              \
		(stack)->data     = NULL;                                                  \
		(stack)->ptr      = 0;                                                     \
		(stack)->capacity = 0;                                                     \
	} while(0)

/* Map.h */

// #pragma once // For LSP 
//! Hasmap implementation in C.
//!
//! License: `GPL-3.0-or-later`.
//!
//! This map has rather interesting API and contains.
//!
//! # No resize
//!
//! It does not resize on its own, but you can approximate usage by checking
//! number of elements in each bucket and reallocate map when it will not create
//! performance problems in your app. This can be done by just creating a new
//! map and copying data from an old one (this is then only way to resize
//! hasmap implemented like this).
//!
//! # Not a tree
//!
//! It is implemented as a real hasmap and has O(1) access time (at least until
//! most buckets have at most one element in them). If buckets are full performace
//! start to degrade to O(n) where n is average number of elements in a bucket.
//!
//! # Managing data is user's responsibility
//!
//! It does not have **set** operation, only **get** and **get_or_alloc**.
//! Because working with generic datatypes in C is pretty hard, this map does not
//! rely on a type of key or data. User need to supply size of a key and full
//! pair struct in bytes, and then key compared and hashed as byte array of
//! specified size. Map functions always take key by pointer because of this.
//! Elements is never written and **get_or_alloc** returns empty zero-initialized
//! memory. You need to write key and value yourself there. You can treat this
//! memory as either two byte arrays one after another or cast it to some struct.
//!
//! Map can also use custom clear function for elements (because it can consist
//! of any data, including pointers that "own" some external memory. While this
//! function primary role is to clear element, it can also be used to properly
//! free key that need some extra care to do that.
//!
//! # Support for C-strings as keys
//!
//! For key one extra case is handled - if key size is declared as 0, then key
//! is assumed to be `const char*` pointer to NULL-terminated string. This will
//! be used only when hashing or comparing keys, so if key is not a constant
//! string and may need to be freed, proper clear function should be provided.
//!
//! # Extensibility
//!
//! If key has some specific properties, or is not stored inline in *pair*
//! structure, then custom **hash** and **keycmp** functions can be provided.
//!
//! If extra data need to be attached to a map, eg. key is an array, with size
//! fixed per map object, structure like this can be defined:
//!
//! ```c
//! typedef struct map_with_array_key_t {
//!     cbuild_map_t map;
//!     size_t key_element_count;
//! } map_with_array_key_t;
//! ```
//!
//! Then pointer to `.map` is passe to map function, but because C places struct
//! fields in memory in a same way they are defined in code, this pointer can
//! later be converted to pointer to outside structure by any of user-passed
//! hook function. This will allow to access any additional data without using
//! standard way of passing arguments to callback via `void*`{.c} parameter, thus
//! simplifying API for a general use case.

// #include "Common.h"

/// Hash function for a key.
///
/// * [pl:map] Pointer to map object. Can be used to retrieve extra data.
/// * [pl:key] Pointer to a start of key (start of pair structure).
///
/// [r:] Hash of this specific key. Will be truncated via `% nbuckets`{.c}.
typedef size_t (*cbuild_map_hash_t)(const void* map, const void* key);
/// Key comparison function. Should return `true` if keys are the same.
///
/// ::: note
/// This function should resolve hash collisions, so it can not rely on
/// comparing hashes.
/// :::
///
/// * [pl:map] Pointer to map object. Can be used to retrieve extra data.
/// * [pl:k1] Pointer to a start of key 1 (start of pair structure).
/// * [pl:k2] Pointer to a start of key 2 (start of pair structure).
typedef bool (*cbuild_map_keycmp_t)(const void* map, const void* k1,
	const void* k2);
/// Element clear function.
///
/// * [pl:map] Pointer to map object. Can be used to retrieve extra data.
/// * [pl:elem] Element that should be cleared. Pointer to a start of a pair structure.
typedef void (*cbuild_map_clear_t)(const void* map, void* elem);

/// This structure is internal and should not be used outside of a map.
/// It is docomented because it can be used to retrieve usage starts from a map.
///
/// * [fl:vals] Data of this bucket. Byte array of all pairs.
/// * [fl:nvals] Number of values stored in this bucket.
typedef struct cbuild_map_bucket_t {
	void* vals;
	size_t nvals;
} cbuild_map_bucket_t;

/// Map structure. Some fields here are considered as configuration data, and
/// should be modified by user by hand, while others are considered internal
/// values and should not be modified (but still can be if you know what you
/// are doing).
///
/// * [fl:buckets] List of bucket this map has. Pre-allocated on by [`cbuild_map_init`](DOC:cbuild_map_init).
/// * [fl:nbuckets] Number of buckets. Parameter to [`cbuild_map_init`](DOC:cbuild_map_init).
/// * [fl:hash_func] Custom hash function, can be left as `NULL`{.c} if unused.
/// * [fl:keycmp_func] Custom key comparison function, can be `NULL`{.c} if unused.
/// * [fl:key_size] Size of key. Should be set manually before call to [`cbuild_map_init`](DOC:cbuild_map_init).
/// * [fl:elem_size] Size of pair structure. Should be set manually before call to [`cbuild_map_init`](DOC:cbuild_map_init).
/// * [fl:iter_buckets] Internal counter used by iterator functions.
/// * [fl:iter_vals] Internal counter used by iterator functions.
typedef struct cbuild_map_t {
	cbuild_map_bucket_t* buckets;
	size_t nbuckets;
	cbuild_map_hash_t hash_func;
	cbuild_map_keycmp_t keycmp_func;
	cbuild_map_clear_t clear_func;
	size_t key_size;
	size_t elem_size;
	size_t iter_buckets;
	size_t iter_vals;
} cbuild_map_t;

/// Initialize a new map. Both *key_size* and *elem_size* must be valid at
/// this point. If needed *hash_func*, *keycmp_func* and *clear_func* can be
/// invalid, but it is better to init everything in one place, so it is a good
/// practive to have them valid before call to init too.
CBUILDDEF void cbuild_map_init(cbuild_map_t* map, size_t nbuckets);
/// Get element from a map. Raw semi-internal function.
///
/// * [pl:map] Map object.
/// * [pl:key] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:] Pointer to a pair structure or NULL if not found.
CBUILDDEF void* cbuild_map_get_raw(const cbuild_map_t* map, const void* key);
/// Get element from a map.
///
/// This function will allocate one stack variable that will hold copy of key.
/// This function will check if size of this variable matches `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:any] Value of a key. Need to be copyable via `=`.
///
/// [r:void*] Pointer to a pair structure or NULL if not found.
#define cbuild_map_get(map, key)                                               \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(__cbuild_tmp_key) == (map)->key_size ||               \
			((map)->key_size == 0 && sizeof(__cbuild_tmp_key) == sizeof(char*)),     \
			"Key type mismatch!\n");                                                 \
		cbuild_map_get_raw((map), &__cbuild_tmp_key);                              \
	})
/// Get element from a map.
///
/// This function will allocate one stack variable that will hold copy of pointer
/// to a key. This function will check if size of this variable matches 
/// `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:const void*] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:void*] Pointer to a pair structure or NULL if not found.
#define cbuild_map_get_ptr(map, key)                                           \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(*__cbuild_tmp_key) == (map)->key_size ||              \
			((map)->key_size == 0 && sizeof(*__cbuild_tmp_key) == sizeof(char*)),    \
			"Key type mismatch!\n");                                                 \
		cbuild_map_get_raw((map), __cbuild_tmp_key);                               \
	})
/// Get element from a map or allocate a new one. Raw semi-internal function.
///
/// ::: note
/// This will not insert key into an element. You will get full pair structure
/// that is at best zero-initialized. If you are inserted value you may copy key
/// unconditionally. If you key is small you can do this too. If key is big and
/// it may take a lot of time for copy you can have some checks to verify that
/// key is valid.
/// :::
///
/// * [pl:map] Map object.
/// * [pl:key] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:] Pointer to a pair structure or NULL if not found.
CBUILDDEF void* cbuild_map_get_or_alloc_raw(cbuild_map_t* map, const void* key);
/// Get element from a map or allocate a new one. Raw semi-internal function.
///
/// ::: note
/// This will not insert key into an element. You will get full pair structure
/// that is at best zero-initialized. If you are inserted value you may copy key
/// unconditionally. If you key is small you can do this too. If key is big and
/// it may take a lot of time for copy you can have some checks to verify that
/// key is valid.
/// :::
///
/// This function will allocate one stack variable that will hold copy of key.
/// This function will check if size of this variable matches `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:any] Value of a key. Need to be copyable via `=`.
///
/// [r:void*] Pointer to a pair structure or NULL if not found.
#define cbuild_map_get_or_alloc(map, key)                                      \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(__cbuild_tmp_key) == (map)->key_size ||               \
			((map)->key_size == 0 && sizeof(__cbuild_tmp_key) == sizeof(char*)),     \
			"Key type mismatch!\n");                                                 \
		cbuild_map_get_or_alloc_raw((map), &__cbuild_tmp_key);                     \
	})
/// Get element from a map or allocate a new one. Raw semi-internal function.
///
/// ::: note
/// This will not insert key into an element. You will get full pair structure
/// that is at best zero-initialized. If you are inserted value you may copy key
/// unconditionally. If you key is small you can do this too. If key is big and
/// it may take a lot of time for copy you can have some checks to verify that
/// key is valid.
/// :::
///
/// This function will allocate one stack variable that will hold copy of pointer
/// to a key. This function will check if size of this variable matches 
/// `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:const void*] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:void*] Pointer to a pair structure or NULL if not found.
#define cbuild_map_get_or_alloc_ptr(map, key)                                  \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(*__cbuild_tmp_key) == (map)->key_size ||              \
			((map)->key_size == 0 && sizeof(*__cbuild_tmp_key) == sizeof(char*)),    \
			"Key type mismatch!\n");                                                 \
		cbuild_map_get_or_alloc_raw((map), __cbuild_tmp_key);                      \
	})
/// Remove element from a map. Custom clear func will be used if provided in
/// the map. Internal data will be freed automatically. Semi-internal function.
///
/// * [pl:map] Map object.
/// * [pl:key] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:] `false` if key was not found.
CBUILDDEF bool cbuild_map_remove_raw(cbuild_map_t* map, const void* key);
/// Remove element from a map. Custom clear func will be used if provided in
/// the map. Internal data will be freed automatically.
///
/// This function will allocate one stack variable that will hold copy of key.
/// This function will check if size of this variable matches `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:any] Value of a key. Need to be copyable via `=`.
///
/// [r:void*] `false` if key was not found.
#define cbuild_map_remove(map, key)                                            \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(__cbuild_tmp_key) == (map)->key_size ||               \
			((map)->key_size == 0 && sizeof(__cbuild_tmp_key) == sizeof(char*)),     \
			"Key type mismatch!\n");                                                 \
		cbuild_map_remove_raw((map), &__cbuild_tmp_key);                           \
	})
/// Remove element from a map. Custom clear func will be used if provided in
/// the map. Internal data will be freed automatically.
///
/// This function will allocate one stack variable that will hold copy of pointer
/// to a key. This function will check if size of this variable matches 
/// `map->key_size`{.c}.
///
/// * [pl:map:cbuild_map_t*] Map object.
/// * [pl:key:const void*] Pointer to a key. Need to contain `map->key_size`{.c} bytes.
///
/// [r:void*] `false` if key was not found.
#define cbuild_map_remove_ptr(map, key)                                        \
	({                                                                           \
		__auto_type __cbuild_tmp_key = key;                                        \
		cbuild_assert(sizeof(*__cbuild_tmp_key) == (map)->key_size ||              \
			((map)->key_size == 0 && sizeof(*__cbuild_tmp_key) == sizeof(char*)),    \
			"Key type mismatch!\n");                                                 \
		cbuild_map_remove_raw((map), __cbuild_tmp_key);                            \
	})
/// Fully clear the map. Custom clear func will be used if provided in the map.
/// Internal data will be freed automatically.
CBUILDDEF void cbuild_map_clear(cbuild_map_t* map);
/// Reset map allocator.
CBUILDDEF void cbuild_map_iter_reset(cbuild_map_t* map);
/// Get pointer to a next map element from iterator. `NULL` will be returned when
/// iterator is empty.
///
/// ::: note
/// Removed or addition of elements in map without reset of iterator is UB.
/// While allocation of a new element should not break iterator, it is not
/// known if new element will be iterated over.
/// :::
CBUILDDEF void* cbuild_map_iter_next(cbuild_map_t* map);
/// Foreach loop over map. Based on iterator. This is raw semi-internal function.
///
/// * [pl:map:const cbuild_map_t*] Map object.
/// * [pl:iter:name] Name of variable that will be used in iteration. Will contain `void*`{.c} value.
#define	cbuild_map_foreach_raw(map, iter)                                      \
	cbuild_map_iter_reset(map);                                                  \
	for (void* iter = NULL; ((iter) = cbuild_map_iter_next(map));)
/// Foreach loop over map. Based on iterator.
///
/// * [pl:map:const cbuild_map_t*] Map object.
/// * [pl:T:type] Type of iterator variable.
/// * [pl:iter:name] Name of variable that will be used in iteration. Will contain `T`{.c} value.
#define cbuild_map_foreach(map, T, iter)                                       \
	cbuild_map_iter_reset(map);                                                  \
	for (T* iter = NULL;((iter) = (T*)cbuild_map_iter_next(map));)

//! # Internal functions [line:cbuild-map-internal]

//@ cbuild-map-internal

/// Default map hash function. Use dbj2 hash.
CBUILDDEF size_t __cbuild_map_hash_func(const void* data, size_t len);

/* Proc.h */

// #pragma once // For LSP 
//! Abstractions for process control.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

/// Dynamic array of processes.
typedef struct cbuild_proclist_t {
	cbuild_proc_t* data;
	size_t size;
	size_t capacity;
} cbuild_proclist_t;
/// Append new element to list of processes.
///
/// * [pl:proclist:cbuild_proclist_t*] Dynamic array object.
/// * [pl:proc:cbuild_proc_t] Process handle.
#define cbuild_proclist_append(proclist, proc) cbuild_da_append(proclist, proc)
/// Clears list of processes.
///
/// * [pl:proclist:cbuild_proclist_t*] Dynamic array object.
#define cbuild_proclist_clear(proclist) cbuild_da_clear(proclist)
/// Set element in process list using its index.
///
/// * [pl:proclist:cbuild_proclist_t*] List of processes.
/// * [pl:idx:size_t] Element index.
/// * [pl:proc:cbuild_proc_t] New element's value.
/// 
/// [r:bool] False on overflow.
#define cbuild_proclist_set(proclist, idx, proc) cbuild_da_set(proclist, idx, proc)
/// Get element from process list using its index.
///
/// * [pl:proclist:cbuild_proclist_t*] List of processes.
/// * [pl:idx:size_t] Slement index.
/// 
/// [r:cbuild_proc_t*] Pointer to an element or `NULL`{.c} on overflow.
#define cbuild_proclist_get(proclist, idx) cbuild_da_get(proclist, idx)
/// Blocking wait until specific process finishes.
///
/// [r:] `false`{.c} if process returned with an error.
CBUILDDEF bool cbuild_proc_wait(cbuild_proc_t proc);
/// Blocking wait until all processes from list exits.
///
/// [r:] `false`{.c} if at least one process returned with an error.
CBUILDDEF bool cbuild_procs_wait(cbuild_proclist_t procs);
/// Blocking wait until specific process finishes.
///
/// [r:] Process exit code or `INT_MIN`{.c} if [p:proc] is invalid or `INT_MAX`{.c} on ECHILD or negative signal value if process exited due to a signal.
CBUILDDEF int cbuild_proc_wait_code(cbuild_proc_t proc);
/// Blocking wait until any process from list exits.
///
/// * [pl:procs] List of processess.
/// * [pl:code] Return value for exit code of exited process, Could be `NULL`{.c}.
///
/// [r:] Index of exited process in [p:procs] array.
CBUILDDEF ssize_t cbuild_procs_wait_any(cbuild_proclist_t procs, int* code);
/// Check if specific process is still running.
CBUILDDEF bool cbuild_proc_is_running(cbuild_proc_t proc);
/// Function that can be used a child process entry point.
///
/// [pl:args] Arguments passed from parent process.
///
/// [r:] Exit code of a process.
typedef int (*cbuild_proc_func_t)(void* args);
/// Start a new process.
///
/// * [pl:func] Function that will be used as process' entry point. Function should return exit code.
/// * [pl:args] Arguments that will be passed to that function
///
/// [r:] Process handle of new process.
CBUILDDEF cbuild_proc_t cbuild_proc_start(cbuild_proc_func_t func, void* args);
/// Pointer wrapper for shared memory.
///
/// * [fl:ptr] Pointer to a memory.
/// * [fl:size] Length of memory region in bytes.
typedef struct cbuild_proc_ptr_t {
	void*  ptr;
	size_t size;
} cbuild_proc_ptr_t;
/// Allocate memory that will be shared between child processes. Same as `malloc`.
CBUILDDEF cbuild_proc_ptr_t cbuild_proc_malloc(size_t n);
/// Free memory that was allocated via [`cbuild_proc_malloc`](DOC:cbuild_proc_malloc).
/// Same as`free`.
CBUILDDEF void cbuild_proc_free(cbuild_proc_ptr_t ptr);
/// Get amount of CPU cores (or threads).
CBUILDDEF int cbuild_nproc(void);

/* Command.h */

// #pragma once // For LSP 
//! Command runner
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"
// #include "StringBuilder.h"
// #include "Proc.h"

/// Command structure - dynamic array of arguments.
///
/// ::: note
/// [f:data] has same format as `arg` in `execv` except NULL termination.
/// :::
typedef struct cbuild_cmd_t {
	const char** data;
	size_t size;
	size_t capacity;
} cbuild_cmd_t;
/// Append new argument to a command.
///
/// * [pl:cmd:cbuild_cmd_t*] Command object.
/// * [pl:val:const char*] Element to append.
#define	cbuild_cmd_append(cmd, val) cbuild_da_append(cmd, val)
/// Append new arguments to a command.
///
/// * [pl:cmd:cbuild_cmd_t*] Command object.
/// * [pl:vals:const char**] Elements to append.
/// * [pl:vals_cnt:size_t] Number of elements to append.
#define	cbuild_cmd_append_arr(cmd, vals, vals_cnt)                             \
	cbuild_da_append_arr(cmd, vals, vals_cnt)
/// Append new arguments to a command.
///
/// * [pl:cmd:cbuild_cmd_t*] Command object.
/// * [pl:...:const char*] Elements to append.
#define	cbuild_cmd_append_many(cmd, ...)                                       \
	cbuild_da_append_many(cmd, __VA_ARGS__)
/// Free command.
///
/// This should be used only if you want to free underlying memory, if you just
/// want to clear array and reuse it, setting `size` to `0` would be much faster.
///
/// * [pl:cmd:cbuild_cmd_t*] Dynamic array object.
#define	cbuild_cmd_clear(cmd) cbuild_da_clear(cmd)
/// Convert command to string builder.
///
/// ::: note
/// This function properly formats arguments with spaces by wrapping them in `'`.
/// But arguments that has both spaces and `'` are not handler properly.
/// :::
CBUILDDEF cbuild_sb_t cbuild_cmd_to_sb(cbuild_cmd_t cmd);
/// Convert command to string builder.
///
/// ::: note
/// This function properly formats arguments with spaces by wrapping them in `'`.
/// But arguments that has both spaces and `'` are not handler properly.
/// :::
#define	cbuild_sb_from_cmd(cmd) cbuild_cmd_to_sb(cmd)
/// Configuration for command.
///
/// This structure consists of a few separate blocks:
///
/// * Async - first two values are mutually exclusive:
///   - [fl:procs] Process list used for async execution. Non-`NULL`{.c} value enables async.
///   - [fl:proc] Process return value used for async execution. Non-`NULL`{.c} value enables async.
///   - [fl:async_threads] Control how many threads async runner can spawn (works in conjunction with [p:procs]). `0` means implementation-defined. `-1` means unlimited.
/// * IO redirection:
///   - [fl:fdstdin] Redirect `stdin` to some file descriptor.
///   - [fl:file_stdin] Redirect `stdin` to some file.
///   - [fl:fdstdout] Redirect `stdout` to some file descriptor.
///   - [fl:file_stdout] Redirect `stdout` to some file.
///   - [fl:fdstderr] Redirect `stderr` to some file descriptor.
///   - [fl:file_stderr] Redirect `stderr` to some file.
/// * Miscellaneous lags:
///   - [fl:no_reset] By default `size` filed of command is reset. This flag disables this feature.
///   - [fl:autokill] Works only on Linux. Automatically kills process if parent dies.
///   - [fl:no_print_cmd] By default command is printed as `TRACE` log. This flag disable this log message.
struct cbuild_cmd_opts_t {
	// Async
	cbuild_proclist_t* procs; // Non-null implies async
	cbuild_proc_t* proc;      // Non-null implies async
	// Redirect 'fdstd* ' and 'file_std*' is mutually exclusive.
	const cbuild_fd_t* fdstdin;
	const char* file_stdin;
	const cbuild_fd_t* fdstdout;
	const char* file_stdout;
	const cbuild_fd_t* fdstderr;
	const char* file_stderr;
	// Async
	int async_threads; // 0 means implementation-defined. -1 means unlimited. Expects 'procs' to be a valid empty array.
	// Flags
	union {
		uint32_t flags;
		struct {
			uint32_t no_reset     : 1;
			uint32_t autokill     : 1;
			uint32_t no_print_cmd : 1;
			uint32_t              : 29;
		};
	};
};
/// Run command. This function is semi-internal.
CBUILDDEF bool cbuild_cmd_run_opt(cbuild_cmd_t* cmd, struct cbuild_cmd_opts_t opts);
/// Run command.
///
/// * [pl:cmd:cbuild_cmd_t*] Command to execute.
/// * [pl:...:...cbuild_cmd_opts_t] Fields of configuration structure in initializer-list form.
#define cbuild_cmd_run(cmd, ...)                                               \
cbuild_cmd_run_opt(cmd, (struct cbuild_cmd_opts_t){ __VA_ARGS__ })

/* FS.h */

// #pragma once // For LSP 
//! File system access abstraction layer.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"
// #include "StringBuilder.h"

/// Dynamic array of paths
typedef struct cbuild_pathlist_t {
	char** data;
	size_t size;
	size_t capacity;
} cbuild_pathlist_t;
/// Get element from list of paths using its index.
///
/// * [pl:list:const cbuild_pathlist_t*] Path list object.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:char**] Pointer to an element or `NULL`{.c} if index out of bounds.
#define cbuild_pathlist_get(list, idx) cbuild_da_get(list, idx)
/// Free pathlist.
/// 
/// ::: note
/// This function should *really* be used instead of simple `free(list->data)`{.c}
/// because pathlist allocated memory in two levels.
/// :::
///
/// * [pl:list:const cbuild_pathlist_t*] Path list object.
CBUILDDEF void cbuild_pathlist_clear(cbuild_pathlist_t* list);
/// Type of file. Mostly cross-platform because no special Unix files are included.
typedef enum {
	CBUILD_FTYPE_MISSING   = -1,
	CBUILD_FTYPE_REGULAR   = 0,
	CBUILD_FTYPE_DIRECTORY = 1,
	CBUILD_FTYPE_SYMLINK   = 2,
	CBUILD_FTYPE_OTHER     = 3,
} cbuild_filetype_t;
/// Close file descriptor.
CBUILDDEF bool cbuild_fd_close(cbuild_fd_t fd);
/// Open file for reading.
CBUILDDEF cbuild_fd_t cbuild_fd_open_read(const char* path);
/// Open file for writing.
CBUILDDEF cbuild_fd_t cbuild_fd_open_write(const char* path);
/// Create and open new pipe. File descriptors for both ends are provided.
CBUILDDEF bool cbuild_fd_open_pipe(cbuild_fd_t* read, cbuild_fd_t* write);
/// POSIX `read`{.c} wrapper.
///
/// * [pl:fd] File descriptor.
/// * [pl:buf] Buffer in which data will be read.
/// * [pl:nbytes] Number of bytes to read.
///
/// [r:] Number of bytes read or `-1`{.c} on error.
CBUILDDEF ssize_t cbuild_fd_read(cbuild_fd_t fd, void* buf, size_t nbytes);
/// POSIX `read`{.c} "wrapper".
/// This function uses [p:path] only for error-reporting.
///
/// * [pl:fd] File descriptor.
/// * [pl:buf] Buffer in which data will be read.
/// * [pl:nbytes] Number of bytes to read.
/// * [pl:path] File name used in error reporting.
///
/// [r:] Number of bytes read or `-1`{.c} on error.
CBUILDDEF ssize_t cbuild_fd_read_file(cbuild_fd_t fd, void* buf, size_t nbytes,
	const char* path);
/// POSIX `write`{.c} wrapper.
///
/// * [pl:fd] File descriptor.
/// * [pl:buf] Buffer from which data will be written.
/// * [pl:nbytes] Number of bytes to write.
///
/// [r:] Number of bytes written or `-1`{.c} on error.
CBUILDDEF ssize_t cbuild_fd_write(cbuild_fd_t fd, const void* buf, size_t nbytes);
/// POSIX `write`{.c} "wrapper".
/// This function uses [p:path] only for error-reporting.
///
/// * [pl:fd] File descriptor.
/// * [pl:buf] Buffer from which data will be written.
/// * [pl:nbytes] Number of bytes to write.
/// * [pl:path] File name used in error reporting.
///
/// [r:] Number of bytes written or `-1`{.c} on error.
CBUILDDEF ssize_t cbuild_fd_write_file(cbuild_fd_t fd, const void* buf,
	size_t nbytes, const char* path);
/// Get length of file in bytes.
///
/// [r:] Length of file in bytes or `-1` on error.
CBUILDDEF ssize_t cbuild_file_len(const char* path);
/// Read file into string builder.
CBUILDDEF bool cbuild_file_read(const char* path, cbuild_sb_t* data);
/// Overwrite file with string builder content.
CBUILDDEF bool cbuild_file_write(const char* path, cbuild_sb_t* data);
/// Copy file.
/// 
/// ::: note
/// This copies file by reading from [p:src] and writing to [p:dst].
/// "Chunked" copy is used to limit memory usage.
/// :::
CBUILDDEF bool cbuild_file_copy(const char* src, const char* dst);
/// Move file.
///
/// Uses optimal method for moving file, but in worst case falls back
/// to `cbuild_file_copy(src, dst); cbuild_file_remove(src)`{.c}.
CBUILDDEF bool cbuild_file_move(const char* src, const char* dst);
/// Rename file. Alias to [`cbuild_file_move`](DOC:cbuild_file_move).
///
/// Uses optimal method for moving file, but in worst case falls back
/// to `cbuild_file_copy(src, dst); cbuild_file_remove(src)`{.c}.
///
/// * [pl:src:const char*]
/// * [pl:dst:const char*]
///
/// [r:bool]
#define cbuild_file_rename(src, dst) cbuild_file_move(src, dst)
/// Check if file exists.
CBUILDDEF bool cbuild_file_check(const char* path);
/// Delete file.
CBUILDDEF bool cbuild_file_remove(const char* path);
/// Create symbolic link. Will overwrite [p:dst] if it exists.
CBUILDDEF bool cbuild_symlink(const char* src, const char* dst);
/// Recursively copy a directory.
CBUILDDEF bool cbuild_dir_copy(const char* src, const char* dst);
/// Move a directory.
///
/// Uses optimal method for moving directory, but in worst case falls back
/// to `cbuild_dir_copy(src, dst); cbuild_dir_remove(src)`{.c}.
CBUILDDEF bool cbuild_dir_move(const char* src, const char* dst);
/// Rename a directory. Alias to [`cbuild_dir_move`](DOC:cbuild_dir_move).
///
/// Uses optimal method for moving directory, but in worst case falls back
/// to `cbuild_dir_copy(src, dst); cbuild_dir_remove(src)`{.c}.
///
/// * [pl:src:const char*]
/// * [pl:dst:const char*]
///
/// [r:bool]
#define cbuild_dir_rename(src, dst) cbuild_dir_move(src, dst)
/// Delete directory.
CBUILDDEF bool cbuild_dir_remove(const char* path);
/// Check if directory exists.
CBUILDDEF bool cbuild_dir_check(const char* path);
/// List all files in directory.
///
/// * [pl:path] Path to a directory.
/// * [pl:elements] List of elements in directory.
CBUILDDEF bool cbuild_dir_list(const char* path, cbuild_pathlist_t* elements);
/// Get current working directory.
///
/// [r:] Current working directory values. Allocated `__CBUILD_MALLOC`.
CBUILDDEF char* cbuild_dir_current(void);
/// Set current working directory.
CBUILDDEF bool cbuild_dir_set_current(const char* path);
/// Create new directory.
CBUILDDEF bool cbuild_dir_create(const char* path);
/// Result for [`cbuild_dir_walk_func_t`](DOC:cbuild_dir_walk_func_t).
///
/// * [fl:CBUILD_DIR_WALK_CONTINUE] Default value. Just continue.
/// * [fl:CBUILD_DIR_WALK_NO_ENTER] Do not enter into a directory.
/// * [fl:CBUILD_DIR_WALK_STOP] Stop entirely (but does not return an error).
enum cbuild_dir_walk_result_t {
	CBUILD_DIR_WALK_CONTINUE = 0,
	CBUILD_DIR_WALK_NO_ENTER,
	CBUILD_DIR_WALK_STOP = -1,
};
/// Arguments for [`cbuild_dir_walk_func_t`](DOC:cbuild_dir_walk_func_t).
///
/// This structure can be divided into multiple sections:
///
/// * Current entry:
///   - [fl:path] Path to the file system object object (relative and including directory passed to [`cbuild_dir_walk`](DOC:cbuild_dir_walk).
///   - [fl:type] Type of current file-system object. 
/// * Iteration state:
///   - [fl:level] How deep iteration goes (`0` means base directory).
/// * Context:
///   - [fl:context] Some arguments passed by user. 
///   - [fl:result] Return value.
typedef struct cbuild_dir_walk_func_args_t {
	const char* path;
	cbuild_filetype_t type;
	size_t level;
	enum cbuild_dir_walk_result_t* result;
	void* context;
} cbuild_dir_walk_func_args_t;
/// Function used as callback by directory walker.
///
/// [r:] `false` means error.
typedef bool (*cbuild_dir_walk_func_t)(cbuild_dir_walk_func_args_t args);
/// Arguments to a directory walker.
///
/// * [fl:context] Some arguments that will be passed to callback.
/// * [fl:visit_dir_last] Inverts order. Now all child files are visited first and parent directory is visited last.
struct cbuild_dir_walk_opts_t {
	void* context;
	bool visit_dir_last;
};
/// Walk over dir. Takes path, callback and some optional options. Semi-internal function.
CBUILDDEF bool cbuild_dir_walk_opt(const char* path, cbuild_dir_walk_func_t func,
	struct cbuild_dir_walk_opts_t opts);
/// Walk over dir.
///
/// * [pl:path:const char*] Path to a file.
/// * [pl:callback:cbuild_dir_walk_func_t] Function that will be called for each file system object.
/// * [pl:...:...cbuild_dir_walk_opts_t] Fields of configuration structure in initializer-list form.
#define cbuild_dir_walk(path, callback, ...)                                        \
cbuild_dir_walk_opt(path, callback, (struct cbuild_dir_walk_opts_t){ __VA_ARGS__ })
/// Open directory.
CBUILDDEF cbuild_dir_t cbuild_dir_open(const char* path);
/// Get next element from directory.
/// 
/// * [pl:dir] Directory handle.
/// * [pl:element] Return value for filename.
///
/// * [r:0] No error and directory iterator is still not empty.
/// * [r:1] No error but directory iterator is empty. [p:element] is `NULL`.
/// * [r:-1] Error happened.
CBUILDDEF int cbuild_dir_next(cbuild_dir_t dir, const char** element);
/// Close directory.
CBUILDDEF bool cbuild_dir_close(cbuild_dir_t dir);
/// Get type of file.
CBUILDDEF cbuild_filetype_t cbuild_path_filetype(const char* path);
/// Optional arguments for [`cbuild_path_ext`](DOC:cbuild_path_ext).
///
/// * [fl:dot] Get extensions after n-th dot from the end. `0` is default value
/// (for now it is same as `1`).
struct cbuild_path_ext_opts_t {
	int dot; // Get n-th dot from the end, 0 is last one (same as 1)
};
/// Get extension of specific file. Semi-internal.
/// File pointer by [p:path] is not checked in any way.
///
/// [r:] String allocated via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
CBUILDDEF char* cbuild_path_ext_opt(const char* path, struct cbuild_path_ext_opts_t opts);
/// Get extension of specific file.
/// File pointer by [p:path:const char*] is not checked in any way.
///
/// * [pl:path:const char*] Path to a file.
/// * [pl:...:...cbuild_path_ext_opts_t] Fields of configuration structure in initializer-list form.
///
/// [r:] String allocated via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
#define cbuild_path_ext(path, ...)                                    \
cbuild_path_ext_opt(path, (struct cbuild_path_ext_opts_t){ __VA_ARGS__ })
/// Get file name with extension (all after last `/`).
/// For directories special case exists that strips last `/` if it is last character.
/// File pointer by [p:path] is not checked in any way.
/// 
/// [r:] String allocated via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
CBUILDDEF char* cbuild_path_name(const char* path);
/// Get base name (all before last `/`).
/// For directories special case exists that strips last `/` if it is last character.
/// File pointer by [p:path] is not checked in any way.
///
/// [r:] String allocated via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
CBUILDDEF char* cbuild_path_base(const char* path);
/// Normalize path. All normalization is based on heuristics and does not involve
/// such steps as eg. symlink resolving.
/// File pointer by [p:path] is not checked in any way.
///
/// [r:] String allocated via [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
CBUILDDEF char* cbuild_path_normalize(const char* path);

/* Compile.h */

// #pragma once // For LSP
//! Some helpers specifically for a compilation
//!
//! License: `GPL-3.0-or-later`.
//!
//! # `CBUILD_CC` [line:cbuild-compiler-self]
//!
//! This will hold compiler command for compiler that was used to compiler CBuild.
//!
//! # `CC` [line:cbuild-bare-cc]
//!
//! CBuild can provide bare define `CC`. Because it is not prefixed it can be disabled.
//! This defined will be an alias to `CBUILD_CC`. If `CBUILD_NO_CC` is defined, CBuild
//! will not provide `CC` define.

// #include "Common.h"
// #include "Command.h"

//@ cbuild-compiler-self
#if !defined(CBUILD_CC)
	#if defined(CBUILD_CC_CLANG)
		#define CBUILD_CC "clang"
	#elif defined(CBUILD_CC_GCC)
		#define CBUILD_CC "gcc"
	#else
		#define CBUILD_CC "cc"
	#endif // Compiler select
#endif // CBUILD_CC

//@ cbuild-bare-cc
#if !defined(CBUILD_NO_CC)
	#define CC CBUILD_CC
#endif

/// Default warning options.
/// Enables almost all warnings except `-Wcomments` and `-Woverride-init`.
#define CBUILD_CARGS_WARN \
	"-Wall", "-Wextra", "-Wno-comments", "-Wconversion", "-Wcast-align", "-Wvla", \
	"-Wno-override-init", "-Wshadow"
/// Promote all warnings to errors.
#define CBUILD_CARGS_WERROR           "-Werror"
/// Setup profiles,
#define CBUILD_CARGS_PROFILE          "-pg"
/// Add debug symbols,
#define CBUILD_CARGS_DEBUG            "-g"
/// Add debug symbols targeting **gdb**.
#define CBUILD_CARGS_DEBUG_GDB        "-ggdb"
/// Enable multithreading (eg. add `-pthread` for POSIX).
#define CBUILD_CARGS_MT               "-pthread"
/// Add a define based on compile-time literal.
#define CBUILD_CARGS_DEFINE(defname)  "-D" defname
/// Add a define based on runtime string.
/// Uses [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
#define CBUILD_CARGS_DEFINE_TEMP(defname) \
	cbuild_temp_sprintf("-D%s", defname)
/// Add a define with a value. Both based on compile-time literal.
#define CBUILD_CARGS_DEFINE_VAL(defname, val)      "-D" defname "=" val
/// Add a define with a value. Both based on runtime string.
/// Uses [`cbuild_temp_malloc`](DOC:cbuild_temp_malloc).
#define CBUILD_CARGS_DEFINE_VAL_TEMP(defname, val) \
	cbuild_temp_sprintf("-D%s=%s", defname, val)
/// Remove define based on compile-time literal.
#define CBUILD_CARGS_UNDEF(defname)   "-U"defname
/// Add include based on compile-time literal.
#define CBUILD_CARGS_INCLUDE(file)    "--include", file
/// Add library include (`-l`) based on compile-time literal.
#define CBUILD_CARGS_LIBINCLUDE(lib)  "-l"lib
/// Add library directories (`-I` and `-L`) based on compile-time literal.
#define CBUILD_CARGS_LIBDIR(src, obj) "-I"src, "-L"obj
/// Set standard based on compile-time literal.
#define CBUILD_CARGS_STD(std)         "-std="std

/// Simple wrapper for self-rebuild. Only `argv[0]`{.c} is used.
/// Other elements are used only to re-exec new binary with same arguments.
#define cbuild_selfrebuild(argc, argv)                                         \
	do {                                                                         \
		const char* expanded_file = __FILE__;                                      \
		cbuild_cmd_t files = { .data = &expanded_file, .size = 1 };                \
		__cbuild_selfrebuild(argc, argv, argv[0], files);                          \
	} while(0)
/// Complex wrapper for self-rebuild. Only `argv[0]`{.c} is used.
/// This function allow to specify additional files to be checked when
/// decision is made to rebuild binary or not.
#define cbuild_selfrebuild_ex(argc, argv, ...)                                 \
	do {                                                                         \
		const char* tmp_files[] = {__FILE__ __VA_OPT__(,) __VA_ARGS__ };           \
		size_t tmp_files_len = cbuild_arr_len(tmp_files);                          \
		cbuild_cmd_t files = { .data = tmp_files, .size = tmp_files_len };         \
		__cbuild_selfrebuild(argc, argv, argv[0], files);                          \
	} while(0)
/// Perms self-rebuild.
/// 
/// * [pl:argc] ARGC of program.
/// * [pl:argv] ARGV of program.
/// * [pl:argv0_path] Path to file specified by `argv[0]`{.c}.
/// * [pl:files] Primary file as first element and additional dependency files as other.
///
/// # When this will result in a new binary
///
/// If any of the files from [p:files] are newer than `argv[0]`{.c}.
/// If any of files in this array does not exist program will exit with error.
///
/// # How rebuild is performed
///
/// `CBUILD_CC` is used as compiler. `CBUILD_CARGS_WARN` is used as default set
/// of arguments. [`cbuild_selfrebuild_hook`](DOC:cbuild_selfrebuild_hook) is called
/// to resolve dynamic or user-provided arguments.
CBUILDDEF void __cbuild_selfrebuild(int argc, char** argv,
	const char* argv0_path, cbuild_cmd_t files);
/// This function receives command before files are appended and can modify it.
typedef void (*cbuild_selfrebuild_hook_t)(cbuild_cmd_t* cmd);
/// This function is called by default [`__cbuild_selfrebuild`](DOC:__cbuild_selfrebuild)
extern cbuild_selfrebuild_hook_t cbuild_selfrebuild_hook;
/// Compare mtime of 2 files.
/// Same check as done by `make`.
///
/// * [r:=0] - Output is newer than input.
/// * [r:<0] - Error.
/// * [r:>0] - Output is older than input.
CBUILDDEF int cbuild_compare_mtime(const char* output, const char* input);
/// Compare mtime of 1 output file and multiple input files.
/// Same check as done by `make`.
///
/// * [r:=0] - Output is newer than input.
/// * [r:<0] - Error.
/// * [r:>0] - Output is older than input. Number of files that are newer.
CBUILDDEF int cbuild_compare_mtime_many(const char* output, const char** inputs,
	size_t num_inputs);

/* DLLoad.h */

// #pragma once // For LSP
//! Abstract runtime loading of dynamic libraries.
//!
//! License: `GPL-3.0-or-later`.

// #include "Common.h"

/// Load dynamic library into handle.
///
/// * [pl:filename] Full path to a library or its platform-dependant "ID".
/// * [pl:eager] By default symbols are loaded lazily (during calls to [`cbuild_dlib_get_sym`](DOC:cbuild_dlib_get_sym). This makes them load eagerly. Can be user to run global constructors.
///
/// [r:] `NULL` if error happened.
CBUILDDEF cbuild_dlib_t cbuild_dlib_open(const char* filename, bool eager);
/// Get symbols from library handle.
///
/// [r:] `NULL` if error happened.
CBUILDDEF void* cbuild_dlib_get_sym(cbuild_dlib_t handle, const char* name);
/// Close dynamic library handle and unload library.
CBUILDDEF void cbuild_dlib_close(cbuild_dlib_t handle);

/* FlagParse.h */

// #pragma once // For LSP
//! Parser for command-line flags with GNU-style format.
//!
//! License: `GPL-3.0-or-later`.
//!
//! # Features
//!
//! - Support for long and short options.
//! - Support for adding `--` to list of long options (for custom parsing
//!   semantics).
//! - It support *terminated* arguments - argument list ends not only when size
//!   is reached (or when argument starts from `-`) but only when terminator
//!   reached.
//! - It support optional arguments.
//! - Flag grouping (to pretty-print them for help).
//! - Help formatter.
//! - Custom help and version hooks (parsing for that flags are built-in).
//!
//! # Limitations
//! 
//! - Short options can be only 1 character.
//! - Arguments are always returned as strings.
//!
//! ::: note
//! Some of these functions takes pointers. They expect this pointers either to
//! be statically allocated or be dynamically allocated and then leaked. This
//! memory will live trough full app lifetime, so this leakage has no problems.
//! But if you use valgrind you should probably not use `reachable` in 
//! `--errors-for-leak-kinds` (because flag context live as global variable).
//! :::
//!
//! # Flags format
//!
//! This parses uses something that roughly mimics "GNU" style of flags. This
//! means that `--` precedes long versions of flags and `-` short versions.
//! Also, should versions can be grouped together and each character will then
//! be parsed as a separate flag. `=` is supported for both long and short flag
//! versions so things like `--flag=argument` or `-f=argument` can be used.
//! Even things like `-abc=argument` will be correctly parsed to 3 flags - `a`,
//! `b` and `c`, with last one having 1 argument - `argument`. Support for `--`
//! is implemented and it stops searching for flags. Positional arguments are
//! supported too.

// #include "Common.h"

/// Options for a new flag
///
/// * [fl:short_option] Character for short options. If `'\0'` (`0`) then disabled.
/// * [fl:optional] Arguments to this flag are optional. This augments `num_arguments` field by allowing another option of passing no arguments (in addition to one provided by `num_arguments`).
/// * [fl:repeat] If 'false' repeated flag invocation result in error. If 'true' each invocation of flag just append argument to shared list (if flag has argument limit it is treated as a per-invocation limit).
/// * [fl:num_arguments] Number of arguments required. `-1` means `>0`.
/// * [fl:group] Name of arguments group. Used only for help message. Can be `NULL`{.c}.
/// * [fl:terminator] Terminator argument. Can be `NULL`{.c} (unset, non-terminated argument list).
/// * [fl:desc] Description. Can be `NULL`{.c}, then nothing will be printed.
/// * [fl:argument_desc] Description for argumentt. Can be `NULL`{.c}, then default 'ARGUMENT' will be used. Used only if `num_arguments != 0`{.c}.
struct cbuild_flag_new_opts_t {
	char short_option;
	union {
		uint8_t __flags;
		struct {
			uint8_t optional : 1;
			uint8_t repeat : 1;
			uint8_t : 6;
		};
	};
	int8_t num_arguments; // No one need more than 127 arguments (at least then fixed count becomes meaningless).
	// uint8_t __padding[1];
	const char* group;
	const char* terminator;
	const char* desc;
	const char* argument_desc;
};
/// Register new flag. Semi-internal function.
CBUILDDEF void cbuild_flag_new_opt(const char* option, struct cbuild_flag_new_opts_t opts);
//// Register a new flag.
///
/// * [pl:option:const char*] Long flag name. Will be used as its internal "ID'.
/// * [pl:...:...cbuild_flag_new_opts_t] Fields of configuration structure in initializer-list form.
#define cbuild_flag_new(option, ...)                                        \
cbuild_flag_new_opt(option, (struct cbuild_flag_new_opts_t){ __VA_ARGS__ })
/// Options for a flag parser
///
/// * [fl:CBUILD_FLAG_PASS_SEPARATOR] Pass `--` inside of pargs. No arguments.
/// * [fl:CBUILD_FLAG_HELP_HOOK] Set hook for `--help`. Argument - [`cbuild_flag_print_func_t`](DOC:cbuild_flag_print_func_t)
/// * [fl:CBUILD_FLAG_VERSION_HOOK] Set hook for `--version`. Argument - [`cbuild_flag_print_func_t`](DOC:cbuild_flag_print_func_t)
enum cbuild_flag_options_t {
	CBUILD_FLAG_PASS_SEPARATOR,
	CBUILD_FLAG_HELP_HOOK,
	CBUILD_FLAG_VERSION_HOOK,
};
/// Function used for print-type hooks. Takes `argv[0]`{.c} as input.
typedef void (*cbuild_flag_print_func_t)(const char* app_name);
/// Set some option for a flag parser.
///
/// ::: note
/// Options can take some flags, they are passed via variadic arguments.
/// :::
CBUILDDEF void cbuild_flag_set_option(enum cbuild_flag_options_t option, ...);
/// List of arguments
typedef struct cbuild_arglist_t {
	char** data;
	size_t size;
	size_t capacity;
} cbuild_arglist_t;
/// Get element from argument list using its index.
///
/// * [pl:arglist:const cbuild_arglist_t*] Argument list.
/// * [pl:idx:size_t] Element index.
/// 
/// [r:char**] Pointer to an element or `NULL`{.c} on overflow.
#define cbuild_arglist_get(arglist, idx) cbuild_da_get(arglist, idx)
/// Parse flags.
///
/// ::: note
/// This function prints error and aborts on invalid flags and when parsed 
/// argument count differs from required (uses `exit(1)`).
/// :::
CBUILDDEF void cbuild_flag_parse(int argc, char** argv);
/// Print help for all flags via [`__CBUILD_PRINT`](Common.html#preprocessor-configuration)
CBUILDDEF void cbuild_flag_print_help(void);
/// Get list of positional arguments.
///
/// [r:] List of positional arguments (pointer to a global state).
CBUILDDEF cbuild_arglist_t* cbuild_flag_get_pargs(void);
/// Get arguments for a specific flag.
///
/// [r:] List of arguments for that flag (pointer to a global state) or NULL if flag was not provided.
CBUILDDEF cbuild_arglist_t* cbuild_flag_get_flag(const char* opt);
/// Get current app name (`argv[0]`{.c}).
CBUILDDEF const char* cbuild_flag_app_name(void);

/* RGlob.h */

// #pragma once // For LSP
//! Glob syntax implementation over POSIX ERE regex.
//!
//! License: `GPL-3.0-or-later`.
//!
//! # Preprocessor
//!
//! This glob is implemented as a preprocessor and a series of wrappers around
//! POSIX regex API. This preprocessor translate glob patter to valid and equal
//! regex pattern. This includes both escaping regex's special characters and
//! translating glob's special characters to equal sequences of regexes.
//!
//! # Pattern format
//!
//! In general any valid glob patter can be used, it support this "operators":
//!
//! * `?` - match any single character. (Translates to `.`).
//! * `*` - match any sequence of characters. (Translates to `.*`).
//! * `[...]` - matches any character from character class. (Translates to
//!   itself literally).
//! * `!` as first element of character class to invert character class.
//!   (Translates to '^').
//!
//! It also supports some syntax extensions:
//!
//! * Often you want to match some specific character multiple characters. In
//!   regex you can just write `[...]*`. This glob engine support same syntax. If
//! `]*` sequence is found it will not be translated in any way.
//! * Capture groups are exposed in an API but have no special pattern syntax.
//! * POSIX ERE support `\` as escape character. Glob also support is so no
//!   preprocessing is needed here. But this preprocessor supports its own escape
//!   character too - `#`. Next character after it will be literally copied into
//!   output regex pattern.
//!
//! These characters are escaped with `\` by default in generated regex:
//!
//! * `(`.
//! * `)`.
//! * `{`.
//! * `}`.
//! * `^`.
//! * `$`.
//! * `.`.
//! * `|`.
//! * `+`.
//!
//! ::: note
//! By default pattern is wrapped in `^...$` to match full string.
//! :::

// #include "Common.h"

/// Glob context.
///
/// * [fl:data] Data for dynamic array of matches.
///   - [fl:data.res] Match text. Points to original data.
///   - [fl:data.res_idx] Match index.
///   - [fl:data.captures] Copy of capture groups for that match.
/// * [fl:capacity] Capacity for dynamic array of matches.
/// * [fl:size] Count of elements for dynamic array of matches.
/// * [fl:regex] Cache for compiled regex.
/// * [fl:captures] Scratchpad captures for regex engine.
typedef struct cbuild_glob_t {
	struct __cbuild_glob_res_t {
		const char* res;
		size_t res_idx;
		regmatch_t captures[CBUILD_GLOB_CAPTURE_COUNT];
	} *data;
	size_t size;
	size_t capacity;
	regex_t regex;
	regmatch_t captures[CBUILD_GLOB_CAPTURE_COUNT];
} cbuild_glob_t;
/// Options for pattern compilation
///
/// * [fl:partial_match] Do not wrap pattern in `^...$`.
struct cbuild_glob_opts_t {
	bool partial_match;
};
/// Compile new glob patter.
///
/// This is a two-step process, first it will transform glob expression
/// into a regx one and then it will call `regcomp`. POSIX ERE will be used.
CBUILDDEF bool cbuild_glob_compile_opt(cbuild_glob_t* glob, const char* pattern,
	struct cbuild_glob_opts_t opts);
/// Compile new glob patter.
///
/// This is a two-step process, first it will transform glob expression
/// into a regx one and then it will call `regcomp`. POSIX ERE will be used.
///
/// * [pl:glob:cbuild_glob_t*] Glob object.
/// * [pl:pattern:const char*] Pattern to compile.
/// * [pl:...:...cbulid_glob_opts_t] Fields of configuration structure in initializer-list form.
#define cbuild_glob_compile(glob, pattern, ...)                                   \
cbuild_glob_compile_opt(glob, pattern, (struct cbuild_glob_opts_t){ __VA_ARGS__ })
/// Find matches in a list.
///
/// [r:] `true` if at least one match was found. `false` if no matches were found.
CBUILDDEF bool cbuild_glob_match(cbuild_glob_t* glob, const char** list, size_t size);
/// Match single element with a glob.
///
/// This function resets glob matches but does not set them. You should check
/// return value to know if glob matched. To get capture groups values you can
/// use outer `captures` array. 
///
/// [r:] `true` if at least one match was found. `false` if no matches were found.
CBUILDDEF bool cbuild_glob_match_single(cbuild_glob_t* glob, const char* elem);
/// Clean underlying regex and free matches array.
CBUILDDEF void cbuild_glob_free(cbuild_glob_t* glob);
#endif // __CBUILD_H__
#ifdef CBUILD_IMPLEMENTATION

/* Common.h */

//! License: `GPL-3.0-or-later`.

// #include "Common.h"
#if defined(CBUILD_API_STRICT_POSIX) || (defined(CBUILD_API_POSIX) && ( \
			(defined(CBUILD_OS_LINUX) && \
				!(defined(CBUILD_OS_LINUX_GLIBC) || defined(CBUILD_OS_LINUX_MUSL))) || \
			defined(CBUILD_OS_MACOS) || \
			defined(CBUILD_OS_BSD) || \
			defined(CBUILD_OS_WINDOWS_CYGWIN)))
	extern const char* __progname;
#endif // CBUILD_API_POSIX + ext check || CBUILD_API_STRICT_POSIX
CBUILDDEF const char* __cbuild_progname(void) {
	#if defined(CBUILD_API_POSIX)
		#if defined(CBUILD_OS_LINUX_GLIBC) || defined(CBUILD_OS_LINUX_MUSL) || \
			defined(CBUILD_OS_WINDOWS_CYGWIN)
			return program_invocation_short_name;
		#elif defined(CBUILD_OS_BSD) || defined(CBUILD_OS_MACOS)
			return getprogname();
		#else
			return __progname;
		#endif // CBUILD_OS_*
	#elif defined(CBUILD_API_STRICT_POSIX)
		return __progname;
	#endif // CBUILD_API_*
}
CBUILDDEF void* __cbuild_memrchr(const void* s, int c, size_t n) {
	#if defined(CBUILD_API_POSIX) && ( \
			defined(CBUILD_OS_LINUX_GLIBC) || defined(CBUILD_OS_LINUX_MUSL) || defined(CBUILD_OS_LINUX_UCLIBC) || \
			defined(CBUILD_OS_BSD))
		return memrchr(s, c, n);
	#else
		char* chrptr = (char*)s;
		chrptr += n;
		do {
			chrptr--;
			if(*chrptr == c) {
				goto loop_end;
			}
		} while(chrptr > (char*)s);
		chrptr = NULL;
	loop_end:
		return chrptr;
	#endif // Extension check
}
CBUILDDEF void* __cbuild_memmem(const void* haystack, size_t hsize,
	const void* needle, size_t nsize) {
	#if defined(CBUILD_API_POSIX) && ( \
			defined(CBUILD_OS_LINUX_GLIBC) || defined(CBUILD_OS_LINUX_MUSL) || defined(CBUILD_OS_LINNUX_UCLIBC) || \
			defined(CBUILD_OS_BSD) || \
			defined(CBUILD_OS_MACOS))
		return memmem(haystack, hsize, needle, nsize);
	#else
		if (hsize < nsize) return NULL;
		const char* orig = haystack;
		const char* p = orig;
		size_t rem = hsize;
		while ((p = memchr(p, *(const char*)needle, rem)) != NULL) {
			size_t offset = (size_t)(p - orig);
			rem = hsize - offset;
			if (rem < nsize) return NULL;
			if (memcmp(needle, p, nsize) == 0) return (void*)p;
			p++;
			rem--;
		}
		return NULL;
	#endif // Extension check
}
CBUILDDEF void __cbuild_assert(const char* file, unsigned int line, const char* func,
	const char* expr, ...) {
	__CBUILD_PRINTF("%s: %s:%u: %s: Assertion \"%s\" failed with message:\n",
		__cbuild_progname(), file, line, func, expr);
	va_list args;
	va_start(args, expr);
	const char* fmt = va_arg(args, char*);
	__CBUILD_VPRINTF(fmt, args);
	va_end(args);
	__CBUILD_FLUSH();
	abort();
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF uint64_t cbuild_time_nanos(void) {
		struct timespec t;
		clock_gettime(CLOCK_MONOTONIC, &t);
		return (uint64_t)(CBUILD_NANOS_PER_SEC * t.tv_sec + t.tv_nsec);
	}
#endif // CBUILD_API_*

/* Version.h */

//! License: `GPL-3.0-or-later`.

// #include "Log.h"
// #include "Common.h"
// #include "Term.h"
// Internals
CBUILDDEF void __cbuild_default_log_handler(const char* level, const char* fmt, 
	va_list args) {
	time_t t = time(NULL);
	struct tm* tm_info = localtime(&t);
	__CBUILD_PRINTF("[%02d:%02d:%02d] ", tm_info->tm_hour, tm_info->tm_min,
		tm_info->tm_sec);
	__CBUILD_PRINT(level);
	__CBUILD_VPRINTF(fmt, args);
	__CBUILD_PRINT("\n");
}
cbuild_log_handler_t __cbuild_curr_log_handler = __cbuild_default_log_handler;
cbuild_log_level_t __cbuild_min_log_level = CBUILD_LOG_MIN_LEVEL;
#ifndef CBUILD_LOG_CUSTOM_LEVELS
	const char* __cbuild_log_level_names[] = {
		[CBUILD_LOG_ERROR] = CBUILD_TERM_FG(CBUILD_TERM_RED)"[ERROR]"CBUILD_TERM_RESET" ",
		[CBUILD_LOG_WARN]  = CBUILD_TERM_FG(CBUILD_TERM_YELLOW)"[WARN]"CBUILD_TERM_RESET" ",
		[CBUILD_LOG_INFO]  = "[INFO] ",
		[CBUILD_LOG_TRACE] = CBUILD_TERM_FG(CBUILD_TERM_BRBLACK)"[TRACE]"CBUILD_TERM_RESET" ",
	};
#else
	extern const char* __cbuild_log_level_names[];
#endif // CBUILD_LOG_CUSTOM_LEVELS
// API
CBUILDDEF void cbuild_log(cbuild_log_level_t level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	cbuild_vlog(level, fmt, args);
	va_end(args);
}
void cbuild_vlog(cbuild_log_level_t level, const char* fmt, va_list args) {
	if (level <= __cbuild_min_log_level) {
		if(__cbuild_curr_log_handler) {
			__cbuild_curr_log_handler(__cbuild_log_level_names[level], fmt, args);
		}
	}
}
CBUILDDEF void cbuild_log_set_min_level(cbuild_log_level_t level) {
	__cbuild_min_log_level = level;
}
CBUILDDEF cbuild_log_level_t cbuild_log_get_min_level(void) {
	return __cbuild_min_log_level;
}
CBUILDDEF void cbuild_log_set_handler(cbuild_log_handler_t handler) {
	__cbuild_curr_log_handler = handler;
}
CBUILDDEF cbuild_log_handler_t cbuild_log_get_handler(void) {
	return __cbuild_curr_log_handler;
}

/* Term.h */

//! License: `GPL-3.0-or-later`.

// #include "Arena.h"
// #include "Common.h"
// #include "Log.h"

CBUILDDEF void cbuild_arena_base_realloc(cbuild_arena_t* arena, size_t new_capacity) {
	arena->base = __CBUILD_REALLOC(arena->base, new_capacity);
	cbuild_assert(arena->base != NULL, "Allocation failed.\n");
	arena->capacity = new_capacity;
}
CBUILDDEF void cbuild_arena_base_free(cbuild_arena_t* arena) {
	arena->pointer = 0;
	arena->capacity = 0;
	__CBUILD_FREE(arena->base);
	arena->base = NULL;
}
CBUILDDEF void* cbuild_arena_malloc(cbuild_arena_t* arena, size_t size) {
	size_t adj_size = (size | ((2*sizeof(void*)) - 1)) + 1;
	if ((arena->pointer + adj_size) >= arena->capacity) return NULL;
	void* ptr = (void*)(arena->base + arena->pointer);
	arena->pointer += adj_size;
	#ifdef CBUILD_PROFILER
		arena->max_pointer = CBUILD_MAX(arena->pointer, arena->max_pointer);
	#endif // CBUILD_PROFILER
	return ptr;
}
CBUILDDEF void* cbuild_arena_realloc(cbuild_arena_t* arena, void* ptr, size_t size) {
	void* new_ptr = cbuild_arena_malloc(arena, size);
	if (!new_ptr) return NULL;
	// NOTE: If allocation not failed it means that 'size' bytes can be safely copied.
	// This can expose some data, so this is not a best idea. But proper size tracking
	// need metadata this allocator does not support.
	memcpy(new_ptr, ptr, size);
	return new_ptr;
}
CBUILDDEF size_t cbuild_arena_checkpoint(cbuild_arena_t* arena) {
	return arena->pointer;
}
CBUILDDEF void cbuild_arena_reset(cbuild_arena_t* arena, size_t checkpoint) {
	arena->pointer = checkpoint;
}
CBUILDDEF char* cbuild_arena_strdup(cbuild_arena_t* arena, const char* src) {
	size_t len = strlen(src);
	char* dst = cbuild_arena_malloc(arena, len + 1);
	if (!dst) return NULL;
	memcpy(dst, src, len);
	dst[len] = 0;
	return dst;
}
CBUILDDEF void* cbuild_arena_memdup(cbuild_arena_t* arena, const void* src, size_t n) {
	void* dst = cbuild_arena_malloc(arena, n);
	if (!dst) return NULL;
	memcpy(dst, src, n);
	return dst;
}
CBUILDDEF char* cbuild_arena_sprintf(cbuild_arena_t* arena, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char* ret = cbuild_arena_vsprintf(arena, fmt, va);
	va_end(va);
	return ret;
}
CBUILDDEF char* cbuild_arena_vsprintf(cbuild_arena_t* arena, const char* fmt,
	va_list ap) {
	va_list va;
	va_copy(va, ap);
	int n = vsnprintf(NULL, 0, fmt, va);
	va_end(va);
	if(n >= 0) {
		char* ret =	cbuild_arena_malloc(arena, (size_t)n + 1);
		if (!ret) return NULL;
		vsnprintf(ret, (size_t)n + 1, fmt, ap);
		return ret;
	} else {
		return NULL;
	}
}
#ifdef CBUILD_PROFILER
	CBUILDDEF void cbuild_arena_profiler(cbuild_arena_t* arena, const char* arena_id) {
		cbuild_log(CBUILD_LOG_TRACE, "Used %zu/%zu bytes of arena %s.",
			arena->max_pointer, arena->capacity, arena_id);
	}
#endif // CBUILD_PROFILER

/* Log.h */

//! License: `GPL-3.0-or-later`.

// #include "Temp.h"
// #include "Arena.h"
// #include "Common.h"
cbuild_arena_t __cbuild_temp_arena = {0};
#define INIT()                                                                \
	do {                                                                        \
		if (!__cbuild_temp_arena.base) {                                          \
			cbuild_arena_base_malloc(&__cbuild_temp_arena, CBUILD_TEMP_ARENA_SIZE); \
		}                                                                         \
	} while(0)
CBUILDDEF void* cbuild_temp_malloc(size_t size) {
	INIT();
	return cbuild_arena_malloc(&__cbuild_temp_arena, size);
}
CBUILDDEF void* cbuild_temp_realloc(void* ptr, size_t size) {
	INIT();
	return cbuild_arena_realloc(&__cbuild_temp_arena, ptr, size);
}
CBUILDDEF char* cbuild_temp_sprintf(const char* fmt, ...) {
	INIT();
	va_list va;
	va_start(va, fmt);
	char* ret = cbuild_arena_vsprintf(&__cbuild_temp_arena, fmt, va);
	va_end(va);
	return ret;
}
CBUILDDEF char* cbuild_temp_vsprintf(const char* fmt, va_list ap) {
	INIT();
	return cbuild_arena_vsprintf(&__cbuild_temp_arena, fmt, ap);
}
CBUILDDEF char* cbuild_temp_strdup(const char* str) {
	INIT();
	return cbuild_arena_strdup(&__cbuild_temp_arena, str);
}
CBUILDDEF void* cbuild_temp_memdup(const void* mem, size_t size) {
	INIT();
	return cbuild_arena_memdup(&__cbuild_temp_arena, mem, size);
}
CBUILDDEF size_t cbuild_temp_checkpoint(void) {
	INIT();
	return cbuild_arena_checkpoint(&__cbuild_temp_arena);
}
CBUILDDEF void cbuild_temp_reset(size_t checkpoint) {
	INIT();
	cbuild_arena_reset(&__cbuild_temp_arena, checkpoint);
}
#ifdef CBUILD_PROFILER
	CBUILDDEF void cbuild_temp_profiler(void) {
		cbuild_log(CBUILD_LOG_TRACE, "Used %zu/%zu bytes of temp.",
			__cbuild_temp_arena.max_pointer, __cbuild_temp_arena.capacity);
	}
#endif // CBUILD_PROFILER
#undef INIT

/* Arena.h */

//! License: `GPL-3.0-or-later`.

// #include "StringView.h"
// #include "Common.h"
// #include "Temp.h"
size_t cbuild_sv_trim_left(cbuild_sv_t* sv) {
	size_t i = 0;
	while(i < sv->size && isspace((unsigned char)sv->data[i])) {
		i++;
	}
	sv->data += i;
	sv->size -= i;
	return i;
}
size_t cbuild_sv_trim_right(cbuild_sv_t* sv) {
	size_t i = sv->size;
	while(i > 0 && isspace((unsigned char)sv->data[i - 1])) {
		i--;
	}
	size_t tmp = sv->size - i;
	sv->size  -= tmp;
	return tmp;
}
size_t cbuild_sv_trim(cbuild_sv_t* sv) {
	size_t ret = cbuild_sv_trim_left(sv);
	ret += cbuild_sv_trim_right(sv);
	return ret;
}
cbuild_sv_t cbuild_sv_chop(cbuild_sv_t* sv, size_t size) {
	if(size > sv->size) {
		size = sv->size;
	}
	char* tmp = sv->data;
	sv->data += size;
	sv->size -= size;
	return cbuild_sv_from_parts(tmp, size);
}
cbuild_sv_t cbuild_sv_chop_right(cbuild_sv_t* sv, size_t size) {
	if(size > sv->size) {
		size = sv->size;
	}
	sv->size -= size;
	return cbuild_sv_from_parts(sv->data + sv->size, size);
}
cbuild_sv_t cbuild_sv_chop_by_delim(cbuild_sv_t* sv, char delim) {
	char* chrptr = memchr(sv->data, delim, sv->size);
	if(chrptr != NULL) {
		size_t i = (size_t)(chrptr - sv->data);
		cbuild_sv_t ret = cbuild_sv_from_parts(sv->data, i);
		sv->data += i + 1;
		sv->size -= i + 1;
		return ret;
	}
	return cbuild_sv_chop(sv, sv->size);
}
cbuild_sv_t cbuild_sv_chop_right_by_delim(cbuild_sv_t* sv, char delim) {
	char* chrptr = __cbuild_memrchr(sv->data, delim, sv->size);
	if(chrptr != NULL) {
		size_t i = (size_t)(chrptr - sv->data);
		cbuild_sv_t ret = cbuild_sv_from_parts(chrptr + 1, (sv->size - i) - 1);
		sv->size -= (sv->size - i);
		return ret;
	}
	return cbuild_sv_chop(sv, sv->size);
}
cbuild_sv_t cbuild_sv_chop_by_sv(cbuild_sv_t* sv, cbuild_sv_t delim) {
	if(delim.size == 0 || delim.size > sv->size) {
		return cbuild_sv_from_parts(sv->data, 0);
	}
	char* chrptr = sv->data - 1;
	size_t i = 0;
	do {
		chrptr = memchr(chrptr + 1, delim.data[0], sv->size - i);
		i = (size_t)(chrptr - sv->data);
		if(chrptr != NULL && sv->size - i >= delim.size &&
			memcmp(chrptr, delim.data, delim.size) == 0) {
			cbuild_sv_t ret = cbuild_sv_from_parts(sv->data, i);
			sv->data += delim.size + i;
			sv->size -= delim.size + i;
			return ret;
		}
	} while(chrptr != NULL);
	return cbuild_sv_chop(sv, sv->size);
}
cbuild_sv_t cbuild_sv_chop_right_by_sv(cbuild_sv_t* sv, cbuild_sv_t delim) {
	if(delim.size == 0 || delim.size > sv->size) {
		return cbuild_sv_from_parts(sv->data, 0);
	}
	char* chrptr = sv->data;
	size_t i = sv->size;
	do {
		chrptr = __cbuild_memrchr(sv->data, delim.data[0], sv->size - (sv->size - i));
		i = (size_t)(chrptr - sv->data);
		if(chrptr != NULL && sv->size - i >= delim.size &&
			memcmp(chrptr, delim.data, delim.size) == 0) {
			cbuild_sv_t ret = cbuild_sv_from_parts(
				sv->data + delim.size + i, 
				sv->size - delim.size - i);
			sv->size = i;
			return ret;
		}
	} while(chrptr != NULL);
	return cbuild_sv_chop(sv, sv->size);
}
cbuild_sv_t cbuild_sv_chop_by_func(cbuild_sv_t* sv, cbuild_sv_delim_func delim,
	void* args) {
	size_t i = 0;
	while(i <= sv->size && !delim(sv, i, args)) {
		i++;
	}
	if(i >= sv->size) {
		return cbuild_sv_chop(sv, i);
	}
	char* tmp = sv->data;
	sv->data += i + 1;
	sv->size -= i + 1;
	return cbuild_sv_from_parts(tmp, i);
}
cbuild_sv_t cbuild_sv_chop_right_by_func(cbuild_sv_t* sv,
	cbuild_sv_delim_func delim, void* args) {
	ssize_t i = (ssize_t)sv->size - 1;
	while(i >= 0 && !delim(sv, (size_t)i, args)) {
		i--;
	}
	if(i < 0) {
		cbuild_sv_t ret = cbuild_sv_from_parts(sv->data, sv->size);
		sv->size = 0;
		return ret;
	}
	size_t tmp = sv->size;
	sv->size = (size_t)i;
	return cbuild_sv_from_parts(sv->data + i + 1, tmp - ((size_t)i + 1));
}
int cbuild_sv_cmp(cbuild_sv_t a, cbuild_sv_t b) {
	if(a.size < b.size) {
		return -2;
	}
	if(a.size > b.size) {
		return 2;
	}
	int ret = memcmp(a.data, b.data, a.size);
	if(ret == 0) {
		return 0;
	} else if(ret < 0) {
		return -1;
	} else if(ret > 0) {
		return 1;
	}
	return 0;
}
int cbuild_sv_cmp_icase(cbuild_sv_t a, cbuild_sv_t b) {
	if(a.size < b.size) {
		return -2;
	}
	if(a.size > b.size) {
		return 2;
	}
	for(size_t i = 0; i < a.size; i++) {
		char ac = 'A' <= a.data[i] && a.data[i] <= 'Z' ? a.data[i] + 32 : a.data[i];
		char bc = 'A' <= b.data[i] && b.data[i] <= 'Z' ? b.data[i] + 32 : b.data[i];
		int diff = ac - bc;
		if(diff < 0) {
			return -1;
		} else if(diff > 0) {
			return 1;
		}
	}
	return 0;
}
bool cbuild_sv_prefix(cbuild_sv_t sv, cbuild_sv_t prefix) {
	if(sv.size < prefix.size) {
		return false;
	}
	return cbuild_sv_cmp(cbuild_sv_from_parts(sv.data, prefix.size), prefix) == 0;
}
bool cbuild_sv_suffix(cbuild_sv_t sv, cbuild_sv_t suffix) {
	if(sv.size < suffix.size) {
		return false;
	}
	return cbuild_sv_cmp(
		cbuild_sv_from_parts(sv.data + sv.size - suffix.size, suffix.size),
		suffix) == 0;
}
ssize_t cbuild_sv_find(cbuild_sv_t sv, char c) {
	if(sv.size == 0) return -1;
	char* chrptr = memchr(sv.data, c, sv.size);
	if(chrptr == NULL) {
		return -1;
	}
	return chrptr - sv.data;
}
ssize_t cbuild_sv_rfind(cbuild_sv_t sv, char c) {
	if(sv.size == 0) return -1;
	char* chrptr = __cbuild_memrchr(sv.data, c, sv.size);
	if(chrptr == NULL) {
		return -1;
	}
	return chrptr - sv.data;
}
ssize_t cbuild_sv_find_sv(cbuild_sv_t sv, cbuild_sv_t needle) {
	char* chrptr = __cbuild_memmem(sv.data, sv.size, needle.data, needle.size);
	if(chrptr == NULL) {
		return -1;
	}
	return chrptr - sv.data;
}
bool cbuild_sv_contains(cbuild_sv_t sv, char c) {
	return cbuild_sv_find(sv, c) != -1;
}
bool cbuild_sv_contains_sv(cbuild_sv_t sv, cbuild_sv_t needle) {
	return cbuild_sv_find_sv(sv, needle) != -1;
}
int cbuild_sv_utf8cp_len(cbuild_sv_t sv) {
	if(sv.size == 0) return 0;
	if((*(unsigned char*)sv.data) < 0x80) return 1;
	if((*sv.data & 0xE0) == 0xC0 && sv.size >= 2) return 2;
	if((*sv.data & 0xF0) == 0xE0 && sv.size >= 3) return 3;
	if((*sv.data & 0xF8) == 0xF0 && sv.size >= 4) return 4;
	return 1;
}
char* cbuild_sv_strchr(cbuild_sv_t sv, char c) {
	return memchr(sv.data, c, sv.size);
}
char* cbuild_sv_utf8chr(cbuild_sv_t sv, uint32_t c) {
	while(sv.size > 0) {
		char* curr = sv.data;
		uint32_t codepoint = cbuild_sv_chop_utf8(&sv);
		if(codepoint == c) return curr;
	}
	return NULL;
}
uint32_t cbuild_sv_chop_utf8(cbuild_sv_t* sv) {
	if(sv->size == 0) return UINT32_MAX;
	unsigned char* s = (unsigned char*)sv->data;
	if(*s < 0x80) {
		sv->data++; s++;
		sv->size--;
		unsigned char byte1 = *(s - 1);
		uint32_t codepoint = byte1;
		return codepoint;
	}
	if((*s & 0xE0) == 0xC0 && sv->size >= 2) {
		sv->data += 2; s += 2;
		sv->size -= 2;
		unsigned char byte1 = *(s - 2),
		byte2 = *(s - 1);
		uint32_t codepoint = ((byte1 & 0x1Fu) << 6) |
			(byte2 & 0x3Fu);
		return codepoint;
	}
	if((*s & 0xF0) == 0xE0 && sv->size >= 3) {
		sv->data += 3; s += 3;
		sv->size -= 3;
		unsigned char byte1 = *(s - 3),
		byte2 = *(s - 2),
		byte3 = *(s - 1);
		uint32_t codepoint = ((byte1 & 0x0Fu) << 12) |
			((byte2 & 0x3Fu) << 6) |
			(byte3 & 0x3Fu);
		return codepoint;
	}
	if((*s & 0xF8) == 0xF0 && sv->size >= 4) {
		sv->data += 4; s += 4;
		sv->size -= 4;
		unsigned char byte1 = *(s - 4),
		byte2 = *(s - 3),
		byte3 = *(s - 2),
		byte4 = *(s - 1);
		uint32_t codepoint = ((byte1 & 0x07u) << 18) |
			((byte2 & 0x3Fu) << 12) |
			((byte3 & 0x3Fu) << 6) |
			(byte4 & 0x3Fu);
		return codepoint;
	}
	sv->data++; s++;
	sv->size--;
	return *(s - 1);
}
cbuild_sv_t cbuild_sv_chop_by_utf8(cbuild_sv_t* sv, uint32_t delim) {
	char* chrptr = cbuild_sv_utf8chr(*sv, delim);
	if(chrptr != NULL) {
		size_t i = (size_t)(chrptr - sv->data);
		cbuild_sv_t ret = cbuild_sv_from_parts(sv->data, i);
		sv->data += i;
		sv->size -= i;
		int codepoint_size = cbuild_sv_utf8cp_len(*sv);
		sv->data += (size_t)codepoint_size;
		sv->size -= (size_t)codepoint_size;
		return ret;
	}
	return cbuild_sv_chop(sv, sv->size);
}
cbuild_sv_t cbuild_sv_chop_by_func_utf8(cbuild_sv_t* sv,
	cbuild_sv_utf8delim_func delim, void* args) {
	cbuild_sv_t ret = cbuild_sv_from_parts(sv->data, 0);
	while(sv->size > 0) {
		size_t cplen = (size_t)cbuild_sv_utf8cp_len(*sv);
		if(delim(sv, args)) {
			sv->data += cplen;
			sv->size -= cplen;
			break;
		} else {
			sv->data += cplen;
			sv->size -= cplen;
			ret.size += cplen;
		}
	}
	return ret;
}
size_t cbuild_sv_utf8len(cbuild_sv_t sv) {
	size_t ret = 0;
	while(sv.size > 0) {
		cbuild_sv_chop_utf8(&sv);
		ret++;
	}
	return ret;
}
bool cbuild_sv_utf8valid(cbuild_sv_t sv, size_t* idx) {
	size_t ret = 0;
	while(sv.size > 0) {
		signed char cs = *(signed char*)sv.data;
		ret++;
		if(cs > 0) { // ASCII, 'signed char' abuse
			sv.size--;
			sv.data++;
			continue;
		}
		unsigned char c = (unsigned char)cs;
		if(0xC2 <= c && c <= 0xDF) { // 2 byte
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
			} else {
				goto invalid;
			}
		} else if(c == 0xE0) { // 3 byte
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0xA0 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(0xE1 <= c && c <= 0xEC) {
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(c == 0xED) {
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0x9F) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(0xEE <= c && c <= 0xEF) {
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(c == 0xF0) { // 4 byte
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x90 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
					if(sv.size == 0) goto invalid;
					unsigned char c3 = *(unsigned char*)sv.data;
					if(0x80 <= c3 && c3 <= 0xBF) {
						sv.size--;
						sv.data++;
					} else {
						goto invalid;
					}
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(0xF1 <= c && c <= 0xF3) {
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0xBF) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
					if(sv.size == 0) goto invalid;
					unsigned char c3 = *(unsigned char*)sv.data;
					if(0x80 <= c3 && c3 <= 0xBF) {
						sv.size--;
						sv.data++;
					} else {
						goto invalid;
					}
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else if(c == 0xF4) {
			sv.size--;
			sv.data++;
			if(sv.size == 0) goto invalid;
			unsigned char c1 = *(unsigned char*)sv.data;
			if(0x80 <= c1 && c1 <= 0x8F) {
				sv.size--;
				sv.data++;
				if(sv.size == 0) goto invalid;
				unsigned char c2 = *(unsigned char*)sv.data;
				if(0x80 <= c2 && c2 <= 0xBF) {
					sv.size--;
					sv.data++;
					if(sv.size == 0) goto invalid;
					unsigned char c3 = *(unsigned char*)sv.data;
					if(0x80 <= c3 && c3 <= 0xBF) {
						sv.size--;
						sv.data++;
					} else {
						goto invalid;
					}
				} else {
					goto invalid;
				}
			} else {
				goto invalid;
			}
		} else {
			goto invalid;
		}
	}
	return true;
invalid:
	if(idx != NULL) *idx = ret;
	return false;
}
char* cbuild_sv_to_cstr(cbuild_sv_t sv) {
	char* ret = __CBUILD_MALLOC(sv.size + 1);
	cbuild_assert(ret != NULL, "Allocation failed.\n");
	memcpy(ret, sv.data, sv.size);
	ret[sv.size] = 0;
	return ret;
}
char* cbuild_sv_to_temp_cstr(cbuild_sv_t sv) {
	char* ret = cbuild_temp_malloc(sv.size + 1);
	cbuild_assert(ret != NULL, "Allocation failed.\n");
	memcpy(ret, sv.data, sv.size);
	ret[sv.size] = 0;
	return ret;
}

/* Temp.h */

//! License: `GPL-3.0-or-later`.

// #include "StringBuilder.h"
// #include "StringView.h"
// #include "Temp.h"
// #include "DynArray.h"
CBUILDDEF int cbuild_sb_cmp(cbuild_sb_t a, cbuild_sb_t b) {
	return cbuild_sv_cmp(cbuild_sb_to_sv(a), cbuild_sb_to_sv(b));
}
CBUILDDEF int cbuild_sb_cmp_icase(cbuild_sb_t a, cbuild_sb_t b) {
	return cbuild_sv_cmp_icase(cbuild_sb_to_sv(a), cbuild_sb_to_sv(b));
}
CBUILDDEF void cbuild_sb_append_utf8(cbuild_sb_t* sb, uint32_t cp) {
	unsigned char buffer[4] = {0};
	size_t len = 0;
	if(cp <= 0x7F) {
		buffer[0] = (unsigned char)cp;
		len = 1;
	} else if(cp <= 0x7FF) {
		buffer[0] = 0xC0 | ((cp >> 6) & 0x1F);
		buffer[1] = 0x80 | (cp & 0x3F);
		len = 2;
	} else if(cp <= 0xFFFF) {
		buffer[0] = 0xE0 | ((cp >> 12) & 0x0F);
		buffer[1] = 0x80 | ((cp >> 6) & 0x3F);
		buffer[2] = 0x80 | (cp & 0x3F);
		len = 3;
	} else if(cp <= 0x10FFFF) {
		buffer[0] = 0xF0 | ((cp >> 18) & 0x07);
		buffer[1] = 0x80 | ((cp >> 12) & 0x3F);
		buffer[2] = 0x80 | ((cp >> 6) & 0x3F);
		buffer[3] = 0x80 | (cp & 0x3F);
		len = 4;
	} else {
		CBUILD_UNREACHABLE("Invalid Unicode codepoint found: 0x%04X\n", cp);
	}
	cbuild_sb_append_arr(sb, buffer, len);
	
}
CBUILDDEF size_t cbuild_sb_utf8len(cbuild_sb_t sb) {
	return cbuild_sv_utf8len(cbuild_sb_to_sv(sb));
}
CBUILDDEF cbuild_sv_t cbuild_sb_to_sv(cbuild_sb_t sb) {
	return (cbuild_sv_t) {
		.data = sb.data, .size = sb.size
	};
}
CBUILDDEF cbuild_sb_t cbuild_sv_to_sb(cbuild_sv_t sv) {
	cbuild_sb_t ret = {0};
	cbuild_sb_append_arr(&ret, sv.data, sv.size);
	return ret;
}
CBUILDDEF int cbuild_sb_appendf(cbuild_sb_t* sb, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = cbuild_sb_vappendf(sb, fmt, args);
	va_end(args);
	return ret;
}
CBUILDDEF int cbuild_sb_vappendf(cbuild_sb_t* sb, const char* fmt, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);
	char buff[CBUILD_QUICK_SPRINTF_SIZE];
	int ret = vsnprintf(buff, CBUILD_QUICK_SPRINTF_SIZE, fmt, args);
	if(ret < 0) {
		va_end(args_copy);
		return ret;
	}
	if((size_t)ret >= CBUILD_QUICK_SPRINTF_SIZE) {
		char* buff1 = __CBUILD_MALLOC((size_t)ret + 1);
		cbuild_assert(buff1 != NULL, "Allocation failed.\n");
		vsnprintf(buff1, (size_t)ret + 1, fmt, args_copy);
		cbuild_sb_append_arr(sb, buff1, (size_t)ret);
		__CBUILD_FREE(buff1);
	} else {
		cbuild_sb_append_arr(sb, buff, (size_t)ret);
	}
	va_end(args_copy);
	return ret;
}

/* DynArray.h */

//! License: `GPL-3.0-or-later`.

// #include "Map.h"
// #include "Common.h"
// #include "Log.h"
CBUILDDEF size_t __cbuild_map_hash_func(const void* data, size_t len) {
	const unsigned char* ucPtr = (const unsigned char*)data;
	size_t hash = 5381;
	for(size_t i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + ucPtr[i]; // hash * 33 + data[i]
	}
	return hash;
}
CBUILDDEF size_t __cbuild_map_get_hash(const cbuild_map_t* map, const void* key) {
	size_t hash = 0;
	if(map->hash_func == NULL) {
		if(map->key_size > 0) {
			hash = CBUILD_MAP_DEFAULT_HASH_FUNC(key, map->key_size)
				% map->nbuckets;
		} else {
			hash = CBUILD_MAP_DEFAULT_HASH_FUNC(*((void**)key), strlen(*((char**)key)))
				% map->nbuckets;
		}
	} else {
		hash = map->hash_func(map, key) % map->nbuckets;
	}
	return hash;
}
CBUILDDEF void* __cbuild_map_check_bucket(const cbuild_map_t* map,
																							const cbuild_map_bucket_t* bucket, const void* key) {
	if(map->keycmp_func == NULL) {
		if(map->key_size > 0) {
			for(size_t i = 0; i < bucket->nvals; i++) {
				if(memcmp(key, ((char*)bucket->vals + (i * map->elem_size)),
							map->key_size) == 0) {
					return ((char*)bucket->vals + (i * map->elem_size));
				}
			}
		} else {
			for(size_t i = 0; i < bucket->nvals; i++) {
				if(strcmp(*((char**)key),
							*(char**)(void*)(((char*)bucket->vals + (i * map->elem_size)))) == 0) {
					return ((char*)bucket->vals + (i * map->elem_size));
				}
			}
		}
	} else {
		for(size_t i = 0; i < bucket->nvals; i++) {
			if(map->keycmp_func(map, key,
											 ((char*)bucket->vals + (i * map->elem_size)))) {
				return ((char*)bucket->vals + (i * map->elem_size));
			}
		}
	}
	return NULL;
}
CBUILDDEF void cbuild_map_init(cbuild_map_t* map, size_t nbuckets) {
	map->nbuckets = nbuckets;
	map->buckets = __CBUILD_MALLOC(map->nbuckets * sizeof(cbuild_map_bucket_t));
	cbuild_assert(map->buckets != NULL, "Allocation failed.\n");
	memset(map->buckets, 0, nbuckets * sizeof(cbuild_map_bucket_t));
}
CBUILDDEF void* cbuild_map_get_raw(const cbuild_map_t* map, const void* key) {
	if(map->nbuckets == 0) {
		cbuild_log_error("Trying to call \"cbuild_map_get\" on an empty map!");
		return CBUILD_PTR_ERR;
	}
	size_t hash = __cbuild_map_get_hash(map, key);
	cbuild_map_bucket_t* bucket = &map->buckets[hash];
	return __cbuild_map_check_bucket(map, bucket, key);
}
CBUILDDEF void* cbuild_map_get_or_alloc_raw(cbuild_map_t* map, const void* key) {
	if(map->nbuckets == 0) {
		cbuild_log_error("Trying to call \"cbuild_map_get_or_alloc\" with an empty map!");
		return NULL;
	}
	size_t hash = __cbuild_map_get_hash(map, key);
	cbuild_map_bucket_t* bucket = &map->buckets[hash];
	void* ret = __cbuild_map_check_bucket(map, bucket, key);
	if(ret != NULL) return ret;
	bucket->nvals++;
	bucket->vals = __CBUILD_REALLOC((char*)bucket->vals,
															 bucket->nvals * map->elem_size);
	cbuild_assert(bucket->vals != NULL, "Allocation failed.\n");
	memset((char*)bucket->vals + (bucket->nvals - 1) * map->elem_size, 0,
				map->elem_size);
	return (char*)bucket->vals + ((bucket->nvals - 1) * map->elem_size);
}
CBUILDDEF bool cbuild_map_remove_raw(cbuild_map_t* map, const void* key) {
	if(map->nbuckets == 0) {
		cbuild_log_error("Trying to call \"cbuild_map_remove_raw\" with an empty map!");
		return false;
	}
	size_t hash = __cbuild_map_get_hash(map, key);
	cbuild_map_bucket_t* bucket = &map->buckets[hash];
	void* elem = __cbuild_map_check_bucket(map, bucket, key);
	if(elem == NULL) return false;
	if(map->clear_func) map->clear_func(map, elem);
	if(bucket->nvals == 1) {
		__CBUILD_FREE(bucket->vals);
		bucket->vals = NULL;
		bucket->nvals = 0;
	} else {
		char* last = ((char*)bucket->vals) +
			((bucket->nvals - 1) * map->elem_size);
		memcpy(elem, last, map->elem_size);
		bucket->nvals--;
	}
	return true;
}
CBUILDDEF void cbuild_map_clear(cbuild_map_t* map) {
	if(map->clear_func == NULL) {
		for(size_t i = 0; i < map->nbuckets; i++) {
			cbuild_map_bucket_t* bucket = &map->buckets[i];
			__CBUILD_FREE(bucket->vals);
			bucket->vals = NULL;
			bucket->nvals = 0;
		}
		__CBUILD_FREE(map->buckets);
		map->buckets = NULL;
		map->nbuckets = 0;
	} else {
		for(size_t i = 0; i < map->nbuckets; i++) {
			cbuild_map_bucket_t* bucket = &map->buckets[i];
			for(size_t j = 0; j < bucket->nvals; j++) {
				map->clear_func(map, bucket->vals + (j * map->elem_size));
			}
			__CBUILD_FREE(bucket->vals);
			bucket->vals = NULL;
			bucket->nvals = 0;
		}
		__CBUILD_FREE(map->buckets);
		map->buckets = NULL;
		map->nbuckets = 0;
	}
}
CBUILDDEF void cbuild_map_iter_reset(cbuild_map_t* map) {
	map->iter_buckets = 0;
	map->iter_vals = 0;
}
CBUILDDEF void* cbuild_map_iter_next(cbuild_map_t* map) {
	while(map->iter_buckets < map->nbuckets) {
		cbuild_map_bucket_t* bucket = &map->buckets[map->iter_buckets];
		while(map->iter_vals < bucket->nvals) {
			return bucket->vals + (map->iter_vals++ * map->elem_size);
		}
		map->iter_buckets++;
		map->iter_vals = 0;
	}
	return NULL;
}

/* StringView.h */

//! License: `GPL-3.0-or-later`.

// #include "Proc.h"
// #include "Common.h"
// #include "Log.h"
// #include "DynArray.h"
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF int cbuild_proc_wait_code(cbuild_proc_t proc) {
		if(proc == CBUILD_INVALID_PROC) {
			return INT_MIN;
		}
		while(true) {
			int status = 0;
			errno = 0;
			if(waitpid(proc, &status, 0) < 0) {
				if(errno ==	ECHILD) {
					return INT_MAX;
				} else if (errno == EINTR) {
					errno = 0;
				} else {
					cbuild_log_error("Could not wait for child process (pid %d), error: \"%s\"",
						proc, strerror(errno));
				}
				errno = 0;
			} else {
				if(WIFEXITED(status)) {
					int code = WEXITSTATUS(status);
					return code;
				}
				if(WIFSIGNALED(status)) {
					cbuild_log_error("Process (pid %d) was terminated by signal \"%d\"",
						proc, WTERMSIG(status));
					return -WTERMSIG(status);
				}
			}
		}
	}
	CBUILDDEF ssize_t cbuild_procs_wait_any(cbuild_proclist_t procs, int* code) {
		if(procs.size == 0) {
			return -1;
		}
		while(true) {
			for(size_t i = 0; i < procs.size; i++) {
				cbuild_proc_t proc = procs.data[i];
				int status = 0;
				errno = 0;
				int ret = waitpid(proc, &status, WNOHANG);
				if(ret < 0) {
					if(errno ==	ECHILD) {
						return INT_MAX;
					} else {
						cbuild_log_error(
							"Could not wait for child process (pid %d), error: \"%s\"", proc,
							strerror(errno));
					}
					errno = 0;
				} else if(ret > 0) {
					if(WIFEXITED(status)) {
						if(code != NULL) *code = WEXITSTATUS(status);
						return (ssize_t)i;
					}
					if(WIFSIGNALED(status)) {
						cbuild_log_error(
							"Process (pid %d) was terminated by signal \"%d\"", proc,
							WTERMSIG(status));
						if(code != NULL) *code = -WTERMSIG(status);
						return (ssize_t)i;
					}
				}
			}
			const struct timespec duration = {
				.tv_sec = 0,
				.tv_nsec = 1000*100,
			};
			nanosleep(&duration, NULL);
		}
		return -1;
	}
	CBUILDDEF bool cbuild_proc_is_running(cbuild_proc_t proc) {
		if(proc <= 0) return false;
		return kill(proc, 0) <= 0;
	}
	CBUILDDEF cbuild_proc_ptr_t cbuild_proc_malloc(size_t n) {
		// Reasoning: Tooo big benefits
		#if defined(MAP_ANONYMOUS) // Also used on strict POSIX
			void* ptr =
				mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		#elif defined(MAP_ANON) // Also used on strict POSIX
			void* ptr =
				mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
		#else // Strict POSIX fallback
			int (*shm_open)(const char* name, int oflag, mode_t mode) =
				(int (*)(const char*, int, mode_t))dlsym(RTLD_DEFAULT, "shm_open");
			if(shm_open == NULL) {
				if(!dlopen("librt.so", RTLD_LAZY | RTLD_GLOBAL)) {
					if(!dlopen("librt.so.1", RTLD_LAZY | RTLD_GLOBAL)) {
						CBUILD_UNREACHABLE("libc dont expose 'shm_open' and librt not found!");
					}
				}
				shm_open = (int (*)(const char*, int, mode_t))dlsym(RTLD_DEFAULT, "shm_open");
			}
			int (*shm_unlink)(const char* name) =
				(int (*)(const char*))dlsym(RTLD_DEFAULT, "shm_unlink");
			int shmemfd = shm_open("/cbuild-proc-malloc-shmem", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
			if(shmemfd == -1) {
				return (cbuild_proc_ptr_t) {
					.ptr = NULL, .size = 0
				};
			}
			if(ftruncate(shmemfd, (long)n) == -1) {
				shm_unlink("/cbuild-proc-malloc-shmem");
				close(shmemfd);
				return (cbuild_proc_ptr_t) {
					.ptr = NULL, .size = 0
				};
			}
			void* ptr =
				mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, shmemfd, 0);
			if(ptr == MAP_FAILED) {
				close(shmemfd);
				shm_unlink("/cbuild-proc-malloc-shmem");
				return (cbuild_proc_ptr_t) {
					.ptr = NULL, .size = 0
				};
			} else {
				close(shmemfd);
				shm_unlink("/cbuild-proc-malloc-shmem");
				return (cbuild_proc_ptr_t) {
					.ptr = ptr, .size = n
				};
			}
		#endif // Extension check
		if(ptr == MAP_FAILED) {
			return (cbuild_proc_ptr_t) {
				.ptr = NULL, .size = 0
			};
		} else {
			return (cbuild_proc_ptr_t) {
				.ptr = ptr, .size = n
			};
		}
	}
	CBUILDDEF void cbuild_proc_free(cbuild_proc_ptr_t ptr) {
		if(ptr.ptr != NULL) {
			munmap(ptr.ptr, ptr.size);
		}
	}
	CBUILDDEF cbuild_proc_t cbuild_proc_start(int (*callback)(void* context),
		void* context) {
		cbuild_proc_t proc = fork();
		if(proc < 0) {
			cbuild_log_error("Could not create child process, error: \"%s\"",
				strerror(errno));
			return CBUILD_INVALID_PROC;
		}
		if(proc == 0) {
			int code = callback(context);
			exit(code);
		}
		return proc;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_proc_wait(cbuild_proc_t proc) {
	return cbuild_proc_wait_code(proc) == 0;
}
CBUILDDEF bool cbuild_procs_wait(cbuild_proclist_t procs) {
	bool ret = true;
	cbuild_da_foreach(procs, proc) {
		if(!cbuild_proc_wait(*proc)) ret = false;
	}
	return ret;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF int cbuild_nproc(void) {
		// Reasoning: no alternatives are available and benefits are high-enough to go slightly off-spec
		#if defined(_SC_NPROCESSORS_ONLN) // Also used on strict POSIX
			return abs((int)sysconf(_SC_NPROCESSORS_ONLN));
		#else
			return 1;
		#endif // Extension check
	}
#endif // CBUILD_API_*

/* StringBuilder.h */

//! License: `GPL-3.0-or-later`.

// #include "Command.h"
// #include "Common.h"
// #include "StringBuilder.h"
// #include "DynArray.h"
// #include "Log.h"
// #include "FS.h"
// #include "Proc.h" 
CBUILDDEF cbuild_sb_t cbuild_cmd_to_sb(cbuild_cmd_t cmd) {
	cbuild_sb_t sb = {0};
	if(cmd.size < 1) {
		return sb;
	}
	for(size_t i = 0; i < cmd.size; i++) {
		const char* tmp = cmd.data[i];
		if(!strchr(tmp, ' ')) {
			cbuild_sb_append_cstr(&sb, tmp);
		} else {
			cbuild_sb_appendf(&sb, "\'%s\'", tmp);
		}
		if(i < cmd.size - 1) {
			cbuild_sb_append(&sb, ' ');
		}
	}
	return sb;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	// We needs opts here, because I dont want to bloat function signature when I will add more call-level flags
	CBUILDDEF cbuild_proc_t __cbuild_cmd_run_opt(cbuild_cmd_t* cmd,
		struct cbuild_cmd_opts_t* opts, cbuild_fd_t fdstdin, 
		cbuild_fd_t fdstdout, cbuild_fd_t fdstderr) {
		// Get args
		cbuild_cmd_t argv = {0};
		cbuild_cmd_append_arr(&argv, cmd->data, cmd->size);
		cbuild_cmd_append(&argv, (char*)NULL);
		cbuild_proc_t proc = fork();
		if(proc < 0) {
			cbuild_log_error( "Could not create child process, error: \"%s\"",
				strerror(errno));
			cbuild_cmd_clear(&argv);
			return CBUILD_INVALID_PROC;
		}
		if(proc == 0) {
			fflush(NULL);
			// Setup stdin, stdout and stderr
			if(fdstdin != CBUILD_INVALID_FD) {
				if(dup2(fdstdin, STDIN_FILENO) < 0) {
					cbuild_log_error(
						"Could not redirect stdin inside of a child process, error: \"%s\"",
						strerror(errno));
					cbuild_cmd_clear(&argv);
					exit(1);
				}
			}
			if(fdstdout != CBUILD_INVALID_FD) {
				if(dup2(fdstdout, STDOUT_FILENO) < 0) {
					cbuild_log_error(
						"Could not redirect stdout inside of a child process, error: \"%s\"",
						strerror(errno));
					cbuild_cmd_clear(&argv);
					exit(1);
				}
			}
			if(fdstderr != CBUILD_INVALID_FD) {
				if(dup2(fdstderr, STDERR_FILENO) < 0) {
					cbuild_log_error(
						"Could not redirect stderr inside of a child process, error: \"%s\"",
						strerror(errno));
					cbuild_cmd_clear(&argv);
					exit(1);
				}
			}
			// Autokill
			if(opts->autokill) {
				#if defined(CBUILD_API_POSIX) && defined(CBUILD_OS_LINUX)
					prctl(PR_SET_PDEATHSIG, SIGKILL);
				#else
					cbuild_log_warn("Autokill is supported only on POSIX with Linux extensions!");
				#endif // Extension check
			}
			// Call command
			if(execvp(argv.data[0], (char* const*)argv.data) < 0) {
				cbuild_log_error("Could not execute command in child process, error: \"%s\"",
					strerror(errno));
				exit(1);
			}
			exit(0);
		}
		cbuild_cmd_clear(&argv);
		return proc;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_cmd_run_opt(cbuild_cmd_t* cmd, struct cbuild_cmd_opts_t opts) {
	if(cmd->size == 0) {
		cbuild_log_error( "Empty command requested to be executed!");
		return false;
	}
	if(!opts.no_print_cmd) {
		cbuild_sb_t cmd_sb = cbuild_cmd_to_sb(*cmd);
		cbuild_log_trace("Running command '"CBuildSBFmt"'", CBuildSBArg(cmd_sb));
		cbuild_sb_clear(&cmd_sb);
	}
	// Open files
	cbuild_fd_t fdstdin = CBUILD_INVALID_FD;
	cbuild_fd_t fdstdout = CBUILD_INVALID_FD;
	cbuild_fd_t fdstderr = CBUILD_INVALID_FD;
	cbuild_fd_t file_stdin = CBUILD_INVALID_FD;
	cbuild_fd_t file_stdout = CBUILD_INVALID_FD;
	cbuild_fd_t file_stderr = CBUILD_INVALID_FD;
	if(opts.fdstdin != NULL) fdstdin = *opts.fdstdin;
	if(opts.fdstdout != NULL) fdstdout = *opts.fdstdout;
	if(opts.fdstderr != NULL) fdstderr = *opts.fdstderr;
	if(opts.file_stdin) {
		file_stdin = cbuild_fd_open_read(opts.file_stdin);
		fdstdin = file_stdin;
	}
	if(opts.file_stdout) {
		file_stdout = cbuild_fd_open_write(opts.file_stdout);
		fdstdout = file_stdout;
	}
	if(opts.file_stderr) {
		file_stderr = cbuild_fd_open_write(opts.file_stderr);
		fdstderr = file_stderr;
	}
	// Check for limits
	size_t proc_idx = 0;
	int code = 0;
	if(opts.procs != NULL && opts.async_threads != -1) {
		if(opts.async_threads == 0) opts.async_threads = cbuild_nproc() + 1;
		if((size_t)opts.async_threads == opts.procs->size) {
			proc_idx = (size_t)cbuild_procs_wait_any(*opts.procs, &code);
			if(code != 0) {
				goto cleanup;
			}
		}
	}
	// Call app
	cbuild_proc_t proc = __cbuild_cmd_run_opt(cmd, &opts, fdstdin, fdstdout, fdstderr);
	if (proc == CBUILD_INVALID_PROC) {
		code = -1;
		goto cleanup;
	}
	// Finish process creation
	if(opts.proc != NULL) {
		*opts.proc = proc;
	} else if(opts.procs != NULL) {
		if(opts.async_threads != -1) {
			if((size_t)opts.async_threads == opts.procs->size) {
				cbuild_da_set(opts.procs, proc_idx, proc);
			} else {
				cbuild_da_append(opts.procs, proc);
			}
		} else {
			cbuild_da_append(opts.procs, proc);
		}
	} else {
		code = cbuild_proc_wait_code(proc);
	}
cleanup:
	// Cleanup
	if(file_stdin != CBUILD_INVALID_FD) {
		cbuild_fd_close(file_stdin);
		fdstdin = CBUILD_INVALID_FD;
	}
	if(file_stdout != CBUILD_INVALID_FD) {
		cbuild_fd_close(file_stdout);
		fdstdout = CBUILD_INVALID_FD;
	}
	if(file_stderr != CBUILD_INVALID_FD) {
		cbuild_fd_close(file_stderr);
		fdstderr = CBUILD_INVALID_FD;
	}
	if(!opts.no_reset) {
		cmd->size = 0;
		if(fdstdin != CBUILD_INVALID_FD) cbuild_fd_close(fdstdin);
		if(fdstdout != CBUILD_INVALID_FD) cbuild_fd_close(fdstdout);
		if(fdstderr != CBUILD_INVALID_FD) cbuild_fd_close(fdstderr);
	}
	return code == 0;
}

/* Stack.h */

//! License: `GPL-3.0-or-later`.

// #include "FS.h"
// #include "Common.h"
// #include "StringBuilder.h"
// #include "DynArray.h"
// #include "Stack.h"
// #include "Log.h"
// #include "Temp.h"
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF bool cbuild_fd_close(cbuild_fd_t fd) {
		if(fd == CBUILD_INVALID_FD) {
			cbuild_log_error("Invalid file descriptor passed to close, error: \"%s\"",
				strerror(errno));
			return false;
		}
		return close(fd) == 0;
	}
	CBUILDDEF cbuild_fd_t cbuild_fd_open_read(const char* path) {
		cbuild_fd_t fd = open(path, O_RDONLY);
		if(fd < 0) {
			cbuild_log_error("Could not open file \"%s\" for reading, error: \"%s\"",
				path, strerror(errno));
			return CBUILD_INVALID_FD;
		}
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		return fd;
	}
	CBUILDDEF cbuild_fd_t cbuild_fd_open_write(const char* path) {
		cbuild_fd_t fd = open(path, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if(fd < 0) {
			cbuild_log_error("Could not open file \"%s\" for writing, error: \"%s\"",
				path, strerror(errno));
			return CBUILD_INVALID_FD;
		}
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		return fd;
	}
	CBUILDDEF bool cbuild_fd_open_pipe(cbuild_fd_t* read, cbuild_fd_t* write) {
		cbuild_fd_t fds[2];
		int ret = pipe(fds);
		if(ret < 0) {
			cbuild_log_error("Could not create pipe, error: \"%s\"",
				strerror(errno));
			*read = CBUILD_INVALID_FD;
			*write = CBUILD_INVALID_FD;
			return false;
		}
		*read = fds[0];
		*write = fds[1];
		fcntl(*read, F_SETFD, FD_CLOEXEC);
		fcntl(*write, F_SETFD, FD_CLOEXEC);
		return true;
	}
	CBUILDDEF ssize_t cbuild_fd_read(cbuild_fd_t fd, void* buf, size_t nbytes) {
		ssize_t len = read(fd, buf, nbytes);
		if(len < 0) {
			cbuild_log_error("Could not execute 'cbuild_fd_read', error: \"%s\"",
				strerror(errno));
		}
		return len;
	}
	CBUILDDEF ssize_t cbuild_fd_read_file(cbuild_fd_t fd, void* buf,
		size_t nbytes, const char* path) {
		ssize_t len = read(fd, buf, nbytes);
		if(len < 0) {
			cbuild_log_error("Could not read from file \"%s\", error: \"%s\"",
				path, strerror(errno));
		}
		return len;
	}
	CBUILDDEF ssize_t cbuild_fd_write(cbuild_fd_t fd, const void* buf,
		size_t nbytes) {
		ssize_t len = write(fd, buf, nbytes);
		if(len < 0) {
			cbuild_log_error("Could not execute 'cbuild_fd_write', error: \"%s\"",
				strerror(errno));
		}
		return len;
	}
	CBUILDDEF ssize_t cbuild_fd_write_file(cbuild_fd_t fd, const void* buf,
		size_t nbytes, const char* path) {
		ssize_t len = write(fd, buf, nbytes);
		if(len < 0) {
			cbuild_log_error("Could not write to file \"%s\", error: \"%s\"",
				path, strerror(errno));
		}
		return len;
	}
	CBUILDDEF ssize_t cbuild_file_len(const char* path) {
		struct stat statbuff;
		if(stat(path, &statbuff) < 0) {
			cbuild_log_error("Could not stat \"%s\", error: \"%s\"", path,
				strerror(errno));
			return -1;
		}
		return statbuff.st_size;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_file_read(const char* path, cbuild_sb_t* data) {
	cbuild_fd_t fd = cbuild_fd_open_read(path);
	if(CBUILD_INVALID_FD == fd) {
		return false;
	}
	size_t size = (size_t)cbuild_file_len(path);
	cbuild_sb_resize(data, size); // Assert data != NULL
	ssize_t len = cbuild_fd_read_file(fd, data->data, size, path);
	if(len < 0) {
		cbuild_fd_close(fd);
		return false;
	}
	data->size = (size_t)len;
	cbuild_fd_close(fd);
	return true;
}
CBUILDDEF bool cbuild_file_write(const char* path, cbuild_sb_t* data) {
	cbuild_fd_t fd = cbuild_fd_open_write(path);
	if(CBUILD_INVALID_FD == fd) {
		return false;
	}
	char* buf = data->data;
	ssize_t cnt = (ssize_t)data->size;
	while(cnt > 0) {
		ssize_t written = cbuild_fd_write_file(fd, buf, (size_t)cnt, path);
		if(written < 0) {
			cbuild_fd_close(fd);
			return false;
		}
		cnt -= written;
		buf += written;
	}
	cbuild_fd_close(fd);
	return true;
}
CBUILDDEF bool cbuild_file_copy(const char* src, const char* dst) {
	cbuild_fd_t src_fd = cbuild_fd_open_read(src);
	cbuild_fd_t dst_fd = cbuild_fd_open_write(dst);
	if(src_fd == CBUILD_INVALID_FD) {
		return false;
	}
	if(dst_fd == CBUILD_INVALID_FD) {
		cbuild_fd_close(src_fd);
		return false;
	}
	char* tmp_buff = (char*)__CBUILD_MALLOC(CBUILD_FS_TMP_SIZE);
	cbuild_assert(tmp_buff != NULL, "Allocation failed.\n");
	while(true) {
		ssize_t cnt = cbuild_fd_read_file(src_fd, tmp_buff, CBUILD_FS_TMP_SIZE, src);
		if(cnt == 0) {
			break;
		}
		if(cnt < 0) {
			cbuild_fd_close(src_fd);
			cbuild_fd_close(dst_fd);
			__CBUILD_FREE(tmp_buff);
			return false;
		}
		char* buf = tmp_buff;
		while(cnt > 0) {
			ssize_t written = cbuild_fd_write_file(dst_fd, buf, (size_t)cnt, dst);
			if(written < 0) {
				cbuild_fd_close(src_fd);
				cbuild_fd_close(dst_fd);
				__CBUILD_FREE(tmp_buff);
				return false;
			}
			cnt -= written;
			buf += written;
		}
	}
	cbuild_fd_close(src_fd);
	cbuild_fd_close(dst_fd);
	__CBUILD_FREE(tmp_buff);
	return true;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF bool cbuild_file_move(const char* src, const char* dst) {
		if(rename(src, dst) == 0) {
			return true;
		}
		if(errno == EXDEV) {
			if(!cbuild_file_copy(src, dst)) return false;
			return cbuild_file_remove(src);
		}
		return false;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_file_check(const char* path) {
	struct stat statbuff;
	return stat(path, &statbuff) == 0;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF bool cbuild_file_remove(const char* path) {
		if(unlink(path) < 0) {
			cbuild_log_error("Could not remove file \"%s\", error: \"%s\"",
				path, strerror(errno));
			return false;
		}
		return true;
	}
	CBUILDDEF bool cbuild_symlink(const char* src, const char* dst) {
		size_t checkpoint = cbuild_temp_checkpoint();
		char* base = cbuild_path_base(dst);
		if(*base) {
			if(!cbuild_dir_check(base)) {
				if(!cbuild_dir_create(base)) {
					cbuild_log_error("Destination base path \"%s\" is invalid!", base);
					cbuild_temp_reset(checkpoint);
					return false;
				}
			}
		}
		cbuild_temp_reset(checkpoint);
		int ret = symlink(src, dst);
		if(ret < 0) {
			if(errno == EEXIST) {
				cbuild_filetype_t type = cbuild_path_filetype(dst);
				switch(type) {
				case CBUILD_FTYPE_DIRECTORY: cbuild_dir_remove(dst); break;
				case CBUILD_FTYPE_REGULAR:
				case CBUILD_FTYPE_SYMLINK:
				case CBUILD_FTYPE_OTHER:
					cbuild_file_remove(dst);
					break;
				case CBUILD_FTYPE_MISSING:
					CBUILD_UNREACHABLE("File cannot be missing if it exists.");
					break;
				default: CBUILD_UNREACHABLE("Invalid filetype in create_symlink.");
				}
				if(symlink(src, dst) == 0) return true;
			}
			cbuild_log_error(
				"Could not create symbolic link \"%s\", error: \"%s\"",
				dst, strerror(errno));
			return false;
		}
		return true;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_dir_copy(const char* src, const char* dst) {
	bool err = cbuild_dir_create(dst);
	if(err == false) {
		cbuild_log_error("Could not create destination directory \"%s\"",
			dst);
		return false;
	}
	cbuild_pathlist_t list = {0};
	err = cbuild_dir_list(src, &list);
	if(err == false) {
		cbuild_log_error("Could not list source directory \"%s\"", src);
		if(list.data != NULL) {
			cbuild_pathlist_clear(&list);
		}
		return false;
	}
	bool ret = true;
	size_t checkpoint = cbuild_temp_checkpoint();
	for(size_t i = 0; i < list.size; i++) {
		const char* tmpsrc = cbuild_temp_sprintf("%s/%s", src, list.data[i]);
		const char* tmpdst = cbuild_temp_sprintf("%s/%s", dst, list.data[i]);
		cbuild_filetype_t f = cbuild_path_filetype(tmpsrc);
		if (f == CBUILD_FTYPE_MISSING) {
			CBUILD_UNREACHABLE("cbuild_dir_list should not return invalid files.");
		} else if(f == CBUILD_FTYPE_DIRECTORY) {
			bool tmp = ret && cbuild_dir_copy(tmpsrc, tmpdst);
			ret = tmp;
		} else {
			bool tmp = ret && cbuild_file_copy(tmpsrc, tmpdst);
			ret = tmp;
		}
		cbuild_temp_reset(checkpoint);
	}
	cbuild_pathlist_clear(&list);
	return ret;
}
CBUILDDEF bool cbuild_dir_move(const char* src, const char* dst) {
	bool ret = cbuild_dir_copy(src, dst);
	if(ret == false) {
		return false;
	}
	return cbuild_dir_remove(src);
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF bool __cbuild_fs_rmdir(const char* path) {
		int stat = rmdir(path);
		if(stat < 0) {
			cbuild_log_error(
				"Could not remove directory directory \"%s\", error: \"%s\"",
				path, strerror(errno));
			return false;
		}
		return true;
	}
#endif // CBUILD_API_*
CBUILDDEF bool cbuild_dir_remove(const char* path) {
	cbuild_pathlist_t list = {0};
	bool err = cbuild_dir_list(path, &list);
	if(err == false) {
		cbuild_log_error(
			"Could not remove source directory \"%s\", error: \"%s\"", path,
			strerror(errno));
		if(list.data != NULL) {
			cbuild_pathlist_clear(&list);
		}
		return false;
	}
	bool ret = true;
	size_t checkpoint = cbuild_temp_checkpoint();
	for(size_t i = 0; i < list.size; i++) {
		const char* tmppath = cbuild_temp_sprintf("%s/%s", path, list.data[i]);
		cbuild_filetype_t f = cbuild_path_filetype(tmppath);
		if (f == CBUILD_FTYPE_MISSING) {
			CBUILD_UNREACHABLE("cbuild_dir_list should not return invalid files.");
		} else if(f == CBUILD_FTYPE_DIRECTORY) {
			if(!cbuild_dir_remove(tmppath)) ret = false;
		} else {
			if(!cbuild_file_remove(tmppath)) ret = false;
		}
		cbuild_temp_reset(checkpoint);
	}
	if(!__cbuild_fs_rmdir(path)) {
		cbuild_pathlist_clear(&list);
		return false;
	}
	cbuild_pathlist_clear(&list);
	return ret;
}
CBUILDDEF bool cbuild_dir_check(const char* path) {
	return cbuild_file_check(path);
}
CBUILDDEF int __cbuild_fs_compare(const void* a, const void* b) {
	return strcmp(*(const char**)a, *(const char**)b);
}
CBUILDDEF bool cbuild_dir_list(const char* path, cbuild_pathlist_t* elements) {
	cbuild_pathlist_clear(elements);
	cbuild_dir_t dir = cbuild_dir_open(path);
	if (dir == CBUILD_INVALID_DIR) return false;
	const char* file = NULL;
	int ret = 0;
	while ((ret = cbuild_dir_next(dir, &file)) == 0) {
		if (strcmp(file, ".")  == 0) continue;
		if (strcmp(file, "..")  == 0) continue;
		size_t len = strlen(file);
		char* file_copy = __CBUILD_MALLOC(len + 1);
		cbuild_assert(file_copy != NULL, "Allocation failed.\n");
		memcpy(file_copy, file, len);
		file_copy[len] = '\0';
		cbuild_da_append(elements, file_copy);
	}
	qsort(elements->data, elements->size, sizeof(char*), __cbuild_fs_compare);
	if (!cbuild_dir_close(dir)) return false;
	if (ret == -1) return false;
	return true;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF cbuild_dir_t cbuild_dir_open(const char* path) {
		cbuild_dir_t dir = opendir(path);
		if (dir == CBUILD_INVALID_DIR) {
			cbuild_log_error("Failed to open directory '%s': %s",
				path, strerror(errno));
			return CBUILD_INVALID_DIR;
		}
		return dir;
	}
	CBUILDDEF int cbuild_dir_next(cbuild_dir_t dir, const char** element) {
		errno = 0;
		struct dirent* ent = readdir(dir);
		if (ent == NULL) {
			if (errno == 0) return 1;
			cbuild_log_error("Failed to get next element from directory: %s",
				strerror(errno));
			return -1;
		}
		*element = ent->d_name;
		return 0;
	}
	CBUILDDEF bool cbuild_dir_close(cbuild_dir_t dir) {
		if (closedir(dir) != 0) {
			cbuild_log_error("Failed to close directory: %s", strerror(errno));
			return false;
		}
		return true;
	}
#endif // CBUILD_API_*
CBUILDDEF bool __cbuild_dir_walk_opt(cbuild_sb_t* path, size_t level, bool* abort,
	cbuild_dir_walk_func_t func, struct cbuild_dir_walk_opts_t opts) {
	size_t curr_path_len = path->size;
	cbuild_sb_append_null(path);
	cbuild_filetype_t ftype = cbuild_path_filetype(path->data);
	if (!opts.visit_dir_last) {
		enum cbuild_dir_walk_result_t res = CBUILD_DIR_WALK_CONTINUE;
		struct cbuild_dir_walk_func_args_t args = {
			.path = path->data,
			.type = ftype,
			.level = level,
			.result = &res,
			.context = opts.context,
		};
		if (!func(args)) return false;
		switch (res) {
		case CBUILD_DIR_WALK_CONTINUE: break;
		case CBUILD_DIR_WALK_STOP: *abort = true; CBUILD_ATTR_FALLTHROUGH();
		case CBUILD_DIR_WALK_NO_ENTER: return true;
		}
	}
	if (ftype == CBUILD_FTYPE_DIRECTORY) {
		cbuild_dir_t dir = cbuild_dir_open(path->data);
		if (dir == CBUILD_INVALID_DIR) return false;
		const char* file = NULL;
		int ret = 0;
		while ((ret = cbuild_dir_next(dir, &file)) == 0) {
			if (strcmp(file, ".")  == 0) continue;
			if (strcmp(file, "..")  == 0) continue;
			path->size = curr_path_len;
			cbuild_sb_append(path, '/');
			cbuild_sb_append_cstr(path, file);
			if (!__cbuild_dir_walk_opt(path, level + 1, abort, func, opts)) {
				if (!cbuild_dir_close(dir)) return false;
				return false;
			}
			path->size = curr_path_len;
			if (*abort) {
				if (!cbuild_dir_close(dir)) return false;
				return true;
			}
		}
		if (!cbuild_dir_close(dir)) return false;
		if (ret == -1) return false;
	}
	if (opts.visit_dir_last) {
		enum cbuild_dir_walk_result_t res = CBUILD_DIR_WALK_CONTINUE;
		struct cbuild_dir_walk_func_args_t args = {
			.path = path->data,
			.type = ftype,
			.level = level,
			.result = &res,
			.context = opts.context,
		};
		if (!func(args)) return false;
		switch (res) {
		case CBUILD_DIR_WALK_CONTINUE: break;
		case CBUILD_DIR_WALK_STOP: *abort = true; CBUILD_ATTR_FALLTHROUGH();
		case CBUILD_DIR_WALK_NO_ENTER: return true;
		}
	}
	return true;
}
CBUILDDEF bool cbuild_dir_walk_opt(const char* path, cbuild_dir_walk_func_t func,
	struct cbuild_dir_walk_opts_t opts) {
	cbuild_sb_t tmp_path = {0};
	cbuild_sb_append_cstr(&tmp_path, path);
	bool abort = false;
	bool ret = __cbuild_dir_walk_opt(&tmp_path, 0, &abort, func, opts);
	cbuild_sb_clear(&tmp_path);
	return ret;
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF char* cbuild_dir_current(void) {
		#if defined(CBUILD_API_POSIX) && \
			(defined(CBUILD_OS_LINUX_GLIBC) || defined(CBUILD_OS_LINUX_MUSL))
			char* tmp = get_current_dir_name();
			size_t len = strlen(tmp);
			char* ret = __CBUILD_MALLOC(len + 1);
			memcpy(ret, tmp, len + 1);
			free(tmp);
			return ret;
		#else
			size_t size;
			long path_max = pathconf(".", _PC_PATH_MAX);
			if (path_max == -1) size = 1024;
			else if (path_max > 10240) size = 10240;
			else size = (size_t)path_max;
			while(true) {
				char* buf = __CBUILD_MALLOC(size);
				cbuild_assert(buf != NULL, "Allocation failed.\n");
				errno = 0;
				if(getcwd(buf, size) == NULL) {
					if(errno == ERANGE) {
						size = size*2;
					} else {
						free(buf);
						return NULL;
					}
				} else {
					return buf;
				}
			}
			return NULL;
		#endif // Extension check
	}
	CBUILDDEF bool cbuild_dir_set_current(const char* path) {
		if(chdir(path) < 0) {
			cbuild_log_error(
				"Could not set current working directory to \"%s\", error: \"%s\"",
				path, strerror(errno));
			return false;
		}
		return true;
	}
#endif // CBUILD_API_*
CBUILDDEF void cbuild_pathlist_clear(cbuild_pathlist_t* list) {
	for(size_t i = 0; i < list->size; i++) {
		__CBUILD_FREE((void*)list->data[i]);
	}
	cbuild_da_clear(list);
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF bool __cbuild_dir_create(const char* path_, bool inplace) {
		int ret = mkdir(path_, 0755);
		if(ret < 0) {
			if(errno == EEXIST) {
				cbuild_log(CBUILD_LOG_WARN, "Directory \"%s\" exist", path_);
				return false;
			} else if(errno == ENOENT) {
				char* path = (char*)path_;
				size_t checkpoint = cbuild_temp_checkpoint();
				if(!inplace) path = cbuild_path_normalize(path_);
				cbuild_log_level_t old_log_level = cbuild_log_get_min_level();
				if(!inplace) cbuild_log_set_min_level(CBUILD_LOG_ERROR);
				char* slash = strrchr(path, '/');
				if(slash) *slash = '\0';
				if(!__cbuild_dir_create(path, true)) {
					if(!inplace) {
						cbuild_temp_reset(checkpoint);
						cbuild_log_set_min_level(old_log_level);
					}
					return false;
				}
				if(slash) *slash = '/';
				ret = mkdir(path, 0755);
				if(ret == 0) {
					if(!inplace) {
						cbuild_temp_reset(checkpoint);
						cbuild_log_set_min_level(old_log_level);
					}
					return true;
				}
				if(!inplace) {
					cbuild_temp_reset(checkpoint);
					cbuild_log_set_min_level(old_log_level);
				}
			}
			cbuild_log_error(
				"Could not create directory \"%s\", error: \"%s\"", path_,
				strerror(errno));
			return false;
		}
		return true;
	}
	CBUILDDEF bool cbuild_dir_create(const char* path) {
		return __cbuild_dir_create(path, false);
	}
	CBUILDDEF cbuild_filetype_t cbuild_path_filetype(const char* path) {
		struct stat statbuff;
		if(lstat(path, &statbuff) < 0) {
			cbuild_log_error("Could not stat file \"%s\", error: \"%s\"", path,
				strerror(errno));
			return CBUILD_FTYPE_MISSING;
		}
		if(S_ISREG(statbuff.st_mode)) {
			return CBUILD_FTYPE_REGULAR;
		}
		if(S_ISDIR(statbuff.st_mode)) {
			return CBUILD_FTYPE_DIRECTORY;
		}
		if(S_ISLNK(statbuff.st_mode)) {
			return CBUILD_FTYPE_SYMLINK;
		}
		return CBUILD_FTYPE_OTHER;
	}
#endif // CBUILD_API_*
CBUILDDEF char* cbuild_path_ext_opt(const char* path, struct cbuild_path_ext_opts_t opts) {
	if (opts.dot == 0) opts.dot = 1;
	const char* dot = path + strlen(path);
	do {
		dot = __cbuild_memrchr(path, '.', (size_t)(dot - path));
		opts.dot--;
	} while (dot != NULL && opts.dot > 0);
	if(dot == NULL) {
		char* ret = cbuild_temp_malloc(1);
		cbuild_assert(ret != NULL, "Allocation failed.\n");
		memcpy(ret, "\0", 1);
		return ret;
	}
	ptrdiff_t i = dot - path;
	size_t len = strlen(path) - (size_t)i;
	char* ret = cbuild_temp_malloc(len);
	cbuild_assert(ret != NULL, "Allocation failed.\n");
	memcpy(ret, path + i + 1, len);
	ret[len - 1] = '\0';
	return ret;
}
CBUILDDEF char* cbuild_path_name(const char* path) {
	size_t i = strlen(path);
	if(path[i - 1] == '/') {
		i -= 2;
	}
	const char* slash = __cbuild_memrchr(path, '/', i);
	if (slash == NULL) {
		char* ret = cbuild_temp_malloc(i + 1);
		cbuild_assert(ret != NULL, "Allocation failed.\n");
		memcpy(ret, path, i);
		ret[i] = '\0';
		return ret;
	}
	ptrdiff_t base = slash - path;
	size_t len = strlen(path) - (size_t)base;
	char* ret = cbuild_temp_malloc(len);
	cbuild_assert(ret != NULL, "Allocation failed.\n");
	memcpy(ret, path + base + 1, len);
	ret[len - 1] = '\0';
	return ret;
}
CBUILDDEF char* cbuild_path_base(const char* path) {
	size_t i = strlen(path);
	if(path[i - 1] == '/') {
		i -= 2;
	}
	const char* slash = __cbuild_memrchr(path, '/', i);
	if(slash == NULL) {
		char* ret = cbuild_temp_malloc(1);
		cbuild_assert(ret != NULL, "Allocation failed.\n");
		memcpy(ret, "\0", 1);
		return ret;
	}
	slash++;
	size_t len = (size_t)(slash - path) + 1;
	char* ret = cbuild_temp_malloc(len);
	cbuild_assert(ret != NULL, "Allocation failed.\n");
	memcpy(ret, path, len - 1);
	ret[len - 1] = '\0';
	return ret;
}
typedef struct __cbuild_stack_cstr_t {
	cbuild_sv_t* data;
	size_t ptr;
	size_t capacity;
} __cbuild_stack_cstr_t;
CBUILDDEF char* cbuild_path_normalize(const char* path_) {
	cbuild_sb_t buff = {0};
	__cbuild_stack_cstr_t dirs = {0};
	cbuild_sv_t path = cbuild_sv_from_cstr(path_);
	// Windows paths can have drive letter
	// Drive letter is only one character
	if (isalpha((unsigned char)path.data[0]) && path.data[1] == ':') {
		cbuild_sb_append_arr(&buff, path.data, 2);
		path.data += 2;
		path.size -= 2;
	}
	if(*path.data == '/') {
		cbuild_sb_append(&buff, '/');
	}
	// Unix paths threat double slash differently
	// POSIX threats paths starting with '//' specially.
	if(cbuild_sv_prefix(path, cbuild_sv_from_lit("//")) &&
		!cbuild_sv_prefix(path, cbuild_sv_from_lit("///"))) {
		cbuild_sb_append(&buff, '/');
	}
	do {
		cbuild_sv_t dir = cbuild_sv_chop_by_delim(&path, '/');
		if(dir.size == 0) continue;
		if(cbuild_sv_cmp(dir, cbuild_sv_from_lit(".")) == 0) {
			// Do nothing
		} else if(cbuild_sv_cmp(dir, cbuild_sv_from_lit("..")) == 0) {
			if(dirs.ptr == 0) { // Underflow
				cbuild_sb_append_cstr(&buff, "../");
				// Underflow on absolute path is undefined anyway
				// and on relative path we can guarantee that this will be fine
				// (we have nothing in directory stack anyway)
			} else {
				cbuild_stack_pop(&dirs);
			}
		} else {
			cbuild_stack_push(&dirs, dir);
		}
	} while(path.size > 0);
	for(size_t i = 0; i <	dirs.ptr; i++) {
		cbuild_sb_appendf(&buff, CBuildSVFmt"/", CBuildSVArg(dirs.data[i]));
	}
	if(buff.size == 0) cbuild_sb_append(&buff, '.');
	if(!((buff.size == 1 && buff.data[0] == '/') ||
			(buff.size == 2 && buff.data[0] == '/' && buff.data[1] == '/') ||
			(buff.size == 3 && isalpha((unsigned char)buff.data[0]) && 
				buff.data[1] == ':' && buff.data[2] == '/')) &&
		(buff.data[buff.size - 1] == '/')) buff.size--;
	cbuild_stack_clear(&dirs);
	char* ret = cbuild_temp_sprintf(CBuildSBFmt, CBuildSBArg(buff));
	cbuild_sb_clear(&buff);
	return ret;
}

/* Map.h */

//! License: `GPL-3.0-or-later`.

// #include "Compile.h"
// #include "Common.h"
// #include "Log.h"
// #include "StringBuilder.h"
// #include "DynArray.h"
// #include "FS.h"
// #include "Command.h"
CBUILDDEF void __cbuild_int_compile_mark_exec(const char* file);
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF void __cbuild_int_compile_mark_exec(const char* file) {
		if(chmod(file, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH |
				S_IXOTH) != 0) {
			cbuild_log_error("Could not mark file \"%s\" as executable, error: \"%s\".",
				file, strerror(errno));
		}
	}
#endif // CBUILD_API_*
void (*cbuild_selfrebuild_hook)(cbuild_cmd_t* cmd) = NULL;
CBUILDDEF void __cbuild_selfrebuild(int argc, char** argv,
	const char* argv0_path, cbuild_cmd_t files) {
	// Prepare paths
	const char* source = files.data[0];
	const char* output = argv0_path;
	cbuild_sb_t backup = {0};
	cbuild_sb_append_cstr(&backup, output);
	cbuild_sb_append_cstr(&backup, ".old");
	cbuild_sb_append_null(&backup);
	// Check if recompilation is needed
	int cond = cbuild_compare_mtime_many(output, files.data, files.size);
	if(cond < 0) {
		cbuild_log_error("Error while performing self-rebuild.");
		cbuild_sb_clear(&backup);
		exit(1);
	}
	if(cond == 0) {
		cbuild_sb_clear(&backup);
		return;
	}
	cbuild_log_info("Rebuilding CBuild buildscript.");
	// Make backup of a binary
	if(!cbuild_file_rename(output, backup.data)) {
		if(!cbuild_file_check(output)) {
			cbuild_log_warn("File %s does not exists, skipping rename step", output);
		} else {
			cbuild_log_error("Could not rename old buildscript, aborting.");
			exit(1);
		}
	}
	// Actual rebuild
	cbuild_cmd_t cmd = {0};
	cbuild_cmd_append_many(&cmd, CBUILD_CC, CBUILD_SELFREBUILD_ARGS);
	if(cbuild_selfrebuild_hook != NULL) {
		cbuild_selfrebuild_hook(&cmd);
	}
	cbuild_cmd_append_many(&cmd, "-o", output, source);
	if(!cbuild_cmd_run(&cmd)) {
		cbuild_file_rename(backup.data, output);
		cbuild_log_error("----------------------------------------");
		cbuild_log_error("Rebuild failed.");
		cbuild_log_error("----------------------------------------");
		return; // If compilation failed the let old version run
	}
	// Re-exec into new binary
	cbuild_sb_clear(&backup);
	__cbuild_int_compile_mark_exec(output);
	// Here real argv[0] used because this can be executed (it was just executed)
	// and argv0_path can be absolute path, which will look bad inside a command
	cbuild_cmd_append_arr(&cmd, argv, (size_t)argc);
	if(!cbuild_cmd_run(&cmd)) {
		// NOTE: This hides app exit code. But this is unfixable in current API
		exit(1);
	}
	exit(0);
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF int cbuild_compare_mtime(const char* output, const char* input) {
		struct stat statbuff;
		if(stat(input, &statbuff) < 0) {
			cbuild_log_error("Could not stat file \"%s\", error: \"%s\"",
				input, strerror(errno));
			return -1;
		}
		time_t in_mtime = statbuff.st_mtime;
		if(stat(output, &statbuff) < 0) {
			if(errno == ENOENT) {
				return 1;
			}
			cbuild_log_error("Could not stat file \"%s\", error: \"%s\"",
				output, strerror(errno));
			return -1;
		}
		if(in_mtime > statbuff.st_mtime) {
			return 1;
		} else {
			return 0;
		}
	}
#endif //CBUILD_API_*
CBUILDDEF int cbuild_compare_mtime_many(const char* output, const char** inputs,
	size_t num_inputs) {
	int ret = 0;
	for(size_t i = 0; i < num_inputs; i++) {
		int check = cbuild_compare_mtime(output, inputs[i]);
		if(check < 0) {
			return check;
		} else {
			ret += check;
		}
	}
	return ret;
}

/* Proc.h */

//! License: `GPL-3.0-or-later`.

// #include "DLLoad.h"
// #include "Common.h"

#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
CBUILDDEF cbuild_dlib_t cbuild_dlib_open(const char* filename, bool eager) {
	if (eager) {
		return dlopen(filename, RTLD_NOW);
	} else {
		return dlopen(filename, RTLD_LAZY);
	}
}
CBUILDDEF void* cbuild_dlib_get_sym(cbuild_dlib_t handle, const char* name) {
	return dlsym(handle, name);
}
CBUILDDEF void cbuild_dlib_close(cbuild_dlib_t handle) {
	cbuild_assert(dlclose(handle) == 0, "Failed to close dynamic library.\n");
}
#endif // CBUILD_API_*

/* Command.h */

//! License: `GPL-3.0-or-later`.

// #include "FlagParse.h"
// #include "Common.h"
// #include "StringBuilder.h"
// #include "DynArray.h"
// #include "Log.h"
// #include "FS.h"
struct __cbuild_flag_t {
	bool found;
	// uint8_t __padding[3];
	const char* option;
	struct cbuild_flag_new_opts_t spec;
	cbuild_arglist_t args;
};
struct __cbuild_flags_t {
	struct __cbuild_flag_t* data;
	size_t size;
	size_t capacity;
};
struct __cbuild_flag_context_t {
	bool pass_separator;
	// uint8_t __padding[3];
	const char* app_name;
	cbuild_flag_print_func_t help;
	cbuild_flag_print_func_t version;
	cbuild_arglist_t pargs;
	struct __cbuild_flags_t flags;
};
struct __cbuild_flag_context_t __cbuild_flag_context;
CBUILDDEF void cbuild_flag_new_opt(const char* option, struct cbuild_flag_new_opts_t opts) {
	struct __cbuild_flag_t flag = {0};
	flag.option = option;
	flag.spec = opts;
	cbuild_da_append(&__cbuild_flag_context.flags, flag);
}
CBUILDDEF void cbuild_flag_set_option(enum cbuild_flag_options_t option, ...) {
	va_list args;
	va_start(args, option);
	switch (option) {
	case CBUILD_FLAG_PASS_SEPARATOR: {
		__cbuild_flag_context.pass_separator = true;
	} break;
	case CBUILD_FLAG_HELP_HOOK: {
		cbuild_flag_print_func_t f = va_arg(args,cbuild_flag_print_func_t);
		__cbuild_flag_context.help = f;
	} break;
	case CBUILD_FLAG_VERSION_HOOK: {
		cbuild_flag_print_func_t f = va_arg(args,cbuild_flag_print_func_t);
		__cbuild_flag_context.version = f;
	} break;
	}
	va_end(args);
}
#if defined(CBUILD_API_POSIX) || defined(CBUILD_API_STRICT_POSIX)
	CBUILDDEF unsigned short __cbuild_flag_term_width(void) {
		#if defined(TIOCGWINSZ)
			cbuild_fd_t fd = cbuild_fd_open_read("/dev/tty");
			if (fd == CBUILD_INVALID_FD) return 80;
			struct winsize w;
			if (ioctl(fd, TIOCGWINSZ, &w) == -1) {
				cbuild_fd_close(fd);
				return 80;
			}
			cbuild_fd_close(fd);
			return w.ws_col;
		#else
			return 80;
		#endif // Extension check
	}
	void __cbuild_flag_term_get_cursor(int* x, int* y) {
		struct termios orig = {0};
		tcgetattr(STDIN_FILENO, &orig);
		struct termios new = orig;
		new.c_lflag &= ~((unsigned int)ICANON | (unsigned int)ECHO);
		new.c_cc[VMIN] = 1;
		new.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		const char req[] = "\e[6n";
		write(STDOUT_FILENO, req, sizeof(req));
		char resp[9] = {0}; 
		cbuild_assert(read(STDIN_FILENO, resp, 8) >= 0,
			"Can not read from stdin.\n");
		sscanf(resp, "\033[%d;%dR", x, y);
		tcsetattr(STDIN_FILENO, TCSANOW, &orig);
	}
#endif // CBUILD_API_*
CBUILDDEF void __cbuild_flag_help(const char* name) {
	__CBUILD_PRINTF("Usage: %s [OPTIONS]\n\n", name);
	cbuild_flag_print_help();
}
CBUILDDEF void __cbuild_flag_version(const char* name) {
	__CBUILD_PRINTF("%s - v1.0\n", name);
}
CBUILDDEF struct __cbuild_flag_t* __cbuild_flag_lookup_flag(const char* name) {
	cbuild_da_foreach(__cbuild_flag_context.flags, flag) {
		if (strcmp(flag->option, name) == 0) {
			return flag;
		}
	}
	return NULL;
}
CBUILDDEF struct __cbuild_flag_t* __cbuild_flag_lookup_sflag(char name) {
	cbuild_da_foreach(__cbuild_flag_context.flags, flag) {
		if (flag->spec.short_option == name) {
			return flag;
		}
	}
	return NULL;
}
CBUILDDEF void __cbuild_flag_parse_arguments(struct __cbuild_flag_t* flag,
	char** argv, int argc, int* parser) {
	// NOTE: This technically allows TList to terminate on one condition besides
	// terminator - end of flags stream. This should not cause too much problems.
	int found = 0;
	while (argc > *parser) {
		char* arg = NULL;
		if (flag->spec.terminator) {
			arg = argv[(*parser)++];
			if (strcmp(arg, flag->spec.terminator) == 0) break;
		} else {
			if (found == flag->spec.num_arguments) break;
			arg = argv[(*parser)++];
			if (arg[0] == '-') {
				(*parser)--;
				break;
			}
		}
		cbuild_da_append(&flag->args, arg);
		found++;
	}
	// Arguments are options and none were found
	if (flag->spec.optional && found == 0) return;
	// Any number of arguments are fine
	if (flag->spec.num_arguments == -1) return;
	// Enough arguments found - this is fine
	if (found == flag->spec.num_arguments) return;
	// More arguments found - can be triggered only for a TList
	if (found > flag->spec.num_arguments) {
		cbuild_log_error(
			
			"Too many arguments for flag --%s, %d provided but %d expected.",
			flag->option, found, flag->spec.num_arguments);
		__cbuild_flag_context.help(__cbuild_flag_context.app_name);
		exit(1);
	}
	// Less arguments found
	if (found < flag->spec.num_arguments) {
		cbuild_log_error(
			
			"Too few arguments for flag --%s, %d provided but %d expected.",
			flag->option, found, flag->spec.num_arguments);
		__cbuild_flag_context.help(__cbuild_flag_context.app_name);
		exit(1);
	}
}
CBUILDDEF void cbuild_flag_parse(int argc, char** argv) {
	// Register built-in flags and parameters
	cbuild_flag_new("help", .short_option = 'h', .desc = "Print this message.");
	cbuild_flag_new("version", .short_option = 'v',
		.desc = "Print version information.");
	if (__cbuild_flag_context.help == NULL) {
		cbuild_flag_set_option(CBUILD_FLAG_HELP_HOOK, __cbuild_flag_help);
	}
	if (__cbuild_flag_context.version == NULL) {
		cbuild_flag_set_option(CBUILD_FLAG_VERSION_HOOK, __cbuild_flag_version);
	}
	// Parse flags
	__cbuild_flag_context.app_name = argv[0];
	bool parse_flags = true;
	int parser = 1;
	while (parser < argc) {
		char* arg = argv[parser++];
		if (!parse_flags) { // Parser is inhibited
			cbuild_da_append(&__cbuild_flag_context.pargs, arg);
		} else if (strcmp(arg, "--") == 0) { // Special case - '--'
			parse_flags = false;
			if (__cbuild_flag_context.pass_separator) {
				cbuild_da_append(&__cbuild_flag_context.pargs, arg);
			}
		} else if (arg[0] == '-' && arg[1] == '-') { // Long option flag
			arg += 2;
			char* equal = strchr(arg, '=');
			if (equal != NULL) {
				*equal = '\0';
				struct __cbuild_flag_t* flag = __cbuild_flag_lookup_flag(arg);
				if (flag == NULL) {
					cbuild_log_error("Invalid flag specified: --%s", arg);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
					continue;
				}
				if (flag->found && !flag->spec.repeat) {
					cbuild_log_error("Flag --%s does not support being repeated.", arg);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				}
				flag->found = true;
				if (flag->spec.terminator != NULL) {
					cbuild_log_error(
						"Terminated list can not take arguments with \"--%s=argument\".", arg);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				} else if (flag->spec.num_arguments == 0) {
					cbuild_log_error(
						"Too many arguments for flag --%s, 1 provided but %d expected.",
						arg, flag->spec.num_arguments);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				} else if (flag->spec.num_arguments > 1) {
					cbuild_log_error(
						"Too few arguments for flag --%s, 1 provided but %d expected.",
						arg, flag->spec.num_arguments);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				}
				cbuild_da_append(&flag->args, equal + 1);
				*equal = '=';
			} else {
				struct __cbuild_flag_t* flag = __cbuild_flag_lookup_flag(arg);
				if (flag == NULL) {
					cbuild_log_error("Invalid flag specified: --%s.", arg);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
					continue;
				}
				if (flag->found && !flag->spec.repeat) {
					cbuild_log_error("Flag --%s does not support being repeated.", arg);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				}
				flag->found = true;
				__cbuild_flag_parse_arguments(flag, argv, argc, &parser);
			}
		} else if (arg[0] == '-') { // Short option flag
			arg += 1;
			char* equal = strchr(arg, '=');
			if (equal != NULL) *equal = '\0';
			size_t len = strlen(arg);
			for (size_t i = 0; i < len; i++) {
				struct __cbuild_flag_t* flag = __cbuild_flag_lookup_sflag(arg[i]);
				if (flag == NULL) {
					cbuild_log_error("Invalid flag specified: -%c.", arg[i]);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
					continue;
				}
				if (flag->found && !flag->spec.repeat) {
					cbuild_log_error("Flag -%c does not support being repeated.", arg[i]);
					__cbuild_flag_context.help(__cbuild_flag_context.app_name);
					exit(1);
				}
				flag->found = true;
				if ((i + 1) == len) {
					if (equal != NULL) {
						if (flag->spec.terminator != NULL) {
							cbuild_log_error(
								"Terminated list can not take arguments with \"-%c=argument\".", arg[i]);
							__cbuild_flag_context.help(__cbuild_flag_context.app_name);
							exit(1);
						} else if (flag->spec.num_arguments == 0) {
							cbuild_log_error(
								"Too many arguments for flag -%c, 1 provided but %d expected.",
								arg[i], flag->spec.num_arguments);
							__cbuild_flag_context.help(__cbuild_flag_context.app_name);
							exit(1);
						} else if (flag->spec.num_arguments > 1) {
							cbuild_log_error(
								"Too few arguments for flag -%c, 1 provided but %d expected.",
								arg[i], flag->spec.num_arguments);
							__cbuild_flag_context.help(__cbuild_flag_context.app_name);
							exit(1);
						}
						cbuild_da_append(&flag->args, equal + 1);
					} else {
						__cbuild_flag_parse_arguments(flag, argv, argc, &parser);
					}
				} else {
					if (flag->spec.num_arguments != 0 && !flag->spec.optional) {
						cbuild_log_error(
							"Too few arguments for flag -%c, 1 provided but %d expected.",
							arg[i], flag->spec.num_arguments);
						__cbuild_flag_context.help(__cbuild_flag_context.app_name);
						exit(1);
					}
				}
			}
			if (equal != NULL) *equal = '=';
		} else { // Positional argument
			cbuild_da_append(&__cbuild_flag_context.pargs, arg);
		}
	}
	// Check built-in flags
	if (cbuild_flag_get_flag("help")) {
		__cbuild_flag_context.help(__cbuild_flag_context.app_name);
		exit(0);
	}
	if (cbuild_flag_get_flag("version")) {
		__cbuild_flag_context.version(__cbuild_flag_context.app_name);
		exit(0);
	}
}
// 'len' is length of column with flag values, 30 max (if flag is longer than it should just wrap first line a little differently, no need to penalize all flags.
CBUILDDEF char* __cbuild_flag_fmt_flag(struct __cbuild_flag_t* flag, int len) {
	cbuild_sb_t sb = {0};
	cbuild_sb_append_cstr(&sb, "  ");
	if (flag->spec.short_option != '\0') {
		cbuild_sb_appendf(&sb, "-%c, ", flag->spec.short_option);
	}
	cbuild_sb_appendf(&sb, "--%s", flag->option);
	if (flag->spec.num_arguments) {
		cbuild_sb_append(&sb, ' ');
		if (flag->spec.optional) cbuild_sb_append(&sb, '[');
		else cbuild_sb_append(&sb, '<');
		if (flag->spec.argument_desc) cbuild_sb_append_cstr(&sb, flag->spec.argument_desc);
		else cbuild_sb_append_cstr(&sb, "ARGUMENT");
		if (flag->spec.terminator) {
			cbuild_sb_appendf(&sb, "|%s", flag->spec.terminator);
		}
		if (flag->spec.optional) cbuild_sb_append(&sb, ']');
		else cbuild_sb_append(&sb, '>');
	}
	for (int i = (int)sb.size; i < len; i++) cbuild_sb_append(&sb, ' ');
	cbuild_sb_append(&sb, ' ');
	size_t col1w = sb.size;
	ssize_t desc_len = (ssize_t)strlen(flag->spec.desc);
	const char* desc = flag->spec.desc;
	const size_t line_len = __cbuild_flag_term_width() - col1w;
	while (desc_len > 0) {
		if (desc != flag->spec.desc) { // This is means this is not a first iteration
			for (size_t i = 0; i < col1w; i++) cbuild_sb_append(&sb, ' ');
		}
		cbuild_sb_append_arr(&sb, desc, CBUILD_MIN(line_len, (size_t)desc_len));
		cbuild_sb_append(&sb, '\n');
		desc += line_len;
		desc_len -= (ssize_t)line_len;
	}
	cbuild_sb_append_null(&sb);
	return sb.data;
}
CBUILDDEF void cbuild_flag_print_help(void) {
	// Measure size of flag name column
	int name_len = 0;
	cbuild_da_foreach(__cbuild_flag_context.flags, flag) {
		int flag_len = 3; // '--' and space after it.
		if (flag->spec.short_option != '\0') flag_len += 4;
		flag_len += (int)strlen(flag->option);
		if (flag->spec.num_arguments != 0) {
			flag_len += 3; // ' <...>' or ' [...]'
			if (flag->spec.argument_desc) flag_len += (int)strlen(flag->spec.argument_desc);
			else flag_len += 8;
			if (flag->spec.terminator != NULL) {
				flag_len += 1; // 'Terminator is shown as 'desc|terminator'
				flag_len += (int)strlen(flag->spec.terminator);
			}
		} else {
			flag_len += 1;
		}
		name_len = CBUILD_MAX(name_len, flag_len);
	}
	name_len = CBUILD_MIN(name_len, 30); // 30 is just arbitrary number.
	// Print ungrouped arguments and collect list of groups
	cbuild_arglist_t groups = {0};
	__CBUILD_PRINT("Flags:\n");
	cbuild_da_foreach(__cbuild_flag_context.flags, flag) {
		if (flag->spec.group == NULL) {
			char* desc = __cbuild_flag_fmt_flag(flag, name_len);
			__CBUILD_PRINT(desc);
			__CBUILD_FREE(desc);
		} else {
			bool group_found = false;
			cbuild_da_foreach(groups, group) {
				if (strcmp(*group, flag->spec.group) == 0) {
					group_found = true;
					break;
				}
			}
			if (!group_found) cbuild_da_append(&groups, (char*)flag->spec.group);
		}
	}
	// Print all grouped flags
	cbuild_da_foreach(groups, group) {
		__CBUILD_PRINTF("%s\n", *group);
		cbuild_da_foreach(__cbuild_flag_context.flags, flag) {
			if (flag->spec.group) {
				if (strcmp(flag->spec.group, *group) == 0) {
					char* desc = __cbuild_flag_fmt_flag(flag, name_len);
					__CBUILD_PRINT(desc);
					__CBUILD_FREE(desc);
				}
			}
		}
	}
	cbuild_da_clear(&groups);
}
CBUILDDEF cbuild_arglist_t* cbuild_flag_get_pargs(void) {
	return &__cbuild_flag_context.pargs;
}
CBUILDDEF cbuild_arglist_t* cbuild_flag_get_flag(const char* opt) {
	struct __cbuild_flag_t* flag = __cbuild_flag_lookup_flag(opt);
	if (flag == NULL) return NULL;
	if (!flag->found) return NULL;
	return &flag->args;
}
CBUILDDEF const char* cbuild_flag_app_name(void) {
	return __cbuild_flag_context.app_name;
}

/* FS.h */

//! License: `GPL-3.0-or-later`.

// #include "RGlob.h"
// #include "Common.h"
// #include "StringBuilder.h"
// #include "DynArray.h"

CBUILDDEF bool cbuild_glob_compile_opt(cbuild_glob_t* glob, const char* pattern,
	struct cbuild_glob_opts_t opts) {
	cbuild_sb_t regex = {0};
	enum {
		PARSE_NORMAL,
		PARSE_ESCAPE,
		PARSE_CHAR_CLASS,
		PARSE_CHAR_CLASS_FIRST,
		PARSE_CHAR_CLASS_AFTER_LAST,
	} state = PARSE_NORMAL;
	if (!opts.partial_match) cbuild_sb_append(&regex, '^');
	for(const char* pptr = pattern; *pptr != '\0'; pptr++) {
		if (state == PARSE_CHAR_CLASS_AFTER_LAST) {
			state = PARSE_NORMAL;
			if (*pptr == '*') {
				cbuild_sb_append(&regex, *pptr);
				continue;
			}
		}
		if (state == PARSE_ESCAPE) {
			cbuild_sb_append(&regex, *pptr);
			state = PARSE_NORMAL;
			continue;
		}
		if (state == PARSE_CHAR_CLASS_FIRST && *pptr == '!') {
			cbuild_sb_append(&regex, '^');
			state = PARSE_CHAR_CLASS;
			continue;
		}
		if ((state == PARSE_CHAR_CLASS || state == PARSE_CHAR_CLASS_FIRST)
			&& *pptr != ']') {
			cbuild_sb_append(&regex, *pptr);
			state = PARSE_CHAR_CLASS; // Clears GPST_CHAR_CLASS_FIRST
			continue;
		}
		switch(*pptr) {
		case '#': {
			state = PARSE_ESCAPE;
		} break;
		case '[': {
			cbuild_sb_append(&regex, '[');
			state = PARSE_CHAR_CLASS_FIRST;
		} break;
		case ']': {
			cbuild_sb_append(&regex, ']');
			state = PARSE_CHAR_CLASS_AFTER_LAST;
		} break;
		case '*': {
			cbuild_sb_append_cstr(&regex, ".*");
		} break;
		case '?': {
			cbuild_sb_append(&regex, '.');
		} break;
		case '(': CBUILD_ATTR_FALLTHROUGH();
		case ')': CBUILD_ATTR_FALLTHROUGH();
		case '{': CBUILD_ATTR_FALLTHROUGH();
		case '}': CBUILD_ATTR_FALLTHROUGH();
		case '^': CBUILD_ATTR_FALLTHROUGH();
		case '$': CBUILD_ATTR_FALLTHROUGH();
		case '.': CBUILD_ATTR_FALLTHROUGH();
		case '|': CBUILD_ATTR_FALLTHROUGH();
		case '+': {
			cbuild_sb_append(&regex, '\\');
			cbuild_sb_append(&regex, *pptr);
		} break;
		default: {
			cbuild_sb_append(&regex, *pptr);
		} break;
		}
	}
	if (!opts.partial_match) cbuild_sb_append(&regex, '$');
	cbuild_sb_append_null(&regex);
	int ecode = regcomp(&glob->regex, regex.data, REG_EXTENDED | REG_NEWLINE);
	cbuild_sb_clear(&regex);
	return ecode == 0;
}
CBUILDDEF bool cbuild_glob_match(cbuild_glob_t* glob, const char** list, size_t size) {
	for (size_t i = 0; i < size; i++) {
		int ecode = regexec(&glob->regex, list[i], 
			CBUILD_GLOB_CAPTURE_COUNT, glob->captures, 0);
		if (ecode == 0) {
			struct __cbuild_glob_res_t res = {0};
			res.res = list[i];
			res.res_idx = i;
			memcpy(res.captures, glob->captures, sizeof(glob->captures));
			cbuild_da_append(glob, res);
		}
	}
	return glob->size != 0;
}
CBUILDDEF bool cbuild_glob_match_single(cbuild_glob_t* glob, const char* elem) {
	int ecode = regexec(&glob->regex, elem,
		CBUILD_GLOB_CAPTURE_COUNT, glob->captures, 0);
	return ecode == 0;
}
CBUILDDEF void cbuild_glob_free(cbuild_glob_t* glob) {
	cbuild_da_clear(glob);
	regfree(&glob->regex);
}
#endif // CBUILD_IMPLEMENTATION