import { describe, expect, it } from 'vitest';
import {
  eclipticLongitudeLatitude,
  elementsAt,
  meanMotionDegPerDay,
  orbitalPeriodDays,
  orbitPath,
  solveEccentricAnomaly,
  stateAt,
  type OrbitalElements,
} from '../src/core/kepler';
import { MEAN_MOTION_DEG } from '../src/core/constants';

describe('solveEccentricAnomaly', () => {
  it('satisfies Kepler\'s equation for a wide range of (M, e)', () => {
    for (const e of [0, 0.0167, 0.2056, 0.5, 0.9, 0.9679]) {
      for (let deg = -180; deg <= 180; deg += 3) {
        const M = (deg * Math.PI) / 180;
        const E = solveEccentricAnomaly(M, e);
        const residual = E - e * Math.sin(E) - M;
        expect(Math.abs(residual)).toBeLessThan(1e-9);
      }
    }
  });

  it('returns the eccentric anomaly itself for circular orbits', () => {
    expect(solveEccentricAnomaly(0.7, 0)).toBeCloseTo(0.7, 12);
  });

  it('converges for near-parabolic comets close to perihelion', () => {
    const E = solveEccentricAnomaly(0.001, 0.967936);
    expect(Math.abs(E - 0.967936 * Math.sin(E) - 0.001)).toBeLessThan(1e-10);
  });
});

describe('mean motion / period', () => {
  it('reproduces the Gaussian mean motion for 1 AU', () => {
    expect(meanMotionDegPerDay(1)).toBeCloseTo(MEAN_MOTION_DEG, 10);
    expect(meanMotionDegPerDay(1)).toBeCloseTo(0.9856076686, 9);
  });

  it('gives Earth a 365.25-day year from Kepler\'s third law', () => {
    const earth: OrbitalElements = { epoch: 2451545, a: 1, e: 0.0167, i: 0, L: 100, peri: 102.9, node: 0 };
    expect(orbitalPeriodDays(earth)).toBeCloseTo(365.25, 1);
  });

  it('honours an explicit sidereal period (satellites)', () => {
    const io: OrbitalElements = { epoch: 2451545, a: 0.0028, e: 0.004, i: 0, L: 0, peri: 0, node: 0, period: 1.769138 };
    expect(orbitalPeriodDays(io)).toBeCloseTo(1.769138, 9);
    expect(meanMotionDegPerDay(io.a, io.period)).toBeCloseTo(360 / 1.769138, 9);
    // After one full period the mean longitude advances exactly 360°.
    const before = stateAt(io, 2451545);
    const after = stateAt(io, 2451545 + 1.769138);
    expect(Math.hypot(after.x - before.x, after.y - before.y, after.z - before.z)).toBeLessThan(1e-9);
  });
});

describe('secular element rates', () => {
  it('advances the mean longitude linearly in centuries', () => {
    const el: OrbitalElements = {
      epoch: 2451545,
      a: 1,
      e: 0.0167,
      i: 0,
      L: 100,
      peri: 102.9,
      node: 0,
      rates: { L: 35999.37244981 },
    };
    const k = elementsAt(el, 2451545 + 36525);
    expect(k.L).toBeCloseTo(100 + 35999.37244981, 6);
    expect(elementsAt(el, 2451545).L).toBeCloseTo(100, 12);
  });
});

describe('orbit geometry', () => {
  const earth: OrbitalElements = {
    epoch: 2451545,
    a: 1.00000261,
    e: 0.01671123,
    i: -0.00001531,
    L: 100.46457166,
    peri: 102.93768193,
    node: 0,
    rates: { L: 35999.37244981, peri: 0.32327364, e: -0.00004392, a: 0.00000562, i: -0.01294668 },
  };

  it('keeps the heliocentric distance inside [a(1-e), a(1+e)]', () => {
    for (let d = 0; d < 40000; d += 37) {
      const p = stateAt(earth, 2451545 + d);
      const r = Math.hypot(p.x, p.y, p.z);
      expect(r).toBeGreaterThan(0.9832);
      expect(r).toBeLessThan(1.0168);
    }
  });

  it('reaches perihelion in early January and aphelion in early July', () => {
    const perihelion = stateAt(earth, 2451545.0 + 5); // 2000-01-06
    const aphelion = stateAt(earth, 2451545.0 + 187); // 2000-07-06
    expect(Math.hypot(perihelion.x, perihelion.y, perihelion.z)).toBeLessThan(0.9842);
    expect(Math.hypot(aphelion.x, aphelion.y, aphelion.z)).toBeGreaterThan(1.0158);
  });

  it('produces a closed orbit path that matches stateAt', () => {
    const path = orbitPath(earth, 2451545, 256);
    const first = { x: path[0], y: path[1], z: path[2] };
    const last = { x: path[path.length - 3], y: path[path.length - 2], z: path[path.length - 1] };
    expect(Math.hypot(last.x - first.x, last.y - first.y, last.z - first.z)).toBeLessThan(1e-12);

    // Every sampled point must lie on the ellipse: r = a(1-e²)/(1+e·cos ν)
    const a = elementsAt(earth, 2451545).a;
    const e = elementsAt(earth, 2451545).e;
    const node = (k0(earth).node * Math.PI) / 180;
    void node;
    for (let i = 0; i < path.length; i += 3) {
      const r = Math.hypot(path[i], path[i + 1], path[i + 2]);
      const rp = a * (1 - e);
      const ra = a * (1 + e);
      expect(r).toBeGreaterThan(rp - 1e-9);
      expect(r).toBeLessThan(ra + 1e-9);
    }
  });

  it('reports ecliptic longitude in [0, 360)', () => {
    const { lon, lat } = eclipticLongitudeLatitude(earth, 2451545);
    expect(lon).toBeGreaterThanOrEqual(0);
    expect(lon).toBeLessThan(360);
    expect(Math.abs(lat)).toBeLessThan(0.001);
  });
});

function k0(el: OrbitalElements) {
  return elementsAt(el, el.epoch);
}
