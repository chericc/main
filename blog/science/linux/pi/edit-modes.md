# pi 编辑模式、Bash 审批与危险命令守卫


pi（`@earendil-works/pi-coding-agent`）本身没有内置的权限 / 编辑确认模式，这类能力通过扩展（extension）实现。这里实现三种编辑模式，并对 `bash` 工具加只读守卫和危险命令审批。

## 行为概览

| 模式 | edit / write | bash 只读命令 | bash 其他命令 |
|------|--------------|---------------|-----------------|
| `ask-to-edit`（默认） | 弹 diff 确认 | 直接执行 | 弹窗确认 |
| `auto-edit` | 自动执行 | 直接执行 | 弹窗确认 |
| `auto-all` | 项目目录内自动执行，项目外弹 diff 确认 | 直接执行 | 命中危险规则才弹窗，其余自动执行 |

`ask-to-edit` / `auto-edit` 下 bash 行为一致：只读命令自动放行，非只读命令一律弹窗确认。`auto-all` 下只有危险命令（见下文）才弹窗。无 UI 时（`-p` / `--mode json`）需要审批的操作直接阻止并返回原因。

## 审批界面

`ask-to-edit`（以及 `auto-all` 下的项目外编辑）弹出的确认框是一个自定义 TUI 组件：

- 默认只显示前 6 行 diff（`+` 绿 / `-` 红 / `@@` 高亮），超出部分显示 `… N more lines`
- 点击预览区，或按 `v` / `ctrl+o`，打开**全屏可滚动的完整 diff**（overlay）
- 全屏中：`↑↓`、`PageUp` / `PageDown`、`Home` / `End`、鼠标滚轮滚动；点击任意处或按 `Esc` / `q` / `ctrl+c` 返回
- 选项 `Allow once` / `Allow all edits` / `Deny`：`↑↓` + `Enter`，或直接用鼠标点击选项行
- 鼠标交互依赖 `tuiMode: "fullscreen"`（见 [开发环境配置](../develop_env_setup.md)）；非 TUI 模式回退为普通 `select` 对话框

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
 *   auto-all               Run edit/write and bash without prompting, except:
 *                          - edit/write to paths outside the project directory;
 *                          - dangerous bash commands (sudo, rm -rf outside the
 *                            project, package/system changes, writes outside the
 *                            project, ...). See classifyDangerousBash.
 *
 * Bash in ask-to-edit / auto-edit (same in BOTH modes):
 *   - read-only commands run without asking;
 *   - any other command (writes, redirections, unknown commands, wrappers, ...)
 *     shows a confirmation dialog.
 *
 * Switch modes:
 *   /edit-mode [ask-to-edit|auto-edit|auto-all]   set explicitly (no arg opens a picker)
 *   /ask-to-edit                         switch to ask-to-edit
 *   /auto-edit                           switch to auto-edit
 *   /auto-all                            switch to auto-all
 *   Alt+E                                cycle through the three modes
 *
 * The active mode is shown in the footer and persisted per session, so it is
 * restored on /reload, /resume and tree navigation. The startup mode can be set
 * with `--edit-mode <mode>` (default: ask-to-edit).
 *
 * Scope: this gates the built-in `edit` and `write` tools, and the `bash` tool
 * for non-read-only commands. The bash checks are heuristic fail-closed
 * classifiers, not a security sandbox. In auto-all mode only known-dangerous
 * commands and common writes outside the project directory ask; whatever a
 * script, build tool or package manager does internally is not inspected.
 * In non-interactive sessions there is no UI to approve with, so blocked
 * actions are refused instead of silently allowed.
 */

import * as os from "node:os";
import * as path from "node:path";
import {
	isToolCallEventType,
	type EditToolCallEvent,
	type ExtensionAPI,
	type ExtensionContext,
	type Theme,
	type WriteToolCallEvent,
} from "@earendil-works/pi-coding-agent";
import {
	matchesKey,
	truncateToWidth,
	visibleWidth,
	type Component,
	type TUI,
	type TuiMouseEvent,
	type TuiMouseEventResult,
} from "@earendil-works/pi-tui";

type EditMode = "ask-to-edit" | "auto-edit" | "auto-all";

const MODES: readonly EditMode[] = ["ask-to-edit", "auto-edit", "auto-all"];
const STATE_TYPE = "edit-mode";
const STARTUP_FLAG = "edit-mode";
const STATUS_KEY = "edit-mode";

// Terminal-size-aware preview caps, so approval dialogs never grow taller than
// the window and push the options out of view. Re-evaluated on /reload.
const TERMINAL_ROWS =
	typeof process.stdout.rows === "number" && process.stdout.rows > 0 ? process.stdout.rows : 30;
const TERMINAL_COLUMNS =
	typeof process.stdout.columns === "number" && process.stdout.columns > 0 ? process.stdout.columns : 100;
const DIALOG_CHROME_LINES = 16; // borders, title, options, hint and spacers
const MAX_PREVIEW_LINES = Math.max(3, Math.min(15, TERMINAL_ROWS - DIALOG_CHROME_LINES));
const MAX_PREVIEW_WIDTH = Math.max(40, Math.min(120, TERMINAL_COLUMNS - 8));
const MAX_BASH_PREVIEW_LINES = Math.max(3, Math.min(12, TERMINAL_ROWS - DIALOG_CHROME_LINES));

function isEditMode(value: unknown): value is EditMode {
	return value === "ask-to-edit" || value === "auto-edit" || value === "auto-all";
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
	redirectTargets: string[];
}

const SAFE_REDIRECT_TARGETS = ["/dev/null", "/dev/stderr", "/dev/stdout"];

function tokenizeShell(command: string): TokenizedShell {
	const tokens: ShellToken[] = [];
	const redirectTargets: string[] = [];
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
				redirectTargets.push(target);
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
				while (command[i] === " " || command[i] === "\t") i++;
				const start = i;
				while (!isWordBoundary(command[i])) i++;
				const target = command.slice(start, i).replace(/^['"]|['"]$/g, "");
				if (target.length > 0) redirectTargets.push(target);
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
	return { tokens, hasWriteRedirection, hasSubstitution, redirectTargets };
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

function splitSegments(tokens: ShellToken[]): string[][] {
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
	return segments;
}

function analyzeBashCommand(command: string): { readOnly: boolean; reason: string } {
	const trimmed = command.trim();
	if (trimmed.length === 0) return { readOnly: true, reason: "empty command" };

	const { tokens, hasWriteRedirection, hasSubstitution } = tokenizeShell(command);
	if (hasSubstitution) return { readOnly: false, reason: "contains command or process substitution" };
	if (hasWriteRedirection) return { readOnly: false, reason: "contains output redirection to a file" };

	const segments = splitSegments(tokens);
	if (segments.length === 0) return { readOnly: true, reason: "empty command" };

	for (const segment of segments) {
		if (!analyzeWords(segment)) {
			return { readOnly: false, reason: `not read-only: ${segment.join(" ")}` };
		}
	}
	return { readOnly: true, reason: "read-only" };
}

// ---------------------------------------------------------------------------
// Dangerous command classifier (auto-all mode)
//
// In auto-all mode every command is auto-approved unless this classifier
// returns a reason. It combines a curated list of dangerous commands with
// path checks for common file-writing commands. It is best-effort, not a
// security boundary: scripts, build tools and package manager hooks are not
// inspected.
// ---------------------------------------------------------------------------

const ALWAYS_DANGEROUS = new Set([
	"sudo", "doas", "su",
	"shutdown", "reboot", "poweroff", "halt", "init",
	"mkfs", "fdisk", "parted", "wipefs", "blkdiscard", "shred", "dd",
	"mount", "umount", "swapon", "swapoff",
	"systemctl", "service",
	"crontab", "useradd", "userdel", "usermod", "groupadd", "groupdel", "passwd", "visudo",
	"iptables", "nft", "ufw", "firewall-cmd",
	"insmod", "rmmod", "modprobe", "chroot", "nsenter",
	"apt", "apt-get", "dpkg", "dnf", "yum", "rpm", "pacman", "zypper", "apk", "brew", "snap",
	"killall", "pkill", "killall5", "fuser",
]);

const SHELL_COMMANDS = new Set(["sh", "bash", "zsh", "dash", "ksh", "fish"]);

/** Commands whose path arguments are created or overwritten. */
const WRITE_PATH_COMMANDS = new Set([
	"cp", "mv", "ln", "install", "mkdir", "rmdir", "touch", "truncate", "tee", "rsync",
]);

const FIND_DESTRUCTIVE_FLAGS = new Set([
	"-delete", "-exec", "-execdir", "-ok", "-okdir", "-fprint", "-fprint0", "-fls", "-fprintf",
]);

/** Wrapper commands whose flags may take a separate value. */
const WRAPPER_VALUE_FLAGS: Record<string, Set<string>> = {
	env: new Set(["-u", "--unset", "-C", "--chdir", "-S", "--split-string"]),
	nice: new Set(["-n", "--adjustment"]),
	ionice: new Set(["-c", "--class", "-n", "--classdata"]),
	stdbuf: new Set(["-i", "--input", "-o", "--output", "-e", "--error"]),
	timeout: new Set(["-s", "--signal", "-k", "--kill-after"]),
	watch: new Set(["-n", "--interval"]),
	xargs: new Set(["-a", "--arg-file", "-d", "--delimiter", "-E", "--eof", "-I", "--replace", "-L", "--max-lines", "-n", "--max-args", "-P", "--max-procs", "-s", "--max-chars"]),
	nohup: new Set(),
	time: new Set(),
	command: new Set(),
	builtin: new Set(),
	exec: new Set(),
	setsid: new Set(),
};
const WRAPPER_COMMANDS = new Set(Object.keys(WRAPPER_VALUE_FLAGS));

function expandHome(p: string): string {
	if (p === "~") return os.homedir();
	if (p.startsWith("~/") || p.startsWith("~\\")) return path.join(os.homedir(), p.slice(2));
	return p;
}

function isWithin(root: string, target: string): boolean {
	const rel = path.relative(root, target);
	return rel === "" || (!rel.startsWith("..") && !path.isAbsolute(rel));
}

/** Resolve a path argument; undefined when it cannot be resolved statically. */
function resolvePathArg(p: string, cwd: string): string | undefined {
	const expanded = expandHome(p);
	if (/[$`]/.test(expanded)) return undefined;
	return path.resolve(cwd, expanded);
}

/** True when writing to `p` may leave the project dir (fail-closed on unknowns). */
function isUnsafeWritePath(p: string, cwd: string): boolean {
	if (p === "" || p === "-") return false;
	if (SAFE_REDIRECT_TARGETS.includes(p) || p.startsWith("/dev/fd/")) return false;
	const resolved = resolvePathArg(p, cwd);
	if (resolved === undefined) return true;
	return !isWithin(cwd, resolved) && !isWithin(os.tmpdir(), resolved);
}

/** True when `p` clearly points outside the project dir (fail-closed on unknowns). */
function isUnsafeReadPath(p: string, cwd: string): boolean {
	const resolved = resolvePathArg(p, cwd);
	if (resolved === undefined) return true;
	return !isWithin(cwd, resolved);
}

function isFlag(arg: string): boolean {
	return arg.startsWith("-") && arg !== "-";
}

function hasFlag(args: string[], ...names: string[]): boolean {
	return args.some((arg) => names.includes(arg));
}

function shortFlagCluster(args: string[], letters: string): boolean {
	return args.some((arg) => /^-[A-Za-z]+$/.test(arg) && [...letters].some((letter) => arg.includes(letter)));
}

function cleanCommandName(word: string): string {
	return baseName(word).replace(/^[({\[]+/, "").replace(/[)}\]]+$/, "");
}

function skipWrapperFlags(args: string[], valueFlags: Set<string>, skipAssignments: boolean): number {
	let j = 0;
	while (j < args.length) {
		const arg = args[j] as string;
		if (skipAssignments && /^[A-Za-z_][A-Za-z0-9_]*=/.test(arg)) {
			j++;
			continue;
		}
		if (!isFlag(arg)) break;
		if (arg.startsWith("--") && valueFlags.has(arg.split("=")[0] as string)) {
			j += arg.includes("=") ? 1 : 2;
			continue;
		}
		const short = arg.startsWith("--") ? undefined : arg.slice(0, 2);
		if (short !== undefined && valueFlags.has(short)) {
			j += arg.length > 2 ? 1 : 2;
			continue;
		}
		j++;
	}
	return j;
}

function parseSegment(
	words: string[],
	cwd: string,
	depth: number,
): { cmd?: string; args?: string[]; reason?: string } | undefined {
	let i = 0;
	while (i < words.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(words[i] as string)) i++;
	if (i >= words.length) return undefined;

	let cmd = cleanCommandName(words[i] as string);
	let args = words.slice(i + 1);

	for (let guard = 0; guard < 8 && WRAPPER_COMMANDS.has(cmd); guard++) {
		const valueFlags = WRAPPER_VALUE_FLAGS[cmd] as Set<string>;
		let j = skipWrapperFlags(args, valueFlags, cmd === "env");
		if (cmd === "timeout" && j < args.length) j++; // positional duration
		if (j >= args.length) return undefined;
		cmd = cleanCommandName(args[j] as string);
		args = args.slice(j + 1);
	}

	if (SHELL_COMMANDS.has(cmd)) {
		const cIndex = args.findIndex((arg) => arg === "-c" || arg === "--command");
		const script = cIndex >= 0 ? args[cIndex + 1] : undefined;
		if (script !== undefined) {
			const reason = classifyDangerousBash(script, cwd, depth + 1);
			return reason ? { reason } : undefined;
		}
		return { cmd, args };
	}

	if (cmd === "eval") {
		const script = args.filter((arg) => !isFlag(arg)).join(" ");
		if (script.length > 0) {
			const reason = classifyDangerousBash(script, cwd, depth + 1);
			return reason ? { reason } : undefined;
		}
		return undefined;
	}

	if (cmd === "source" || cmd === ".") {
		const target = args.find((arg) => !isFlag(arg));
		if (target !== undefined && isUnsafeReadPath(target, cwd)) {
			return { reason: `sourcing a script outside the project: ${target}` };
		}
		return undefined;
	}

	return { cmd, args };
}

function rmDanger(args: string[], cwd: string): string | undefined {
	const recursive = shortFlagCluster(args, "rR") || hasFlag(args, "--recursive");
	const force = shortFlagCluster(args, "f") || hasFlag(args, "--force");
	const targets = args.filter((arg) => !isFlag(arg));
	if (!recursive && !force) {
		for (const target of targets) {
			if (isUnsafeReadPath(target, cwd)) return `rm on a path outside the project: ${target}`;
		}
		return undefined;
	}
	if (targets.length === 0) return "rm with recursive/force flags and no explicit target";
	for (const target of targets) {
		const resolved = resolvePathArg(target, cwd);
		if (resolved === undefined) return `rm on an unresolved path: ${target}`;
		if (resolved === path.resolve(cwd)) return `rm targeting the project root: ${target}`;
		if (isUnsafeWritePath(target, cwd)) return `rm outside the project: ${target}`;
	}
	return undefined;
}

function permissionDanger(cmd: string, args: string[], cwd: string): string | undefined {
	if (shortFlagCluster(args, "R") || hasFlag(args, "--recursive")) return `recursive ${cmd}`;
	const positional = args.filter((arg) => !isFlag(arg));
	if (positional.length < 2) return undefined;
	if (cmd === "chmod" && positional[0] === "777") return "chmod 777";
	for (const target of positional.slice(1)) {
		if (isUnsafeReadPath(target, cwd)) return `${cmd} on a path outside the project: ${target}`;
	}
	return undefined;
}

function writePathDanger(cmd: string, args: string[], cwd: string): string | undefined {
	const targets = args.filter((arg) => !isFlag(arg));
	if (targets.length === 0) return undefined;
	const check = (target: string): string | undefined =>
		isUnsafeWritePath(target, cwd) ? `${cmd} writing outside the project: ${target}` : undefined;
	if (cmd === "cp" || cmd === "mv" || cmd === "install" || cmd === "ln" || cmd === "rsync") {
		return check(targets[targets.length - 1] as string);
	}
	for (const target of targets) {
		const reason = check(target);
		if (reason) return reason;
	}
	return undefined;
}

function tarDanger(args: string[], cwd: string): string | undefined {
	for (let i = 0; i < args.length; i++) {
		const arg = args[i] as string;
		if ((arg === "-C" || arg === "--directory") && args[i + 1] !== undefined) {
			const dir = args[i + 1] as string;
			if (isUnsafeWritePath(dir, cwd)) return `tar extracting outside the project: ${dir}`;
			i++;
		}
	}
	if (shortFlagCluster(args, "x") && shortFlagCluster(args, "P")) return "tar extracting with absolute paths";
	return undefined;
}

function unzipDanger(args: string[], cwd: string): string | undefined {
	const index = args.findIndex((arg) => arg === "-d" || arg === "--directory");
	const dir = index >= 0 ? args[index + 1] : undefined;
	if (dir !== undefined && isUnsafeWritePath(dir, cwd)) return `unzip extracting outside the project: ${dir}`;
	return undefined;
}

function downloadDanger(cmd: string, args: string[], cwd: string): string | undefined {
	const valueFlags =
		cmd === "curl"
			? new Set(["-o", "--output", "--output-dir"])
			: new Set(["-O", "--output-document", "-P", "--directory-prefix"]);
	for (let i = 0; i < args.length; i++) {
		const arg = args[i] as string;
		let value: string | undefined;
		if (valueFlags.has(arg)) {
			value = args[i + 1];
			i++;
		} else if (arg.startsWith("--")) {
			const equalsIndex = arg.indexOf("=");
			if (equalsIndex > 0 && valueFlags.has(arg.slice(0, equalsIndex))) value = arg.slice(equalsIndex + 1);
		}
		if (value !== undefined && isUnsafeWritePath(value, cwd)) {
			return `${cmd} writing outside the project: ${value}`;
		}
	}
	return undefined;
}

function gitDanger(args: string[]): string | undefined {
	let i = 0;
	while (i < args.length && isFlag(args[i] as string)) {
		i += ["-C", "-c", "--git-dir", "--work-tree", "--config-env"].includes(args[i] as string) ? 2 : 1;
	}
	const sub = args[i];
	const rest = args.slice(i + 1);
	if (sub === undefined) return undefined;
	if (sub === "reset" && rest.includes("--hard")) return "git reset --hard discards local changes";
	if (sub === "clean" && (rest.includes("--force") || shortFlagCluster(rest, "f"))) return "git clean deletes untracked files";
	if (sub === "push" && (rest.includes("--force") || shortFlagCluster(rest, "f"))) return "git push --force rewrites remote history";
	if (sub === "push" && (rest.includes("--delete") || rest.includes("-d"))) return "git push --delete removes a remote branch";
	if (sub === "branch" && rest.includes("-D")) return "git branch -D force-deletes a branch";
	if (sub === "stash" && (rest[0] === "clear" || rest[0] === "drop")) return "git stash clear/drop discards stashes";
	if ((sub === "checkout" || sub === "restore") && rest.includes(".")) return `git ${sub} . discards uncommitted changes`;
	if (sub === "filter-branch" || sub === "filter-repo") return `git ${sub} rewrites history`;
	return undefined;
}

function packageManagerDanger(cmd: string, args: string[]): string | undefined {
	if (cmd === "npm" || cmd === "yarn" || cmd === "pnpm" || cmd === "bun") {
		if (hasFlag(args, "-g", "--global") || (cmd === "yarn" && args[0] === "global")) return `${cmd} global install`;
		if (args.includes("publish")) return `${cmd} publish`;
		return undefined;
	}
	if (cmd === "pip" || cmd === "pip3") {
		if (args[0] === "uninstall") return "pip uninstall";
		if (args[0] === "install" && (hasFlag(args, "--user", "--system", "--break-system-packages") || args.includes("--target"))) {
			return "pip install outside the current environment";
		}
		return undefined;
	}
	if ((cmd === "gem" || cmd === "cargo" || cmd === "go" || cmd === "dotnet") && args[0] === "install") {
		return `${cmd} install writes outside the project`;
	}
	if (cmd === "make" && args.filter((arg) => !isFlag(arg)).some((target) => target === "install" || target === "uninstall")) {
		return "make install";
	}
	if (cmd === "cmake" && args.includes("--install")) return "cmake --install";
	return undefined;
}

function dangerForCommand(cmd: string, args: string[], cwd: string): string | undefined {
	if (cmd.length === 0) return undefined;
	if (ALWAYS_DANGEROUS.has(cmd) || cmd.startsWith("mkfs.")) return `privileged or system-level command: ${cmd}`;
	if (cmd === "rm") return rmDanger(args, cwd);
	if (cmd === "chmod" || cmd === "chown") return permissionDanger(cmd, args, cwd);
	if (WRITE_PATH_COMMANDS.has(cmd)) return writePathDanger(cmd, args, cwd);
	if (cmd === "tar") return tarDanger(args, cwd);
	if (cmd === "unzip") return unzipDanger(args, cwd);
	if (cmd === "find" || cmd === "fd" || cmd === "fdfind") {
		return args.some((arg) => FIND_DESTRUCTIVE_FLAGS.has(arg)) ? `${cmd} with a destructive action flag` : undefined;
	}
	if (cmd === "git") return gitDanger(args);
	if (cmd === "cd" || cmd === "pushd") {
		const target = args.find((arg) => !isFlag(arg));
		return target !== undefined && isUnsafeReadPath(target, cwd)
			? `changing directory outside the project: ${target}`
			: undefined;
	}
	if (cmd === "curl" || cmd === "wget") return downloadDanger(cmd, args, cwd);
	return packageManagerDanger(cmd, args);
}

function classifyDangerousBash(command: string, cwd: string, depth = 0): string | undefined {
	if (depth > 3) return "command nesting too deep to analyze";
	const trimmed = command.trim();
	if (trimmed.length === 0) return undefined;

	const { tokens, hasSubstitution, redirectTargets } = tokenizeShell(command);
	for (const target of redirectTargets) {
		if (isUnsafeWritePath(target, cwd)) return `output redirection outside the project: ${target}`;
	}
	if (hasSubstitution && /(?:^|[;&|]\s*)(eval|source)\b/.test(trimmed)) {
		return "evaluating command substitution";
	}

	const segments = splitSegments(tokens);
	const commandNames: string[] = [];
	for (const segment of segments) {
		const parsed = parseSegment(segment, cwd, depth);
		if (!parsed) continue;
		if (parsed.reason) return parsed.reason;
		if (parsed.cmd === undefined || parsed.args === undefined) continue;
		commandNames.push(parsed.cmd);
		const reason = dangerForCommand(parsed.cmd, parsed.args, cwd);
		if (reason) return reason;
	}

	const piped = command.split("||").some((part) => part.includes("|"));
	if (piped && commandNames.some((name) => SHELL_COMMANDS.has(name))) return "piping into a shell";
	return undefined;
}

// ---------------------------------------------------------------------------
// Full-diff review UI
//
// The approval dialog shows a short collapsed preview. Clicking the preview (or
// pressing "v") opens a fullscreen, scrollable overlay so the whole change can
// be reviewed before deciding. The overlay is a separate ctx.ui.custom() call so
// the two UIs never nest.
// ---------------------------------------------------------------------------

type DiffLineKind = "add" | "del" | "hunk" | "context";

interface DiffLine {
	text: string;
	kind: DiffLineKind;
}

type ApprovalChoice = "allow" | "allow-all" | "deny";
type ApprovalResult = ApprovalChoice | "view" | undefined;

function buildEditDiffLines(input: EditToolCallEvent["input"]): DiffLine[] {
	const lines: DiffLine[] = [];
	input.edits.forEach((edit, index) => {
		if (input.edits.length > 1) {
			lines.push({ text: `@@ change ${index + 1} of ${input.edits.length} @@`, kind: "hunk" });
		}
		for (const line of edit.oldText.split("\n")) lines.push({ text: `- ${line}`, kind: "del" });
		for (const line of edit.newText.split("\n")) lines.push({ text: `+ ${line}`, kind: "add" });
	});
	return lines;
}

function buildWriteDiffLines(input: WriteToolCallEvent["input"]): DiffLine[] {
	return input.content.split("\n").map((line) => ({ text: `+ ${line}`, kind: "add" as const }));
}

function colorDiffLine(line: DiffLine, theme: Theme): string {
	const color =
		line.kind === "add"
			? "toolDiffAdded"
			: line.kind === "del"
				? "toolDiffRemoved"
				: line.kind === "hunk"
					? "accent"
					: "toolDiffContext";
	return theme.fg(color, line.text);
}

/** Lines shown in the collapsed approval dialog before the user expands. */
const COLLAPSED_PREVIEW_LINES = 6;

const APPROVAL_OPTIONS: ReadonlyArray<{ label: string; choice: ApprovalChoice }> = [
	{ label: "Allow once", choice: "allow" },
	{ label: "Allow all edits (switch to auto-edit)", choice: "allow-all" },
	{ label: "Deny", choice: "deny" },
];

class ApprovalDialog implements Component {
	private selected = 0;
	private previewTop = -1;
	private previewBottom = -1;
	private optionRows: Array<{ start: number; end: number; choice: ApprovalChoice }> = [];

	constructor(
		private readonly mode: EditMode,
		private readonly summary: string,
		private readonly lines: DiffLine[],
		private readonly theme: Theme,
		private readonly tui: TUI,
		private readonly done: (value: ApprovalResult) => void,
	) {}

	handleInput(data: string): void {
		if (matchesKey(data, "escape") || matchesKey(data, "ctrl+c")) {
			this.done(undefined);
			return;
		}
		if (matchesKey(data, "up")) {
			this.selected = Math.max(0, this.selected - 1);
			this.tui.requestRender();
			return;
		}
		if (matchesKey(data, "down")) {
			this.selected = Math.min(APPROVAL_OPTIONS.length - 1, this.selected + 1);
			this.tui.requestRender();
			return;
		}
		if (matchesKey(data, "enter")) {
			this.done(APPROVAL_OPTIONS[this.selected]?.choice);
			return;
		}
		if (matchesKey(data, "v") || matchesKey(data, "ctrl+o")) {
			this.done("view");
		}
	}

	handleMouse(event: TuiMouseEvent): TuiMouseEventResult | undefined {
		if (event.type !== "click" || event.button !== "left") return undefined;
		if (this.previewTop >= 0 && event.y >= this.previewTop && event.y <= this.previewBottom) {
			this.done("view");
			return { handled: true };
		}
		for (const row of this.optionRows) {
			if (event.y >= row.start && event.y <= row.end) {
				this.done(row.choice);
				return { handled: true };
			}
		}
		return undefined;
	}

	render(width: number): string[] {
		const th = this.theme;
		const out: string[] = [];
		out.push(truncateToWidth(th.fg("warning", th.bold(`${this.mode}: ${this.summary}`)), width, "…"));
		out.push("");

		const shown = this.lines.slice(0, COLLAPSED_PREVIEW_LINES);
		this.previewTop = out.length;
		for (const line of shown) {
			out.push(truncateToWidth(`  ${colorDiffLine(line, th)}`, width, "…"));
		}
		this.previewBottom = out.length - 1;

		const hidden = this.lines.length - shown.length;
		const more = hidden > 0 ? `${th.fg("muted", `… ${hidden} more line${hidden === 1 ? "" : "s"}`)} ` : "";
		const viewHint = th.fg("dim", hidden > 0 ? "[ click here or press v to view full ]" : "[ press v to view full ]");
		out.push(truncateToWidth(`  ${more}${viewHint}`, width, "…"));
		out.push("");

		this.optionRows = [];
		APPROVAL_OPTIONS.forEach((option, index) => {
			const start = out.length;
			const marker = index === this.selected ? th.fg("accent", "▶") : " ";
			const label = index === this.selected ? th.bold(option.label) : th.fg("text", option.label);
			out.push(truncateToWidth(` ${marker} ${label}`, width, "…"));
			this.optionRows.push({ start, end: out.length - 1, choice: option.choice });
		});

		out.push("");
		out.push(truncateToWidth(th.fg("dim", " ↑↓ select · enter confirm · v full diff · esc cancel"), width, "…"));
		return out;
	}

	invalidate(): void {}
}

class DiffViewer implements Component {
	private offset = 0;

	constructor(
		private readonly title: string,
		private readonly lines: DiffLine[],
		private readonly theme: Theme,
		private readonly tui: TUI,
		private readonly done: () => void,
	) {}

	private viewportHeight(): number {
		return Math.max(3, this.tui.terminal.rows - 5);
	}

	private maxOffset(): number {
		return Math.max(0, this.lines.length - this.viewportHeight());
	}

	handleInput(data: string): void {
		if (matchesKey(data, "escape") || matchesKey(data, "ctrl+c") || matchesKey(data, "q")) {
			this.done();
			return;
		}
		const page = Math.max(1, this.viewportHeight() - 1);
		if (matchesKey(data, "up")) this.offset = Math.max(0, this.offset - 1);
		else if (matchesKey(data, "down")) this.offset = Math.min(this.maxOffset(), this.offset + 1);
		else if (matchesKey(data, "pageUp")) this.offset = Math.max(0, this.offset - page);
		else if (matchesKey(data, "pageDown")) this.offset = Math.min(this.maxOffset(), this.offset + page);
		else if (matchesKey(data, "home")) this.offset = 0;
		else if (matchesKey(data, "end")) this.offset = this.maxOffset();
		else return;
		this.tui.requestRender();
	}

	handleMouse(event: TuiMouseEvent): TuiMouseEventResult | undefined {
		if (event.type === "wheel" && typeof event.wheelDelta === "number") {
			this.offset = Math.max(0, Math.min(this.maxOffset(), this.offset + event.wheelDelta));
			this.tui.requestRender();
			return { handled: true };
		}
		if (event.type === "click" && event.button === "left") {
			this.done();
			return { handled: true };
		}
		return undefined;
	}

	render(width: number): string[] {
		const th = this.theme;
		const innerWidth = Math.max(1, width - 2);
		const border = (text: string) => th.fg("border", text);
		const padLine = (text: string) => {
			const clipped = truncateToWidth(` ${text}`, innerWidth, "…");
			const padding = Math.max(0, innerWidth - visibleWidth(clipped));
			return `${clipped}${" ".repeat(padding)}`;
		};

		const viewport = this.viewportHeight();
		const out: string[] = [];
		const titleText = truncateToWidth(` ${this.title} `, innerWidth, "…");
		const topFill = "─".repeat(Math.max(0, innerWidth - visibleWidth(titleText)));
		out.push(border("╭") + th.fg("accent", titleText) + border(`${topFill}╮`));

		const first = this.lines.length === 0 ? 0 : this.offset + 1;
		const last = Math.min(this.lines.length, this.offset + viewport);
		const info = `lines ${first}-${last} / ${this.lines.length}  ·  ↑↓/wheel/pageUp-pageDown · home/end · click/esc close`;
		out.push(border("│") + padLine(th.fg("dim", info)) + border("│"));

		const shown = this.lines.slice(this.offset, this.offset + viewport);
		for (const line of shown) out.push(border("│") + padLine(colorDiffLine(line, th)) + border("│"));
		for (let i = shown.length; i < viewport; i++) out.push(border("│") + padLine("") + border("│"));

		out.push(border(`╰${"─".repeat(innerWidth)}╯`));
		return out;
	}

	invalidate(): void {}
}

/**
 * Show the edit/write approval UI. In TUI mode this uses a custom component with
 * a collapsed preview and a fullscreen scrollable diff; other modes fall back to
 * the plain select dialog.
 */
async function approveEditChange(
	ctx: ExtensionContext,
	mode: EditMode,
	summary: string,
	lines: DiffLine[],
): Promise<ApprovalResult> {
	if (ctx.mode !== "tui") {
		const preview = truncateForPreview(
			lines.map((line) => line.text).join("\n"),
			MAX_PREVIEW_LINES,
			MAX_PREVIEW_WIDTH,
		);
		const choice = await ctx.ui.select(
			`${mode}: ${summary}\n\n${preview}\n`,
			APPROVAL_OPTIONS.map((option) => option.label),
		);
		if (choice === APPROVAL_OPTIONS[0]!.label) return "allow";
		if (choice === APPROVAL_OPTIONS[1]!.label) return "allow-all";
		if (choice === APPROVAL_OPTIONS[2]!.label) return "deny";
		return undefined;
	}

	for (;;) {
		const result = await ctx.ui.custom<ApprovalResult>((tui, theme, _keybindings, done) =>
			new ApprovalDialog(mode, summary, lines, theme, tui, done),
		);
		if (result !== "view") return result;

		await ctx.ui.custom<void>(
			(tui, theme, _keybindings, done) => new DiffViewer(summary, lines, theme, tui, done),
			{
				overlay: true,
				overlayOptions: { width: "100%", maxHeight: "100%", margin: 0, anchor: "center" },
			},
		);
	}
}

// ---------------------------------------------------------------------------
// Extension
// ---------------------------------------------------------------------------

export default function (pi: ExtensionAPI) {
	let mode: EditMode = "ask-to-edit";
	let bashAutoApprove = false;

	pi.registerFlag(STARTUP_FLAG, {
		description: "Start in an edit approval mode: ask-to-edit | auto-edit | auto-all",
		type: "string",
		default: "ask-to-edit",
	});

	function applyStatus(ctx: ExtensionContext): void {
		if (!ctx.hasUI) return;
		const color = mode === "ask-to-edit" ? "warning" : mode === "auto-edit" ? "success" : "accent";
		const suffix = bashAutoApprove ? " · bash:auto" : "";
		ctx.ui.setStatus(STATUS_KEY, ctx.ui.theme.fg(color, `${mode}${suffix}`));
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
		description: "Set edit approval mode (ask-to-edit | auto-edit | auto-all)",
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
				ctx.ui.notify(`Unknown edit mode: ${arg}. Use ${MODES.join(", ")}.`, "error");
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

	pi.registerCommand("auto-all", {
		description: "Auto-run everything; only dangerous commands and edits outside the project ask",
		handler: async (_args, ctx) => setMode("auto-all", ctx),
	});

	pi.registerShortcut("alt+e", {
		description: "Cycle edit mode (ask-to-edit → auto-edit → auto-all)",
		handler: async (ctx) => {
			const next: EditMode =
				mode === "ask-to-edit" ? "auto-edit" : mode === "auto-edit" ? "auto-all" : "ask-to-edit";
			setMode(next, ctx);
		},
	});

	pi.on("tool_call", async (event, ctx) => {
		// --- bash: read-only allowed; in auto-all only dangerous commands ask ---
		if (isToolCallEventType("bash", event)) {
			if (bashAutoApprove) return;

			const command = event.input.command;
			let askReason: string | undefined;
			if (mode === "auto-all") {
				askReason = classifyDangerousBash(command, ctx.cwd);
			} else {
				const verdict = analyzeBashCommand(command);
				if (!verdict.readOnly) askReason = verdict.reason;
			}
			if (askReason === undefined) return;

			const preview = truncateForPreview(command, MAX_BASH_PREVIEW_LINES, MAX_PREVIEW_WIDTH);
			const label = mode === "auto-all" ? "⚠ Dangerous bash" : "⚠ Non-read-only bash";

			if (!ctx.hasUI) {
				return {
					block: true,
					reason:
						`Blocked by ${mode} guard: ${askReason}, and no interactive UI is available ` +
						`to approve it. Run a safe command or use an interactive session.`,
				};
			}

			const choice = await ctx.ui.select(
				`${label} (${askReason}):\n\n$ ${preview}\n`,
				["Allow once", "Allow all commands (this session)", "Deny"],
			);

			if (choice === "Allow once") return;
			if (choice === "Allow all commands (this session)") {
				bashAutoApprove = true;
				applyStatus(ctx);
				ctx.ui.notify("Bash: auto-approving all commands for this session", "warning");
				return;
			}
			return { block: true, reason: `User denied bash command: ${askReason}` };
		}

		// --- edit/write: gated in ask-to-edit, and outside the project in auto-all ---
		if (mode === "auto-edit") return;

		const isEditCall = isToolCallEventType("edit", event);
		const isWriteCall = isToolCallEventType("write", event);
		if (!isEditCall && !isWriteCall) return;

		const targetPath = event.input.path;
		if (mode === "auto-all") {
			const resolved = resolvePathArg(targetPath, ctx.cwd);
			if (resolved !== undefined && isWithin(ctx.cwd, resolved)) return;
		}

		const summary = isEditCall
			? `edit ${targetPath} (${event.input.edits.length} change${event.input.edits.length === 1 ? "" : "s"})`
			: `write ${targetPath}`;

		if (!ctx.hasUI) {
			return {
				block: true,
				reason:
					`Blocked by ${mode} mode: ${summary} needs approval, but no interactive UI is available. ` +
					`Switch to auto-edit (--edit-mode auto-edit) to allow edits in non-interactive sessions.`,
			};
		}

		const lines = isEditCall ? buildEditDiffLines(event.input) : buildWriteDiffLines(event.input);
		const decision = await approveEditChange(ctx, mode, summary, lines);

		if (decision === "allow") return;
		if (decision === "allow-all") {
			setMode("auto-edit", ctx);
			return;
		}
		if (decision === "deny") {
			return { block: true, reason: `User denied: ${summary} (${mode} mode)` };
		}

		// Cancelled the dialog (Esc / Ctrl+C) counts as a denial.
		return { block: true, reason: `User cancelled approval for: ${summary} (${mode} mode)` };
	});
}
```

## 使用方式

```text
/edit-mode                  # 打开选择器
/edit-mode auto-edit        # 直接设置（Tab 可补全）
/edit-mode auto-all
/auto-edit                  # 切到自动编辑
/auto-all                   # 切到全自动（危险命令仍弹窗）
/ask-to-edit                # 切到确认编辑
Alt+E                       # 三种模式循环：ask → auto-edit → auto-all
pi --edit-mode auto-edit    # 启动时指定默认模式（也可 auto-all）
```

- 当前模式显示在 footer 状态栏。
- 模式按 session 持久化（`pi.appendEntry`），`/reload`、`/resume`、`/tree` 后自动恢复；新 session 回到 `--edit-mode` 指定的默认值（未指定则 `ask-to-edit`）。
- `Allow all commands (this session)` 只对当前 session 生效，不写入 session、不跨会话，footer 会显示 `ask-to-edit · bash:auto`。
- `auto-all` 的 footer 显示 `auto-all`（accent 色）。

## edit / write 确认弹窗

`ask-to-edit` 模式下每次 `edit` / `write` 都会弹出 diff 预览，选项：

- `Allow once`：只允许这一次
- `Allow all edits (switch to auto-edit)`：允许并切到 `auto-edit`
- `Deny`：拒绝；按 Esc / Ctrl+C 取消弹窗同样视为拒绝

`auto-edit` 模式下 `edit` / `write` 直接执行；`auto-all` 模式下路径在 cwd（项目目录）内的直接执行，项目外仍弹同样的 diff 确认。

弹窗预览按终端高度自适应并设置了固定上限（edit / write 最多 15 行，bash 最多 12 行，宽度也按终端宽度截断），避免内容过长把窗口占满、选项不可见；被截断时末尾显示 `… (N more lines)`，完整内容仍可在上方的工具调用块中展开查看。

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

## auto-all 危险命令判定

`auto-all` 下 bash 默认自动执行，只有命中以下规则才弹窗（启发式、fail-closed）：

- **系统 / 特权命令**：`sudo` `doas` `su`；`shutdown` `reboot` `poweroff` `halt` `init`；`mkfs*` `fdisk` `parted` `wipefs` `blkdiscard` `shred` `dd`；`mount` `umount` `swapon` `swapoff`；`systemctl` `service`；`crontab` `useradd` `userdel` `usermod` `groupadd` `groupdel` `passwd` `visudo`；`iptables` `nft` `ufw`；`chroot` `nsenter` `insmod` `rmmod` `modprobe`；`killall` `pkill` `killall5` `fuser`
- **系统包管理**：`apt` `apt-get` `dpkg` `dnf` `yum` `rpm` `pacman` `zypper` `apk` `brew` `snap`
- **路径写到项目外**（项目 = 启动时的 cwd；`/tmp` 白名单）：
  - 输出重定向 `>` / `>>` / `&>` 到项目外文件（`/dev/null`、`/dev/fd/*` 除外）
  - `rm` 递归 / 强制删除项目外路径，或删除项目根目录本身
  - `cp` `mv` `ln` `install` `rsync` 的目标路径；`mkdir` `rmdir` `touch` `truncate` `tee` 的路径参数；`tar -C`、`unzip -d`、`curl -o` / `wget -O` 的输出路径
  - `cd` / `pushd` 到项目外
  - `chmod -R`、`chmod 777`、`chown` 项目外路径
- **git 破坏性操作**：`reset --hard`、`clean -f`、`push --force` / `push --delete`、`branch -D`、`stash clear/drop`、`filter-branch` / `filter-repo`、`checkout .` / `restore .`
- **包管理器副作用**：`npm` / `yarn` / `pnpm` / `bun` 带 `-g` / `--global` 或 `publish`；`pip uninstall`、`pip install --user/--system/--target`；`gem install`、`cargo install`、`go install`、`dotnet install`；`make install`；`cmake --install`
- **其他**：`find` 带 `-delete` / `-exec` / `-ok` / `-fprint*`；`curl ... | sh` 之类管道进 shell；`eval` / `source` + 命令替换；路径含 `$VAR` / 反引号无法静态解析；`~` 展开为 home 后再判断

包装器 `env` / `time` / `timeout` / `nohup` / `nice` / `ionice` / `stdbuf` / `setsid` / `command` / `builtin` / `exec` / `xargs` 会被拆开检查内层命令，`bash -c '...'` / `sh -c '...'` / `eval '...'` 会递归检查（最多 4 层）。

edit / write 在 `auto-all` 下只做路径判断：解析后的路径在 cwd 内（含子目录）直接执行，项目外或含 `$` 无法解析时弹 diff 确认。

> 再次强调：这是启发式判断，不是安全沙箱。脚本、构建工具、包管理器内部实际做的事情不会被检查。

## 验证

扩展可被自动发现且无加载错误：

```bash
# 通过 pi 的扩展发现接口确认
extensions: ['/home/test/.pi/agent/extensions/edit-modes.ts']
errors: []
```

行为测试覆盖：26 条只读命令全部放行、32 条非只读命令全部弹窗/拒绝、`auto-edit` 下仍拦截非只读 bash、session 级放行与重置、无 UI 时只读放行 / 非只读阻止、edit 模式恢复。`auto-all` 的危险规则为启发式，未做穷举测试。

## 注意事项

- 这是启发式守卫，不是沙箱；扩展以当前用户权限运行，shell 语法仍可能绕过（例如允许的只读命令自身带副作用）。
- `auto-all` 只检查命令文本，不检查脚本 / 构建工具 / 包管理器内部实际写入的内容；`kill`、`docker` 等未列入危险表的命令默认放行。
- 只拦 `bash` 工具，不拦用户手动执行的 `!` / `!!` 命令。
- 新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效。
