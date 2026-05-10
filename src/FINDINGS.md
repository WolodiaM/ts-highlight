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
