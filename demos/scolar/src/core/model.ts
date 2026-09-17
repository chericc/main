/**
 * The solar-system model: evaluates every body's position and orientation for
 * a given Julian Date, entirely in the mean ecliptic and equinox of J2000
 * frame (AU). The rendering layer is responsible for converting to three.js
 * world coordinates — keeping this module free of graphics dependencies makes
 * it directly unit-testable.
 */
import { ALL_BODIES, type SolarBody } from '../data';
import { AU_KM, DEG, J2000, OBLIQUITY_J2000_DEG, SECONDS_PER_DAY } from './constants';
import { orbitPath, stateAt, type Vec3 } from './kepler';
import { lunarPositionEcliptic } from './lunar-theory';
import {
  fromEquatorialRaDec,
  quatFromAxisAngle,
  quatFromUnitVectors,
  quatRotate,
  rotateX,
  type Quat,
} from './quat';

const TAU = Math.PI * 2;
const ECLIPTIC_NORTH: Vec3 = { x: 0, y: 0, z: 1 };
const ECLIPTIC_X: Vec3 = { x: 1, y: 0, z: 0 };
const OBLIQUITY_RAD = OBLIQUITY_J2000_DEG * DEG;

/** Position and orientation of one body at the model's current epoch. */
export interface BodyState {
  body: SolarBody;
  /** Heliocentric ecliptic position, AU. */
  position: Vec3;
  /** Position relative to the parent body (satellites), AU. */
  local: Vec3;
  /** Unit vector along the body's north pole, in ecliptic coordinates. */
  pole: Vec3;
  /** Rotation angle about the pole, radians. */
  spinRad: number;
}

/** Equatorial (ICRF) pole direction -> ecliptic J2000 unit vector. */
export function poleToEcliptic(raDeg: number, decDeg: number): Vec3 {
  return rotateX(fromEquatorialRaDec(raDeg, decDeg), -OBLIQUITY_RAD);
}

/**
 * Rotation taking the ecliptic frame to the body's equatorial frame.
 * Uses the real IAU pole when available, otherwise a tilt of `obliquityDeg`
 * about the equinox axis.
 */
export function tiltQuaternion(body: SolarBody): Quat {
  const pole = body.physical.pole;
  if (pole) return quatFromUnitVectors(ECLIPTIC_NORTH, poleToEcliptic(pole.raDeg, pole.decDeg));
  return quatFromAxisAngle(ECLIPTIC_X, -body.physical.obliquityDeg * DEG);
}

function add(a: Vec3, b: Vec3): Vec3 {
  return { x: a.x + b.x, y: a.y + b.y, z: a.z + b.z };
}

export class SolarSystemModel {
  readonly bodies: readonly SolarBody[] = ALL_BODIES;

  private readonly index = new Map<string, SolarBody>();
  private readonly tilts = new Map<string, Quat>();
  private readonly states = new Map<string, BodyState>();
  private jd = J2000;

  constructor() {
    for (const body of this.bodies) {
      this.index.set(body.id, body);
      this.tilts.set(body.id, tiltQuaternion(body));
      this.states.set(body.id, {
        body,
        position: { x: 0, y: 0, z: 0 },
        local: { x: 0, y: 0, z: 0 },
        pole: ECLIPTIC_NORTH,
        spinRad: 0,
      });
    }
    this.update(J2000);
  }

  /** Current epoch of the model. */
  get epochJD(): number {
    return this.jd;
  }

  body(id: string): SolarBody {
    const b = this.index.get(id);
    if (!b) throw new Error(`Unknown body: ${id}`);
    return b;
  }

  state(id: string): BodyState {
    const s = this.states.get(id);
    if (!s) throw new Error(`Unknown body: ${id}`);
    return s;
  }

  tilt(id: string): Quat {
    return this.tilts.get(id) ?? tiltQuaternion(this.body(id));
  }

  /** Recomputes every body's position/orientation for `jd`. */
  update(jd: number): void {
    this.jd = jd;
    // `ALL_BODIES` is ordered parents-first, so a single pass suffices.
    for (const body of this.bodies) {
      const state = this.states.get(body.id)!;
      state.pole = quatRotate(this.tilts.get(body.id)!, ECLIPTIC_NORTH);
      state.spinRad = this.spinAngle(body, jd);

      if (!body.orbit) {
        state.position = { x: 0, y: 0, z: 0 };
        state.local = { x: 0, y: 0, z: 0 };
        continue;
      }

      if (body.parentId) {
        const parent = this.states.get(body.parentId)!;
        const local = this.localPosition(body, jd);
        state.local = local;
        state.position = add(parent.position, local);
      } else {
        const position = stateAt(body.orbit, jd);
        state.local = position;
        state.position = position;
      }
    }
  }

  /** Position of a satellite in ecliptic coordinates relative to its parent. */
  private localPosition(body: SolarBody, jd: number): Vec3 {
    if (body.useLunarTheory) return lunarPositionEcliptic(jd);
    const local = stateAt(body.orbit!, jd);
    if (body.frame === 'ecliptic' || !body.parentId) return local;
    // The mean elements are expressed in the parent's equatorial (or Laplace)
    // plane: rotate them into the ecliptic with the parent's tilt.
    return quatRotate(this.tilt(body.parentId), local);
  }

  private spinAngle(body: SolarBody, jd: number): number {
    const periodDays = body.physical.rotationPeriodHours / 24;
    if (!periodDays) return 0;
    const angle = (TAU * (jd - J2000)) / periodDays;
    return angle % TAU;
  }

  /**
   * Samples the body's orbit for drawing.
   * Heliocentric for Sun-orbiting bodies, parent-relative for satellites.
   * Returns a flat `[x,y,z, …]` array of ecliptic coordinates in AU.
   */
  orbitPathEcliptic(body: SolarBody, segments = 512): Float64Array {
    if (body.useLunarTheory) {
      // Trace one sidereal month of the perturbed geocentric path.
      const period = body.orbit?.period ?? 27.322;
      const n = Math.max(16, Math.floor(segments));
      const out = new Float64Array((n + 1) * 3);
      for (let i = 0; i <= n; i++) {
        const p = lunarPositionEcliptic(this.jd + (period * i) / n);
        out[i * 3] = p.x;
        out[i * 3 + 1] = p.y;
        out[i * 3 + 2] = p.z;
      }
      return out;
    }

    if (!body.orbit) return new Float64Array(0);

    if (!body.parentId || body.frame === 'ecliptic') {
      return orbitPath(body.orbit, this.jd, segments);
    }

    const local = orbitPath(body.orbit, this.jd, segments);
    const q = this.tilt(body.parentId);
    const out = new Float64Array(local.length);
    const v: Vec3 = { x: 0, y: 0, z: 0 };
    for (let i = 0; i < local.length; i += 3) {
      v.x = local[i];
      v.y = local[i + 1];
      v.z = local[i + 2];
      const r = quatRotate(q, v);
      out[i] = r.x;
      out[i + 1] = r.y;
      out[i + 2] = r.z;
    }
    return out;
  }

  /** Heliocentric distance of a body, AU. */
  distanceFromSun(id: string): number {
    const p = this.state(id).position;
    return Math.hypot(p.x, p.y, p.z);
  }

  /**
   * Position of a body relative to its parent (or the Sun) at an arbitrary
   * Julian Date, without touching the cached state.
   */
  localPositionAt(body: SolarBody, jd: number): Vec3 {
    if (!body.orbit) return { x: 0, y: 0, z: 0 };
    if (body.parentId) return this.localPosition(body, jd);
    return stateAt(body.orbit, jd);
  }

  /**
   * Orbital speed in km/s, from a centred finite difference of the position.
   * Heliocentric for Sun-orbiting bodies, parent-relative for satellites.
   */
  velocityKms(body: SolarBody, jd: number, h = 0.02): number {
    if (!body.orbit) return 0;
    const a = this.localPositionAt(body, jd - h);
    const b = this.localPositionAt(body, jd + h);
    const d = Math.hypot(b.x - a.x, b.y - a.y, b.z - a.z);
    return ((d / (2 * h)) * AU_KM) / SECONDS_PER_DAY;
  }

  /** Distance between two bodies, AU. */
  distanceBetween(a: string, b: string): number {
    const pa = this.state(a).position;
    const pb = this.state(b).position;
    return Math.hypot(pa.x - pb.x, pa.y - pb.y, pa.z - pb.z);
  }
}
