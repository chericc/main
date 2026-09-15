/**
 * compact-tools.ts
 *
 * Make every built-in tool show only a short preview by default. Click a tool row
 * (fullscreen TUI mode) or press ctrl+o to expand/collapse the full body.
 *
 * Collapsed tools show at most COLLAPSED_LINES lines of body content (plus the
 * one-line header). `bash`/`powershell` keep their *tail*, everything else its head.
 *
 * We reuse the exported built-in tool *definitions* (createBashToolDefinition, ...) so
 * the original execution, truncation and tool-call rendering are preserved; only the
 * call/result bodies are replaced.
 *
 * `edit` and `write` are special-cased: their built-in renderers dump a full diff /
 * full file body and ignore the expanded flag, so we replace their bodies with a
 * preview that expands into the diff (edit) or the written content (write).
 */
import * as fs from "node:fs";
import * as path from "node:path";
import type { ExtensionAPI } from "@earendil-works/pi-coding-agent";
import {
	createBashToolDefinition,
	createEditToolDefinition,
	createFindToolDefinition,
	createGrepToolDefinition,
	createLsToolDefinition,
	createPowerShellToolDefinition,
	createReadToolDefinition,
	createWriteToolDefinition,
	getLanguageFromPath,
	highlightCode,
	renderDiff,
	type AgentToolResult,
	type Theme,
	type ToolRenderContext,
	type ToolRenderResultOptions,
} from "@earendil-works/pi-coding-agent";
import { type Component, Container, Text } from "@earendil-works/pi-tui";

/** How many body lines to show while collapsed. Change this to taste. */
const COLLAPSED_LINES = 5;

/** Tools that should keep their *last* lines when collapsed (their tail is the useful part). */
const TAIL_TOOLS = new Set(["bash", "powershell"]);

/** Hint appended to collapsed rows when content is hidden. */
const EXPAND_HINT = "click / ctrl+o to expand";

function textOf(result: AgentToolResult<any>): string {
	return (result.content ?? [])
		.filter((c: any): c is { type: "text"; text: string } => c.type === "text")
		.map((c) => c.text ?? "")
		.join("\n")
		.trim();
}

/** Render a set of already-coloured body lines, truncating to COLLAPSED_LINES when not expanded. */
function previewText(lines: string[], expanded: boolean, theme: Theme): string {
	const shown = expanded ? lines : lines.slice(0, COLLAPSED_LINES);
	const hidden = lines.length - shown.length;
	let body = `\n${shown.join("\n")}`;
	if (hidden > 0) {
		body += theme.fg("muted", `\n… (${hidden} more, ${EXPAND_HINT})`);
	}
	return body;
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
	const chosen = options.expanded
		? lines
		: TAIL_TOOLS.has(toolName)
			? lines.slice(-COLLAPSED_LINES)
			: lines.slice(0, COLLAPSED_LINES);
	const hidden = lines.length - chosen.length;

	let body = chosen.map((line) => theme.fg("toolOutput", line)).join("\n");
	if (hidden > 0) {
		body += theme.fg("muted", `\n… (${hidden} more, ${EXPAND_HINT})`);
	}
	return new Text(`\n${body}`, 0, 0);
}

// ---------------------------------------------------------------------------
// edit / write: preview that expands to the full diff / body
// ---------------------------------------------------------------------------

interface EditArg {
	oldText?: string;
	newText?: string;
}

function rawPathOf(args: any): string {
	const value = args?.file_path ?? args?.path;
	return typeof value === "string" ? value : "";
}

function editArgs(args: any): EditArg[] {
	return Array.isArray(args?.edits) ? (args.edits as EditArg[]) : [];
}

function editHeader(args: any, theme: Theme): string {
	const filePath = rawPathOf(args) || "...";
	const edits = editArgs(args);
	let header = `${theme.fg("toolTitle", theme.bold("edit"))} ${theme.fg("accent", filePath)}`;
	if (edits.length > 0) {
		header += theme.fg("muted", ` (${edits.length} change${edits.length === 1 ? "" : "s"})`);
	}
	return header;
}

type DiffLineKind = "add" | "del" | "hunk";

interface DiffLine {
	text: string;
	kind: DiffLineKind;
}

function colorDiffLine(line: DiffLine, theme: Theme): string {
	const color =
		line.kind === "add" ? "toolDiffAdded" : line.kind === "del" ? "toolDiffRemoved" : "accent";
	return theme.fg(color, line.text);
}

/** Fallback diff built straight from the tool arguments (available before the result arrives). */
function editArgsDiffLines(args: any): DiffLine[] {
	const edits = editArgs(args);
	const lines: DiffLine[] = [];
	edits.forEach((edit, index) => {
		if (edits.length > 1) {
			lines.push({ text: `@@ change ${index + 1} of ${edits.length} @@`, kind: "hunk" });
		}
		for (const line of (edit.oldText ?? "").split("\n")) lines.push({ text: `- ${line}`, kind: "del" });
		for (const line of (edit.newText ?? "").split("\n")) lines.push({ text: `+ ${line}`, kind: "add" });
	});
	return lines;
}

function renderEditCall(args: any, theme: Theme, context: ToolRenderContext): Component {
	const text = context.lastComponent instanceof Text ? context.lastComponent : new Text("", 0, 0);
	text.setText(editHeader(args, theme));
	return text;
}

function renderEditResult(
	result: AgentToolResult<any>,
	options: ToolRenderResultOptions,
	theme: Theme,
	context: ToolRenderContext,
): Component {
	const component = context.lastComponent instanceof Container ? context.lastComponent : new Container();
	component.clear();

	if (context.isError) {
		const errorText = textOf(result);
		if (errorText) component.addChild(new Text(`\n${theme.fg("error", errorText)}`, 0, 0));
		return component;
	}

	const diff = (result.details as { diff?: string } | undefined)?.diff;
	const bodyLines = diff
		? renderDiff(diff).split("\n")
		: editArgsDiffLines(context.args).map((line) => colorDiffLine(line, theme));
	if (bodyLines.length > 0) {
		component.addChild(new Text(previewText(bodyLines, options.expanded, theme), 0, 0));
	}
	return component;
}

function trimTrailingEmpty(lines: string[]): string[] {
	let end = lines.length;
	while (end > 0 && lines[end - 1] === "") end--;
	return lines.slice(0, end);
}

function countLines(content: string): number {
	if (content === "") return 0;
	return trimTrailingEmpty(content.replace(/\r/g, "").split("\n")).length;
}

function writeHeader(args: any, theme: Theme): string {
	const filePath = rawPathOf(args) || "...";
	const content = typeof args?.content === "string" ? args.content : null;
	let header = `${theme.fg("toolTitle", theme.bold("write"))} ${theme.fg("accent", filePath)}`;
	if (content !== null) {
		const lines = countLines(content);
		if (lines > 0) header += theme.fg("muted", ` (${lines} line${lines === 1 ? "" : "s"})`);
	}
	return header;
}

function renderWriteCall(args: any, theme: Theme, context: ToolRenderContext): Component {
	const text = context.lastComponent instanceof Text ? context.lastComponent : new Text("", 0, 0);
	let output = writeHeader(args, theme);

	const content = typeof args?.content === "string" ? args.content : null;
	if (content !== null && content !== "") {
		const allLines = trimTrailingEmpty(content.replace(/\r/g, "").split("\n"));
		const shown = context.expanded ? allLines : allLines.slice(0, COLLAPSED_LINES);
		const hidden = allLines.length - shown.length;
		if (shown.length > 0) {
			const lang = getLanguageFromPath(rawPathOf(args));
			const rendered = lang
				? highlightCode(shown.join("\n"), lang)
				: shown.map((line) => theme.fg("toolOutput", line));
			output += `\n\n${rendered.join("\n")}`;
		}
		if (hidden > 0) {
			output += theme.fg("muted", `\n… (${hidden} more, ${EXPAND_HINT})`);
		}
	}

	text.setText(output);
	return text;
}

function renderWriteResult(
	result: AgentToolResult<any>,
	_theme: Theme,
	context: ToolRenderContext,
): Component {
	const component = context.lastComponent instanceof Container ? context.lastComponent : new Container();
	component.clear();
	if (context.isError) {
		const errorText = textOf(result);
		if (errorText) component.addChild(new Text(`\n${_theme.fg("error", errorText)}`, 0, 0));
	}
	return component;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

/** True when an executable can be found on PATH (used to decide whether to offer powershell). */
function hasExecutable(name: string): boolean {
	const dirs = (process.env.PATH ?? "").split(path.delimiter).filter(Boolean);
	const suffixes = process.platform === "win32" ? [`${name}.exe`, `${name}.cmd`, name] : [name];
	for (const dir of dirs) {
		for (const suffix of suffixes) {
			try {
				if (fs.existsSync(path.join(dir, suffix))) return true;
			} catch {
				// ignore unreadable PATH entries
			}
		}
	}
	return false;
}

export default function (pi: ExtensionAPI) {
	// execute() uses ctx.cwd at runtime, so the value passed here is only a fallback.
	const cwd = process.cwd();

	const definitions: Record<string, any> = {
		bash: createBashToolDefinition(cwd),
		read: createReadToolDefinition(cwd),
		grep: createGrepToolDefinition(cwd),
		find: createFindToolDefinition(cwd),
		ls: createLsToolDefinition(cwd),
		write: createWriteToolDefinition(cwd),
		edit: createEditToolDefinition(cwd),
	};

	// powershell is only meaningful where a PowerShell binary exists. Registering it
	// unconditionally would surface a broken tool to the model.
	if (process.platform === "win32" || hasExecutable("pwsh") || hasExecutable("powershell")) {
		definitions.powershell = createPowerShellToolDefinition(cwd);
	}

	for (const [name, definition] of Object.entries(definitions)) {
		const overrides: {
			renderResult: (
				result: AgentToolResult<any>,
				options: ToolRenderResultOptions,
				theme: Theme,
				context: ToolRenderContext,
			) => Component;
			renderCall?: (args: any, theme: Theme, context: ToolRenderContext) => Component;
		} = {
			renderResult(result, options, theme, context) {
				if (name === "edit") return renderEditResult(result, options, theme, context);
				if (name === "write") return renderWriteResult(result, theme, context);
				return compactResult(result, options, theme, name);
			},
		};

		if (name === "edit") overrides.renderCall = renderEditCall;
		if (name === "write") overrides.renderCall = renderWriteCall;

		pi.registerTool({
			...definition,
			...overrides,
		} as any);
	}
}
