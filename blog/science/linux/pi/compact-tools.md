# pi 工具输出折叠（compact-tools）

让内置工具（`bash` / `read` / `grep` / `find` / `ls` / `write`）的结果默认只显示 1 行，点击工具行（或 `ctrl+o`）展开 / 收拢，长输出下保持 transcript 紧凑。

## 行为

- 折叠态默认显示 1 行：`bash` 显示**末尾** 1 行（结果通常在尾部），其余工具显示**开头** 1 行
- 超出时追加灰色提示 `… (N more, click / ctrl+o to expand)`
- 点击该工具行，或按 `ctrl+o`（全局）：展开为完整输出
- 再次点击 / `ctrl+o`：收拢
- 流式运行期间只要已有 partial 结果即可点击展开；展开状态会随新输出保持
- `edit` 工具不处理（它用 `renderShell: "self"` 画 diff），保持内置展示

## 前提

- 鼠标点击需要 `tuiMode: "fullscreen"`（`~/.pi/agent/settings.json`）；regular 模式 pi 不抓鼠标，只能用 `ctrl+o`
- `ctrl+o` 是全局展开 / 收拢所有工具行，与逐行点击互不影响

## 扩展文件

```bash
~/.pi/agent/extensions/compact-tools.ts
```

## 完整代码

```ts
/**
 * compact-tools.ts
 *
 * Make built-in tool results show only 1 line by default. Click a tool row (fullscreen
 * TUI mode) or press ctrl+o to expand/collapse.
 *
 * We reuse the exported built-in tool *definitions* (createBashToolDefinition, ...) so the
 * original execution, syntax highlighting, truncation, and renderCall are preserved; only
 * renderResult (the collapsed/expanded body) is replaced.
 */
import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";
import {
	createBashToolDefinition,
	createFindToolDefinition,
	createGrepToolDefinition,
	createLsToolDefinition,
	createReadToolDefinition,
	createWriteToolDefinition,
	type AgentToolResult,
	type Theme,
	type ToolRenderResultOptions,
} from "@earendil-works/pi-coding-agent";
import { type Component, Container, Text } from "@earendil-works/pi-tui";

/** How many lines to show while collapsed. Change this to taste. */
const COLLAPSED_LINES = 1;

/** Tools that should keep their *last* lines when collapsed (their tail is the useful part). */
const TAIL_TOOLS = new Set(["bash"]);

function textOf(result: AgentToolResult<any>): string {
	return (result.content ?? [])
		.filter((c: any): c is { type: "text"; text: string } => c.type === "text")
		.map((c) => c.text ?? "")
		.join("\n")
		.trim();
}

function compactResult(
	result: AgentToolResult<any>,
	options: ToolRenderResultOptions,
	theme: Theme,
	toolName: string,
): Component {
	const output = textOf(result);
	if (!output) return new Container();

	const lines = output.split("\n");
	const shown = options.expanded
		? lines
		: TAIL_TOOLS.has(toolName)
			? lines.slice(-COLLAPSED_LINES)
			: lines.slice(0, COLLAPSED_LINES);
	const hidden = lines.length - shown.length;

	let body = shown.map((line) => theme.fg("toolOutput", line)).join("\n");
	if (hidden > 0) {
		body += theme.fg("muted", `\n… (${hidden} more, click / ctrl+o to expand)`);
	}
	return new Text(`\n${body}`, 0, 0);
}

export default function (pi: ExtensionAPI) {
	// execute() uses ctx.cwd at runtime, so the value passed here is only a fallback.
	const cwd = process.cwd();

	const definitions = {
		bash: createBashToolDefinition(cwd),
		read: createReadToolDefinition(cwd),
		grep: createGrepToolDefinition(cwd),
		find: createFindToolDefinition(cwd),
		ls: createLsToolDefinition(cwd),
		write: createWriteToolDefinition(cwd),
	} as const;

	for (const [name, definition] of Object.entries(definitions)) {
		pi.registerTool({
			...definition,
			renderResult(result, options, theme) {
				return compactResult(result as AgentToolResult<any>, options, theme, name);
			},
		} as any);
	}
}
```

## 实现说明

- 复用包根公开导出的 `createBashToolDefinition` / `createReadToolDefinition` / `createGrepToolDefinition` / `createFindToolDefinition` / `createLsToolDefinition` / `createWriteToolDefinition` 构造工具定义，保留内置的 `execute`、截断、语法高亮与 `renderCall`
- 渲染槽位按槽位继承：只覆盖 `renderResult`，未覆盖的 `renderCall` 自动用内置实现
- 折叠行数：`COLLAPSED_LINES`（默认 1）；折叠时显示末尾的工具：`TAIL_TOOLS`（默认 `bash`）
- 覆盖内置工具时交互模式会提示一次 "overrides built-in"，无害
- 新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效
