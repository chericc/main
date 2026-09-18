/**
 * Texture loading plus procedural fall-backs.
 *
 * Real maps (Solar System Scope, CC BY 4.0 — see README) are pre-loaded at
 * start-up. Every body that has no published map (most satellites, Pluto) gets
 * a procedurally generated equirectangular albedo/bump pair derived from its
 * real size, colour and surface style, so the app is fully self-contained and
 * always renders something plausible.
 */
import * as THREE from 'three';

/** Seeded PRNG (mulberry32) so procedural textures are deterministic. */
function mulberry32(seed: number): () => number {
  let a = seed >>> 0;
  return () => {
    a = (a + 0x6d2b79f5) >>> 0;
    let t = a;
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

/** Tileable value noise over a `gw × gh` lattice. */
function makeNoise(seed: number, gw: number, gh: number) {
  const rnd = mulberry32(seed);
  const grid = new Float32Array(gw * gh);
  for (let i = 0; i < grid.length; i++) grid[i] = rnd();

  return (u: number, v: number): number => {
    const x = u * gw;
    const y = v * gh;
    const x0 = Math.floor(x);
    const y0 = Math.floor(y);
    const fx = x - x0;
    const fy = y - y0;
    const sx = fx * fx * (3 - 2 * fx);
    const sy = fy * fy * (3 - 2 * fy);
    const i0 = ((x0 % gw) + gw) % gw;
    const i1 = (i0 + 1) % gw;
    const j0 = Math.min(Math.max(y0, 0), gh - 1);
    const j1 = Math.min(j0 + 1, gh - 1);
    const a = grid[j0 * gw + i0];
    const b = grid[j0 * gw + i1];
    const c = grid[j1 * gw + i0];
    const d = grid[j1 * gw + i1];
    return (a * (1 - sx) + b * sx) * (1 - sy) + (c * (1 - sx) + d * sx) * sy;
  };
}

function fbm(seed: number, octaves = 5): (u: number, v: number) => number {
  const layers = Array.from({ length: octaves }, (_, i) => makeNoise(seed + i * 977, 4 << i, 2 << i));
  let amp = 1;
  let norm = 0;
  for (const _ of layers) {
    norm += amp;
    amp *= 0.5;
  }
  return (u, v) => {
    let sum = 0;
    let a = 1;
    for (const layer of layers) {
      sum += layer(u, v) * a;
      a *= 0.5;
    }
    return sum / norm;
  };
}

export type SurfaceStyle = 'rocky' | 'icy' | 'gas' | 'cratered';

export interface ProceduralOptions {
  seed: number;
  color: number;
  style?: SurfaceStyle;
  /** Contrast of the albedo variation, 0..1. */
  contrast?: number;
}

export interface ProceduralMaps {
  map: THREE.CanvasTexture;
  bumpMap: THREE.CanvasTexture;
}

const TEX_W = 512;
const TEX_H = 256;

/** Generates a deterministic albedo + bump pair for a body. */
export function proceduralPlanetMaps(options: ProceduralOptions): ProceduralMaps {
  const { seed, color, style = 'rocky', contrast = 0.35 } = options;
  const base = new THREE.Color(color);
  const noise = fbm(seed, style === 'gas' ? 6 : 5);
  const rnd = mulberry32(seed ^ 0x9e3779b9);

  const albedo = document.createElement('canvas');
  albedo.width = TEX_W;
  albedo.height = TEX_H;
  const actx = albedo.getContext('2d')!;
  const aimg = actx.createImageData(TEX_W, TEX_H);

  const bump = document.createElement('canvas');
  bump.width = TEX_W;
  bump.height = TEX_H;
  const bctx = bump.getContext('2d')!;
  const bimg = bctx.createImageData(TEX_W, TEX_H);

  const [hr, hg, hb] = [base.r, base.g, base.b];
  const hsl = { h: 0, s: 0, l: 0 };
  base.getHSL(hsl);

  for (let y = 0; y < TEX_H; y++) {
    const v = y / TEX_H;
    for (let x = 0; x < TEX_W; x++) {
      const u = x / TEX_W;
      let n: number;

      if (style === 'gas') {
        // Banded gas giant: strong latitude banding, weak longitude variation.
        const band = Math.sin(v * Math.PI * 14 + noise(u * 0.35, v * 0.9) * 5.5);
        const fine = noise(u * 2.2, v * 6.5);
        n = 0.5 + 0.28 * band + 0.22 * (fine - 0.5);
      } else if (style === 'icy') {
        n = 0.55 + 0.45 * (noise(u, v) - 0.5) + 0.18 * (noise(u * 3.1, v * 2.4) - 0.5);
      } else {
        n = 0.5 + (noise(u, v) - 0.5) * (1 + contrast);
      }

      const shade = THREE.MathUtils.clamp(n, 0, 1);
      const l = THREE.MathUtils.clamp(hsl.l * (0.55 + 0.95 * shade), 0.02, 0.98);
      const c2 = new THREE.Color().setHSL(hsl.h, hsl.s * (0.75 + 0.5 * shade), l);

      const i = (y * TEX_W + x) * 4;
      aimg.data[i] = c2.r * 255;
      aimg.data[i + 1] = c2.g * 255;
      aimg.data[i + 2] = c2.b * 255;
      aimg.data[i + 3] = 255;

      const b = Math.round(shade * 255);
      bimg.data[i] = b;
      bimg.data[i + 1] = b;
      bimg.data[i + 2] = b;
      bimg.data[i + 3] = 255;
    }
  }

  actx.putImageData(aimg, 0, 0);
  bctx.putImageData(bimg, 0, 0);

  // Impact craters give the rocky bodies their characteristic pitted look.
  if (style === 'rocky' || style === 'cratered') {
    const count = style === 'cratered' ? 320 : 140;
    for (let i = 0; i < count; i++) {
      const cx = rnd() * TEX_W;
      const cy = TEX_H * (0.08 + rnd() * 0.84);
      const r = 1.5 + Math.pow(rnd(), 2.6) * 20;
      const darkness = 0.16 + rnd() * 0.3;

      actx.beginPath();
      actx.arc(cx, cy, r, 0, Math.PI * 2);
      actx.fillStyle = `rgba(0,0,0,${darkness})`;
      actx.fill();

      actx.beginPath();
      actx.arc(cx, cy, r * 1.16, 0, Math.PI * 2);
      actx.strokeStyle = `rgba(255,255,255,${darkness * 0.5})`;
      actx.lineWidth = Math.max(0.7, r * 0.12);
      actx.stroke();

      // Craters also cut into the bump map.
      const grad = bctx.createRadialGradient(cx, cy, 0, cx, cy, r);
      grad.addColorStop(0, 'rgba(0,0,0,0.75)');
      grad.addColorStop(1, 'rgba(0,0,0,0)');
      bctx.fillStyle = grad;
      bctx.beginPath();
      bctx.arc(cx, cy, r, 0, Math.PI * 2);
      bctx.fill();
    }
  }

  void hr;
  void hg;
  void hb;

  const map = new THREE.CanvasTexture(albedo);
  map.colorSpace = THREE.SRGBColorSpace;
  map.anisotropy = 4;
  const bumpMap = new THREE.CanvasTexture(bump);
  bumpMap.anisotropy = 4;
  return { map, bumpMap };
}

/**
 * Radial alpha profile for a ring system.
 * `bands` are `[start, end, alpha]` in 0..1 of the radial span.
 */
export function proceduralRingTexture(
  bands: Array<[number, number, number]>,
  baseColor: number,
): THREE.CanvasTexture {
  const w = 1024;
  const canvas = document.createElement('canvas');
  canvas.width = w;
  canvas.height = 1;
  const ctx = canvas.getContext('2d')!;
  const img = ctx.createImageData(w, 1);
  const c = new THREE.Color(baseColor);

  for (let x = 0; x < w; x++) {
    const t = x / (w - 1);
    let a = 0;
    for (const [s, e, alpha] of bands) {
      if (t >= s && t <= e) {
        // Smooth the band edges to avoid hard aliasing.
        const local = (t - s) / Math.max(1e-6, e - s);
        const edge = Math.min(1, Math.min(local, 1 - local) * 12 + 0.25);
        a = Math.max(a, alpha * edge);
      }
    }
    const i = x * 4;
    img.data[i] = c.r * 255;
    img.data[i + 1] = c.g * 255;
    img.data[i + 2] = c.b * 255;
    img.data[i + 3] = THREE.MathUtils.clamp(a, 0, 1) * 255;
  }

  ctx.putImageData(img, 0, 0);
  const tex = new THREE.CanvasTexture(canvas);
  tex.colorSpace = THREE.SRGBColorSpace;
  tex.wrapS = THREE.ClampToEdgeWrapping;
  tex.wrapT = THREE.ClampToEdgeWrapping;
  return tex;
}

/** Soft radial glow, used for the solar corona sprite. */
export function glowTexture(inner = 'rgba(255,246,214,1)', outer = 'rgba(255,170,60,0)'): THREE.CanvasTexture {
  const size = 256;
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const ctx = canvas.getContext('2d')!;
  const grad = ctx.createRadialGradient(size / 2, size / 2, 0, size / 2, size / 2, size / 2);
  grad.addColorStop(0, inner);
  grad.addColorStop(0.18, 'rgba(255,224,150,0.85)');
  grad.addColorStop(0.45, 'rgba(255,170,60,0.28)');
  grad.addColorStop(1, outer);
  ctx.fillStyle = grad;
  ctx.fillRect(0, 0, size, size);
  const tex = new THREE.CanvasTexture(canvas);
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

/**
 * Pre-loads a set of textures. Failures are collected (not thrown) so the
 * caller can fall back to procedural maps.
 *
 * `paths` are already-resolved URLs (Vite asset imports) — data URLs in the
 * single-file build, plain files otherwise.
 */
export class TextureLibrary {
  private readonly textures = new Map<string, THREE.Texture>();
  readonly failed = new Set<string>();

  async preload(paths: Iterable<string>, manager: THREE.LoadingManager): Promise<void> {
    const loader = new THREE.TextureLoader(manager);
    const unique = [...new Set(paths)];
    await Promise.all(
      unique.map(async (path) => {
        try {
          const tex = await loader.loadAsync(path);
          tex.colorSpace = THREE.SRGBColorSpace;
          tex.anisotropy = 8;
          this.textures.set(path, tex);
        } catch {
          this.failed.add(path);
        }
      }),
    );
  }

  has(path: string | undefined): boolean {
    return !!path && this.textures.has(path);
  }

  get(path: string): THREE.Texture | undefined {
    return this.textures.get(path);
  }
}
