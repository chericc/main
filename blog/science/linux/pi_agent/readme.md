# pi 配置与扩展

本目录保存 pi 的配置文件和自定义扩展源码，按文件一一对应实际部署路径，可直接拷贝回本机使用。

## 文件说明

| 文件 | 部署路径 | 说明 |
|------|----------|------|
| `settings.json` | `~/.pi/agent/settings.json` | 全局设置：默认模型/提供商、主题、TUI 模式、已安装的 npm 扩展包（`packages`）等 |
| `acp.json` | `~/.pi/acp.json` | ACP（上下文管理）配置：压缩触发阈值 `compress.nudgeGrowthTokens`、是否启用 `delegate` 委派 |
| `keybindings.json` | `~/.pi/agent/keybindings.json` | 按键绑定自定义：只保留需要的绑定（如 `app.thinking.cycle`），其余置空以禁用默认快捷键 |
| `edit-modes.ts` | `~/.pi/agent/extensions/edit-modes.ts` | 扩展：编辑模式 `ask-to-edit` / `auto-edit` / `auto-all`，`shift+tab` 切换，bash 只读守卫，`git commit` 强制确认 |
| `compact-tools.ts` | `~/.pi/agent/extensions/compact-tools.ts` | 扩展：工具输出默认折叠 5 个视觉行（折行后），点击 / `ctrl+o` 展开 |
| `token-speed.ts` | `~/.pi/agent/extensions/token-speed.ts` | 扩展：footer 显示生成速度 |

## 扩展安装 / 管理

```bash
# 查看已安装
pi list

# 安装 / 卸载
pi install npm:<package>
pi remove npm:<package>

# 更新扩展
pi update --extensions
```

`settings.json` 的 `packages` 字段记录已安装的 npm 扩展包：

```json
"packages": [
  "npm:@ollama/pi-web-search",
  "npm:billion-context-pi"
]
```

## 真彩色

```bash
# ~/.bashrc
export COLORTERM=truecolor
```
