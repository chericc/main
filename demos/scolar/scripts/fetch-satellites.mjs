#!/usr/bin/env node
/**
 * Downloads the JPL "Planetary Satellite Mean Elements" page into
 * `reference/raw/satellite-elements.html`.
 *
 * Usage:
 *   HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:satellites
 *
 * Run `node scripts/parse-satellites.mjs` afterwards (the npm script does both),
 * then `npm run gen:moons` to regenerate src/data/moon-orbits.generated.ts.
 */
import { spawn } from 'node:child_process';
import { mkdir } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const OUT = resolve(ROOT, 'reference/raw/satellite-elements.html');
const PROXY =
  process.env.HTTPS_PROXY || process.env.https_proxy || process.env.HTTP_PROXY || process.env.http_proxy;

await mkdir(dirname(OUT), { recursive: true });

const args = ['-sSL', '--fail', '--retry', '5', '--retry-delay', '2', '--retry-all-errors', '--connect-timeout', '20', '-o', OUT];
if (PROXY) args.push('-x', PROXY);
args.push('https://ssd.jpl.nasa.gov/sats/elem/');

await new Promise((resolvePromise, reject) => {
  const child = spawn('curl', args, { stdio: 'inherit' });
  child.on('error', reject);
  child.on('close', (code) => (code === 0 ? resolvePromise() : reject(new Error(`curl exited ${code}`))));
});

console.log(`Saved ${OUT}`);
