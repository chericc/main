# vscode 配置记录

## vscode 启用 clangd

相比推荐的 C/C++ 插件，clangd 在功能可用性上有质的提升。

首先在 ubuntu 上安装 clangd

```bash
# first install clangd
apt-get install clangd-11

# set clangd11 as default
update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-11 100
```


clangd 的一些默认配置并不是很好用，需要调整一下

在配置文件中增加启动参数：

```bash
--header-insertion=never --completion-style=bundled
```

## plantuml with remote mode on ubuntu 20.04

- Step 0x1

Install plantuml extension in vscode.

- Step 0x2

Install graphviz on ubuntu.

`sudo apt-get install graphviz`

- Step 0x3

Install jre on ubuntu.

`sudo apt-get install openjdk-8-jre`

- Step 0x4

Enjoy it.

## includepath

```json

{
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
}

```

## clangd

交叉编译项目（compile_commands.json 里的编译器是 arm-linux-gnueabi-gcc 之类的交叉工具链）必须加 `--query-driver`：
clangd 默认不执行 CDB 里的编译器，只会拿宿主机 `/usr/include` 当系统头文件路径，
于是 sysroot 里的头文件（如 sys/time.h、errno.h）全部报 "file not found"，并连锁出一堆
undeclared identifier。加上 `--query-driver` 后 clangd 会执行一次交叉编译器提取真实的
include 路径，问题消失。

```json
{
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "clangd.arguments": [
        "--header-insertion=never",
        // "--compile-commands-dir=build",
        "--function-arg-placeholders=0",
        "--clang-tidy",
        "--completion-style=bundled",
        "--ranking-model=decision_forest",
        "-j=4",
        "--background-index", 
        "--compile-commands-dir=${workspaceFolder}/build/release",
        "--query-driver=/home/test/toolchain/bin/*",
    ],
    "clangd.path": "/home/test/opensrc/clangd/bin/clangd",
    "files.exclude": {
        "rootfs/var/lib/dhcp": true,
        "**/.git/objects/**": true,
        "**/.git/subtree-cache/**": true,
        "**/node_modules/*/**": true,
        "/tmp/**": true,
        "/rootfs/var/**": true,
    },
    "git.repositoryScanMaxDepth": -1,
    "git.repositoryScanIgnoredFolders": [
        "node_modules",
        "linux*,u-boot*"
    ],
    "python.analysis.typeCheckingMode": "standard"
}
```

```yaml
# ~/.config/clangd/config.yaml
Diagnostics:
  ClangTidy:
    Add:
      [
        performance-*,
        bugprone-*,
        portability-*,
        modernize-*,
        readability-*,
        cppcoreguidelines-pro-type-member-init,
        cppcoreguidelines-slicing,
        cppcoreguidelines-prefer-member-initializer,
        cppcoreguidelines-special-member-functions,
        cert-err34-c,
        cert-err60-cpp,
        cert-oop57-cpp,
        cert-flp30-c,
        misc-unused-parameters,
        misc-definitions-in-headers,
      ]
    Remove:
      [
        modernize-use-trailing-return-type,
        bugprone-easily-swappable-parameters,
        modernize-use-using,
        performance-enum-size,
        readability-identifier-length
      ]
    CheckOptions:
      WarnOnFloatingPointNarrowingConversion: false
InlayHints:
  BlockEnd: Yes
  Designators: Yes
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes
  TypeNameLimit: 32
Completion:
  AllScopes: Yes
  ArgumentLists: None
  HeaderInsertion: Never
  CodePatterns: All
```

## width 

```json
    "editor.rulers": [96]
```

## clang-format

```json

    "editor.formatOnSave": true,
    "editor.defaultFormatter": "xaver.clang-format",
    "editor.formatOnType": true,
    "editor.formatOnPaste": true,
    "editor.detectIndentation": false

# We'll use defaults from the LLVM style, but with 4 columns indentation.
BasedOnStyle: Google
IndentWidth: 4

Language: Cpp
# Force pointers to the type for C++.
```

## max_user_watches

```bash
cat /proc/sys/fs/inotify/max_user_watches
# /etc/sysctl.conf
fs.inotify.max_user_watches=524288
# sudo sysctl -p

"files.watcherExclude": {
    "**/.git/objects/**": true,
    "**/.git/subtree-cache/**": true,
    "**/node_modules/*/**": true
  }
```