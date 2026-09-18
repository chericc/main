/**
 * Orbit traces. Heliocentric for Sun-orbiting bodies, parent-relative for
 * satellites (the containing group tracks the parent's position).
 */
import * as THREE from 'three';
import type { SolarBody } from '../data';
import type { SolarSystemModel } from '../core/model';
import { eclipticToWorld, type Scaling } from './scaling';

/** Rebuild the trace when the elements have drifted by this many days. */
const REBUILD_INTERVAL_DAYS = 365;

const SEGMENTS: Partial<Record<SolarBody['kind'], number>> = {
  star: 0,
  planet: 720,
  'dwarf-planet': 640,
  asteroid: 512,
  comet: 512,
  moon: 192,
};

export class OrbitLine {
  readonly group = new THREE.Group();
  private readonly body: SolarBody;
  private readonly line: THREE.Line;
  private readonly geometry: THREE.BufferGeometry;
  private readonly material: THREE.LineBasicMaterial;
  private builtJD = Number.NaN;

  constructor(body: SolarBody, model: SolarSystemModel, scaling: Scaling) {
    this.body = body;
    const segments = SEGMENTS[body.kind] ?? 256;

    this.geometry = new THREE.BufferGeometry();
    this.material = new THREE.LineBasicMaterial({
      color: body.color,
      transparent: true,
      opacity: body.kind === 'moon' ? 0.28 : body.kind === 'comet' ? 0.45 : 0.36,
      depthWrite: false,
    });
    this.line = new THREE.Line(this.geometry, this.material);
    this.line.frustumCulled = false;
    this.group.add(this.line);
    this.group.name = `orbit:${body.id}`;

    this.rebuild(model, model.epochJD, segments);
    void scaling;
  }

  private rebuild(model: SolarSystemModel, jd: number, segments: number): void {
    const path = model.orbitPathEcliptic(this.body, segments);
    const count = path.length / 3;
    // The lunar trace is rebuilt every frame, so reuse the buffer when possible.
    const existing = this.geometry.getAttribute('position') as THREE.BufferAttribute | undefined;
    let positions: Float32Array;
    if (existing && existing.count === count) {
      positions = existing.array as Float32Array;
    } else {
      positions = new Float32Array(count * 3);
      this.geometry.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    }
    const tmp = new THREE.Vector3();
    for (let i = 0; i < count; i++) {
      eclipticToWorld({ x: path[i * 3], y: path[i * 3 + 1], z: path[i * 3 + 2] }, tmp);
      positions[i * 3] = tmp.x;
      positions[i * 3 + 1] = tmp.y;
      positions[i * 3 + 2] = tmp.z;
    }
    (this.geometry.getAttribute('position') as THREE.BufferAttribute).needsUpdate = true;
    this.geometry.computeBoundingSphere();
    this.builtJD = jd;
  }

  update(model: SolarSystemModel, jd: number): void {
    const segments = SEGMENTS[this.body.kind] ?? 256;
    // The lunar trace is anchored on the Moon's current position, so it must
    // follow the epoch; every other body traces a static ellipse that only
    // needs rebuilding once its elements have drifted.
    const maxAge = this.body.useLunarTheory ? 0 : REBUILD_INTERVAL_DAYS;
    if (!Number.isFinite(this.builtJD) || Math.abs(jd - this.builtJD) > maxAge) {
      this.rebuild(model, jd, segments);
    }

    if (this.body.parentId) {
      const parent = model.state(this.body.parentId);
      this.group.position.copy(eclipticToWorld(parent.position));
    } else {
      this.group.position.set(0, 0, 0);
    }
  }

  setVisible(visible: boolean): void {
    this.group.visible = visible;
  }

  dispose(): void {
    this.geometry.dispose();
    this.material.dispose();
  }
}
