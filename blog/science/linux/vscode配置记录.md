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
    "C_Cpp.default.includePath": [
        "${default}",
        "${workspaceFolder}/**",
        "/home/test/opensrc/googletest/build/output/include"
    ],
}

```

## clangd

```json
{
    "clangd.arguments": [
        "--header-insertion=never",
        "--compile-commands-dir=build",
        "--clang-tidy",
        "--completion-style=detailed",
        "--ranking-model=decision_forest",
        "-j=1",
        "--background-index", 
    ],
    "clangd.path": "/home/test/opensrc/clangd/clangd_17.0.3/bin/clangd"
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
      ]
    Remove: modernize-use-trailing-return-type
    CheckOptions:
      WarnOnFloatingPointNarrowingConversion: false
InlayHints:
  BlockEnd: Yes
  Designators: Yes
  Enabled: Yes
  ParameterNames: No
  DeducedTypes: No
  TypeNameLimit: 24
```

## width 

```json
    "editor.rulers": [96]
```