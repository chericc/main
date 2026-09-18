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
 *                          - edit/write to paths outside the project directory
 *                            (the temp dir, e.g. /tmp, counts as inside);
 *                          - dangerous bash commands (sudo, rm -rf outside the
 *                            project, package/system changes, writes outside the
 *                            project, ...). See classifyDangerousBash;
 *                          - `git commit`, which ALWAYS asks (see below).
 *
 * Bash in ask-to-edit / auto-edit (same in BOTH modes):
 *   - read-only commands run without asking;
 *   - any other command (writes, redirections, unknown commands, wrappers, ...)
 *     shows a confirmation dialog;
 *   - `git commit` always asks in every mode (see next paragraph).
 *
 * git commit (all modes):
 *   - never auto-approved: `git commit` always shows a confirmation dialog, in
 *     every edit mode and even after "Allow all commands (this session)". That
 *     dialog offers only "Allow once" / "Deny"; without an interactive UI the
 *     command is refused. Deliberate: no pi mode may create a commit on its own.
 *     See isGitCommitCommand for what is detected.
 *
 * Switch modes:
 *   /edit-mode [ask-to-edit|auto-edit|auto-all]   set explicitly (no arg opens a picker)
 *   /ask-to-edit                         switch to ask-to-edit
 *   /auto-edit                           switch to auto-edit
 *   /auto-all                            switch to auto-all
 *   Shift+Tab                            cycle through the three modes
 *
 * The active mode is shown in the footer and persisted per session, so it is
 * restored on /reload, /resume and tree navigation. The startup mode can be set
 * with `--edit-mode <mode>` (default: ask-to-edit).
 *
 * Scope: this gates the built-in `edit` and `write` tools, and the `bash` tool
 * for non-read-only commands. The bash checks are heuristic fail-closed
 * classifiers, not a security sandbox. In auto-all mode only known-dangerous
 * commands and common writes outside the project directory ask; whatever a
 * script, build tool or package manager does internally is not inspected. The
 * git commit guard is a plain text match for `git commit`, so wrappers such as
 * `sudo git commit` are caught while `git -C dir commit` is not; a script that
 * commits internally is not detected either.
 * In non-interactive sessions there is no UI to approve with, so blocked
 * actions are refused instead of silently allowed.
 */

import * as os from "node:os";
import * as path from "node:path";
import { createRequire } from "node:module";
import { Parser, Language } from "web-tree-sitter";
import {
	isToolCallEventType,
	type EditToolCallEvent,
	type ExtensionAPI,
	type ExtensionContext,
	type Theme,
	type WriteToolCallEvent,
} from "@earendil-works/pi-coding-agent";
import {
	Text,
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
// Read-only allow-list (used by the tree-sitter AST classifier below)
// ---------------------------------------------------------------------------

const SAFE_REDIRECT_TARGETS = ["/dev/null", "/dev/stderr", "/dev/stdout"];

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
	"read", "declare", "typeset", "local", "readonly", "let", "shift", "return",
	"break", "continue", "hash", "getopts", "mapfile", "readarray",
]);

/** sed script commands that execute helpers or write files. */
const SED_SIDE_EFFECT_PATTERNS = [/[^\\]e\s/, /^e\s/, /[^\\]w\s/, /^w\s/, /[^\\]r\s/, /^r\s/];

/** awk programs that run commands, write files or read via getline. */
const AWK_SIDE_EFFECT_PATTERNS = [
	/(^|[^A-Za-z_])system\s*\(/,
	/(print|printf)[^>|]*>>?\s*"/,
	/(print|printf)[^|]*\|\s*"/,
	/getline\s*</,
	/\|\s*getline/,
];

const GIT_READ_ONLY_SUBCOMMANDS = new Set([
	"status", "log", "diff", "show", "describe", "rev-parse", "rev-list", "ls-files", "ls-tree",
	"cat-file", "blame", "shortlog", "show-ref", "for-each-ref", "name-rev", "merge-base",
	"whatchanged", "grep", "fsck", "count-objects", "check-ignore", "check-attr", "version",
	"help", "ls-remote", "var",
]);

/** git global flags that take no value and may precede the subcommand. */
const GIT_GLOBAL_NO_VALUE_FLAGS = new Set([
	"--no-pager", "--paginate", "-p", "--bare", "--literal-pathspecs", "--glob-pathspecs",
	"--noglob-pathspecs", "--icase-pathspecs", "--no-replace-objects", "--no-optional-locks",
]);

/**
 * git subcommand flags that write files or execute helpers, so the subcommand
 * is not read-only even when the verb is. Mirrors OpenAI Codex CLI.
 */
const GIT_UNSAFE_ARGS = ["--output", "--ext-diff", "--textconv", "--exec", "--paginate"];

function isUnsafeGitArg(arg: string): boolean {
	return (
		GIT_UNSAFE_ARGS.includes(arg) ||
		arg.startsWith("--output=") ||
		arg.startsWith("--exec=") ||
		arg.startsWith("--ext-diff=") ||
		arg.startsWith("--textconv=")
	);
}

function gitReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	const rest = args.slice(1);

	if (sub.startsWith("-")) {
		if (sub === "--version" || sub === "-v" || sub === "--help" || sub === "-h") return true;
		if (GIT_GLOBAL_NO_VALUE_FLAGS.has(sub)) return gitReadOnly(rest);
		// Global override flags (-C, -c, --git-dir, --work-tree, --config-env,
		// --exec-path, --namespace, --super-prefix) can redirect git to
		// attacker-controlled config, helpers or hooks, so they are NOT treated
		// as read-only. Mirrors OpenAI Codex CLI's git safety rules.
		return false;
	}
	if (GIT_READ_ONLY_SUBCOMMANDS.has(sub)) return !rest.some(isUnsafeGitArg);

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

/**
 * `for NAME [in WORD...]` / `select NAME [in WORD...]`. The header only binds a
 * loop variable and iterates over data; command/process substitution is already
 * rejected globally, and the loop body arrives as its own segments, so a header
 * of plain literals is read-only.
 */
function forLoopHeaderReadOnly(args: string[]): boolean {
	let i = 0;
	if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(args[i] ?? "")) return false;
	i++;
	if (args[i] === "in") i++;
	return args.slice(i).every((w) => !w.includes("$(") && !w.includes("`"));
}

function analyzeWords(words: string[], depth = 0): boolean {
	if (depth > 4) return false;
	let i = 0;
	while (i < words.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(words[i] as string)) i++;
	if (i >= words.length) return true; // only variable assignments

	const raw = baseName(words[i] as string);
	if (raw.length === 0 || raw.includes("$")) return false;
	// Strip grouping punctuation attached to the command word, so subshells
	// `(cd x && ls)` and brace groups `{ cat a; }` tokenize cleanly. `[`/`]` are
	// deliberately left alone: `[` is a real command.
	const cmd = raw.replace(/^\(+/, "").replace(/\)+$/, "").replace(/^\{+/, "").replace(/\}+$/, "");
	const args = words.slice(i + 1);
	if (cmd.length === 0) return analyzeWords(args, depth + 1); // bare `(`, `)`, `{`, `}`

	// Shell reserved words for compound commands. The tokenizer splits
	// `for x in a b; do ...; done` into separate segments, so the keyword shows
	// up as the leading word of its own segment. The keywords have no file
	// effects themselves: validate whatever command is attached to a prefix
	// keyword and treat pure terminators as neutral, leaving the body segments
	// to be checked individually.
	if (cmd === "for" || cmd === "select") return forLoopHeaderReadOnly(args);
	if (cmd === "while" || cmd === "until" || cmd === "if" || cmd === "elif") {
		return analyzeWords(args, depth + 1);
	}
	if (cmd === "do" || cmd === "then" || cmd === "else" || cmd === "!") {
		return analyzeWords(args, depth + 1);
	}
	if (cmd === "done" || cmd === "fi" || cmd === "esac") return true;

	if (cmd === "git") return gitReadOnly(args);
	if (cmd === "gh") return ghReadOnly(args);
	if (cmd === "npm" || cmd === "pnpm" || cmd === "yarn") return packageManagerReadOnly(args);
	if (cmd === "bun") return bunReadOnly(args);
	if (cmd === "docker" || cmd === "podman") return containerReadOnly(args);
	if (cmd === "kubectl") return kubectlReadOnly(args);
	if (RUNTIME_COMMANDS.has(cmd)) return runtimeReadOnly(cmd, args);

	// `command -v` / `command -V` only look up a name; they never execute it.
	if (cmd === "command" && args.some((a) => a === "-v" || a === "-V")) return true;

	// Wrapper commands (`env`, `nice`, `stdbuf`, `timeout`, `xargs`, `command`,
	// `exec`, ...): skip their own flags/assignments, then classify the wrapped
	// command. `timeout` takes a positional duration before the command.
	if (WRAPPER_COMMANDS.has(cmd)) {
		const valueFlags = WRAPPER_VALUE_FLAGS[cmd] ?? new Set<string>();
		let j = skipWrapperFlags(args, valueFlags, cmd === "env");
		if (cmd === "timeout" && j < args.length) j++;
		if (j >= args.length) return true;
		return analyzeWords(args.slice(j), depth + 1);
	}

	if (cmd === "find" || cmd === "fd" || cmd === "fdfind") {
		return !args.some((a) => a.startsWith("-delete") || a.startsWith("-exec") || a.startsWith("-ok") || a.startsWith("-fprint") || a.startsWith("-fls"));
	}
	if (cmd === "sed") {
		if (args.some((a) => a.startsWith("-i") || a.startsWith("--in-place"))) return false;
		const script = args.filter((a) => !isFlag(a)).join(" ");
		return !SED_SIDE_EFFECT_PATTERNS.some((p) => p.test(script));
	}
	if (cmd === "awk" || cmd === "gawk" || cmd === "mawk") {
		return !AWK_SIDE_EFFECT_PATTERNS.some((p) => p.test(args.join(" ")));
	}
	if (cmd === "rg" || cmd === "ripgrep") {
		return !args.some(
			(a) =>
				a === "--pre" ||
				a.startsWith("--pre=") ||
				a === "--hostname-bin" ||
				a.startsWith("--hostname-bin=") ||
				a === "-z" ||
				a === "--search-zip",
		);
	}
	if (cmd === "sort") {
		return !args.some((a) => a.startsWith("-o") || a.startsWith("--output"));
	}
	return READ_ONLY_SIMPLE.has(cmd);
}

/**
 * Short version of a classifier reason for the confirmation dialog — the command
 * itself is already shown in the dialog body.
 */
function shortAskReason(reason: string): string {
	const prefix = "not read-only: ";
	if (!reason.startsWith(prefix)) return reason;
	const first = reason.slice(prefix.length).trim().split(/\s+/)[0] ?? "";
	return first.length > 0 ? `not in the read-only allowlist: ${first}` : "not in the read-only allowlist";
}

// ---------------------------------------------------------------------------
// Bash read-only classifier (tree-sitter AST)
//
// Shell structure is parsed with tree-sitter-bash; the structural walk below
// follows the classifiers used by OpenAI Codex CLI and Qwen Code. Per-command
// allow-listing still goes through analyzeWords(), so the allow-lists live in
// one place. The parser is a hard dependency: if it cannot be initialised,
// analyzeBashCommand fails closed instead of falling back to a heuristic.
// ---------------------------------------------------------------------------

let astParser: Parser | undefined;
let astParserInit: Promise<Parser | undefined> | undefined;
let astParserFailed = false;

const AST_WRITE_REDIRECT_OPS = new Set([">", ">>", "&>", "&>>", ">|"]);
const AST_REDIRECT_NODES = new Set(["file_redirect", "heredoc_redirect", "herestring_redirect"]);

/** Nodes with no effects of their own: recurse into their children. */
const AST_STRUCTURAL_NODES = new Set([
	"program", "list", "pipeline", "do_group", "else_clause", "elif_clause",
	"if_statement", "while_statement", "until_statement", "for_statement",
	"c_style_for_statement", "case_statement", "case_item", "subshell",
	"compound_statement", "negated_command", "test_command", "binary_expression",
	"unary_expression", "postfix_expression", "expansion", "simple_expansion",
	"concatenation", "string", "variable_assignment", "declaration_command",
	"command_substitution", "process_substitution", "heredoc_redirect", "herestring_redirect",
	"array", "arithmetic_expansion", "subscript", "unset_command",
]);

/** Leaf nodes that are pure data. */
const AST_TERMINAL_NODES = new Set([
	"word", "number", "string_content", "raw_string", "variable_name",
	"heredoc_start", "heredoc_body", "heredoc_end", "comment", "file_descriptor",
	"test_operator",
]);

async function getAstParser(): Promise<Parser | undefined> {
	if (astParser) return astParser;
	if (astParserFailed) return undefined;
	if (!astParserInit) {
		astParserInit = (async () => {
			const require = createRequire(import.meta.url);
			const runtimeWasm = require.resolve("web-tree-sitter/web-tree-sitter.wasm");
			const bashWasm = require.resolve("tree-sitter-bash/tree-sitter-bash.wasm");
			await Parser.init({ locateFile: () => runtimeWasm });
			const bash = await Language.load(bashWasm);
			const parser = new Parser();
			parser.setLanguage(bash);
			astParser = parser;
			return parser;
		})().catch((error: unknown) => {
			astParserFailed = true;
			astParserInit = undefined;
			console.error("[edit-modes] tree-sitter-bash parser init failed:", error);
			return undefined;
		});
	}
	return astParserInit;
}

function stripOuterQuotes(text: string): string {
	if (text.length >= 2) {
		const first = text[0];
		const last = text[text.length - 1];
		if ((first === '"' && last === '"') || (first === "'" && last === "'")) return text.slice(1, -1);
	}
	return text;
}

function astHasSubstitution(node: Parser.SyntaxNode): boolean {
	for (const child of node.namedChildren) {
		if (child.type === "command_substitution" || child.type === "process_substitution") return true;
		if (astHasSubstitution(child)) return true;
	}
	return false;
}

/** Literal text of a command_name, or undefined when the name is dynamic. */
function astLiteralCommandName(nameNode: Parser.SyntaxNode | null): string | undefined {
	if (!nameNode || nameNode.type !== "command_name") return undefined;
	const kids = nameNode.namedChildren;
	if (kids.length !== 1) return undefined;
	const only = kids[0] as Parser.SyntaxNode;
	if (only.type === "word" || only.type === "number" || only.type === "string" || only.type === "raw_string") {
		return stripOuterQuotes(only.text);
	}
	return undefined;
}

/** Argument text for allow-list checks; substitutions become opaque placeholders. */
function astArgText(node: Parser.SyntaxNode): string {
	switch (node.type) {
		case "command_substitution":
		case "process_substitution":
			return "__sub__";
		case "expansion":
		case "simple_expansion":
			// Keep the `$` so path checks treat the value as unresolved (fail closed).
			return "$__var__";
		case "string":
			return astHasSubstitution(node) ? "__sub__" : stripOuterQuotes(node.text);
		case "concatenation":
			return astHasSubstitution(node) ? "__sub__" : node.text;
		case "raw_string":
			return stripOuterQuotes(node.text);
		default:
			return node.text;
	}
}

function astRedirectTarget(node: Parser.SyntaxNode): string {
	const dest = node.childForFieldName("destination");
	return dest ? stripOuterQuotes(dest.text) : "";
}

function astRedirectIsWrite(node: Parser.SyntaxNode): boolean {
	for (const child of node.children) {
		if (AST_WRITE_REDIRECT_OPS.has(child.type)) {
			const target = astRedirectTarget(node);
			return !(SAFE_REDIRECT_TARGETS.includes(target) || target.startsWith("/dev/fd/"));
		}
	}
	return false;
}

/** Commands whose read-only-ness depends on argument/flag semantics. */
const AST_FLAG_SENSITIVE = new Set([
	"find", "fd", "fdfind", "sed", "sort", "awk", "gawk", "mawk", "rg", "ripgrep",
	"git", "gh", "npm", "pnpm", "yarn", "bun", "docker", "podman", "kubectl",
]);

function astChildrenReason(node: Parser.SyntaxNode, depth: number): string | undefined {
	for (const child of node.namedChildren) {
		const reason = astNodeReason(child, depth);
		if (reason) return reason;
	}
	return undefined;
}

function astCommandReason(node: Parser.SyntaxNode, depth: number): string | undefined {
	const nameNode = node.childForFieldName("name");
	const name = astLiteralCommandName(nameNode);
	if (name === undefined) return "dynamic command name";
	const words = [name];
	for (const child of node.namedChildren) {
		if (child.type === "command_name") continue;
		const reason = astNodeReason(child, depth + 1);
		if (reason) return reason;
		if (child.type === "variable_assignment") continue;
		words.push(astArgText(child));
	}
	if (words.includes("__sub__")) {
		// A command substitution can change flag/argument semantics at runtime
		// (e.g. `find . $(echo -delete)`), so flag-sensitive commands and wrappers
		// are rejected. For plain data commands the inner program was already
		// validated above, so `echo $(date)` stays read-only.
		const base = words[0] as string;
		if (AST_FLAG_SENSITIVE.has(base) || RUNTIME_COMMANDS.has(base) || WRAPPER_COMMANDS.has(base)) {
			return `command substitution in a flag-sensitive command: ${words.join(" ")}`;
		}
	}
	if (!analyzeWords(words)) return `not read-only: ${words.join(" ")}`;
	return undefined;
}

function astRedirectedReason(node: Parser.SyntaxNode, depth: number): string | undefined {
	for (const child of node.namedChildren) {
		if (child.type === "file_redirect" && astRedirectIsWrite(child)) {
			return "contains output redirection to a file";
		}
	}
	for (const child of node.namedChildren) {
		if (AST_REDIRECT_NODES.has(child.type)) continue;
		const reason = astNodeReason(child, depth);
		if (reason) return reason;
	}
	return undefined;
}

function astNodeReason(node: Parser.SyntaxNode, depth: number): string | undefined {
	if (depth > 8) return "shell nesting too deep to analyze";
	const type = node.type;
	if (type === "command") return astCommandReason(node, depth);
	if (type === "redirected_statement") return astRedirectedReason(node, depth);
	if (type === "file_redirect") return astRedirectIsWrite(node) ? "contains output redirection to a file" : undefined;
	if (type === "function_definition") return "defines a shell function";
	if (AST_TERMINAL_NODES.has(type)) return undefined;
	if (AST_STRUCTURAL_NODES.has(type)) return astChildrenReason(node, depth);
	return `unsupported shell construct: ${type}`;
}

async function analyzeBashCommand(command: string): Promise<{ readOnly: boolean; reason: string }> {
	const trimmed = command.trim();
	if (trimmed.length === 0) return { readOnly: true, reason: "empty command" };
	const parser = await getAstParser();
	// tree-sitter-bash is a hard dependency; fail closed rather than guessing.
	if (!parser) return { readOnly: false, reason: "read-only parser unavailable (tree-sitter-bash)" };
	const tree = parser.parse(command);
	const reason = tree.rootNode.hasError ? "shell parse error" : astChildrenReason(tree.rootNode, 0);
	tree.delete();
	if (reason) return { readOnly: false, reason };
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

/**
 * Extra roots treated like the project dir: operations underneath them do not
 * ask. `/tmp` plus whatever the OS reports as the temp dir, so a TMPDIR override
 * still matches.
 */
const TEMP_ALLOWED_ROOTS = ["/tmp", os.tmpdir()];

/** True when `resolved` is inside the project dir or an allowed temp root. */
function isAllowedRoot(resolved: string, cwd: string): boolean {
	return isWithin(cwd, resolved) || TEMP_ALLOWED_ROOTS.some((root) => isWithin(root, resolved));
}

/** True when `resolved` is exactly an allowed temp root (e.g. `/tmp` itself). */
function isTempRootItself(resolved: string): boolean {
	return TEMP_ALLOWED_ROOTS.some((root) => resolved === path.resolve(root));
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
	return !isAllowedRoot(resolved, cwd);
}

/** True when `p` clearly points outside the project dir (fail-closed on unknowns). */
function isUnsafeReadPath(p: string, cwd: string): boolean {
	const resolved = resolvePathArg(p, cwd);
	if (resolved === undefined) return true;
	return !isAllowedRoot(resolved, cwd);
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
		// `-c`, `--command`, and combined short flags such as `-lc` / `-cl`.
		const cIndex = args.findIndex(
			(arg) => arg === "-c" || arg === "--command" || /^-[a-zA-Z]*c[a-zA-Z]*$/.test(arg),
		);
		const script = cIndex >= 0 ? args[cIndex + 1] : undefined;
		if (script !== undefined) {
			const reason = classifyDangerousBashSync(script, cwd, depth + 1);
			return reason ? { reason } : undefined;
		}
		return { cmd, args };
	}

	if (cmd === "eval") {
		const script = args.filter((arg) => !isFlag(arg)).join(" ");
		if (script.length > 0) {
			const reason = classifyDangerousBashSync(script, cwd, depth + 1);
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
		if (isTempRootItself(resolved)) return `rm targeting the temp root: ${target}`;
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

const GIT_OVERRIDE_FLAGS = [
	"-C", "-c", "--git-dir", "--work-tree", "--config-env", "--exec-path", "--namespace", "--super-prefix",
];

/** git global flags that can redirect config/helpers to attacker-controlled code. */
function gitOverrideDanger(args: string[]): string | undefined {
	for (const arg of args) {
		if (GIT_OVERRIDE_FLAGS.includes(arg)) return `git global override flag ${arg} can execute external helpers`;
		if (arg.startsWith("-C") && arg.length > 2) return "git global override flag -C";
		if (arg.startsWith("--") && arg.includes("=")) {
			const name = arg.split("=")[0] as string;
			if (GIT_OVERRIDE_FLAGS.includes(name)) return `git global override flag ${name}`;
		}
	}
	return undefined;
}

function gitDanger(args: string[]): string | undefined {
	const override = gitOverrideDanger(args);
	if (override) return override;
	if (args.some(isUnsafeGitArg)) return "git with a file-writing or helper-executing flag";
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

interface DangerFacts {
	invocations: { words: string[]; dynamicName: boolean; hasSubArg: boolean }[];
	writeTargets: string[];
	pipeIntoshell: boolean;
}

/** Walk the AST collecting every executed command and write-redirect target. */
function collectDangerFacts(node: Parser.SyntaxNode, facts: DangerFacts): void {
	if (node.type === "command") {
		const name = astLiteralCommandName(node.childForFieldName("name"));
		const words: string[] = [];
		let hasSubArg = false;
		if (name !== undefined) words.push(name);
		for (const child of node.namedChildren) {
			if (child.type === "command_name" || child.type === "variable_assignment") continue;
			if (
				child.type === "command_substitution" ||
				child.type === "process_substitution" ||
				((child.type === "string" || child.type === "concatenation") && astHasSubstitution(child))
			) {
				hasSubArg = true;
			}
			words.push(astArgText(child));
		}
		facts.invocations.push({ words, dynamicName: name === undefined, hasSubArg });
	}
	if (node.type === "file_redirect") {
		for (const child of node.children) {
			if (AST_WRITE_REDIRECT_OPS.has(child.type)) {
				facts.writeTargets.push(astRedirectTarget(node));
				break;
			}
		}
	}
	if (node.type === "pipeline") {
		const commands = node.namedChildren.filter((child) => child.type === "command");
		for (let i = 1; i < commands.length; i++) {
			const name = astLiteralCommandName((commands[i] as Parser.SyntaxNode).childForFieldName("name"));
			if (name !== undefined && SHELL_COMMANDS.has(name)) facts.pipeIntoshell = true;
		}
	}
	for (const child of node.namedChildren) collectDangerFacts(child, facts);
}

/**
 * auto-all bash policy: auto-approve everything unless an AST-derived effect
 * crosses the danger policy (privileged/system commands, recursive or
 * out-of-project writes, system/package changes, git history rewrites).
 * Dynamic command names, flag-sensitive command substitutions and parse errors
 * fail closed. `sh -c` / `eval` scripts recurse here.
 */
function classifyDangerousBashSync(command: string, cwd: string, depth = 0): string | undefined {
	if (depth > 3) return "command nesting too deep to analyze";
	const trimmed = command.trim();
	if (trimmed.length === 0) return undefined;
	const parser = astParser;
	if (!parser) return "read-only parser unavailable (tree-sitter-bash)";

	const tree = parser.parse(command);
	if (tree.rootNode.hasError) {
		tree.delete();
		return "shell parse error";
	}
	const facts: DangerFacts = { invocations: [], writeTargets: [], pipeIntoshell: false };
	collectDangerFacts(tree.rootNode, facts);
	tree.delete();

	for (const target of facts.writeTargets) {
		if (isUnsafeWritePath(target, cwd)) return `output redirection outside the project: ${target}`;
	}

	for (const invocation of facts.invocations) {
		if (invocation.dynamicName) return "dynamic command name";
		const parsed = parseSegment(invocation.words, cwd, depth);
		if (parsed?.reason) return parsed.reason;
		const effective = parsed?.cmd ?? cleanCommandName(invocation.words[0] ?? "");
		if (
			invocation.hasSubArg &&
			(AST_FLAG_SENSITIVE.has(effective) ||
				effective === "eval" ||
				effective === "source" ||
				effective === "." ||
				RUNTIME_COMMANDS.has(effective) ||
				WRAPPER_COMMANDS.has(effective))
		) {
			return `command substitution in a sensitive command: ${invocation.words.join(" ")}`;
		}
		if (!parsed || parsed.cmd === undefined || parsed.args === undefined) continue;
		const reason = dangerForCommand(parsed.cmd, parsed.args, cwd);
		if (reason) return reason;
	}

	if (facts.pipeIntoshell) return "piping into a shell";
	return undefined;
}

async function classifyDangerousBash(command: string, cwd: string, depth = 0): Promise<string | undefined> {
	const parser = await getAstParser();
	if (!parser) return "read-only parser unavailable (tree-sitter-bash)";
	return classifyDangerousBashSync(command, cwd, depth);
}

// ---------------------------------------------------------------------------
// git commit guard (all modes)
//
// `git commit` is never auto-approved. The check is deliberately a plain text
// match, so anything containing `git commit` (`sudo git commit`,
// `bash -c 'git commit'`, `git add -A && git commit`) is caught as well. It is
// not a security sandbox: options between `git` and `commit` (`git -C dir
// commit`) and a script or build tool that commits internally are not detected,
// and only the built-in `bash` tool is gated.
// ---------------------------------------------------------------------------

/** True when the command text contains a `git commit` call. */
function isGitCommitCommand(command: string): boolean {
	return /\bgit\s+commit(?![\w-])/.test(command);
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
const COLLAPSED_PREVIEW_LINES = 8;

const APPROVAL_OPTIONS: ReadonlyArray<{ label: string; choice: ApprovalChoice }> = [
	{ label: "Allow once", choice: "allow" },
	{ label: "Allow all edits (switch to auto-edit)", choice: "allow-all" },
	{ label: "Deny", choice: "deny" },
];

/** Choices offered by the bash confirmation dialog. */
type BashChoice = "allow" | "deny";

const BASH_OPTIONS: ReadonlyArray<{ label: string; choice: BashChoice }> = [
	{ label: "Allow once", choice: "allow" },
	{ label: "Deny", choice: "deny" },
];

type ThemeColor = Parameters<Theme["fg"]>[0];

/** Panel background matching a dialog's severity, mirroring the tool-block shells. */
function severityBg(color: ThemeColor): Parameters<Theme["bg"]>[0] {
	return color === "error" ? "toolErrorBg" : color === "warning" ? "toolPendingBg" : "customMessageBg";
}

/** Everything the confirmation dialog needs in order to render one approval request. */
interface ApprovalSpec<T> {
	/** Plain (uncoloured) title line, e.g. `ask-to-edit: edit src/index.ts`. */
	title: string;
	titleColor: ThemeColor;
	/** One-line explanation of why the confirmation is needed. */
	subtitle?: string;
	/** Body lines, already tagged for colouring. */
	lines: DiffLine[];
	/** How many body lines the collapsed dialog shows before `v`. */
	maxBodyLines: number;
	/** Body line budget for the plain select fallback used outside TUI mode. */
	maxFallbackLines: number;
	/** What `v` opens, e.g. `full diff` or `full command`. */
	viewLabel: string;
	options: ReadonlyArray<{ label: string; choice: T }>;
}

class ApprovalDialog<T> implements Component {
	private selected = 0;
	private previewTop = -1;
	private previewBottom = -1;

	constructor(
		private readonly spec: ApprovalSpec<T>,
		private readonly theme: Theme,
		private readonly tui: TUI,
		private readonly done: (value: T | "view" | undefined) => void,
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
			this.selected = Math.min(this.spec.options.length - 1, this.selected + 1);
			this.tui.requestRender();
			return;
		}
		if (matchesKey(data, "enter")) {
			this.done(this.spec.options[this.selected]?.choice);
			return;
		}
		if (matchesKey(data, "v") || matchesKey(data, "ctrl+o")) {
			this.done("view");
		}
	}

	handleMouse(event: TuiMouseEvent): TuiMouseEventResult | undefined {
		// The only mouse action is opening the full viewer; a click must never confirm a decision.
		if (event.type !== "click" || event.button !== "left") return undefined;
		if (this.previewTop >= 0 && event.y >= this.previewTop && event.y <= this.previewBottom) {
			this.done("view");
			return { handled: true };
		}
		return undefined;
	}

	render(width: number): string[] {
		const th = this.theme;
		const innerWidth = Math.max(1, width - 2);
		const border = (text: string) => th.fg(this.spec.titleColor, text);
		const sevBg = severityBg(this.spec.titleColor);
		const padLine = (text: string) => {
			const clipped = truncateToWidth(` ${text}`, innerWidth, "…");
			const padding = Math.max(0, innerWidth - visibleWidth(clipped));
			return `${clipped}${" ".repeat(padding)}`;
		};
		// Every interior row shares the severity background, so the dialog reads as
		// one tinted panel like the tool blocks; the selected option keeps selectedBg.
		const row = (text = "") => border("│") + th.bg(sevBg, padLine(text)) + border("│");

		const out: string[] = [];
		// Title as a filled "chip" on the top border, tinted by severity, so the
		// prompt reads as one prominent box instead of blending into the transcript.
		const chipLabel = truncateToWidth(` ${this.spec.title} `, Math.max(1, innerWidth - 1), "…");
		const topFill = "─".repeat(Math.max(0, innerWidth - 1 - visibleWidth(chipLabel)));
		out.push(border("╭─") + th.bg(sevBg, th.fg("text", th.bold(chipLabel))) + border(`${topFill}╮`));

		if (this.spec.subtitle) out.push(row(th.fg("muted", this.spec.subtitle)));
		out.push(row(""));

		const shown = this.spec.lines.slice(0, this.spec.maxBodyLines);
		this.previewTop = out.length;
		for (const line of shown) out.push(row(colorDiffLine(line, th)));
		this.previewBottom = out.length - 1;

		const hidden = this.spec.lines.length - shown.length;
		const more = hidden > 0 ? `${th.fg("muted", `… ${hidden} more line${hidden === 1 ? "" : "s"}`)} ` : "";
		const viewHint = th.underline(
			th.fg(
				"dim",
				hidden > 0
					? `[ click here or press v to view ${this.spec.viewLabel} ]`
					: `[ press v to view ${this.spec.viewLabel} ]`,
			),
		);
		out.push(row(`${more}${viewHint}`));
		out.push(row(""));

		this.spec.options.forEach((option, index) => {
			if (index === this.selected) {
				const highlighted = padLine(`${th.fg("accent", "▶")} ${th.bold(option.label)}`);
				out.push(border("│") + th.bg("selectedBg", highlighted) + border("│"));
			} else {
				out.push(row(`  ${th.fg("text", option.label)}`));
			}
		});

		out.push(row(""));
		out.push(row(th.fg("dim", `↑↓ select · enter confirm · v ${this.spec.viewLabel} · esc cancel`)));
		out.push(border(`╰${"─".repeat(innerWidth)}╯`));
		return out;
	}

	invalidate(): void {}
}

class DiffViewer<T> implements Component {
	private offset = 0;
	/** Index of the highlighted confirmation option (↑↓ select, enter confirms). */
	private selected = 0;
	/** Cache of `lines` wrapped to the current render width. */
	private wrapped?: { width: number; lines: string[] };

	constructor(
		private readonly title: string,
		private readonly titleColor: ThemeColor,
		private readonly lines: DiffLine[],
		private readonly theme: Theme,
		private readonly tui: TUI,
		private readonly done: (value: T | "back" | undefined) => void,
		private readonly options: ReadonlyArray<{ label: string; choice: T }> = [],
	) {}

	private viewportHeight(): number {
		// top border + info + blank + options + hint + bottom border, plus a
		// two-row margin below the overlay.
		const chrome = this.options.length > 0 ? this.options.length + 5 : 3;
		return Math.max(3, this.tui.terminal.rows - chrome - 2);
	}

	/**
	 * Wrap each logical line to the pane width so long single-line content (e.g.
	 * a shell command or a minified JS line) is shown in full instead of being
	 * truncated with "…". Wrapped lines carry the original colour codes.
	 */
	private wrapLines(width: number): string[] {
		if (this.wrapped && this.wrapped.width === width) return this.wrapped.lines;
		const textWidth = Math.max(1, width - 2 - 1); // inner width minus the leading pad space
		const lines: string[] = [];
		for (const line of this.lines) {
			const rendered = new Text(colorDiffLine(line, this.theme), 0, 0).render(textWidth);
			lines.push(...(rendered.length > 0 ? rendered : [""]));
		}
		this.wrapped = { width, lines };
		return lines;
	}

	private maxOffset(): number {
		const total = this.wrapped?.lines.length ?? this.lines.length;
		return Math.max(0, total - this.viewportHeight());
	}

	handleInput(data: string): void {
		// v / ctrl+o collapse back to the smaller dialog, mirroring the keys that open this view.
		if (
			matchesKey(data, "escape") ||
			matchesKey(data, "ctrl+c") ||
			matchesKey(data, "q") ||
			matchesKey(data, "v") ||
			matchesKey(data, "ctrl+o")
		) {
			this.done("back");
			return;
		}
		// Same option-selection model as the collapsed dialog: ↑↓ selects, enter confirms.
		if (this.options.length > 0) {
			if (matchesKey(data, "enter")) {
				const option = this.options[this.selected];
				if (option) this.done(option.choice);
				return;
			}
			if (matchesKey(data, "up")) {
				this.selected = Math.max(0, this.selected - 1);
				this.tui.requestRender();
				return;
			}
			if (matchesKey(data, "down")) {
				this.selected = Math.min(this.options.length - 1, this.selected + 1);
				this.tui.requestRender();
				return;
			}
		}
		const page = Math.max(1, this.viewportHeight() - 1);
		if (matchesKey(data, "pageUp")) this.offset = Math.max(0, this.offset - page);
		else if (matchesKey(data, "pageDown")) this.offset = Math.min(this.maxOffset(), this.offset + page);
		else if (matchesKey(data, "home")) this.offset = 0;
		else if (matchesKey(data, "end")) this.offset = this.maxOffset();
		else return;
		this.tui.requestRender();
	}

	handleMouse(event: TuiMouseEvent): TuiMouseEventResult | undefined {
		// The wheel scrolls the preview; a mouse click must never decide for the user.
		if (event.type === "wheel" && typeof event.wheelDelta === "number") {
			this.offset = Math.max(0, Math.min(this.maxOffset(), this.offset + event.wheelDelta));
			this.tui.requestRender();
			return { handled: true };
		}
		return undefined;
	}

	render(width: number): string[] {
		const th = this.theme;
		const innerWidth = Math.max(1, width - 2);
		const border = (text: string) => th.fg(this.titleColor, text);
		const sevBg = severityBg(this.titleColor);
		const padLine = (text: string) => {
			const clipped = truncateToWidth(` ${text}`, innerWidth, "…");
			const padding = Math.max(0, innerWidth - visibleWidth(clipped));
			return `${clipped}${" ".repeat(padding)}`;
		};
		// Same severity tint as the collapsed dialog: interior rows sit on sevBg,
		// the selected option on selectedBg.
		const row = (text = "") => border("│") + th.bg(sevBg, padLine(text)) + border("│");

		const wrapped = this.wrapLines(width);
		const total = wrapped.length;
		const viewport = this.viewportHeight();
		this.offset = Math.min(this.offset, Math.max(0, total - viewport));

		const out: string[] = [];
		const titleText = truncateToWidth(` ${this.title} `, innerWidth, "…");
		const topFill = "─".repeat(Math.max(0, innerWidth - visibleWidth(titleText)));
		out.push(border("╭") + th.bg(sevBg, th.fg("text", th.bold(titleText))) + border(`${topFill}╮`));

		const first = total === 0 ? 0 : this.offset + 1;
		const last = Math.min(total, this.offset + viewport);
		const info = `lines ${first}-${last} / ${total}  ·  wheel/pageUp-pageDown/home/end scroll`;
		out.push(row(th.fg("dim", info)));

		const shown = wrapped.slice(this.offset, this.offset + viewport);
		for (const line of shown) out.push(row(line));
		for (let i = shown.length; i < viewport; i++) out.push(row(""));

		// Confirmation options, so the decision can be made without leaving fullscreen.
		if (this.options.length > 0) {
			out.push(row(""));
			this.options.forEach((option, index) => {
				const isSelected = index === this.selected;
				const line = isSelected
					? `${th.fg("accent", "▶")} ${th.bold(option.label)}`
					: ` ${th.fg("text", option.label)}`;
				const rendered = padLine(line);
				out.push(border("│") + (isSelected ? th.bg("selectedBg", rendered) : th.bg(sevBg, rendered)) + border("│"));
			});
			out.push(row(th.fg("dim", " ↑↓ select · enter confirm · pageUp/pageDown scroll · esc back")));
		}

		out.push(border(`╰${"─".repeat(innerWidth)}╯`));
		return out;
	}

	invalidate(): void {
		this.wrapped = undefined;
	}
}

/**
 * Show a confirmation dialog. In TUI mode this uses a custom component with a
 * collapsed preview, an option list driven by ↑↓ + enter, and a fullscreen viewer;
 * other modes fall back to the plain select dialog.
 */
async function runApprovalDialog<T>(
	ctx: ExtensionContext,
	spec: ApprovalSpec<T>,
	viewerTitle: string,
): Promise<T | undefined> {
	if (ctx.mode !== "tui") {
		const preview = truncateForPreview(
			spec.lines.map((line) => line.text).join("\n"),
			spec.maxFallbackLines,
			MAX_PREVIEW_WIDTH,
		);
		const header = spec.subtitle ? `${spec.title} (${spec.subtitle})` : spec.title;
		const choice = await ctx.ui.select(
			`${header}\n\n${preview}\n`,
			spec.options.map((option) => option.label),
		);
		return spec.options.find((option) => option.label === choice)?.choice;
	}

	for (;;) {
		const result = await ctx.ui.custom<T | "view" | undefined>((tui, theme, _keybindings, done) =>
			new ApprovalDialog<T>(spec, theme, tui, done),
		);
		if (result !== "view") return result;

		const choice = await ctx.ui.custom<T | "back" | undefined>(
			(tui, theme, _keybindings, done) =>
				new DiffViewer<T>(viewerTitle, spec.titleColor, spec.lines, theme, tui, done, spec.options),
			{
				overlay: true,
				overlayOptions: { width: "100%", maxHeight: "100%", margin: 0, anchor: "center" },
			},
		);
		if (choice !== "back") return choice;
	}
}

/** Show the edit/write approval UI (custom dialog in TUI mode, plain select otherwise). */
async function approveEditChange(
	ctx: ExtensionContext,
	mode: EditMode,
	summary: string,
	lines: DiffLine[],
): Promise<ApprovalResult> {
	return runApprovalDialog<ApprovalChoice>(
		ctx,
		{
			title: `${mode}: ${summary}`,
			titleColor: "warning",
			lines,
			maxBodyLines: COLLAPSED_PREVIEW_LINES,
			maxFallbackLines: MAX_PREVIEW_LINES,
			viewLabel: "full diff",
			options: APPROVAL_OPTIONS,
		},
		summary,
	);
}

// ---------------------------------------------------------------------------
// Extension
// ---------------------------------------------------------------------------

export default function (pi: ExtensionAPI) {
	let mode: EditMode = "ask-to-edit";

	pi.registerFlag(STARTUP_FLAG, {
		description: "Start in an edit approval mode: ask-to-edit | auto-edit | auto-all",
		type: "string",
		default: "ask-to-edit",
	});

	function applyStatus(ctx: ExtensionContext): void {
		if (!ctx.hasUI) return;
		const color = mode === "ask-to-edit" ? "warning" : mode === "auto-edit" ? "success" : "accent";
		ctx.ui.setStatus(STATUS_KEY, ctx.ui.theme.fg(color, mode));
	}

	function setMode(next: EditMode, ctx: ExtensionContext, options: { persist?: boolean; notify?: boolean } = {}): void {
		const { persist = true, notify = true } = options;
		mode = next;
		if (persist) pi.appendEntry(STATE_TYPE, { mode });
		applyStatus(ctx);
		if (notify && ctx.hasUI) ctx.ui.notify(`Edit mode: ${mode}`, "info");
	}

	pi.on("session_start", async (_event, ctx) => {
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

	pi.registerShortcut("shift+tab", {
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
			const command = event.input.command;
			const commit = isGitCommitCommand(command);

			let askReason: string | undefined;
			if (commit) {
				askReason = "git commit is never auto-approved: each commit needs explicit confirmation";
			} else if (mode === "auto-all") {
				askReason = await classifyDangerousBash(command, ctx.cwd);
			} else {
				const verdict = await analyzeBashCommand(command);
				if (!verdict.readOnly) askReason = verdict.reason;
			}
			if (askReason === undefined) return;

			const label = commit
				? "⚠ git commit"
				: mode === "auto-all"
					? "⚠ Dangerous bash"
					: "⚠ Non-read-only bash";

			if (!ctx.hasUI) {
				return {
					block: true,
					reason: commit
						? `Blocked: ${askReason}, and no interactive UI to approve it. ` +
							`Commit manually, or run pi in an interactive session.`
						: `Blocked by ${mode} guard: ${askReason}, and no interactive UI is available ` +
							`to approve it. Run a safe command or use an interactive session.`,
				};
			}

			const choice = await runApprovalDialog<BashChoice>(
				ctx,
				{
					title: label,
					titleColor: commit || mode === "auto-all" ? "error" : "warning",
					subtitle: commit
						? "never auto-approved · every commit needs explicit confirmation"
						: shortAskReason(askReason),
					lines: command
						.split("\n")
						.map((line, index) => ({
							text: index === 0 ? `$ ${line}` : `  ${line}`,
							kind: index === 0 ? ("hunk" as const) : ("context" as const),
						})),
					maxBodyLines: COLLAPSED_PREVIEW_LINES,
					maxFallbackLines: MAX_BASH_PREVIEW_LINES,
					viewLabel: "full command",
					options: BASH_OPTIONS,
				},
				label,
			);

			if (choice === "allow") return;
			// Denying stops the current agent run entirely (like pressing Esc) instead of
			// just blocking this tool call and letting the model continue working.
			ctx.abort();
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
			if (resolved !== undefined && isAllowedRoot(resolved, ctx.cwd)) return;
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
			// Denying stops the current agent run entirely (like pressing Esc) instead
			// of just blocking this tool call and letting the model continue working.
			ctx.abort();
			return { block: true, reason: `User denied: ${summary} (${mode} mode)` };
		}

		// Cancelled the dialog (Esc / Ctrl+C) counts as a denial.
		ctx.abort();
		return { block: true, reason: `User cancelled approval for: ${summary} (${mode} mode)` };
	});
}
