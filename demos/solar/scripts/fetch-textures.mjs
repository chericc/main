#!/usr/bin/env node
/**
 * Downloads the planet/star textures used by Solar into `masters/textures/`.
 * These are the full-resolution masters; `npm run optimize:textures` derives
 * the down-scaled copies that are actually bundled from them.
 *
 * Source: Solar System Scope — https://www.solarsystemscope.com/textures/
 * Licence: CC BY 4.0 (free to use with attribution; see README.md).
 *
 * Usage: npm run fetch:textures
 *
 * Missing textures are not fatal: the renderer falls back to a procedurally
 * generated texture, so the app always runs offline.
 */
import { spawn } from 'node:child_process';
import { mkdir, stat } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const OUT_DIR = resolve(ROOT, 'masters/textures');
const BASE = 'https://www.solarsystemscope.com/textures/download/';

/** Local file name -> remote name on solarsystemscope.com */
const TEXTURES = {
  'sun.jpg': '2k_sun.jpg',
  'mercury.jpg': '2k_mercury.jpg',
  'venus-atmosphere.jpg': '2k_venus_atmosphere.jpg',
  'earth-day.jpg': '2k_earth_daymap.jpg',
  'earth-night.jpg': '2k_earth_nightmap.jpg',
  'earth-clouds.jpg': '2k_earth_clouds.jpg',
  'moon.jpg': '2k_moon.jpg',
  'mars.jpg': '2k_mars.jpg',
  'jupiter.jpg': '2k_jupiter.jpg',
  'saturn.jpg': '2k_saturn.jpg',
  'saturn-ring.png': '2k_saturn_ring_alpha.png',
  'uranus.jpg': '2k_uranus.jpg',
  'neptune.jpg': '2k_neptune.jpg',
  'ceres.jpg': '2k_ceres_fictional.jpg',
  'eris.jpg': '2k_eris_fictional.jpg',
  'haumea.jpg': '2k_haumea_fictional.jpg',
  'makemake.jpg': '2k_makemake_fictional.jpg',
  'stars-milkyway.jpg': '2k_stars_milky_way.jpg',
};

async function exists(path) {
  try {
    const s = await stat(path);
    return s.size > 1024;
  } catch {
    return false;
  }
}

/**
 * Downloads with curl so that an HTTP(S) proxy can be used. Set `HTTPS_PROXY`
 * (or `https_proxy`) to route the requests, e.g.
 *   HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:textures
 */
const PROXY = process.env.HTTPS_PROXY || process.env.https_proxy || process.env.HTTP_PROXY || process.env.http_proxy;

function run(cmd, args) {
  return new Promise((resolvePromise, reject) => {
    const child = spawn(cmd, args, { stdio: ['ignore', 'ignore', 'pipe'] });
    let stderr = '';
    child.stderr.on('data', (d) => (stderr += d));
    child.on('error', reject);
    child.on('close', (code) => (code === 0 ? resolvePromise() : reject(new Error(`curl exit ${code}: ${stderr.trim()}`))));
  });
}

async function download(url, dest) {
  const args = ['-sSL', '--fail', '--retry', '8', '--retry-delay', '2', '--retry-all-errors', '--retry-connrefused', '--connect-timeout', '30', '-o', dest];
  if (PROXY) args.push('-x', PROXY);
  args.push(url);
  await run('curl', args);
  const s = await stat(dest);
  if (s.size < 1024) throw new Error(`suspiciously small file (${s.size} B)`);
  return s.size;
}

const force = process.argv.includes('--force');

await mkdir(OUT_DIR, { recursive: true });
if (PROXY) console.log(`Using proxy ${PROXY}\n`);

let ok = 0;
let skipped = 0;
const failed = [];

for (const [local, remote] of Object.entries(TEXTURES)) {
  const dest = resolve(OUT_DIR, local);
  if (!force && (await exists(dest))) {
    skipped++;
    console.log(`  skip  ${local}`);
    continue;
  }
  process.stdout.write(`  get   ${local} … `);
  try {
    const size = await download(BASE + remote, dest);
    ok++;
    console.log(`${(size / 1024).toFixed(0)} KiB`);
  } catch (err) {
    failed.push(local);
    console.log(`FAILED (${err.message})`);
  }
}

console.log(`\n${ok} downloaded, ${skipped} skipped, ${failed.length} failed.`);
if (failed.length) {
  console.log(`Fallback (procedural) textures will be used for: ${failed.join(', ')}`);
}
