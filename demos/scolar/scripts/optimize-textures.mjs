#!/usr/bin/env node
/**
 * Produces the down-scaled textures that the app actually bundles
 * (`src/assets/textures/`) from the full-resolution masters downloaded into
 * `masters/textures/` by `npm run fetch:textures`.
 *
 * Down-scaling keeps the single-file build (which inlines every texture as a
 * data URL) small enough to be pleasant to open. Uses macOS `sips`; when it is
 * not available the masters are copied through unchanged.
 *
 * Usage: npm run optimize:textures
 */
import { execFile } from 'node:child_process';
import { mkdir, readdir, copyFile, stat } from 'node:fs/promises';
import { dirname, extname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { promisify } from 'node:util';

const execFileAsync = promisify(execFile);
const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const SRC = resolve(ROOT, 'masters/textures');
const OUT = resolve(ROOT, 'src/assets/textures');
const MAX_SIZE = Number(process.env.TEXTURE_MAX_SIZE ?? 1024);
const JPEG_QUALITY = process.env.TEXTURE_JPEG_QUALITY ?? '72';

async function hasSips() {
  try {
    await execFileAsync('sips', ['--version']);
    return true;
  } catch {
    return false;
  }
}

const sipsAvailable = await hasSips();
if (!sipsAvailable) {
  console.warn('sips not found — copying textures at full resolution.');
}

await mkdir(OUT, { recursive: true });
const files = (await readdir(SRC)).filter((f) => /\.(jpe?g|png)$/i.test(f));

let total = 0;
for (const file of files) {
  const input = join(SRC, file);
  const output = join(OUT, file);
  if (sipsAvailable) {
    const args = ['-Z', String(MAX_SIZE)];
    if (/\.jpe?g$/i.test(file)) args.push('-s', 'format', 'jpeg', '-s', 'formatOptions', JPEG_QUALITY);
    args.push(input, '--out', output);
    await execFileAsync('sips', args);
  } else {
    await copyFile(input, output);
  }
  const size = (await stat(output)).size;
  total += size;
  console.log(`  ${file.padEnd(26)} ${(size / 1024).toFixed(0)} KiB`);
}

console.log(`\n${files.length} textures, ${(total / 1024 / 1024).toFixed(2)} MiB total (max ${MAX_SIZE}px).`);
void extname;
