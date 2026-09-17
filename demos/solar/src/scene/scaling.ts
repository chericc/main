/**
 * World-unit scaling.
 *
 * Solar renders at **true scale**: one three.js world unit is one
 * astronomical unit, and every body radius is its real radius. The only
 * concession is `radiusExaggeration`, an optically labelled aid that is
 * 1 by default (i.e. off) — see the README for why true scale is otherwise
 * unusable for the outer system.
 */
import * as THREE from 'three';
import { AU_KM } from '../core/constants';
import type { Vec3 } from '../core/kepler';

/** World units per astronomical unit. */
export const AU = 1;

/**
 * Ecliptic J2000 (+x equinox, +z north) -> three.js world (Y up).
 * The ecliptic plane becomes the XZ plane and ecliptic north becomes +Y.
 */
export function eclipticToWorld(v: Vec3, target: THREE.Vector3 = new THREE.Vector3()): THREE.Vector3 {
  return target.set(v.x, v.z, -v.y).multiplyScalar(AU);
}

/** Writes an ecliptic vector into an existing three.js vector. */
export function writeEclipticToWorld(v: Vec3, target: THREE.Vector3): THREE.Vector3 {
  return target.set(v.x, v.z, -v.y).multiplyScalar(AU);
}

export class Scaling {
  /**
   * Multiplier applied to every body radius. 1 = true scale (default).
   * Purely a visual aid; orbital distances are never scaled.
   */
  radiusExaggeration = 1;

  /** Radius of a body in world units. */
  radius(radiusKm: number): number {
    return (radiusKm / AU_KM) * this.radiusExaggeration;
  }

  /** Radii for an oblate spheroid: `[equatorial, polar]` in world units. */
  radii(radiusKm: number, flattening = 0): [number, number] {
    const eq = this.radius(radiusKm);
    return [eq, eq * (1 - flattening)];
  }
}
