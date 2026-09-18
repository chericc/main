# pi TUI 渲染调试方法

排查 pi 扩展（如 `compact-tools.ts`）的工具块渲染问题时，**不能只看代码**：pi 的终端 UI 由 ANSI 转义序列逐行绘制，工具块的外壳/背景由 `ToolExecutionComponent` 组合，颜色最终是否出现取决于运行时。本文记录两套可复现的调试方法，以及构造回放会话、解析 ANSI 的完整脚本。

> 起因案例：`compact-tools.ts` 重写 `edit` 的 `renderCall`/`renderResult` 后，edit 块丢失了背景色（read/bash/write 都有）。根因见文末「常见坑」。

---

## 0. 先决知识

### 工具块外壳：default vs self

pi 的 `ToolExecutionComponent` 有两种外壳：

| `renderShell` | 谁画外壳 | 背景色 |
|---|---|---|
| 缺省 / `"default"` | `ToolExecutionComponent` 的 `contentBox`（`Box(1,1,bgFn)`） | 自动按状态加 `toolPendingBg` / `toolSuccessBg` / `toolErrorBg` |
| `"self"` | 工具自己的渲染器 | 渲染器必须自己画，否则**没有背景色** |

内置工具里**只有 `edit` 声明了 `renderShell: "self"`**（它要画一个稳定的 diff 预览框）。其余工具（bash/read/write/grep/find/ls）都用默认外壳。

### ANSI 颜色码（dark 主题）

背景色以 `48;2;R;G;B` 开头，前景色以 `38;2;R;G;B` 开头。判断「这个块有没有底色」就是看该行前缀有没有 `48;2;`。

| 语义 | 主题值 | ANSI |
|---|---|---|
| `toolSuccessBg` | `#283228` | `48;2;40;50;40` |
| `toolPendingBg` | `#282832` | `48;2;40;40;50` |
| `toolErrorBg` | `#3c2828` | `48;2;60;40;40` |
| `toolDiffAdded`（green） | `#b5bd68` | `38;2;181;189;104` |
| `toolDiffRemoved`（red） | `#cc6666` | `38;2;204;102;102` |
| `toolDiffContext`（gray） | `#808080` | `38;2;128;128;128` |
| `toolOutput`（gray） | `#808080` | `38;2;128;128;128` |

> 终端需真彩色：本机 `settings.json` 已设 `terminal.trueColor=true`，`~/.bashrc` 里 `COLORTERM=truecolor`。

---

## 1. 方法一：用 jiti 加载扩展，直接调用渲染函数（最快）

pi 用 `jiti/static` 加载 `extensions/*.ts`，并把 `@earendil-works/pi-coding-agent` 等别名指向自身 dist。复刻这套别名后就能在普通 node 脚本里拿到扩展注册的 `renderCall`/`renderResult`，直接看输出字节。

```js
// /tmp/load-plugin.mjs  —— 用 `node load-plugin.mjs` 运行
import { createJiti } from "/home/test/.local/lib/node_modules/@earendil-works/pi-coding-agent/node_modules/jiti/lib/jiti-static.mjs";
import * as path from "node:path";

const PKG = "/home/test/.local/lib/node_modules/@earendil-works/pi-coding-agent";
const dist = path.join(PKG, "dist");

const jiti = createJiti(import.meta.url, {
  alias: {
    "@earendil-works/pi-coding-agent": path.join(dist, "index.js"),
    "@earendil-works/pi-tui": path.join(PKG, "node_modules/@earendil-works/pi-tui/dist/index.js"),
    "@earendil-works/pi-ai": path.join(PKG, "node_modules/@earendil-works/pi-ai/dist/compat.js"),
  },
});

// 1) 加载扩展，拦截 registerTool
const mod = await jiti.import("/home/test/.pi/agent/extensions/compact-tools.ts", { default: true });
const reg = {};
mod({ registerTool: (d) => (reg[d.name] = d), registerCommand() {}, registerShortcut() {},
      registerFlag() {}, on() {}, getFlag() { return undefined; }, appendEntry() {} });
for (const [n, d] of Object.entries(reg)) console.log(n.padEnd(6), "renderShell =", JSON.stringify(d.renderShell));

// 2) 初始化主题（渲染器内部会读全局 theme，未初始化会抛 "Theme not initialized"）
const themeMod = await jiti.import(path.join(dist, "modes/interactive/theme/theme.js"));
themeMod.initTheme("dark");

// 3) 直接调用 renderResult / renderCall，打印每行（含 ANSI）
const diff = [" 1 ctx", "-2 old A", "+2 new A"].join("\n");
const result = { content: [{ type: "text", text: "Successfully replaced 1 block(s)." }],
                 details: { diff } };
const ctx = { args: { path: "x.js", edits: [{ oldText: "old", newText: "new" }] },
              lastComponent: undefined, isError: false, state: {}, cwd: "/tmp" };
const comp = reg.edit.renderResult(result, { expanded: false, isPartial: false }, themeMod.theme, ctx);
for (const line of comp.render(80)) console.log(JSON.stringify(line));
```

输出里带 `\u001b[48;2;...` 说明有底色，带 `\u001b[38;2;181;189;104m` 说明 diff 有前景色。

**适用**：快速验证渲染函数本身的输出；**不覆盖** `ToolExecutionComponent` 的外壳组合（比如 `renderShell` 的影响），所以要配合方法二。

---

## 2. 方法二：伪终端驱动真实 pi TUI（最真实）

用 Python `pty.fork()` 启动真实 `pi`，喂一个回放会话，抓取它实际写进终端的字节。

```python
# /tmp/capture_tui.py —— python3 capture_tui.py
import pty, os, time, select, signal

pid, fd = pty.fork()
if pid == 0:  # 子进程：exec pi
    os.environ["TERM"] = "xterm-256color"
    os.environ["LINES"] = "60"
    os.environ["COLUMNS"] = "120"
    os.execvp("pi", ["pi", "--tui-mode", "fullscreen", "--offline",
                     "--session", "/tmp/probe-session.jsonl"])

buf, deadline = b"", time.time() + 30
while time.time() < deadline:
    r, _, _ = select.select([fd], [], [], 0.5)
    if not r:
        continue
    try:
        data = os.read(fd, 65536)
    except OSError:
        break
    if not data:
        break
    buf += data
    if b"TARGET_MARKER" in buf:          # 看到目标内容后退出
        time.sleep(1.5)
        os.write(fd, b"\x03"); time.sleep(0.3)   # ctrl+c
        os.write(fd, b"\x03"); time.sleep(0.5)   # ctrl+c 退出
        break

os.kill(pid, signal.SIGKILL); os.waitpid(pid, 0)
open("/tmp/tui-capture.bin", "wb").write(buf)
print("captured", len(buf), "bytes")
```

要点：

- `--tui-mode fullscreen`：整屏重绘，便于在单个视口里抓全；`regular` 只回放增量，容易抓不到历史。
- `--offline`：禁止启动联网检查。
- 想要**对照实验**时加 `--no-extensions`（内置渲染器）对比带扩展的输出。
- 退出：连发两次 `ctrl+c`。

---

## 3. 构造回放用的最小会话

pi 的会话是 JSONL，每条 entry 需要 `id` + `parentId` 串成链。回放一个工具调用最少需要：

```
session 头 → model_change → 用户消息 → assistant(toolCall) → toolResult
```

```python
# build_session.py
import json
session, prev = [], None
def emit(e):
    global prev
    e["id"] = f"b{len(session):07x}"; e["parentId"] = prev; prev = e["id"]; session.append(e)

emit({"type": "session", "version": 3, "id": "22222222-3333-4444-5555-666666666666",
      "timestamp": "2026-09-18T10:00:00.000Z", "cwd": "/tmp/editcolor-test"})
emit({"type": "model_change", "provider": "ollama", "modelId": "glm-5.3:cloud"})
emit({"type": "message", "message": {"role": "user",
      "content": [{"type": "text", "text": "render"}], "timestamp": 1789697617560}})

# usage 必须字段齐全，否则 footer 渲染会崩（见「常见坑」）
USAGE = {"input": 10, "output": 10, "cacheRead": 0, "cacheWrite": 0, "reasoning": 0,
         "totalTokens": 20,
         "cost": {"input": 0, "output": 0, "cacheRead": 0, "cacheWrite": 0, "total": 0}}

def turn(tool, callid, args, result):
    emit({"type": "message", "message": {
        "role": "assistant",
        "content": [{"type": "toolCall", "id": callid, "name": tool, "arguments": args}],
        "api": "openai-completions", "provider": "ollama", "model": "glm-5.3:cloud",
        "usage": USAGE, "stopReason": "toolUse", "timestamp": 1789697617563}})
    emit({"type": "message", "message": {**result, "toolCallId": callid, "toolName": tool,
          "timestamp": 1789697617600}})

turn("read", "c_read", {"path": "test.txt"},
     {"role": "toolResult", "content": [{"type": "text", "text": "READ_MARK\nline two"}], "isError": False})
turn("edit", "c_edit", {"path": "test.txt", "edits": [{"oldText": "line two", "newText": "line two edited"}]},
     {"role": "toolResult", "content": [{"type": "text", "text": "Successfully replaced 1 block(s)."}],
      "details": {"diff": " 1 line one\n-2 line two\n+2 line two edited\n 3 line three",
                  "patch": "", "firstChangedLine": 2}, "isError": False})

open("probe-session.jsonl", "w").write("\n".join(json.dumps(e, ensure_ascii=False) for e in session) + "\n")
```

也可以用真实会话裁剪：从 `~/.pi/agent/sessions/<项目>/<时间>_<uuid>.jsonl` 里挑一条 assistant(edit toolCall) 及其 toolResult，保留前面的 `session`/`model_change`/首个 user 消息即可。

---

## 4. 解析抓取到的字节

```python
# analyze_capture.py
import re

def rows_of(data):
    """按绝对光标定位 \x1b[<row>;<col>H 切分，返回 (可见文本, SGR 码列表)。"""
    out = []
    for part in re.split(rb'\x1b\[\d+;\d+H', data):
        text = re.sub(rb'\x1b\[[0-9;?]*[a-zA-Z]', b'', part)   # 去 CSI
        text = re.sub(rb'\x1b\][^\x07]*\x07', b'', text).strip()  # 去 OSC
        if not text:
            continue
        codes = [c.decode() for c in re.findall(rb'\x1b\[([0-9;]*)m', part)
                 if c.decode() not in ('0', '')]
        out.append((text, codes))
    return out

MARKERS = {b"READ_MARK": "read", b"line two edited": "edit",
           b"edit /tmp": "edit-header", b"read test.txt": "read-header"}
for text, codes in rows_of(open("/tmp/tui-capture.bin", "rb").read()):
    for marker, name in MARKERS.items():
        if marker in text:
            bg = [c for c in codes if c.startswith("48;")]   # 48;2;R;G;B = 背景
            print(f"{name:14} bg={'YES ' + bg[0] if bg else 'NO':18} | {text[:70].decode('utf-8', 'replace')}")
```

判定「块有没有颜色」：`bg=NO` 就是丢了外壳背景；再检查 `38;2;181;189;104`（绿）/`38;2;204;102;102`（红）是否存在，判断 diff 前景色有没有丢。

---

## 5. 常见坑

- **`renderShell: "self"` + 普通 `Text`/`Container` = 丢背景色。** 默认外壳才会自动加 `toolPendingBg`/`toolSuccessBg`/`toolErrorBg`。`compact-tools.ts` 的 `edit` 就踩了这个坑；修复是让 `edit` 回到 `"default"` 外壳：`renderShell: name === "edit" ? "default" : definition.renderShell`。
- **`--no-session` 与 `--session` 不能同用。** 加了 `--no-session` 时 `--session` 指定回放文件不会渲染历史，界面会是空的。
- **合成会话的 `usage` 必须含 `totalTokens` 和 `cost`。** 否则 footer 渲染直接崩：`TypeError: Cannot read properties of undefined (reading 'total')`。
- **渲染前必须 `initTheme("dark")`。** 渲染器读全局 `theme`（`globalThis[Symbol.for("...:theme")]` 代理），未初始化调用会抛 `Theme not initialized. Call initTheme() first.`。扩展与主程序通过这个 Symbol 共享主题，所以方法一里 initTheme 一次即可。
- **jiti 别名要对齐 pi 的 loader**（见方法一）。别名缺失时扩展会去就近的 `node_modules` 找 `@earendil-works/*`，可能拿到别的副本。
- **对照实验**：同一会话分别用 `--no-extensions` 和默认（加载扩展）各抓一次，逐行比 `48;2;` 与 `38;2;` 码，差异一目了然。
- **`read` 的高亮差异**：`compact-tools.ts` 的通用 `compactResult()` 把行统一涂成 `toolOutput`，会丢掉内置 `read` 的语法高亮（`highlightCode`）；write 因单独处理而保留高亮。这是插件设计取舍，不是「没颜色」bug。

---

## 6. 快速命令清单

```bash
# 方法一：直接调渲染函数
node /tmp/load-plugin.mjs

# 方法二：抓真实 TUI（记得先 build_session.py 生成会话）
python3 build_session.py
python3 capture_tui.py
python3 analyze_capture.py

# 对照组：不带扩展
#   把 os.execvp 里的参数加上 "--no-extensions" 再抓一次

# 查看内置工具是否使用 self 外壳
grep -rn "renderShell" \
  /home/test/.local/lib/node_modules/@earendil-works/pi-coding-agent/dist/core/tools/

# 查看主题颜色定义
python3 - <<'PY'
import json
d = json.load(open("/home/test/.local/lib/node_modules/@earendil-works/pi-coding-agent/dist/modes/interactive/theme/dark.json"))
print(json.dumps(d["vars"], indent=1))
PY
```
