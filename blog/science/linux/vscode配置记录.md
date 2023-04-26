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

- Stop 0x3

Enjoy it.