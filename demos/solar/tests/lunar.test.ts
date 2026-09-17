/**
 * Validates the lunar perturbation theory against JPL Horizons geocentric
 * Moon vectors (reference/horizons-vectors.json, key "Moon (geocentric)").
 *
 * The theory returns the mean equinox of date while Horizons returns J2000,
 * so a systematic error of up to the general precession over the interval
 * (0.014°/year, i.e. ~0.7° in 2050) is expected and accounted for.
 */
import { readFileSync } from 'node:fs';
import { describe, expect, it } from 'vitest';
import { AU_KM } from '../src/core/constants';
import { lunarDistanceKm, lunarLongitudeLatitude, lunarPositionEcliptic } from '../src/core/lunar-theory';
import type { Vec3 } from '../src/core/kepler';

const reference = JSON.parse(readFileSync('reference/horizons-vectors.json', 'utf8')) as {
  vectors: Record<string, Array<{ jd: number; date: string; x: number; y: number; z: number }>>;
};

function angularSeparationDeg(a: Vec3, b: Vec3): number {
  const na = Math.hypot(a.x, a.y, a.z);
  const nb = Math.hypot(b.x, b.y, b.z);
  const dot = (a.x * b.x + a.y * b.y + a.z * b.z) / (na * nb);
  return (Math.acos(Math.min(1, Math.max(-1, dot))) * 180) / Math.PI;
}

describe('lunar theory', () => {
  const geo = reference.vectors['Moon (geocentric)'];
  it('has reference vectors to compare against', () => {
    expect(geo.length).toBeGreaterThanOrEqual(3);
  });

  it('matches the geocentric distance to better than 1%', () => {
    for (const ref of geo) {
      const expectedKm = Math.hypot(ref.x, ref.y, ref.z) * AU_KM;
      const actualKm = lunarDistanceKm(ref.jd);
      const relErr = Math.abs(actualKm - expectedKm) / expectedKm;
      expect(relErr, `${ref.date}: ${actualKm.toFixed(0)} km vs ${expectedKm.toFixed(0)} km`).toBeLessThan(0.01);
    }
  });

  it('matches the geocentric direction to better than 1.5°', () => {
    for (const ref of geo) {
      const actual = lunarPositionEcliptic(ref.jd);
      const sep = angularSeparationDeg(actual, { x: ref.x, y: ref.y, z: ref.z });
      expect(sep, `${ref.date}: angular error ${sep.toFixed(3)}°`).toBeLessThan(1.5);
    }
  });

  it('keeps the Moon between 356 000 and 407 000 km over a full year', () => {
    let min = Infinity;
    let max = -Infinity;
    for (let d = 0; d < 366; d += 0.25) {
      const km = lunarDistanceKm(2_451_545 + d);
      min = Math.min(min, km);
      max = Math.max(max, km);
    }
    expect(min).toBeGreaterThan(355_000);
    expect(max).toBeLessThan(408_000);
    // Perigee/apogee spread is about 42 000 km.
    expect(max - min).toBeGreaterThan(35_000);
  });

  it('reports ecliptic coordinates in range', () => {
    const { lon, lat } = lunarLongitudeLatitude(2_451_545);
    expect(lon).toBeGreaterThanOrEqual(0);
    expect(lon).toBeLessThan(360);
    expect(Math.abs(lat)).toBeLessThan(6);
  });
});
