# pi 编辑模式与只读 Bash 守卫


pi（`@earendil-works/pi-coding-agent`）本身没有内置的权限 / 编辑确认模式，这类能力通过扩展（extension）实现。这里实现两种编辑模式，并对 `bash` 工具加一层只读守卫。

## 行为概览

| 模式 | edit / write | bash 只读命令 | bash 非只读命令 |
|------|--------------|---------------|-----------------|
| `ask-to-edit`（默认） | 弹 diff 确认 | 直接执行 | 弹窗确认 |
| `auto-edit` | 自动执行 | 直接执行 | 弹窗确认 |

两种模式下 bash 行为一致：只读命令自动放行，非只读命令一律弹窗确认；无 UI 时（`-p` / `--mode json`）非只读命令直接阻止并返回原因。

## 扩展文件

放到全局扩展目录即可被自动发现，无需修改 `settings.json`：

```bash
~/.pi/agent/extensions/edit-modes.ts
```

## 完整代码

```ts
/**
 * Edit Modes Extension
 *
 * Approval modes for the built-in `edit` / `write` tools, plus a read-only
 * gate for the built-in `bash` tool.
 *
 * Edit modes:
 *   ask-to-edit (default)  Show a diff and wait for approval before every edit/write.
 *   auto-edit              Run edit/write without prompting.
 *
 * Bash (same in BOTH edit modes):
 *   - read-only commands run without asking;
 *   - any other command (writes, redirections, unknown commands, wrappers, ...)
 *     shows a confirmation dialog.
 *
 * Switch modes:
 *   /edit-mode [ask-to-edit|auto-edit]   set explicitly (no arg opens a picker)
 *   /ask-to-edit                         switch to ask-to-edit
 *   /auto-edit                           switch to auto-edit
 *   Alt+E                                cycle between the two modes
 *
 * The active mode is shown in the footer and persisted per session, so it is
 * restored on /reload, /resume and tree navigation. The startup mode can be set
 * with `--edit-mode <mode>` (default: ask-to-edit).
 *
 * Scope: this gates the built-in `edit` and `write` tools, and the `bash` tool
 * for non-read-only commands. The bash check is a heuristic fail-closed
 * classifier, not a security sandbox. In non-interactive sessions there is no
 * UI to approve with, so blocked actions are refused instead of silently allowed.
 */

import {
	isToolCallEventType,
	type EditToolCallEvent,
	type ExtensionAPI,
	type ExtensionContext,
	type WriteToolCallEvent,
} from "@earendil-works/pi-coding-agent";

type EditMode = "ask-to-edit" | "auto-edit";

const MODES: readonly EditMode[] = ["ask-to-edit", "auto-edit"];
const STATE_TYPE = "edit-mode";
const STARTUP_FLAG = "edit-mode";
const STATUS_KEY = "edit-mode";

const MAX_PREVIEW_LINES = 40;
const MAX_PREVIEW_WIDTH = 120;
const MAX_BASH_PREVIEW_LINES = 20;

function isEditMode(value: unknown): value is EditMode {
	return value === "ask-to-edit" || value === "auto-edit";
}

function truncateForPreview(text: string, maxLines: number, maxWidth: number): string {
	const lines = text.split("\n");
	const visible = lines
		.slice(0, maxLines)
		.map((line) => (line.length > maxWidth ? `${line.slice(0, maxWidth - 1)}…` : line));
	if (lines.length > maxLines) {
		visible.push(`… (${lines.length - maxLines} more lines)`);
	}
	return visible.join("\n");
}

// ---------------------------------------------------------------------------
// Bash read-only classifier
// ---------------------------------------------------------------------------

type ShellToken = { type: "word"; value: string } | { type: "operator"; value: string };

interface TokenizedShell {
	tokens: ShellToken[];
	hasWriteRedirection: boolean;
	hasSubstitution: boolean;
}

const SAFE_REDIRECT_TARGETS = ["/dev/null", "/dev/stderr", "/dev/stdout"];

function tokenizeShell(command: string): TokenizedShell {
	const tokens: ShellToken[] = [];
	let current = "";
	let hasWriteRedirection = false;
	let hasSubstitution = false;
	let quote: "'" | '"' | null = null;
	let i = 0;

	const pushWord = () => {
		if (current.length > 0) {
			tokens.push({ type: "word", value: current });
			current = "";
		}
	};

	const isWordBoundary = (ch: string | undefined) => ch === undefined || /[\s;&|<>]/.test(ch);

	while (i < command.length) {
		const ch = command[i] as string;

		if (quote === "'") {
			if (ch === "'") quote = null;
			else current += ch;
			i++;
			continue;
		}

		if (quote === '"') {
			if (ch === "\\") {
				const next = command[i + 1];
				if (next === '"' || next === "\\" || next === "$" || next === "`") {
					current += next;
					i += 2;
					continue;
				}
				current += ch;
				i++;
				continue;
			}
			if (ch === '"') {
				quote = null;
				i++;
				continue;
			}
			if (ch === "`") hasSubstitution = true;
			if (ch === "$" && command[i + 1] === "(") hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}

		if (ch === "\\") {
			const next = command[i + 1];
			if (next !== undefined) {
				current += next;
				i += 2;
				continue;
			}
			i++;
			continue;
		}
		if (ch === "'") {
			quote = "'";
			i++;
			continue;
		}
		if (ch === '"') {
			quote = '"';
			i++;
			continue;
		}
		if (ch === "`") {
			hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}
		if (ch === "$" && command[i + 1] === "(") {
			hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}

		if (ch === ">") {
			let j = i + 1;
			if (command[j] === ">") j++;
			while (command[j] === " " || command[j] === "\t") j++;
			if (command[j] === "&") {
				// File-descriptor duplication (2>&1), not a file write.
				j++;
				while (!isWordBoundary(command[j])) j++;
			} else {
				const start = j;
				while (!isWordBoundary(command[j])) j++;
				const target = command.slice(start, j).replace(/^['"]|['"]$/g, "");
				const safe =
					SAFE_REDIRECT_TARGETS.includes(target) || target.startsWith("/dev/fd/");
				if (!safe) hasWriteRedirection = true;
			}
			pushWord();
			i = j;
			continue;
		}
		if (ch === "<") {
			pushWord();
			i++;
			if (command[i] === "<") i++;
			if (command[i] === "<") i++;
			continue;
		}
		if (ch === "|") {
			pushWord();
			if (command[i + 1] === "|") {
				tokens.push({ type: "operator", value: "||" });
				i += 2;
			} else {
				tokens.push({ type: "operator", value: "|" });
				i++;
			}
			continue;
		}
		if (ch === ";" || ch === "\n") {
			pushWord();
			tokens.push({ type: "operator", value: ";" });
			i++;
			continue;
		}
		if (ch === "&") {
			if (command[i + 1] === "&") {
				pushWord();
				tokens.push({ type: "operator", value: "&&" });
				i += 2;
				continue;
			}
			if (command[i + 1] === ">") {
				hasWriteRedirection = true;
				pushWord();
				i += 2;
				if (command[i] === ">") i++;
				continue;
			}
			pushWord();
			tokens.push({ type: "operator", value: "&" });
			i++;
			continue;
		}
		if (ch === " " || ch === "\t") {
			pushWord();
			i++;
			continue;
		}

		current += ch;
		i++;
	}

	pushWord();
	if (command.includes("<(") || command.includes(">(")) hasSubstitution = true;
	return { tokens, hasWriteRedirection, hasSubstitution };
}

/** Commands that only read data; any argument is safe (subject to flag checks). */
const READ_ONLY_SIMPLE = new Set([
	"ls", "cat", "tac", "bat", "head", "tail", "less", "more", "nl", "wc", "uniq", "cut", "tr",
	"column", "rev", "fold", "paste", "join", "comm", "diff", "cmp", "md5sum", "sha1sum",
	"sha256sum", "shasum", "cksum", "basename", "dirname", "realpath", "readlink", "stat",
	"file", "du", "df", "tree", "grep", "egrep", "fgrep", "rg", "ag", "ack", "awk", "gawk",
	"mawk", "jq", "yq", "xmllint", "pwd", "echo", "printf", "true", "false", ":", "test", "[",
	"which", "type", "whereis", "whoami", "id", "groups", "uname", "hostname", "date", "uptime",
	"locale", "tty", "ps", "pgrep", "free", "vmstat", "iostat", "nproc", "arch", "who", "w",
	"last", "cal", "bc", "man", "help", "history", "dirs", "jobs", "umask", "sleep", "cd",
	"pushd", "popd", "export", "set", "unset", "alias", "unalias",
]);

const GIT_READ_ONLY_SUBCOMMANDS = new Set([
	"status", "log", "diff", "show", "describe", "rev-parse", "rev-list", "ls-files", "ls-tree",
	"cat-file", "blame", "shortlog", "show-ref", "for-each-ref", "name-rev", "merge-base",
	"whatchanged", "grep", "fsck", "count-objects", "check-ignore", "check-attr", "version",
	"help",
]);

function gitReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	const rest = args.slice(1);

	if (sub.startsWith("-")) {
		if (sub === "--version" || sub === "-v" || sub === "--help" || sub === "-h") return true;
		if (["-C", "--git-dir", "--work-tree", "-c", "--config-env"].includes(sub)) {
			return rest.length >= 2 && gitReadOnly(rest.slice(1));
		}
		return false;
	}
	if (GIT_READ_ONLY_SUBCOMMANDS.has(sub)) return true;

	if (sub === "branch" || sub === "tag") return rest.every((a) => a.startsWith("-"));
	if (sub === "remote") {
		return rest.length === 0 || rest.every((a) => a.startsWith("-")) || ["show", "get-url"].includes(rest[0] as string);
	}
	if (sub === "config") {
		if (rest.some((a) => a === "--list" || a === "-l" || a.startsWith("--get"))) return true;
		return rest.filter((a) => !a.startsWith("-")).length <= 1;
	}
	if (sub === "stash") return rest[0] === "list" || rest[0] === "show";
	if (sub === "worktree") return rest.length === 0 || rest[0] === "list";
	if (sub === "reflog") return rest.length === 0 || rest[0] === "show";
	if (sub === "submodule") return rest.length === 0 || rest[0] === "status" || rest[0] === "summary";
	if (sub === "notes") return rest.length === 0 || rest[0] === "list" || rest[0] === "show";
	if (sub === "symbolic-ref") return rest.filter((a) => !a.startsWith("-")).length <= 1;

	return false;
}

const GH_READ_ONLY_VERBS = new Set(["view", "list", "status", "diff", "checks", "ls", "get"]);

function ghReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	if (["--version", "-v", "--help", "-h"].includes(args[0] as string)) return true;
	if (args[0] === "auth" && (args[1] === "status" || args[1] === "token")) return true;
	if (args[0] === "config" && (args[1] === "get" || args[1] === "list")) return true;
	return GH_READ_ONLY_VERBS.has(args[1] as string);
}

const PKG_READ_ONLY_SUBCOMMANDS = new Set([
	"ls", "list", "view", "show", "info", "outdated", "why", "root", "prefix", "bin", "ping",
	"doctor", "fund", "help",
]);

function packageManagerReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (["--version", "-v", "--help", "-h", "version"].includes(sub)) return true;
	if (sub === "config") return args.length <= 1 || ["get", "list", "ls"].includes(args[1] as string);
	return PKG_READ_ONLY_SUBCOMMANDS.has(sub);
}

function bunReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (["--version", "-v", "--help", "-h"].includes(sub)) return true;
	if (sub === "pm" && (args[1] === "ls" || args[1] === "list")) return true;
	return PKG_READ_ONLY_SUBCOMMANDS.has(sub);
}

const CONTAINER_READ_ONLY_SUBCOMMANDS = new Set([
	"ps", "images", "logs", "inspect", "version", "info", "stats", "top", "port", "diff",
	"history", "search", "events",
]);
const CONTAINER_READ_ONLY_CHILD = new Set(["ls", "ps", "logs", "inspect", "top", "stats", "port", "diff", "config", "df", "info", "events"]);

function containerReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (sub.startsWith("-")) return true; // docker --version / --help
	if (CONTAINER_READ_ONLY_SUBCOMMANDS.has(sub)) return true;
	if (["image", "container", "volume", "network", "compose", "system"].includes(sub)) {
		return CONTAINER_READ_ONLY_CHILD.has(args[1] as string);
	}
	return false;
}

function kubectlReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (sub.startsWith("-")) return true;
	if (["get", "describe", "logs", "explain", "api-resources", "api-versions", "cluster-info", "version", "top", "events"].includes(sub)) return true;
	if (sub === "auth") return args[1] === "can-i";
	if (sub === "config") {
		return ["view", "get-contexts", "current-context", "get-clusters", "get-users", "get-context"].includes(args[1] as string);
	}
	return false;
}

function runtimeReadOnly(cmd: string, args: string[]): boolean {
	if (args.length === 0) return false;
	const versionFlags = new Set(["--version", "-V", "-v", "--help", "-h", "-version", "version"]);
	if (cmd === "go") return ["version", "env", "list", "doc"].includes(args[0] as string);
	if (cmd === "cargo") return ["tree", "metadata", "search", "--version", "-V", "--list"].includes(args[0] as string);
	if (cmd === "pip" || cmd === "pip3") return ["list", "show", "freeze", "check", "index"].includes(args[0] as string);
	if (cmd === "poetry") return ["show", "check", "env", "--version", "-V"].includes(args[0] as string);
	if (cmd === "gem") return ["list", "search", "info", "environment", "--version", "-v"].includes(args[0] as string);
	if (cmd === "deno") return ["info", "--version", "-V"].includes(args[0] as string);
	return args.every((a) => versionFlags.has(a));
}

const RUNTIME_COMMANDS = new Set([
	"node", "python", "python3", "pip", "pip3", "poetry", "cargo", "rustc", "go", "java", "javac",
	"dotnet", "ruby", "gem", "php", "composer", "mvn", "gradle", "make", "cmake", "deno",
]);

function baseName(p: string): string {
	const idx = Math.max(p.lastIndexOf("/"), p.lastIndexOf("\\"));
	return idx >= 0 ? p.slice(idx + 1) : p;
}

function analyzeWords(words: string[], depth = 0): boolean {
	if (depth > 4) return false;
	let i = 0;
	while (i < words.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(words[i] as string)) i++;
	if (i >= words.length) return true; // only variable assignments

	const cmd = baseName(words[i] as string);
	const args = words.slice(i + 1);
	if (cmd.length === 0 || cmd.includes("$")) return false;

	if (cmd === "git") return gitReadOnly(args);
	if (cmd === "gh") return ghReadOnly(args);
	if (cmd === "npm" || cmd === "pnpm" || cmd === "yarn") return packageManagerReadOnly(args);
	if (cmd === "bun") return bunReadOnly(args);
	if (cmd === "docker" || cmd === "podman") return containerReadOnly(args);
	if (cmd === "kubectl") return kubectlReadOnly(args);
	if (RUNTIME_COMMANDS.has(cmd)) return runtimeReadOnly(cmd, args);

	if (cmd === "env" || cmd === "time" || cmd === "nohup") {
		let j = 0;
		while (j < args.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(args[j] as string)) j++;
		if (j >= args.length) return true;
		return analyzeWords(args.slice(j), depth + 1);
	}
	if (cmd === "timeout") return args.length >= 2 && analyzeWords(args.slice(2), depth + 1);

	if (cmd === "find" || cmd === "fd" || cmd === "fdfind") {
		return !args.some((a) => a.startsWith("-delete") || a.startsWith("-exec") || a.startsWith("-ok") || a.startsWith("-fprint") || a.startsWith("-fls"));
	}
	if (cmd === "sed") {
		return !args.some((a) => a.startsWith("-i") || a.startsWith("--in-place"));
	}
	if (cmd === "sort") {
		return !args.some((a) => a.startsWith("-o") || a.startsWith("--output"));
	}
	return READ_ONLY_SIMPLE.has(cmd);
}

function analyzeBashCommand(command: string): { readOnly: boolean; reason: string } {
	const trimmed = command.trim();
	if (trimmed.length === 0) return { readOnly: true, reason: "empty command" };

	const { tokens, hasWriteRedirection, hasSubstitution } = tokenizeShell(command);
	if (hasSubstitution) return { readOnly: false, reason: "contains command or process substitution" };
	if (hasWriteRedirection) return { readOnly: false, reason: "contains output redirection to a file" };

	const segments: string[][] = [];
	let current: string[] = [];
	for (const token of tokens) {
		if (token.type === "operator") {
			if (current.length > 0) segments.push(current);
			current = [];
		} else {
			current.push(token.value);
		}
	}
	if (current.length > 0) segments.push(current);
	if (segments.length === 0) return { readOnly: true, reason: "empty command" };

	for (const segment of segments) {
		if (!analyzeWords(segment)) {
			return { readOnly: false, reason: `not read-only: ${segment.join(" ")}` };
		}
	}
	return { readOnly: true, reason: "read-only" };
}

// ---------------------------------------------------------------------------
// Edit/write previews
// ---------------------------------------------------------------------------

function buildEditPreview(input: EditToolCallEvent["input"]): string {
	const parts: string[] = [];
	input.edits.forEach((edit, index) => {
		if (input.edits.length > 1) {
			parts.push(`@@ change ${index + 1} of ${input.edits.length} @@`);
		}
		for (const line of edit.oldText.split("\n")) parts.push(`- ${line}`);
		for (const line of edit.newText.split("\n")) parts.push(`+ ${line}`);
	});
	return truncateForPreview(parts.join("\n"), MAX_PREVIEW_LINES, MAX_PREVIEW_WIDTH);
}

function buildWritePreview(input: WriteToolCallEvent["input"]): string {
	const parts = input.content.split("\n").map((line) => `+ ${line}`);
	return truncateForPreview(parts.join("\n"), MAX_PREVIEW_LINES, MAX_PREVIEW_WIDTH);
}

// ---------------------------------------------------------------------------
// Extension
// ---------------------------------------------------------------------------

export default function (pi: ExtensionAPI) {
	let mode: EditMode = "ask-to-edit";
	let bashAutoApprove = false;

	pi.registerFlag(STARTUP_FLAG, {
		description: "Start in an edit approval mode: ask-to-edit | auto-edit",
		type: "string",
		default: "ask-to-edit",
	});

	function applyStatus(ctx: ExtensionContext): void {
		if (!ctx.hasUI) return;
		const color = mode === "auto-edit" ? "success" : "warning";
		const suffix = bashAutoApprove ? " · bash:auto" : "";
		ctx.ui.setStatus(STATUS_KEY, ctx.ui.theme.fg(color, `✎ ${mode}${suffix}`));
	}

	function setMode(next: EditMode, ctx: ExtensionContext, options: { persist?: boolean; notify?: boolean } = {}): void {
		const { persist = true, notify = true } = options;
		mode = next;
		if (persist) pi.appendEntry(STATE_TYPE, { mode });
		applyStatus(ctx);
		if (notify && ctx.hasUI) ctx.ui.notify(`Edit mode: ${mode}`, "info");
	}

	pi.on("session_start", async (_event, ctx) => {
		// "Allow all bash commands (this session)" is intentionally not persisted.
		bashAutoApprove = false;

		let restored: EditMode | undefined;
		for (const entry of ctx.sessionManager.getBranch()) {
			if (entry.type === "custom" && entry.customType === STATE_TYPE) {
				const candidate = (entry.data as { mode?: unknown } | undefined)?.mode;
				if (isEditMode(candidate)) restored = candidate;
			}
		}

		const flagValue = pi.getFlag(STARTUP_FLAG);
		mode = restored ?? (isEditMode(flagValue) ? flagValue : "ask-to-edit");
		applyStatus(ctx);
	});

	pi.registerCommand("edit-mode", {
		description: "Set edit approval mode (ask-to-edit | auto-edit)",
		getArgumentCompletions: (prefix: string) =>
			MODES.filter((m) => m.startsWith(prefix)).map((m) => ({ value: m, label: m })),
		handler: async (args, ctx) => {
			const arg = args.trim();
			if (!arg) {
				if (!ctx.hasUI) {
					ctx.ui.notify(`Edit mode: ${mode}`, "info");
					return;
				}
				const choice = await ctx.ui.select(`Edit mode (current: ${mode})`, [...MODES]);
				if (choice && isEditMode(choice)) setMode(choice, ctx);
				return;
			}
			if (!isEditMode(arg)) {
				ctx.ui.notify(`Unknown edit mode: ${arg}. Use ask-to-edit or auto-edit.`, "error");
				return;
			}
			setMode(arg, ctx);
		},
	});

	pi.registerCommand("ask-to-edit", {
		description: "Ask for approval before every edit/write",
		handler: async (_args, ctx) => setMode("ask-to-edit", ctx),
	});

	pi.registerCommand("auto-edit", {
		description: "Auto-approve every edit/write",
		handler: async (_args, ctx) => setMode("auto-edit", ctx),
	});

	pi.registerShortcut("alt+e", {
		description: "Cycle edit mode (ask-to-edit ↔ auto-edit)",
		handler: async (ctx) => setMode(mode === "auto-edit" ? "ask-to-edit" : "auto-edit", ctx),
	});

	pi.on("tool_call", async (event, ctx) => {
		// --- bash: read-only allowed, everything else asks (in both edit modes) ---
		if (isToolCallEventType("bash", event)) {
			if (bashAutoApprove) return;
			const verdict = analyzeBashCommand(event.input.command);
			if (verdict.readOnly) return;

			const preview = truncateForPreview(event.input.command, MAX_BASH_PREVIEW_LINES, MAX_PREVIEW_WIDTH);

			if (!ctx.hasUI) {
				return {
					block: true,
					reason:
						`Blocked by read-only bash guard: ${verdict.reason}, and no interactive UI is available ` +
						`to approve it. Use a read-only command or run an interactive session.`,
				};
			}

			const choice = await ctx.ui.select(
				`⚠ Non-read-only bash (${verdict.reason}):\n\n$ ${preview}\n`,
				["Allow once", "Allow all commands (this session)", "Deny"],
			);

			if (choice === "Allow once") return;
			if (choice === "Allow all commands (this session)") {
				bashAutoApprove = true;
				applyStatus(ctx);
				ctx.ui.notify("Bash: auto-approving all commands for this session", "warning");
				return;
			}
			return { block: true, reason: `User denied non-read-only bash command: ${verdict.reason}` };
		}

		// --- edit/write: gated only in ask-to-edit mode ---
		if (mode === "auto-edit") return;

		const isEditCall = isToolCallEventType("edit", event);
		const isWriteCall = isToolCallEventType("write", event);
		if (!isEditCall && !isWriteCall) return;

		const path = event.input.path;
		const summary = isEditCall
			? `edit ${path} (${event.input.edits.length} change${event.input.edits.length === 1 ? "" : "s"})`
			: `write ${path}`;

		if (!ctx.hasUI) {
			return {
				block: true,
				reason:
					`Blocked by ask-to-edit mode: ${summary} needs approval, but no interactive UI is available. ` +
					`Switch to auto-edit (--edit-mode auto-edit) to allow edits in non-interactive sessions.`,
			};
		}

		const preview = isEditCall ? buildEditPreview(event.input) : buildWritePreview(event.input);
		const title = `✎ ask-to-edit: ${summary}\n\n${preview}\n`;

		const choice = await ctx.ui.select(title, [
			"Allow once",
			"Allow all edits (switch to auto-edit)",
			"Deny",
		]);

		if (choice === "Allow once") return;
		if (choice === "Allow all edits (switch to auto-edit)") {
			setMode("auto-edit", ctx);
			return;
		}
		if (choice === "Deny") {
			return { block: true, reason: `User denied: ${summary} (ask-to-edit mode)` };
		}

		// Cancelled the dialog (Esc / Ctrl+C) counts as a denial.
		return { block: true, reason: `User cancelled approval for: ${summary} (ask-to-edit mode)` };
	});
}
```

## 使用方式

```text
/edit-mode                  # 打开选择器
/edit-mode auto-edit        # 直接设置（Tab 可补全）
/auto-edit                  # 切到自动编辑
/ask-to-edit                # 切到确认编辑
Alt+E                       # 两种模式循环
pi --edit-mode auto-edit    # 启动时指定默认模式
```

- 当前模式显示在 footer 状态栏。
- 模式按 session 持久化（`pi.appendEntry`），`/reload`、`/resume`、`/tree` 后自动恢复；新 session 回到 `--edit-mode` 指定的默认值（未指定则 `ask-to-edit`）。
- `Allow all commands (this session)` 只对当前 session 生效，不写入 session、不跨会话，footer 会显示 `✎ ask-to-edit · bash:auto`。

## edit / write 确认弹窗

`ask-to-edit` 模式下每次 `edit` / `write` 都会弹出 diff 预览，选项：

- `Allow once`：只允许这一次
- `Allow all edits (switch to auto-edit)`：允许并切到 `auto-edit`
- `Deny`：拒绝；按 Esc / Ctrl+C 取消弹窗同样视为拒绝

`auto-edit` 模式下 `edit` / `write` 直接执行。

## bash 只读判定规则（fail-closed）

判定逻辑：先把命令按 `;`、`&&`、`||`、`|`、`&` 拆成多段，**每一段**都必须命中只读白名单，否则视为非只读。启发式、fail-closed，不是安全沙箱。

自动放行的判断：

1. 段内命令（取 basename）在白名单中，且带有的参数不触发写操作。
2. 输出重定向只允许到 `/dev/null`、`/dev/stderr`、`/dev/stdout`、`/dev/fd/*`；`2>&1` 这类 fd 重定向不算写文件。
3. 管道 / 列表两侧都必须是只读命令。

直接判为非只读的情况：

- 未识别的命令、`$VAR` 作为命令名
- 输出重定向到普通文件（`>` / `>>` / `&>`）
- 命令替换 `$(...)`、反引号，进程替换 `<(...)` / `>(...)`
- 写入类命令：`rm`、`mv`、`cp`、`mkdir`、`touch`、`chmod`、`chown`、`tee`、`dd`、`ln`、`truncate` 等
- `find` 携带 `-delete` / `-exec` / `-ok` / `-fprint` / `-fls`
- `sed -i` / `--in-place`、`sort -o` / `--output`
- 包装器 `env` / `time` / `nohup` / `timeout` 会递归检查内层命令；`sudo`、`source` / `.`、`eval`、`xargs`、`command` 一律视为非只读

常见只读命令（部分）：

| 类别 | 命令 |
|------|------|
| 文件 / 文本 | `ls` `cat` `head` `tail` `wc` `sort`（无 `-o`）`uniq` `cut` `tr` `grep` `rg` `diff` `find`（无写参数）`grep` `jq` |
| 系统信息 | `pwd` `echo` `which` `whoami` `uname` `date` `ps` `free` `df` `du` |
| git | `status` `log` `diff` `show` `branch`（仅列表）`tag`（仅列表）`rev-parse` `config --get/--list` `stash list` 等 |
| 包管理器 | `npm` / `pnpm` / `yarn` 的 `ls` `list` `view` `show` `outdated` `why` `config get` |
| 容器 | `docker ps/images/logs/inspect/compose logs` 等、`kubectl get/describe/logs/config view` |
| 运行时版本 | `node --version`、`python3 -V`、`go version`、`cargo --version` 等 |

非只读命令弹窗选项：

- `Allow once`：只允许这一次
- `Allow all commands (this session)`：本次 session 内所有 bash 命令不再询问（不持久化）
- `Deny`：拒绝

## 验证

扩展可被自动发现且无加载错误：

```bash
# 通过 pi 的扩展发现接口确认
extensions: ['/home/test/.pi/agent/extensions/edit-modes.ts']
errors: []
```

行为测试覆盖：26 条只读命令全部放行、32 条非只读命令全部弹窗/拒绝、`auto-edit` 下仍拦截非只读 bash、session 级放行与重置、无 UI 时只读放行 / 非只读阻止、edit 模式恢复。

## 注意事项

- 这是启发式守卫，不是沙箱；扩展以当前用户权限运行，shell 语法仍可能绕过（例如允许的只读命令自身带副作用）。
- 只拦 `bash` 工具，不拦用户手动执行的 `!` / `!!` 命令。
- 新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效。
