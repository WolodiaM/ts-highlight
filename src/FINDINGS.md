# This file contains some information about both Tree-Sitter and NeoVim's query format I was not able to find anywhere or which are explained porrly in official documentation

## Tree-Sitter

Both predicates and directives are same entity and named predicates.

If quantified capture, like `(comment)+ @comment`, is used then all of captured nodes are inlined into resulting array of `TSQueryCapture`.

While all predicates/directives start from `#` this `#` character is not included into string returned by `ts_query_string_value_for_id`.

Predicates can be treated as small VM with 3 instruction encoding - string, capture and done:
* string: `const char* ts_query_string_value_for_id(TSQuery* query, uint32_t TSQueryPredicateStep.value_id, &uint32_t string length)` is used to retrieve string. It is length-based string.
* capture: `const char* ts_query_capture_name_for_id(TSQuery* query, uint32_t TSQueryPredicateStep.value_id, &uint32_t string length)` is used to retrieve capture name as a string. It is length-based string. You need to manually find capture in captures array.
* done: It just marks end of predicate "subprogram".

## NeoVim queries

`eq?` technically has 3 variants. Two of them take one capture and one string. In these cases you need to iterate over all captures and if it is one of captures that is compared, its content is compared to string. If it didn't match - predicate fails. In third case it takes two captures and from what I understand you need to have outer loop that finds all captures with first name and inner that finds all captures with second name and then compare all possible combinations.

`; inherits: <lang a>,<lang b>` allows this query to "inherit" other ones. From what I understand it just append content of that query to this in order. It is often used in languages based on another (like `typescript` inherits `ecma`) or to have "base" query used by multiple other higher-level queries (lie both `html` and `vue` inherits `html_tags`).

It seems like it is an undefined behaviour to use quantified capture in `any-of?`.

`offset!` seems to always have positive numbers for first two values and negative for next two values.

It seems like `%b` is unused in lua patters which make it possible to implement them on top of PCRE2.

':echo matchstr("string", "regex")'                   -> Either returns match or empty string

### Used regexes

Vim's regex is complex, and implemented only by vim. SO I implemented partial support over pcre2. This section contains some interesting and non-trivial regex found in nvim-treesitter. It sems to default to "very magic" mode, which simplify things a lot.

* `^(!?\\=|-[a-zA-Z]+)$`: This `\\=` is annoying, why not use `*` here.
* `^[A-Z][A-Za-z_0-9]{2,}$`: Open-ended range `{2,}`.
* `^assert[A-Za-z_0-9]*|error|info|debug|print|warning|warning_once$`: Just pretty complex regex with `|` in top scope.
* `/\\*!([a-zA-Z]+:)?re2c`: Optional groups are supported, but this is quite normal, just interesting regex (at first I though that `re2c` have some mening, but it turned out to be just literal text).
* `(cl:|cffi:)?(with-accessors|with-foreign-objects|let[*]?)`: `[*]` to mask `8`, othervise just set of capture groups.
* `\\c^Comments$"`: Caseless match (probably ;) ). From what I saw `\\c` is always first two characters
* `^(:|v-bind|v-|\\@)`: This `\\@` will be tricky.
* `^[A-Z][A-Z0-9_]+$|^[a-z]{2,3}[A-Z].+$`: Just more complicated regex.
* `^((GPVAL|MOUSE|FIT)_\\w+|GNUTERM|NaN|VoxelDistance|GridDistance|pi)$`: Nested capture groups (hope pcre2 support this ;) ) and `\\w` (this one seems to supported by pcre2).
* `^[eE][nN][dD]([dD][oO])?$`: Pretty cool way to do caseless comparison is regex without any special support! Just cool regex, nothing special here.
* `\\c^(1|on|yes|true|y|0|off|no|false|n|ignore|notfound|.*-notfound)$`: Another `\\c` usage and complex candidates for next entry.
