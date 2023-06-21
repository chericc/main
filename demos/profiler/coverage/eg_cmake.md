# eg_cmake

## options

```bash
add_compile_options(--coverage)
add_link_options(--coverage)
```

## run and parse

```bash
lcov -c -d . -o test.info
genhtml test.info -o html
```