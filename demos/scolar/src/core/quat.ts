/**
 * Minimal quaternion helpers (plain objects, no dependency on three.js) so
 * that the orbital model stays framework-free and easy to unit-test.
 *
 * The reference frame is the mean ecliptic and equinox of J2000:
 *   +x towards the vernal equinox, +y in the ecliptic plane,
 *   +z towards the ecliptic north pole.
 */
import type { Vec3 } from './kepler';

export interface Quat {
  x: number;
  y: number;
  z: number;
  w: number;
}

export const IDENTITY: Quat = { x: 0, y: 0, z: 0, w: 1 };

/** Rotation of `angle` radians about a unit axis. */
export function quatFromAxisAngle(axis: Vec3, angle: number): Quat {
  const h = angle / 2;
  const s = Math.sin(h);
  return { x: axis.x * s, y: axis.y * s, z: axis.z * s, w: Math.cos(h) };
}

/**
 * Shortest-arc rotation taking unit vector `from` to unit vector `to`.
 * Handles the antiparallel case by picking an arbitrary perpendicular axis.
 */
export function quatFromUnitVectors(from: Vec3, to: Vec3): Quat {
  const dot = from.x * to.x + from.y * to.y + from.z * to.z;
  if (dot > 1 - 1e-12) return { ...IDENTITY };
  if (dot < -1 + 1e-12) {
    // 180°: rotate about any axis perpendicular to `from`.
    let ax = Math.abs(from.x) < 0.9 ? { x: 1, y: 0, z: 0 } : { x: 0, y: 1, z: 0 };
    const perp = normalize(cross(from, ax));
    return quatFromAxisAngle(perp, Math.PI);
  }
  const c = cross(from, to);
  return normalizeQuat({ x: c.x, y: c.y, z: c.z, w: 1 + dot });
}

/** Hamilton product `a ∘ b` (apply `b` first, then `a`). */
export function quatMultiply(a: Quat, b: Quat): Quat {
  return {
    x: a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
    y: a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    z: a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
    w: a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
  };
}

/** Rotates a vector by a quaternion. */
export function quatRotate(q: Quat, v: Vec3): Vec3 {
  // t = 2 * (q_vec × v); v' = v + q_w * t + q_vec × t
  const tx = 2 * (q.y * v.z - q.z * v.y);
  const ty = 2 * (q.z * v.x - q.x * v.z);
  const tz = 2 * (q.x * v.y - q.y * v.x);
  return {
    x: v.x + q.w * tx + (q.y * tz - q.z * ty),
    y: v.y + q.w * ty + (q.z * tx - q.x * tz),
    z: v.z + q.w * tz + (q.x * ty - q.y * tx),
  };
}

/** Unit vector from spherical equatorial coordinates (IAU pole convention). */
export function fromEquatorialRaDec(raDeg: number, decDeg: number): Vec3 {
  const ra = (raDeg * Math.PI) / 180;
  const dec = (decDeg * Math.PI) / 180;
  return { x: Math.cos(dec) * Math.cos(ra), y: Math.cos(dec) * Math.sin(ra), z: Math.sin(dec) };
}

/** Rotates a vector about the x-axis (right-handed). */
export function rotateX(v: Vec3, angle: number): Vec3 {
  const c = Math.cos(angle);
  const s = Math.sin(angle);
  return { x: v.x, y: c * v.y - s * v.z, z: s * v.y + c * v.z };
}

export function cross(a: Vec3, b: Vec3): Vec3 {
  return { x: a.y * b.z - a.z * b.y, y: a.z * b.x - a.x * b.z, z: a.x * b.y - a.y * b.x };
}

export function normalize(v: Vec3): Vec3 {
  const n = Math.hypot(v.x, v.y, v.z) || 1;
  return { x: v.x / n, y: v.y / n, z: v.z / n };
}

function normalizeQuat(q: Quat): Quat {
  const n = Math.hypot(q.x, q.y, q.z, q.w) || 1;
  return { x: q.x / n, y: q.y / n, z: q.z / n, w: q.w / n };
}
