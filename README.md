# Code highlighter using Tree-Sitter and NeoVim's queries

You can see `src/test.c` for usage example.

Use `cc cbuild.c -o cbuild.run` to bootstrap buildsystem. Then you can use `./cbuild.run test <file>` to try to parse file and get list of its spans (only on priority 100) or `./cbuild build` to amalgamate into single-header library.

To use you need to have `libpcre2-8`, `libtree-sitter` and `libdl` on your system.

## NOTE

This does not implement everything, it is fast enough and compatible enough for my needs. In future I plan to improve compatibility and correctness, but speed seems fast-enough for me.

Vim regex is supported only partially (mostly unsupported, actually), but this seems enough looking at queries from `nvim-treesitter`.

## LICENSE

GPL-3.0-or-later
