# pi 配置与扩展

本目录保存 pi 的配置文件和自定义扩展源码，按文件/目录一一对应实际部署路径，可直接拷贝回本机使用。

## 文件说明

| 文件 / 目录 | 部署路径 | 说明 |
|------|----------|------|
| `settings.json` | `~/.pi/agent/settings.json` | 全局设置：默认模型/提供商、主题、TUI 模式、已安装的 npm 扩展包（`packages`）等 |
| `acp.json` | `~/.pi/acp.json` | ACP（上下文管理）配置：压缩触发阈值 `compress.nudgeGrowthTokens`、是否启用 `delegate` 委派 |
| `keybindings.json` | `~/.pi/agent/keybindings.json` | 按键绑定自定义：只保留需要的绑定（如 `app.thinking.cycle`），其余置空以禁用默认快捷键 |
| `edit-modes/` | `~/.pi/agent/extensions/edit-modes/` | 扩展（目录）：编辑模式 `ask-to-edit` / `auto-edit` / `auto-all`，`shift+tab` 切换，bash 只读守卫（tree-sitter-bash AST），`git commit` 强制确认 |
| `compact-tools.ts` | `~/.pi/agent/extensions/compact-tools.ts` | 扩展：工具输出默认折叠 5 个视觉行（折行后），点击 / `ctrl+o` 展开 |
| `token-speed.ts` | `~/.pi/agent/extensions/token-speed.ts` | 扩展：footer 显示生成速度 |

## edit-modes 扩展（目录扩展）

`edit-modes/` 是目录扩展，入口为 `edit-modes/index.ts`（pi 自动发现 `extensions/*/index.ts`）。

bash 只读判定使用 **tree-sitter-bash** 解析成 AST 后遍历，运行依赖：

| 包 | 版本 | 用途 |
|----|------|------|
| `web-tree-sitter` | `0.27.0` | WASM 版 tree-sitter 运行时 |
| `tree-sitter-bash` | `0.25.1` | bash 语法（提供 `tree-sitter-bash.wasm`） |

依赖不入库（可通过命令安装，故只在此说明）。恢复步骤：

```bash
# 1) 拷贝本目录：edit-modes/index.ts、package.json、package-lock.json
mkdir -p ~/.pi/agent/extensions/edit-modes
cp index.ts package.json package-lock.json ~/.pi/agent/extensions/edit-modes/

# 2) 安装依赖（--ignore-scripts 跳过 tree-sitter-bash 的 native 构建，只用其 .wasm）
cd ~/.pi/agent/extensions/edit-modes
npm install --ignore-scripts

# 3) 确认 wasm 存在
ls node_modules/web-tree-sitter/web-tree-sitter.wasm \
   node_modules/tree-sitter-bash/tree-sitter-bash.wasm
```

安装后 `/reload`（或重启 pi）生效。

tree-sitter-bash 是**硬依赖**：如果 wasm 无法加载，bash 判定会失败关闭（要求确认），不会退回到不安全的猜测。

## bash 权限模型

三档编辑模式（`shift+tab` 切换）对 bash 的判定，全部基于同一份 tree-sitter AST：

- `ask-to-edit` / `auto-edit`：**只读**命令放行，其余弹确认；
- `auto-all`：自动放行，仅当 AST 推导出的效果命中危险策略时弹确认（特权/系统命令、递归或项目外写入、系统/包变更、git 历史重写）；命令名是变量、flag 敏感命令里含命令替换、解析失败 → **fail-closed 弹确认**；
- `git commit`：任何模式下都强制确认。

bash 确认对话框只有 **Allow once / Deny**（已移除 “Allow all (this session)”）。

## 测试用例

[tests/edit-modes-bash.md](tests/edit-modes-bash.md) 记录了 bash 判定的完整对拍用例：

- 只读策略（`ask-to-edit` / `auto-edit`）：109 放行 + 62 弹确认；
- auto-all 策略：33 自动放行 + 40 弹确认。

改动分类器后可按该文档复现验证。

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
  "npm:@ollama/pi-web-search"
]
```

## 真彩色

```bash
# ~/.bashrc
export COLORTERM=truecolor
```
