# fmtlib

## syntax

ref: https://fmt.dev/12.0/syntax/

```bash
replacement_field ::= "{" [arg_id] [":" (format_spec | chrono_format_spec)] "}"
arg_id            ::= integer | identifier
integer           ::= digit+
digit             ::= "0"..."9"
identifier        ::= id_start id_continue*
id_start          ::= "a"..."z" | "A"..."Z" | "_"
id_continue       ::= id_start | digit
```

```bash
format_spec ::= [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
fill        ::= <a character other than '{' or '}'>
align       ::= "<" | ">" | "^"
sign        ::= "+" | "-" | " "
width       ::= integer | "{" [arg_id] "}"
precision   ::= integer | "{" [arg_id] "}"
type        ::= "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" |
                "g" | "G" | "o" | "p" | "s" | "x" | "X" | "?"
```
