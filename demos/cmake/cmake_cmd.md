# cmake cmds

## standard

```bash
set (CMAKE_CXX_STANDARD 11)
```

## install path

```bash
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

or

cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/output
```

## virables

```bash
cmake -LH .
```

## cross compile

cmake -DCMAKE_TOOLCHAIN_FILE=<crossfile>

## cmake --build

```bash
# cmake build with mul threads
cmake --build . -- -j4
```