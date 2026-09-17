/**
 * Keplerian orbit propagation.
 *
 * The model is the one described by E. M. Standish & J. G. Williams (1992),
 * "Orbital Ephemerides of the Sun, Moon and Planets", used by JPL's
 * *Approximate Positions of the Planets*:
 *
 *   https://ssd.jpl.nasa.gov/planets/approx_pos.html
 *
 * Six Keplerian elements are defined at a reference epoch. Either
 *   (a) they carry secular rates (per Julian century), as for the eight
 *       planets, or
 *   (b) they are osculating elements at a given epoch that are propagated
 *       with a constant mean motion (Kepler's third law, or an explicit
 *       sidereal period for satellites).
 *
 * Positions are returned as heliocentric (or parent-centric) rectangular
 * coordinates in the **mean ecliptic and equinox of J2000** frame, in AU:
 *   +x towards the vernal equinox, +z towards the ecliptic north pole.
 */
import { DAYS_PER_CENTURY, DEG, MEAN_MOTION_DEG, RAD } from './constants';

const TAU = Math.PI * 2;

/** Secular rates of the elements, per Julian century. */
export interface SecularRates {
  /** Semi-major axis rate, AU/century. */
  a?: number;
  /** Eccentricity rate, per century. */
  e?: number;
  /** Inclination rate, deg/century. */
  i?: number;
  /** Mean-longitude rate, deg/century. */
  L?: number;
  /** Longitude-of-perihelion rate, deg/century. */
  peri?: number;
  /** Longitude-of-ascending-node rate, deg/century. */
  node?: number;
}

/** Six Keplerian elements plus propagation metadata. */
export interface OrbitalElements {
  /** Reference epoch as a Julian Date. */
  epoch: number;
  /** Semi-major axis, AU. */
  a: number;
  /** Eccentricity. */
  e: number;
  /** Inclination to the reference plane, degrees. */
  i: number;
  /** Mean longitude at epoch, degrees. */
  L: number;
  /** Longitude of perihelion (ϖ = ω + Ω), degrees. */
  peri: number;
  /** Longitude of the ascending node (Ω), degrees. */
  node: number;
  /** Optional secular rates; when present the mean motion is implicit. */
  rates?: SecularRates;
  /** Sidereal period in days; overrides Kepler's third law when present. */
  period?: number;
}

/** Rectangular coordinates in AU (ecliptic J2000 frame). */
export interface Vec3 {
  x: number;
  y: number;
  z: number;
}

/** The elements evaluated for a specific instant. */
export interface InstantaneousElements {
  a: number;
  e: number;
  i: number;
  L: number;
  peri: number;
  node: number;
}

/** Mean motion in degrees per day. */
export function meanMotionDegPerDay(a: number, period?: number): number {
  if (period !== undefined && period !== 0) return 360 / period;
  return MEAN_MOTION_DEG / Math.pow(a, 1.5);
}

/** Orbital (sidereal) period in days, from the semi-major axis or `period`. */
export function orbitalPeriodDays(el: OrbitalElements): number {
  if (el.period !== undefined && el.period !== 0) return Math.abs(el.period);
  // Kepler's third law around a 1 M_sun primary: P[years] = a^1.5
  return Math.pow(el.a, 1.5) * 365.25;
}

/** Evaluates the six elements at a Julian Date. */
export function elementsAt(el: OrbitalElements, jd: number): InstantaneousElements {
  const dt = jd - el.epoch;
  if (el.rates) {
    const T = dt / DAYS_PER_CENTURY;
    const r = el.rates;
    return {
      a: el.a + (r.a ?? 0) * T,
      e: el.e + (r.e ?? 0) * T,
      i: el.i + (r.i ?? 0) * T,
      L: el.L + (r.L ?? 0) * T,
      peri: el.peri + (r.peri ?? 0) * T,
      node: el.node + (r.node ?? 0) * T,
    };
  }
  const n = meanMotionDegPerDay(el.a, el.period);
  return { a: el.a, e: el.e, i: el.i, L: el.L + n * dt, peri: el.peri, node: el.node };
}

/**
 * Solves Kepler's equation `M = E - e·sin E` for the eccentric anomaly.
 * Newton–Raphson with a guaranteed bisection fallback, so it also converges
 * for the high-eccentricity comets (e up to ~0.97).
 */
export function solveEccentricAnomaly(M: number, e: number, tol = 1e-12): number {
  // Reduce the mean anomaly to [-π, π].
  let m = M % TAU;
  if (m > Math.PI) m -= TAU;
  else if (m < -Math.PI) m += TAU;

  if (e < 1e-10) return m;

  // Newton–Raphson from the Danby start value.
  let E = m + e * Math.sin(m) * (1 + e * Math.cos(m));
  for (let iter = 0; iter < 40; iter++) {
    const sinE = Math.sin(E);
    const cosE = Math.cos(E);
    const f = E - e * sinE - m;
    const fp = 1 - e * cosE;
    if (fp === 0) break;
    const step = f / fp;
    E -= step;
    if (Math.abs(step) < tol) return E;
  }

  // Bisection: E must lie in [m - e, m + e] because |E - M| = e·|sin E| ≤ e.
  let lo = m - e;
  let hi = m + e;
  for (let iter = 0; iter < 200 && hi - lo > tol; iter++) {
    const mid = 0.5 * (lo + hi);
    if (mid - e * Math.sin(mid) - m > 0) hi = mid;
    else lo = mid;
  }
  return 0.5 * (lo + hi);
}

/** True anomaly (radians) from the eccentric anomaly. */
export function trueAnomalyFromEccentric(E: number, e: number): number {
  return 2 * Math.atan2(Math.sqrt(1 + e) * Math.sin(E / 2), Math.sqrt(1 - e) * Math.cos(E / 2));
}

/**
 * Rotates perifocal coordinates into the reference (ecliptic J2000) frame.
 * Implements the matrix of Standish & Williams, step 5.
 */
function perifocalToEcliptic(
  xp: number,
  yp: number,
  nodeRad: number,
  argPeriRad: number,
  incRad: number,
  out: Vec3,
): Vec3 {
  const cosW = Math.cos(argPeriRad);
  const sinW = Math.sin(argPeriRad);
  const cosO = Math.cos(nodeRad);
  const sinO = Math.sin(nodeRad);
  const cosI = Math.cos(incRad);
  const sinI = Math.sin(incRad);

  out.x = (cosW * cosO - sinW * sinO * cosI) * xp + (-sinW * cosO - cosW * sinO * cosI) * yp;
  out.y = (cosW * sinO + sinW * cosO * cosI) * xp + (-sinW * sinO + cosW * cosO * cosI) * yp;
  out.z = sinW * sinI * xp + cosW * sinI * yp;
  return out;
}

/** Heliocentric ecliptic position (AU) at a Julian Date. */
export function stateAt(el: OrbitalElements, jd: number, out: Vec3 = { x: 0, y: 0, z: 0 }): Vec3 {
  const k = elementsAt(el, jd);
  const e = k.e;
  const argPeri = (k.peri - k.node) * DEG;
  const node = k.node * DEG;
  const inc = k.i * DEG;
  const M = (k.L - k.peri) * DEG;
  const E = solveEccentricAnomaly(M, e);
  const xp = k.a * (Math.cos(E) - e);
  const yp = k.a * Math.sqrt(Math.max(0, 1 - e * e)) * Math.sin(E);
  return perifocalToEcliptic(xp, yp, node, argPeri, inc, out);
}

/**
 * Samples a full orbit ellipse at the elements valid for `jd`.
 * Returns a flat `[x0,y0,z0, x1,y1,z1, …]` array that ends on the first point.
 */
export function orbitPath(el: OrbitalElements, jd: number, segments = 512): Float64Array {
  const k = elementsAt(el, jd);
  const e = k.e;
  const argPeri = (k.peri - k.node) * DEG;
  const node = k.node * DEG;
  const inc = k.i * DEG;
  const b = k.a * Math.sqrt(Math.max(0, 1 - e * e));

  const n = Math.max(16, Math.floor(segments));
  const out = new Float64Array((n + 1) * 3);
  const tmp: Vec3 = { x: 0, y: 0, z: 0 };
  for (let idx = 0; idx <= n; idx++) {
    const E = (idx / n) * TAU;
    perifocalToEcliptic(k.a * (Math.cos(E) - e), b * Math.sin(E), node, argPeri, inc, tmp);
    out[idx * 3] = tmp.x;
    out[idx * 3 + 1] = tmp.y;
    out[idx * 3 + 2] = tmp.z;
  }
  return out;
}

/** Distance from the primary, AU. */
export function distanceAU(el: OrbitalElements, jd: number): number {
  const p = stateAt(el, jd);
  return Math.hypot(p.x, p.y, p.z);
}

/** Heliocentric ecliptic longitude and latitude (degrees) at a Julian Date. */
export function eclipticLongitudeLatitude(el: OrbitalElements, jd: number): { lon: number; lat: number } {
  const p = stateAt(el, jd);
  const r = Math.hypot(p.x, p.y, p.z);
  const lon = Math.atan2(p.y, p.x) * RAD;
  const lat = Math.asin(r === 0 ? 0 : p.z / r) * RAD;
  return { lon: (lon + 360) % 360, lat };
}
