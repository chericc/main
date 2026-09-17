# Scolar

**真实比例太阳系模拟 · A real-scale, data-driven 3D solar system for the web.**

Scolar 用 JPL 的真实轨道要素实时求解开普勒方程，按 **1 世界单位 = 1 AU**
的真实比例渲染太阳、8 大行星、5 颗矮行星、4 颗主带小行星、2 颗周期彗星、
23 颗卫星以及小行星带与柯伊伯带。天体半径、轨道距离、自转轴指向均取自
真实观测数据，不做艺术化缩放（可选的半径放大滑块默认关闭）。

---

## 快速开始

```bash
npm install
npm run dev          # http://127.0.0.1:5173
```

## 构建

```bash
npm run build        # 单文件构建：dist/index.html 可直接双击打开
npm run build:server # 常规多文件构建（需静态服务器）
npm run preview      # 预览构建产物
npm test             # 27 项单元测试（含与 JPL Horizons 的对拍）
npm run typecheck
```

`npm run build` 产出**单个自包含的 `dist/index.html`（约 3.5 MB）**，
JS、CSS 与全部贴图都已内联为 data URL，因此无需任何服务器，直接双击即可在
浏览器中查看（`file://` 协议下浏览器会以 CORS 拦截外部图片加载，内联是唯一
可靠的方案）。

---

## 操作

| 操作 | 说明 |
| --- | --- |
| 拖拽 / 滚轮 | 旋转 / 缩放（视口内的滚轮只缩放相机，不会缩放网页） |
| 点击天体或标签 | 飞抵并跟随该天体 |
| `Space` | 暂停 / 继续 |
| `[` `]` | 切换上一个 / 下一个天体 |
| `0` 或 `Esc` | 回到全景 |
| `+` `-` | 快速拉近 / 拉远 |
| 左侧列表 | 搜索并跳转到任意天体（真实比例下最可靠的定位方式） |
| URL 深链 | `index.html#saturn` 直接定位到指定天体 |

右侧信息面板显示该天体的**真实物理数据**（半径、质量、密度、重力、逃逸速度、
自转周期、转轴倾角、反照率、轨道要素、来源历元）以及**实时状态**
（日心距、黄经黄纬、距地球、轨道速度、距相机）。

---

## 项目结构

```
scolar/
├─ index.html                    入口 HTML
├─ vite.config.ts                构建配置（含单文件内联插件）
├─ src/
│  ├─ core/                      纯数学，不依赖任何图形库，可直接单测
│  │  ├─ constants.ts            AU、J2000、高斯引力常数、黄赤交角
│  │  ├─ time.ts                 儒略日换算 + 仿真时钟
│  │  ├─ kepler.ts               开普勒方程求解、六要素传播、轨道采样
│  │  ├─ lunar-theory.ts         月球摄动理论（12 个主要周期项）
│  │  ├─ quat.ts                 纯 TS 四元数工具
│  │  └─ model.ts                太阳系模型：位置 / 姿态 / 轨道线 / 速度
│  ├─ data/                      真实数据（每个文件都标注来源）
│  │  ├─ types.ts                数据模型定义
│  │  ├─ sun.ts / planets.ts / dwarf-planets.ts / small-bodies.ts
│  │  ├─ moons.ts / moon-physical.ts
│  │  ├─ moon-orbits.generated.ts  由脚本从 JPL 原始表生成
│  │  ├─ textures.ts             贴图资源（经 Vite 导入）
│  │  └─ index.ts                组装天体层级 + 查找索引
│  ├─ scene/                     three.js 渲染层
│  │  ├─ scaling.ts              AU ↔ 世界坐标、真实比例
│  │  ├─ textures.ts             贴图加载 + 程序化回退
│  │  ├─ body-view.ts            球体 / 扁率 / 云层 / 光环 / 标签
│  │  ├─ orbit-line.ts           轨道轨迹线
│  │  ├─ belt.ts                 小行星带 / 柯伊伯带
│  │  ├─ camera-controller.ts    动态近裁剪面、飞行动画、跟随模式
│  │  └─ solar-system-scene.ts   场景总装、屏幕空间拾取
│  ├─ ui/                        原生 DOM 界面（无框架）
│  └─ assets/textures/           打包用贴图（由 masters/ 降采样而来）
├─ tests/                        Vitest：开普勒求解、月球理论、Horizons 对拍
├─ scripts/                      数据抓取 / 解析 / 生成脚本
├─ reference/                    下载到的 JPL 原始数据（见 reference/README.md）
├─ masters/textures/             全分辨率贴图原图
└─ dist/                         构建产物（单文件）
```

---

## 数据来源

| 用途 | 来源 |
| --- | --- |
| 8 大行星轨道要素（J2000 + 世纪变率） | **E. M. Standish & J. G. Williams (1992)**，即 JPL *Approximate Positions of the Planets* 所用表，有效期 1800–2050 AD |
| 矮行星 / 小行星 / 彗星轨道要素 | **JPL Small-Body Database**，全精度瞬时要素 |
| 卫星轨道要素（含真实 M、ω、Ω、i、a、e、P） | **JPL Planetary Satellite Mean Elements** |
| 半径 / 质量 / 密度 / 重力 / 自转 / 温度 / 反照率 | **NASA Planetary Fact Sheet**、JPL 卫星物理参数表 |
| 自转轴指向（RA/Dec） | **IAU WGCCRE** 报告（Archinal et al.） |
| 月球位置 | Schlyter 月球摄动理论（经向/纬向精度约角分量级） |
| 行星扁率 | IAU / NASA（木星、土星扁率肉眼可见） |

`reference/` 保存了全部原始下载数据，`reference/README.md` 记录了每个文件的
出处与所做的换算。

### 实测精度（对比 JPL Horizons 黄道 J2000 直角坐标）

```
水星   10.4″      473 km      木星  309.0″   549 524 km
金星   21.4″    2 465 km      土星  516.8″ 1 724 223 km
地球    8.5″    5 014 km      天王星 79.3″ 1 149 887 km
火星   58.8″    7 264 km      海王星 45.0″   725 629 km
```

这些偏差与 Standish & Williams 公布的标称精度一致；测试用例把它们作为上界断言。
冥王星使用单一历元的瞬时要素且未建模海王星摄动，34 年后位置偏差约 0.24%，
测试中单独以 1% 为上界并注明原因。

---

## 已知限制

Scolar 追求的是**几何与轨道上的真实**，以下方面做了明确的简化（均不影响行星
位置与相对大小）：

- **自转相位（本初子午线）未建模**。自转周期与自转轴指向是真实的，但「哪一面
  朝向太阳」的初相位是任意的，因此地球大陆与昼夜的对应关系不代表某个真实时刻。
- **岁差与章动未建模**。自转轴指向固定为 IAU 报告给出的 J2000 指向。
- **卫星参考面近似**。JPL 表中参考面标注为 `Laplace`（拉普拉斯面）的卫星，
  本模拟用母星赤道面近似；`equatorial` 与 `ecliptic` 面则是精确的。
- **光照不遵守平方反比**。太阳点光源使用 `decay = 0`，否则海王星会比地球
  暗约 900 倍而完全不可见；光照**方向**（昼夜分界线）仍然正确。
- **小行星带 / 柯伊伯带是合成星群**，轨道要素按各自族群的真实分布抽样，
  个体并非真实天体。
- **行星使用地月质心要素**（Standish 表的 EM Bary），由此带来的地心误差
  < 0.0001 AU，远小于显示分辨率。
- **矮行星贴图 / 全部卫星贴图**为程序化生成（SSS 未发布冥王星贴图），
  颜色取自真实观测色彩。

---

## 重新生成数据与贴图

```bash
# JPL 数据（需要 curl，可用本机代理）
HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:reference
HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:satellites
npm run gen:moons             # 从 reference/satellite-elements.json 重新生成卫星轨道

# 贴图
HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:textures   # 下载原图 + 降采样
npm run optimize:textures                                  # 只重新降采样
```

---

## 许可与致谢

- 代码：MIT。
- 行星 / 恒星贴图：**Solar System Scope**（https://www.solarsystemscope.com/textures/），
  **CC BY 4.0**；原图存于 `masters/textures/`，打包用的降采样版本存于
  `src/assets/textures/`。
- 轨道与物理数据：NASA / JPL（Horizons、SBDB、Planetary Fact Sheet、
  Planetary Satellite Mean Elements）与 IAU WGCCRE 报告，均为公开数据。

若贴图加载失败，渲染器会自动回退到由天体真实颜色与尺寸生成的程序化贴图，
因此项目在完全离线的环境下依然可用。
