#!/usr/bin/env node
/**
 * Fetches the authoritative source data used by Solar and stores it under
 * `reference/` so that it can be inspected/audited later.
 *
 * Sources
 *   - JPL Horizons (https://ssd.jpl.nasa.gov/horizons/) — heliocentric
 *     ecliptic-J2000 state vectors, used as the ground truth in our tests.
 *   - JPL Small-Body Database (https://ssd-api.jpl.nasa.gov/sbdb.api) —
 *     osculating elements for dwarf planets, large asteroids and comets.
 *
 * Usage:
 *   HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:reference
 *
 * Requires `curl` (needed because Node's fetch does not honour *_PROXY).
 */
import { spawn } from 'node:child_process';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const REF = resolve(ROOT, 'reference');
const RAW = resolve(REF, 'raw');
const PROXY =
  process.env.HTTPS_PROXY || process.env.https_proxy || process.env.HTTP_PROXY || process.env.http_proxy;

function curlOnce(url, useProxy) {
  return new Promise((res, rej) => {
    const args = [
      '-sS',
      '--fail',
      '--retry',
      '3',
      '--retry-delay',
      '1',
      '--retry-all-errors',
      '--retry-connrefused',
      '--max-time',
      '40',
      '--connect-timeout',
      '15',
      '-A',
      'solar/1.0',
    ];
    if (useProxy) args.push('-x', useProxy);
    args.push(url);
    const child = spawn('curl', args, { stdio: ['ignore', 'pipe', 'pipe'] });
    let out = '';
    let err = '';
    child.stdout.on('data', (d) => (out += d));
    child.stderr.on('data', (d) => (err += d));
    child.on('error', rej);
    child.on('close', (code) => (code === 0 ? res(out) : rej(new Error(`curl exit ${code}: ${err.trim()}`))));
  });
}

/** Try the proxy first, then fall back to a direct connection. */
async function curl(url) {
  if (PROXY) {
    try {
      return await curlOnce(url, PROXY);
    } catch (e) {
      process.stdout.write(`(proxy failed: ${e.message.split('\n')[0]}, retrying direct) `);
    }
  }
  return curlOnce(url, null);
}

/** Fetch `url`, caching the raw body at `cachePath` so reruns are instant. */
async function curlCached(url, cachePath) {
  if (existsSync(cachePath)) return readFile(cachePath, 'utf8');
  const text = await curl(url);
  await mkdir(dirname(cachePath), { recursive: true });
  await writeFile(cachePath, text);
  return text;
}

// ---------------------------------------------------------------------------
// 1. Horizons heliocentric ecliptic-J2000 state vectors
// ---------------------------------------------------------------------------

/** Horizons body id -> label */
const HORIZONS_BODIES = {
  199: 'Mercury',
  299: 'Venus',
  399: 'Earth',
  499: 'Mars',
  599: 'Jupiter',
  699: 'Saturn',
  799: 'Uranus',
  899: 'Neptune',
  999: 'Pluto',
};

const HORIZONS_EPOCHS = ['2000-01-01 12:00', '2025-01-01 00:00', '2050-01-01 00:00'];

function horizonsUrl(id, start, center = '500@10') {
  const d = new Date(start.replace(' ', 'T') + 'Z');
  const stop = new Date(d.getTime() + 86400000).toISOString().slice(0, 19).replace('T', ' ');
  const params = new URLSearchParams({
    format: 'text',
    COMMAND: `'${id}'`,
    OBJ_DATA: "'NO'",
    MAKE_EPHEM: "'YES'",
    EPHEM_TYPE: "'VECTORS'",
    CENTER: `'${center}'`,
    START_TIME: `'${start}'`,
    STOP_TIME: `'${stop}'`,
    STEP_SIZE: "'1 d'",
    REF_PLANE: "'ECLIPTIC'",
    REF_SYSTEM: "'J2000'",
    VEC_TABLE: "'1'",
    OUT_UNITS: "'AU-D'",
    CSV_FORMAT: "'YES'",
  });
  return `https://ssd.jpl.nasa.gov/api/horizons.api?${params}`;
}

function parseHorizonsCsv(text) {
  const start = text.indexOf('$$SOE');
  const end = text.indexOf('$$EOE');
  if (start < 0 || end < 0) throw new Error('missing $$SOE/$$EOE block');
  const rows = text
    .slice(start + 5, end)
    .trim()
    .split('\n')
    .map((l) => l.trim())
    .filter(Boolean);
  // Header: JDTDB, Calendar Date (TDB), X, Y, Z
  const headerIdx = rows.findIndex((r) => /^JDTDB/i.test(r));
  const dataRows = headerIdx >= 0 ? rows.slice(headerIdx + 1) : rows;
  return dataRows.map((row) => {
    const c = row.split(',').map((s) => s.trim());
    return { jd: Number(c[0]), date: c[1], x: Number(c[2]), y: Number(c[3]), z: Number(c[4]) };
  });
}

async function fetchHorizons() {
  const result = { source: 'JPL Horizons, center=Sun (500@10), ecliptic J2000, AU', vectors: {} };
  const failures = [];
  for (const [id, name] of Object.entries(HORIZONS_BODIES)) {
    for (const epoch of HORIZONS_EPOCHS) {
      process.stdout.write(`  horizons ${name} @ ${epoch} … `);
      const cachePath = resolve(RAW, `horizons-${id}-${epoch.slice(0, 10)}.txt`);
      try {
        const text = await curlCached(horizonsUrl(id, epoch), cachePath);
        const parsed = parseHorizonsCsv(text);
        if (!parsed.length) throw new Error('no data rows');
        const v = parsed[0];
        (result.vectors[name] ??= []).push(v);
        console.log(`x=${v.x.toFixed(6)} y=${v.y.toFixed(6)} z=${v.z.toFixed(6)}`);
      } catch (e) {
        failures.push(`horizons ${name} @ ${epoch}: ${e.message.split('\n')[0]}`);
        console.log('FAILED (will retry on next run)');
      }
    }
  }

  // Geocentric Moon, used to validate the lunar theory implementation.
  for (const epoch of HORIZONS_EPOCHS) {
    process.stdout.write(`  horizons Moon(geocentric) @ ${epoch} … `);
    const cachePath = resolve(RAW, `horizons-moon-geo-${epoch.slice(0, 10)}.txt`);
    const url = horizonsUrl(301, epoch, '500@399');
    try {
      const parsed = parseHorizonsCsv(await curlCached(url, cachePath));
      const v = parsed[0];
      (result.vectors['Moon (geocentric)'] ??= []).push(v);
      console.log(`x=${v.x.toFixed(6)} y=${v.y.toFixed(6)} z=${v.z.toFixed(6)}`);
    } catch (e) {
      failures.push(`horizons Moon @ ${epoch}: ${e.message.split('\n')[0]}`);
      console.log('FAILED (will retry on next run)');
    }
  }
  await writeFile(resolve(REF, 'horizons-vectors.json'), JSON.stringify(result, null, 2));
  return failures;
}

// ---------------------------------------------------------------------------
// 2. Small-body osculating elements (SBDB)
// ---------------------------------------------------------------------------

const SBDB_BODIES = [
  'Ceres',
  'Pluto',
  'Haumea',
  'Makemake',
  'Eris',
  'Vesta',
  'Pallas',
  'Juno',
  'Hygiea',
  '1P', // 1P/Halley
  '2P', // 2P/Encke
];

async function fetchSbdb() {
  const result = { source: 'JPL SBDB API (ssd-api.jpl.nasa.gov/sbdb.api), osculating elements', objects: {} };
  const failures = [];
  for (const name of SBDB_BODIES) {
    process.stdout.write(`  sbdb ${name} … `);
    const url = `https://ssd-api.jpl.nasa.gov/sbdb.api?sstr=${encodeURIComponent(name)}&full-prec=true`;
    try {
      const text = await curlCached(url, resolve(RAW, `sbdb-${name}.json`));
      const j = JSON.parse(text);
      const el = Object.fromEntries(j.orbit.elements.map((e) => [e.name, e.value]));
      result.objects[name] = {
        fullname: j.object.fullname,
        epoch: j.orbit.epoch,
        a: el.a,
        e: el.e,
        i: el.i,
        om: el.om,
        w: el.w,
        ma: el.ma,
        ...(el.tp ? { tp: el.tp } : {}),
      };
      console.log(`a=${Number(el.a).toFixed(4)} AU  e=${Number(el.e).toFixed(5)}`);
    } catch (e) {
      failures.push(`sbdb ${name}: ${e.message.split('\n')[0]}`);
      console.log('FAILED (will retry on next run)');
    }
  }
  await writeFile(resolve(REF, 'sbdb-elements.json'), JSON.stringify(result, null, 2));
  return failures;
}

// ---------------------------------------------------------------------------

await mkdir(REF, { recursive: true });
if (PROXY) console.log(`Using proxy ${PROXY}`);
console.log('\nHorizons:');
const f1 = await fetchHorizons();
console.log('\nSBDB:');
const f2 = await fetchSbdb();
const failures = [...f1, ...f2];
console.log(`\nWrote reference data to ${REF}`);
if (failures.length) {
  console.log(`${failures.length} request(s) failed — rerun to fill in the cache:`);
  for (const f of failures) console.log(`  - ${f}`);
  process.exitCode = 1;
} else {
  console.log('All reference data is complete.');
}
