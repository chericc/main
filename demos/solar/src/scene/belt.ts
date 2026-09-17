/**
 * The main asteroid belt and the Kuiper belt, rendered as GPU points with a
 * constant screen size (so they remain visible at true scale, where a 100 km
 * body is far below one pixel).
 *
 * Members are not real objects but a synthetic population drawn from the real
 * orbital-element distributions of their belt; their positions are still
 * propagated with the same Keplerian solver as the planets.
 */
import * as THREE from 'three';
import { stateAt, type OrbitalElements } from '../core/kepler';
import { eclipticToWorld } from './scaling';

export interface BeltConfig {
  count: number;
  /** Semi-major axis range, AU. */
  aMin: number;
  aMax: number;
  /** Maximum eccentricity. */
  eMax: number;
  /** Maximum inclination, degrees. */
  iMax: number;
  /** Bias of the semi-major-axis distribution (`a^bias`). */
  aBias?: number;
  color: number;
  size: number;
  opacity: number;
  seed: number;
}

function mulberry32(seed: number): () => number {
  let a = seed >>> 0;
  return () => {
    a = (a + 0x6d2b79f5) >>> 0;
    let t = a;
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

export class CelestialBelt {
  readonly points: THREE.Points;
  private readonly elements: OrbitalElements[];
  private readonly geometry: THREE.BufferGeometry;
  private readonly positions: Float32Array;
  private readonly tmp = new THREE.Vector3();

  constructor(config: BeltConfig) {
    const rnd = mulberry32(config.seed);
    const n = config.count;

    this.elements = new Array(n);
    for (let i = 0; i < n; i++) {
      const t = Math.pow(rnd(), config.aBias ?? 1);
      const a = config.aMin + (config.aMax - config.aMin) * t;
      const e = config.eMax * Math.pow(rnd(), 1.7);
      // Inclination grows with |a - a_centre| in the real belts; approximate
      // that with a mild bias toward low inclinations plus a few high ones.
      const iDeg = config.iMax * Math.pow(rnd(), 2.2);
      this.elements[i] = {
        epoch: 2_451_545,
        a,
        e,
        i: iDeg,
        node: rnd() * 360,
        peri: rnd() * 360,
        L: rnd() * 360,
      };
    }

    this.positions = new Float32Array(n * 3);
    this.geometry = new THREE.BufferGeometry();
    this.geometry.setAttribute('position', new THREE.BufferAttribute(this.positions, 3));
    this.geometry.setDrawRange(0, n);

    const material = new THREE.PointsMaterial({
      color: config.color,
      size: config.size,
      sizeAttenuation: false,
      transparent: true,
      opacity: config.opacity,
      depthWrite: false,
    });

    this.points = new THREE.Points(this.geometry, material);
    this.points.frustumCulled = false;
    this.points.name = 'belt';
    this.update(2_451_545);
  }

  update(jd: number): void {
    const p = this.tmp;
    for (let i = 0; i < this.elements.length; i++) {
      const v = stateAt(this.elements[i], jd);
      eclipticToWorld(v, p);
      this.positions[i * 3] = p.x;
      this.positions[i * 3 + 1] = p.y;
      this.positions[i * 3 + 2] = p.z;
    }
    this.geometry.attributes.position.needsUpdate = true;
    this.geometry.computeBoundingSphere();
  }

  setVisible(visible: boolean): void {
    this.points.visible = visible;
  }

  dispose(): void {
    this.geometry.dispose();
    (this.points.material as THREE.Material).dispose();
  }
}

/** The main belt, between the 3:1 and 2:1 Kirkwood gaps. */
export function createAsteroidBelt(): CelestialBelt {
  return new CelestialBelt({
    count: 4_200,
    aMin: 2.06,
    aMax: 3.28,
    eMax: 0.32,
    iMax: 22,
    color: 0xb9ac97,
    size: 1.7,
    opacity: 0.75,
    seed: 0x5eed01,
  });
}

/** The classical Kuiper belt, 30–50 AU. */
export function createKuiperBelt(): CelestialBelt {
  return new CelestialBelt({
    count: 6_000,
    aMin: 30,
    aMax: 50,
    eMax: 0.25,
    iMax: 25,
    aBias: 0.8,
    color: 0x8fa8c8,
    size: 1.4,
    opacity: 0.5,
    seed: 0x5eed02,
  });
}
