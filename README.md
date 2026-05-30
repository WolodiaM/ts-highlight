# Code highlighter using Tree-Sitter and NeoVim's queries

You can see `src/test.c` for usage example.

Use `cc cbuild.c -o cbuild.run` to bootstrap buildsystem.

Then you can use `./cbuild.run test <file> [language or 'markdown' if not set]` to try to parse file and get list of its spans (only on priority 100).

You can also use `./cbuild.run print <file> [language or 'markdown' if not set]` to print highlighted file content to terminal.

To amalgamate code into two single-header libraries use `./cbuild build`. It will create `ts-highlight.h` (parser that returns metadata) and `ts-highlight-print.h` (core of "printer", that takes metadata from parser and string and can format it into terminal using ANSI codes). 

To use you need to have `libpcre2-8`, `libtree-sitter` and `libdl` on your system.

Please note, that all both `test` and `run` subcommands expect neovim and nvim-treesitter to be installed, and to have parsers in `$HOME/.local/share/nvim/site/parser/<language>.so` and  queries in `$HOME/.local/share/nvim/site/pack/core/opt/nvim-treesitter/runtime/queries/<language name>/`. To change this you need to modify paths (or logic if more substantial change is needed) in `src/test.c` and `src/print.c` in functions `load_parser` and `get_query_dir`.

## NOTE

`print.c` uses colors taken directly from [gruvbox.nvim](https://github.com/ellisonleao/gruvbox.nvim), which is licensed under the MIT license.

## NOTE 1

This does not implement everything, it is fast enough and compatible enough for my needs. In future I plan to improve compatibility and correctness, but speed seems fast-enough for me.

Vim regex is supported only partially (mostly unsupported, actually), but this seems enough looking at queries from `nvim-treesitter`. If regex matches all requirements of pcre2 it is supported. Also few quirks are provided for regex found in `nvim-treesitter` repo. Lua patters are also not fully supported, but only missing feature is `%b` which matches balanced parentheses (which seems to not be used and probably are not even needed in treesitter queries).

## LICENSE

GPL-3.0-or-later
