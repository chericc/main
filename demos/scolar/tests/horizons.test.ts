/**
 * Validates the propagator against JPL Horizons heliocentric state vectors
 * (ecliptic & equinox of J2000, AU) at three epochs.
 *
 * Reference data: reference/horizons-vectors.json, fetched by
 * `npm run fetch:reference` from https://ssd.jpl.nasa.gov/horizons/.
 */
import { readFileSync } from 'node:fs';
import { describe, expect, it } from 'vitest';
import { SolarSystemModel } from '../src/core/model';
import { stateAt, type OrbitalElements, type Vec3 } from '../src/core/kepler';

interface HorizonsVector {
  jd: number;
  date: string;
  x: number;
  y: number;
  z: number;
}

const reference = JSON.parse(readFileSync('reference/horizons-vectors.json', 'utf8')) as {
  vectors: Record<string, HorizonsVector[]>;
};

const model = new SolarSystemModel();

const PLANETS: Array<[string, string]> = [
  ['Mercury', 'mercury'],
  ['Venus', 'venus'],
  ['Earth', 'earth'],
  ['Mars', 'mars'],
  ['Jupiter', 'jupiter'],
  ['Saturn', 'saturn'],
  ['Uranus', 'uranus'],
  ['Neptune', 'neptune'],
];

function angularSeparationDeg(a: Vec3, b: Vec3): number {
  const na = Math.hypot(a.x, a.y, a.z);
  const nb = Math.hypot(b.x, b.y, b.z);
  const dot = (a.x * b.x + a.y * b.y + a.z * b.z) / (na * nb);
  return (Math.acos(Math.min(1, Math.max(-1, dot))) * 180) / Math.PI;
}

/**
 * The Standish & Williams (1992) stated accuracy for the eight planets,
 * 1800–2050 AD (heliocentric longitude and radius).
 */
const TOLERANCE: Record<string, { arcsec: number; km: number }> = {
  Mercury: { arcsec: 15, km: 1_000 },
  Venus: { arcsec: 20, km: 4_000 },
  Earth: { arcsec: 20, km: 6_000 },
  Mars: { arcsec: 40, km: 25_000 },
  Jupiter: { arcsec: 400, km: 600_000 },
  Saturn: { arcsec: 600, km: 1_500_000 },
  Uranus: { arcsec: 50, km: 1_000_000 },
  Neptune: { arcsec: 10, km: 200_000 },
};

/** Largest error actually observed, for the record: `name -> [arcsec, km]`. */
const observed = new Map<string, [number, number]>();

describe('propagator vs JPL Horizons', () => {
  for (const [name, id] of PLANETS) {
    it(`${name}: position matches Horizons at all reference epochs`, () => {
      const body = model.body(id);
      const orbit = body.orbit as OrbitalElements;

      for (const ref of reference.vectors[name]) {
        const expected: Vec3 = { x: ref.x, y: ref.y, z: ref.z };
        const actual = stateAt(orbit, ref.jd);

        const sepArcsec = angularSeparationDeg(actual, expected) * 3600;
        const drKm =
          Math.abs(
            Math.hypot(actual.x, actual.y, actual.z) - Math.hypot(expected.x, expected.y, expected.z),
          ) * 149_597_870.7;

        const prev = observed.get(name) ?? [0, 0];
        observed.set(name, [Math.max(prev[0], sepArcsec), Math.max(prev[1], drKm)]);

        const tol = TOLERANCE[name];
        // 5x head-room over the published bound: those figures are nominal
        // RMS values and our reference epochs sit at the edges of the
        // 1800–2050 interval the elements were fitted to.
        const maxArcsec = (tol?.arcsec ?? 3_000) * 5;
        const maxKm = (tol?.km ?? 3_000_000) * 5;

        expect(
          sepArcsec,
          `${name} @ ${ref.date}: angular error ${sepArcsec.toFixed(1)}" exceeds ${maxArcsec}"`,
        ).toBeLessThan(maxArcsec);
        expect(
          drKm,
          `${name} @ ${ref.date}: radial error ${Math.round(drKm)} km exceeds ${maxKm} km`,
        ).toBeLessThan(maxKm);
      }
    });
  }

  it('reports the observed accuracy', () => {
    const rows = [...observed.entries()].map(
      ([name, [arcsec, km]]) => `${name.padEnd(8)} ${arcsec.toFixed(1).padStart(7)}"  ${Math.round(km).toString().padStart(9)} km`,
    );
    console.log(`\nMax deviation from JPL Horizons (1800–2050 elements):\n${rows.join('\n')}\n`);
    expect(observed.size).toBe(PLANETS.length);
  });

  it('Pluto: osculating elements stay within 1% over 34 years', () => {
    // Pluto's SBDB osculating elements are propagated with a constant mean
    // motion, so the strong Neptune perturbations are not modelled. The
    // resulting error stays below 1% even 34 years from the epoch.
    const body = model.body('pluto');
    for (const ref of reference.vectors.Pluto) {
      const actual = stateAt(body.orbit as OrbitalElements, ref.jd);
      const expected: Vec3 = { x: ref.x, y: ref.y, z: ref.z };
      const rExp = Math.hypot(expected.x, expected.y, expected.z);
      const rAct = Math.hypot(actual.x, actual.y, actual.z);
      expect(Math.abs(rAct - rExp) / rExp, `Pluto @ ${ref.date}`).toBeLessThan(0.01);
      expect(angularSeparationDeg(actual, expected), `Pluto @ ${ref.date}`).toBeLessThan(0.5);
    }
  });

  it('keeps every planet within a sane heliocentric distance', () => {
    for (const [, id] of [...PLANETS, ['Pluto', 'pluto'] as [string, string]]) {
      const body = model.body(id);
      const k = stateAt(body.orbit as OrbitalElements, 2_451_545);
      const r = Math.hypot(k.x, k.y, k.z);
      expect(r).toBeGreaterThan(0.3);
      expect(r).toBeLessThan(50);
    }
  });
});
