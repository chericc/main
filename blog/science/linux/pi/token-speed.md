# pi token 速度显示

## 背景

pi（`@earendil-works/pi-coding-agent`）默认 footer **不显示**生成速度（tokens/s）。默认 footer 只显示：

- 工作目录、session 名；
- token / cache 用量（`↑` 输入、`↓` 输出、`R` cache read、`W` cache write、`CH` 最新缓存命中率）；
- 成本、上下文占用、当前模型。

其中 `CH` 是缓存命中率，不是速度，pi 本身也没有内置的 tok/s 指标。要显示速度只能通过扩展自己算。

## 实现

下面的扩展只在 `message_end`（消息结束）时计算并显示最近一次回复的生成速度：

- `message_start`：重置状态，并清空上一次的速度；
- `message_update`：只记录**第一个流式 delta** 的时间戳（排除首 token 延迟 / TTFT），并累计字符数作为兜底，**不刷新界面**；
- `message_end`：用 `event.message.usage.output`（真实输出 token 数）除以「首个 delta → 消息结束」的耗时，得到 `tok/s`，写入 footer 状态区。

provider 没有返回 `usage.output` 时，退化为按字符数 `/4` 估算。

> 注意：`usage.output` 包含 reasoning/thinking 的 token（如果模型开启思考），所以显示的是包含思考的整体生成速度。速度在消息结束后保留，直到下一次 assistant 消息开始时被清空。

## 扩展文件

放到全局扩展目录即可被自动发现，无需修改 `settings.json`：

```bash
~/.pi/agent/extensions/token-speed.ts
```

## 完整代码

```ts
/**
 * Token speed extension
 *
 * Shows the generation speed of the last assistant response in the footer,
 * computed once the message finishes (`message_end`).
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

	const formatSpeed = (event: { message: { usage?: { output?: number } } }, ctx: ExtensionContext) => {
		if (!active || startedAt === 0) return;
		const elapsedSeconds = (Date.now() - startedAt) / 1000;
		if (elapsedSeconds <= 0) return;

		const reported = event.message.usage?.output ?? 0;
		const tokens = reported > 0 ? reported : chars / 4; // rough estimate for providers without usage
		const tps = tokens / elapsedSeconds;

		const text = `${tps >= 100 ? Math.round(tps) : tps.toFixed(1)} tok/s`;
		ctx.ui.setStatus("token-speed", ctx.ui.theme.fg("dim", text));
	};

	pi.on("message_start", async (event, ctx) => {
		if (event.message.role !== "assistant") return;
		startedAt = 0;
		chars = 0;
		active = true;
		ctx.ui.setStatus("token-speed", undefined);
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
```

## 验证

用本地 ollama 模型实测，事件与 usage 均正常：

```text
message_start assistant
update type=thinking_delta deltaLen=3
...
update type=text_delta deltaLen=2
message_end usage.output=25 usage={"input":1513,"output":25,"cacheRead":0,"cacheWrite":0,"reasoning":0,"totalTokens":1538,...}
```

说明 `assistantMessageEvent` 的 `thinking_delta` / `text_delta` 都会触发，且 `usage.output` 有真实值。

## 注意事项

- `-p`（print）/ `--mode json` 模式没有 footer，看不到速度属正常，只影响交互式 TUI。
- 想换显示内容（比如加上总耗时、TTFT），改 `formatSpeed` 里的 `text` 即可。
- 新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效。
