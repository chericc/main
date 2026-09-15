/**
 * Token speed extension
 *
 * Shows a smoothed generation speed (median + average over the last few
 * responses) in the footer, computed once a message finishes (`message_end`).
 * The value stays visible while the next response is generating (it is only
 * replaced when a new speed is computed, never cleared).
 *
 * Very short responses are filtered out (MIN_SAMPLE_TOKENS / MIN_SAMPLE_SECONDS):
 * a handful of tokens over a few hundred ms can look like a huge burst.
 *
 * Speed = output tokens / time between the first streamed delta and message end,
 * so time-to-first-token is excluded. Falls back to a character-based estimate
 * when the provider reports no output token usage.
 *
 * Placement: ~/.pi/agent/extensions/token-speed.ts (hot-reload with /reload)
 */

import type { ExtensionAPI, ExtensionContext } from "@earendil-works/pi-coding-agent";

export default function (pi: ExtensionAPI) {
	let startedAt = 0;
	let chars = 0;
	let active = false;

	// Rolling window of per-message speeds; the footer shows the median
	// (robust against short-message spikes) and the arithmetic mean.
	const WINDOW = 5;
	const samples: number[] = [];

	// Responses that are too short to measure reliably are not recorded; the
	// footer keeps showing the previous value instead.
	const MIN_SAMPLE_TOKENS = 20;
	const MIN_SAMPLE_SECONDS = 0.5;

	const fmt = (v: number) => (v >= 100 ? Math.round(v).toString() : v.toFixed(1));

	const median = (values: number[]) => {
		const sorted = [...values].sort((a, b) => a - b);
		const mid = Math.floor(sorted.length / 2);
		return sorted.length % 2 === 1 ? sorted[mid] : (sorted[mid - 1] + sorted[mid]) / 2;
	};

	const formatSpeed = (event: { message: { usage?: { output?: number } } }, ctx: ExtensionContext) => {
		if (!active || startedAt === 0) return;
		const elapsedSeconds = (Date.now() - startedAt) / 1000;
		if (elapsedSeconds <= 0) return;

		const reported = event.message.usage?.output ?? 0;
		const tokens = reported > 0 ? reported : chars / 4; // rough estimate for providers without usage
		if (tokens < MIN_SAMPLE_TOKENS || elapsedSeconds < MIN_SAMPLE_SECONDS) return;

		const tps = tokens / elapsedSeconds;

		samples.push(tps);
		if (samples.length > WINDOW) samples.shift();

		const med = median(samples);
		const avg = samples.reduce((sum, v) => sum + v, 0) / samples.length;
		const text = samples.length === 1 ? `${fmt(med)} tok/s` : `med ${fmt(med)} · avg ${fmt(avg)} tok/s`;
		ctx.ui.setStatus("token-speed", ctx.ui.theme.fg("dim", text));
	};

	pi.on("message_start", async (event) => {
		if (event.message.role !== "assistant") return;
		startedAt = 0;
		chars = 0;
		active = true;
	});

	pi.on("message_update", async (event) => {
		if (!active || startedAt !== 0) return;
		const streamEvent = event.assistantMessageEvent;
		if (
			streamEvent.type === "text_delta" ||
			streamEvent.type === "thinking_delta" ||
			streamEvent.type === "toolcall_delta"
		) {
			startedAt = Date.now();
			const delta = (streamEvent as { delta?: unknown }).delta;
			chars += (typeof delta === "string" ? delta : JSON.stringify(delta ?? "")).length;
		}
	});

	pi.on("message_end", async (event, ctx) => {
		if (event.message.role !== "assistant") return;
		formatSpeed(event, ctx);
		active = false;
	});
}
