#!/usr/bin/env node
/**
 * Parses the JPL "Planetary Satellite Mean Elements" table
 * (https://ssd.jpl.nasa.gov/sats/elem/) into `reference/satellite-elements.json`.
 *
 * Run `npm run fetch:satellites` first (or fetch-reference, which caches the
 * raw HTML), then this script.
 */
import { readFile, writeFile } from 'node:fs/promises';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const html = await readFile(resolve(ROOT, 'reference/raw/satellite-elements.html'), 'utf8');

const table = html.match(/<table[\s\S]*?<\/table>/i)?.[0];
if (!table) throw new Error('no table found');

const rows = [...table.matchAll(/<tr[\s\S]*?<\/tr>/gi)].map((m) => m[0]);

function cells(row) {
  return [...row.matchAll(/<t[dh][\s\S]*?<\/t[dh]>/gi)].map((m) =>
    m[0]
      .replace(/<[^>]+>/g, ' ')
      .replace(/&nbsp;/g, ' ')
      .replace(/&amp;/g, '&')
      .replace(/\s+/g, ' ')
      .trim(),
  );
}

const out = {};
for (const row of rows) {
  const c = cells(row);
  if (c.length < 14) continue;
  const [id, planet, sat, code, eph, frame, epoch, a, e, w, M, i, node, P] = c;
  if (!/^\d+$/.test(id) || !planet || !sat) continue;
  out[sat] = {
    jplId: Number(id),
    planet,
    code,
    ephemeris: eph,
    frame,
    epoch: epoch || '2000-01-01.5',
    aKm: Number(a),
    e: Number(e),
    argPeriDeg: Number(w),
    meanAnomalyDeg: Number(M),
    inclinationDeg: Number(i),
    nodeDeg: Number(node),
    periodDays: Number(P),
  };
}

await writeFile(
  resolve(ROOT, 'reference/satellite-elements.json'),
  JSON.stringify(
    {
      source: 'JPL Planetary Satellite Mean Elements — https://ssd.jpl.nasa.gov/sats/elem/',
      note: 'Frame column states the reference plane of the mean elements (ecliptic / equator / Laplace).',
      satellites: out,
    },
    null,
    2,
  ),
);

const want = [
  'Moon',
  'Phobos',
  'Deimos',
  'Io',
  'Europa',
  'Ganymede',
  'Callisto',
  'Mimas',
  'Enceladus',
  'Tethys',
  'Dione',
  'Rhea',
  'Titan',
  'Hyperion',
  'Iapetus',
  'Miranda',
  'Ariel',
  'Umbriel',
  'Titania',
  'Oberon',
  'Triton',
  'Proteus',
  'Charon',
];
console.log(`${Object.keys(out).length} satellites parsed.`);
for (const n of want) {
  const s = out[n];
  if (!s) {
    console.log(`  MISSING ${n}`);
    continue;
  }
  console.log(
    `  ${n.padEnd(10)} code=${s.code.padEnd(6)} frame=${s.frame.padEnd(18)} a=${s.aKm} e=${s.e} w=${s.argPeriDeg} M=${s.meanAnomalyDeg} i=${s.inclinationDeg} node=${s.nodeDeg} P=${s.periodDays}`,
  );
}
