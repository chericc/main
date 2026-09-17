/**
 * Geocentric lunar position from a truncated perturbation theory.
 *
 * Implements the method popularised by Paul Schlyter ("How to compute
 * planetary positions"), which is itself a compact form of the classical
 * lunar theory: a Keplerian mean orbit plus the twelve largest periodic
 * perturbations (evection, variation, annual equation, parallactic equation…).
 * Accuracy is of the order of a few arc-minutes in longitude/latitude.
 *
 * Result frame: geocentric ecliptic coordinates in AU, +x towards the equinox,
 * +z towards ecliptic north. The equinox is the mean equinox of date, which
 * differs from J2000 by the general precession (~0.014°/year, i.e. < 0.7°
 * over the 2000–2050 interval spanned by the reference data).
 */
import { AU_KM, DEG, RAD } from './constants';
import { solveEccentricAnomaly, type Vec3 } from './kepler';

/** Earth equatorial radius, km — the unit of the theory's mean distance a. */
const EARTH_RADIUS_KM = 6_378.137;

/** Julian Date of 2000 January 0.0, the epoch of the theory's elements. */
const JD_2000_JAN_0 = 2_451_543.5;

/**
 * Geocentric ecliptic position of the Moon at a Julian Date, in AU.
 */
export function lunarPositionEcliptic(jd: number, out: Vec3 = { x: 0, y: 0, z: 0 }): Vec3 {
  const d = jd - JD_2000_JAN_0;

  // --- Sun (only needed for the perturbation arguments) -------------------
  const sunPeri = 282.9404 + 4.70935e-5 * d; // longitude of perihelion, deg
  const sunM = 356.047 + 0.9856002585 * d; // mean anomaly, deg
  const sunL = sunM + sunPeri; // mean longitude, deg

  // --- Moon's mean elements ----------------------------------------------
  const N = 125.1228 - 0.0529538083 * d; // longitude of ascending node, deg
  const inc = 5.1454; // inclination, deg
  const w = 318.0634 + 0.1643573223 * d; // argument of perigee, deg
  const a = 60.2666; // mean distance, Earth radii
  const e = 0.0549; // eccentricity
  const M = 115.3654 + 13.0649929509 * d; // mean anomaly, deg

  // --- Kepler's equation --------------------------------------------------
  const E = solveEccentricAnomaly(M * DEG, e);
  const cosE = Math.cos(E);
  const sinE = Math.sin(E);
  const xp = a * (cosE - e);
  const yp = a * Math.sqrt(1 - e * e) * sinE;

  const v = Math.atan2(yp, xp); // true anomaly, rad
  let r = Math.hypot(xp, yp); // distance, Earth radii

  // --- Orbit plane -> ecliptic -------------------------------------------
  const vw = v + w * DEG;
  const Nr = N * DEG;
  const ir = inc * DEG;
  let xe = r * (Math.cos(Nr) * Math.cos(vw) - Math.sin(Nr) * Math.sin(vw) * Math.cos(ir));
  let ye = r * (Math.sin(Nr) * Math.cos(vw) + Math.cos(Nr) * Math.sin(vw) * Math.cos(ir));
  let ze = r * Math.sin(vw) * Math.sin(ir);

  // --- Periodic perturbations --------------------------------------------
  const Lm = M + w + N; // Moon's mean longitude, deg
  const D = Lm - sunL; // mean elongation, deg
  const F = Lm - N; // argument of latitude, deg
  const Mm = M; // Moon's mean anomaly, deg
  const Ms = sunM; // Sun's mean anomaly, deg

  const sin = (deg: number) => Math.sin(deg * DEG);
  const cos = (deg: number) => Math.cos(deg * DEG);

  const dLon =
    -1.274 * sin(Mm - 2 * D) + // evection
    0.658 * sin(2 * D) + // variation
    -0.186 * sin(Ms) + // annual equation
    -0.059 * sin(2 * Mm - 2 * D) +
    -0.057 * sin(Mm - 2 * D + Ms) +
    0.053 * sin(Mm + 2 * D) +
    0.046 * sin(2 * D - Ms) +
    0.041 * sin(Mm - Ms) +
    -0.035 * sin(D) + // parallactic equation
    -0.031 * sin(Mm + Ms) +
    -0.015 * sin(2 * F - 2 * D) +
    0.011 * sin(Mm - 4 * D);

  const dLat =
    -0.173 * sin(F - 2 * D) +
    -0.055 * sin(Mm - F - 2 * D) +
    -0.046 * sin(Mm + F - 2 * D) +
    0.033 * sin(F + 2 * D) +
    0.017 * sin(2 * Mm + F);

  const dR = -0.58 * cos(Mm - 2 * D) + -0.46 * cos(2 * D);

  let lon = Math.atan2(ye, xe) + dLon * DEG;
  let lat = Math.atan2(ze, Math.hypot(xe, ye)) + dLat * DEG;
  r += dR;

  // --- Back to rectangular, then to AU ------------------------------------
  const cosLat = Math.cos(lat);
  const scale = (EARTH_RADIUS_KM * r) / AU_KM;
  out.x = scale * Math.cos(lon) * cosLat;
  out.y = scale * Math.sin(lon) * cosLat;
  out.z = scale * Math.sin(lat);
  return out;
}

/** Geocentric distance of the Moon in kilometres at a Julian Date. */
export function lunarDistanceKm(jd: number): number {
  const p = lunarPositionEcliptic(jd);
  return Math.hypot(p.x, p.y, p.z) * AU_KM;
}

/** Geocentric ecliptic longitude and latitude of the Moon, in degrees. */
export function lunarLongitudeLatitude(jd: number): { lon: number; lat: number } {
  const p = lunarPositionEcliptic(jd);
  const r = Math.hypot(p.x, p.y, p.z);
  const lon = Math.atan2(p.y, p.x) * RAD;
  const lat = Math.asin(r === 0 ? 0 : p.z / r) * RAD;
  return { lon: (lon + 360) % 360, lat };
}
