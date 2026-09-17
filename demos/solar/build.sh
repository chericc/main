#!/usr/bin/env bash
#
# 一键构建 Solar 单文件版本。
#
# 产出：dist/index.html —— 一个自包含的 HTML（JS / CSS / 贴图全部内联，
# 约 3.5 MB），可直接双击用浏览器打开，无需任何服务器。
#
# 用法：
#   ./build.sh                  # 构建到 dist/index.html
#   ./build.sh out/solar.html   # 额外复制一份到指定路径
#   ./build.sh --server         # 常规多文件构建（需静态服务器）
#
set -euo pipefail

cd "$(dirname "$0")"

MODE="standalone"
OUT=""

for arg in "$@"; do
  case "$arg" in
    -h|--help)
      sed -n '3,12p' "$0" | sed 's/^# \{0,1\}//'
      exit 0
      ;;
    --server)
      MODE="server"
      ;;
    -*)
      echo "未知参数：$arg" >&2
      exit 1
      ;;
    *)
      OUT="$arg"
      ;;
  esac
done

# ---- 环境检查 ----------------------------------------------------------------
command -v node >/dev/null 2>&1 || { echo "错误：未找到 node，请先安装 Node.js ≥ 20.19。" >&2; exit 1; }

NODE_MAJOR="$(node -p 'process.versions.node.split(".")[0]')"
NODE_MINOR="$(node -p 'process.versions.node.split(".")[1]')"
if (( NODE_MAJOR < 20 )) || { (( NODE_MAJOR == 20 )) && (( NODE_MINOR < 19 )); }; then
  echo "错误：需要 Node.js ≥ 20.19，当前为 $(node -v)。" >&2
  exit 1
fi

# ---- 安装依赖 ----------------------------------------------------------------
if [[ ! -d node_modules ]]; then
  if [[ -f package-lock.json ]]; then
    echo "==> 安装依赖（npm ci）"
    npm ci
  else
    echo "==> 安装依赖（npm install）"
    npm install
  fi
else
  echo "==> 依赖已存在，跳过安装（如需重装请删除 node_modules）"
fi

# ---- 类型检查 + 构建 ---------------------------------------------------------
if [[ "$MODE" == "server" ]]; then
  echo "==> 类型检查并构建（多文件模式，输出到 dist/）"
  npm run build:server
  echo
  echo "✔ 构建完成：$(pwd)/dist/index.html（多文件产物，需通过静态服务器访问）"
  exit 0
fi

echo "==> 类型检查并构建（单文件模式）"
npm run build

# ---- 校验产物 ----------------------------------------------------------------
TARGET="dist/index.html"
[[ -f "$TARGET" ]] || { echo "错误：构建结束但未找到 $TARGET。" >&2; exit 1; }

SIZE="$(du -h "$TARGET" | cut -f1)"

if [[ -n "$OUT" ]]; then
  mkdir -p "$(dirname "$OUT")"
  cp "$TARGET" "$OUT"
  echo
  echo "✔ 构建完成："
  echo "   - $TARGET ($SIZE)"
  echo "   - $OUT ($SIZE)"
else
  echo
  echo "✔ 构建完成：$TARGET ($SIZE)"
fi
echo "   可直接双击用浏览器打开。"
