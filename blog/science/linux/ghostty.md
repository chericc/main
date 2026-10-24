# Ghostty 配置

Ghostty 是跨平台的终端模拟器。配置文件路径（二选一，按优先级从高到低）：

```bash
# macOS
~/Library/Application Support/com.mitchellh.ghostty/config
# 跨平台（macOS / Linux 都读）
~/.config/ghostty/config
```

旧版文档里的 `config.ghostty` 文件名仍被兼容读取（macOS 上就在
`~/Library/Application Support/com.mitchellh.ghostty/config.ghostty`），新配置建议直接用 `config`。
配置文件改动需要重载：macOS 按 `cmd + shift + ,`，Linux 按 `ctrl + shift + ,`（或重启 Ghostty）。
注释必须单独成行，Ghostty 不支持行内注释（否则会被当成值的一部分）。

检查语法与生效结果：

```bash
ghostty +validate-config   # 只报错不输出；exit 0 即无错
ghostty +show-config       # 打印合并后的最终配置（含默认值），确认自己的行被读进来了
```

## macOS

（实测环境：macOS 26.7.1 + Ghostty 1.3.1）

和 Linux 用同一个字体：**Sarasa Term SC Nerd**（西文 Iosevka Term + 汉字思源黑体 SC，严格 2:1，
自带 Nerd 图标），单一家族搞定，不需要 `font-codepoint-map`，也不依赖 CoreText 隐式回退。

安装（用户级，不需要 sudo；`bsdtar` 就是 macOS 自带的 libarchive，能直接解 7z）：

```bash
mkdir -p /tmp/sarasa && cd /tmp/sarasa
# 国内网络可加 -x http://127.0.0.1:7897
curl -fL --retry 5 --retry-all-errors -o sarasa.7z \
  https://github.com/laishulu/Sarasa-Term-SC-Nerd/releases/download/v2.3.1/SarasaTermSCNerd.ttf.7z
bsdtar -xf sarasa.7z \
  SarasaTermSCNerd-Regular.ttf SarasaTermSCNerd-Bold.ttf \
  SarasaTermSCNerd-Italic.ttf SarasaTermSCNerd-BoldItalic.ttf
mkdir -p ~/Library/Fonts
cp SarasaTermSCNerd-*.ttf ~/Library/Fonts/     # 4 个字重共约 115MB
```

配置：

```ini
# ~/Library/Application Support/com.mitchellh.ghostty/config
theme = light:Catppuccin Latte,dark:Catppuccin Mocha
font-family = Sarasa Term SC Nerd
```

实测结论（`fontTools` 直接读 hmtx，upm = 1000）：

| 字符 | 码位 | advance |
|---|---|---|
| `a` `W` `M` `0` `★` `─` `│` 等 | — | `0.500em` |
| 汉字 `置`、全角 `，。！？「」` | — | `1.000em` |

- 严格 2:1，中英混排、`vim` 里手对齐的中文都不会错位。
- 覆盖够用：常用汉字、全角标点、框线（`U+2500` 起）都在；连 `★`(`U+2605`)、
  `𠮷`(`U+20BB7`，扩展 B) 也有，所以 **不需要** `font-family = PingFang SC` 兜底，不会落到日文/繁体面。

```bash
ghostty +validate-config
# 每个字符都应报 found in face “Sarasa Term SC Nerd”
ghostty +show-face --string="aWiM0 置直骨争，。！？「」★𠮷"
```

> 老写法是用 `font-codepoint-map` 把汉字及标点映射到系统字体 PingFang SC。macOS 走 **CoreText**，
> 按 family/style 名匹配、不使用 collection index，所以不受 Linux 小节那个 `.ttc` 丢 index 的 bug 影响，
> 一般能正确命中简体面。但 PingFang SC 的汉字也是 1em，而默认的 JetBrains Mono 是 0.6em，
> 汉字只占 1.66 格，同样有「汉字偏小、框线错位」的问题，已不再推荐。

## Linux

（实测环境：Ubuntu 24.04 + Ghostty 1.3.1）

中英文混排的关键只有一个数：**汉字宽度必须是西文字宽的整数倍（一般是 2 倍）**。
汉字固定 1em，主字体西文若是 0.5em，汉字正好占满 2 格；只有 1:2 时框线表格、
`vim` 里手工对齐的中文才能对齐，汉字大小也才和西文协调。常见西文字宽实测：

| 西文字体 | 西文字宽 | 1em 汉字占格 | 结论 |
|---|---|---|---|
| Iosevka Term（Sarasa Term 的西文） | 0.5em | 2.00 | ✅ 完美 |
| Noto Sans Mono CJK 的西文（Source Sans） | 0.5em | 2.00 | ✅ 完美 |
| DejaVu Sans Mono | 0.602em | 1.66 | ❌ 汉字偏小、框线错位 |
| Ubuntu Mono / Ubuntu Sans Mono（24.04） | 0.56em | 1.79 | ❌ 同样对不齐 |
| JetBrains Mono / Fira Code / Source Code Pro | 0.6em | 1.67 | ❌ |

> Ubuntu 24.04 的 `Ubuntu Mono` 已从 0.5em 改成 560/1000 = 0.56em，所以
> 「Ubuntu Mono + Noto CJK」也不再是 2:1，别照抄老教程。
> Ghostty 内置的默认字体是 JetBrains Mono（0.6em），完全不配字体时中文同样偏小。

### 方案一：Sarasa Term SC Nerd（推荐）

更纱黑体的终端版：西文 Iosevka Term + 汉字思源黑体 SC，严格 2:1，字重齐全，
并打好 Nerd 图标补丁（Powerline、`eza`/`lsd`、`p10k` 可直接用）。

用户级安装，不需要 sudo（4 个常用字重约 115MB；只留 Regular + Bold 约 58MB）：

```bash
mkdir -p /tmp/sarasa && cd /tmp/sarasa
curl -fL -x http://127.0.0.1:7897 -o sarasa.7z \
  https://github.com/laishulu/Sarasa-Term-SC-Nerd/releases/download/v2.3.1/SarasaTermSCNerd.ttf.7z
# bsdtar（libarchive）能直接解 7z，Ubuntu 自带；也可以换成 7z x
bsdtar -xf sarasa.7z \
  SarasaTermSCNerd-Regular.ttf SarasaTermSCNerd-Bold.ttf \
  SarasaTermSCNerd-Italic.ttf SarasaTermSCNerd-BoldItalic.ttf
mkdir -p ~/.local/share/fonts/SarasaTermSCNerd
cp SarasaTermSCNerd-*.ttf ~/.local/share/fonts/SarasaTermSCNerd/
fc-cache -f ~/.local/share/fonts
```

```ini
font-family = Sarasa Term SC Nerd
```

### 方案二：零安装，只用 Noto Sans Mono CJK SC

`fonts-noto-cjk` 自带（`fc-list :lang=zh` 可确认）。它的西文是 Source Sans 0.5em、
汉字 1em，正好 2:1：

```ini
font-family = Noto Sans Mono CJK SC
```

缺点是西文不够“编程”：`0/O`、`1/l/I` 区分较弱，也没有连字；适合不想装字体、只求对齐。
（Nerd 图标由 Ghostty 内置符号字体兜底，不会变豆腐块。）

### 验证是否真的生效

```bash
# 拉丁和汉字应落在同一个 face；汉字必须是 SC，不能是 JP
ghostty +show-face --string="aA1中置直骨争"
```

方案一期望每个字符都来自 `Sarasa Term SC Nerd`；方案二全部来自 `Noto Sans Mono CJK SC`。
如果汉字落到别的字体、或出现 `Noto Sans Mono CJK JP`，说明配置没生效。

想自己核对字宽（1em = 1000）：

```bash
python3 - <<'EOF'
import os
from PIL import ImageFont
for p in ["~/.local/share/fonts/SarasaTermSCNerd/SarasaTermSCNerd-Regular.ttf"]:
    f = ImageFont.truetype(os.path.expanduser(p), 1000)
    print(p.split("/")[-1], "M =", f.getlength("M"), "中 =", f.getlength("中"))
EOF
```

### 卸载 / 回滚

```bash
rm -rf ~/.local/share/fonts/SarasaTermSCNerd
fc-cache -f
```

然后改回 `font-family = Noto Sans Mono CJK SC` 即可。

### 坑：不要用 `font-codepoint-map` 指定 CJK 字体

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
