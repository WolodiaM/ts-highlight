# Code highlighter using Tree-Sitter and NeoVim's queries

You can see `src/main.c` for usage example.

Use `cc cbuild.c -o cbuild.run` to bootstrap buildsystem. Then you can use `./cbuild.run run <file>` to try to parse file and get list of its spans (only on priority 100) or `./cbuild.run build` to amalgamate into single-header library.

## NOTE

For now it depend on NeoVim to evaluate complex predicate (just calls it from command line). This will be fixed in future.

## LICENSE

GPL-3.0-or-later
