# Ghostty 配置

Ghostty 是跨平台的终端模拟器。配置文件路径：

```bash
~/.config/ghostty/config.ghostty
```

配置修改后热加载：macOS 按 `cmd + shift + ,`，Linux 按 `ctrl + shift + ,`。

## macOS

```ini
# 将 CJK 汉字及标点映射到系统字体 PingFang SC，保持默认英文字体不变
font-codepoint-map = U+4E00-U+9FFF=PingFang SC   # CJK 统一表意文字（汉字）
font-codepoint-map = U+3000-U+303F=PingFang SC   # CJK 符号和标点（。、「」等）
font-codepoint-map = U+FF00-U+FFEF=PingFang SC   # 全角形式（，：；！？等）
font-codepoint-map = U+2000-U+206F=PingFang SC   # 通用标点（—破折号、…省略号等）
font-codepoint-map = U+3400-U+4DBF=PingFang SC   # CJK 扩展A（生僻字/姓名用字）
```

> 说明：上文那个 `.ttc` 丢 index 的 bug 出在 Ghostty 的 **fontconfig** 字体发现路径（Linux/BSD）。
> macOS 走 **CoreText**，按 family/style 名匹配、不使用 collection index，所以这里用
> `font-codepoint-map` 指向 PingFang SC 一般能正确命中简体面，不受该 bug 影响。
>
> 仍建议实测确认（与 Linux 同一条命令）：
>
> ```bash
> ghostty +show-face --string="置直骨争"   # 期望输出：PingFang SC
> ```
>
> 若想与 Linux 保持一致的写法（免维护 Unicode 区间，缺字自动回退），也可改成
> `font-family` 回退链。注意 Ghostty 默认英文等宽字体是内置的 JetBrains Mono，
> 且**必须把它放第一行**，否则 PingFang 会成为主字体、拉丁字符也用它渲染：
>
> ```ini
> font-family = JetBrains Mono   # 可省略，就是默认值，写出来更明确
> font-family = PingFang SC
> ```

## Linux

```ini
theme = Dark+

# 主字体（拉丁/ASCII）
font-family = DejaVu Sans Mono

# CJK 回退字体
font-family = Noto Sans Mono CJK SC
```

`Noto Sans Mono CJK SC` 来自 `fonts-noto-cjk`，可用 `fc-list :lang=zh` 确认是否已安装。

> **坑：Linux 上不要用 `font-codepoint-map` 指定 CJK 字体。**
>
> Ghostty 的 `font-codepoint-map` 在加载 `.ttc` 字体集合（如 `NotoSansCJK-Regular.ttc`）时会**丢掉 face index**，
> 一律按 index 0 处理。而该 TTC 的 index 0 是 **JP** face，index 7 才是 **Mono SC**，
> 于是 `font-codepoint-map = ...=Noto Sans Mono CJK SC` 实际渲染出来的是**日文字形**——
> 「置」「直」等简繁/中日写法有差异的字会一眼看出不对。
>
> 用 `font-family` 则能正确带上 index。注意：**Linux 上不给任何配置时，默认回退也会选到 JP face。**
>
> 验证方式（无需截图）：
>
> ```bash
> # font-codepoint-map 方式 → ❌ Noto Sans Mono CJK JP
> # font-family        方式 → ✅ Noto Sans Mono CJK SC
> ghostty +show-face --string="置"
> ```
>
> 补充：`font-family` 可以写多行，按顺序作为「主字体 + 回退链」——拉丁取第一个，
> 缺字回退到后续字体，因此英文观感不变。若更喜欢比例字形的汉字，把第二行换成
> `font-family = Noto Sans CJK SC` 即可（同样是 SC，且 `font-family` 路径 index 正确）。

## io_uring 后端导致 `wa`(iowait) 虚高

现象：`top` 的 `wa` 常年 20%~40%，但磁盘完全空闲（`iostat -x 1` 的 `%util`、`aqu-sz` 都 ≈0），`ps` 里也没有 `D` 状态进程。判据是 `procs_blocked` 一直 >0 且不归零：

```bash
awk '/procs_blocked/{print $2}' /proc/stat   # 空闲机器上正常应为 0
```

原因：Ghostty 默认 `async-backend = auto`，在 Linux 上会选 **io_uring**。io_uring 等待事件时**直接置位 `in_iowait`**（不走 `io_schedule()`，所以既不是 `D` 状态、栈里也没有 `io_schedule`），内核 `nr_iowait` 因此常驻 >0；而 `account_idle_time()` 的规则是“本 CPU 有 iowait 任务就把**空闲时间**记为 iowait”，于是整台机器的 idle 时间都被记成 `wa`。

修正：改用 epoll 后端，重启 Ghostty 后即恢复（`procs_blocked` 归 0，`wa` 降到个位数）。

```ini
# ~/.config/ghostty/config.ghostty
async-backend = epoll
```

参考 ghostty-org/ghostty#10700、axboe/liburing#943。上游正解是应用调 `io_uring_set_iowait(ring, false)`（内核 6.15+），但 Ghostty 未采用。

## SSH terminfo

通过 SSH 连接远程服务器（如 Ubuntu）时，如果出现 `missing or unsuitable terminal: xterm-ghostty` 警告，
说明远程系统缺少 Ghostty 的 terminfo 条目。可以使用以下命令将本地的 terminfo 复制到远程服务器：

```bash
infocmp -x xterm-ghostty | ssh USER@SERVER -- tic -x -
```

注意事项：

- **macOS 版本要求**：macOS Sonoma (14.x) 及以上的系统自带的 `infocmp` 可用。如果使用 macOS Ventura (13.x) 或更早版本，
  需要先通过 Homebrew 安装较新版本的 ncurses：
  ```bash
  brew install ncurses
  ```
  然后使用完整路径调用：
  ```bash
  /opt/homebrew/opt/ncurses/bin/infocmp -x xterm-ghostty | ssh USER@SERVER -- tic -x -
  ```

- 如果远程 `tic` 提示 `older tic versions may treat the description field as an alias`，可以忽略。

- `tic` 默认写入系统数据库 `/usr/share/terminfo`，如果没有写权限则回退到 `$HOME/.terminfo`。

- 如果远程 ncurses 版本 >= 6.5-20241228，则系统已自带 `xterm-ghostty` 条目，无需手动安装。
