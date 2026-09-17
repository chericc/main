/**
 * Camera rig: OrbitControls with a dynamic near plane, animated "fly to body"
 * transitions and a follow mode. True-scale distances span ~14 orders of
 * magnitude, so the zoom speed is tuned for logarithmic travel and the UI
 * additionally exposes an explicit distance slider.
 */
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const UP = new THREE.Vector3(0, 1, 0);

interface Flight {
  fromPosition: THREE.Vector3;
  toPosition: THREE.Vector3;
  fromTarget: THREE.Vector3;
  toTarget: THREE.Vector3;
  elapsed: number;
  duration: number;
}

function easeInOutCubic(t: number): number {
  return t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2;
}

export class CameraController {
  readonly camera: THREE.PerspectiveCamera;
  readonly controls: OrbitControls;

  /** Body id currently followed, if any. */
  following: string | null = null;

  private flight: Flight | null = null;
  private lastTargetPosition = new THREE.Vector3();
  private readonly scratch = new THREE.Vector3();

  constructor(domElement: HTMLElement, aspect: number) {
    this.camera = new THREE.PerspectiveCamera(48, aspect, 1e-8, 3000);
    this.camera.position.set(0, 18, 46);
    this.camera.up.copy(UP);

    this.controls = new OrbitControls(this.camera, domElement);
    this.controls.enableDamping = true;
    this.controls.dampingFactor = 0.08;
    this.controls.rotateSpeed = 0.6;
    this.controls.zoomSpeed = 4.5; // ~1.25x per wheel notch
    this.controls.panSpeed = 0.6;
    this.controls.screenSpacePanning = true;
    this.controls.minDistance = 1e-9;
    this.controls.maxDistance = 900;
    this.controls.target.set(0, 0, 0);
  }

  get distance(): number {
    return this.camera.position.distanceTo(this.controls.target);
  }

  /** Moves the camera to `distance` world units from the current target. */
  setDistance(distance: number): void {
    const dir = this.scratch.copy(this.camera.position).sub(this.controls.target);
    if (dir.lengthSq() < 1e-24) dir.set(0, 0.4, 1);
    dir.normalize();
    this.camera.position.copy(this.controls.target).addScaledVector(dir, distance);
    this.flight = null;
  }

  /**
   * Flies the camera to frame a body.
   * @param id          body id (enables follow mode)
   * @param position    world position of the body
   * @param radius      world radius of the body
   */
  focus(id: string, position: THREE.Vector3, radius: number, instant = false): void {
    this.following = id;
    const toTarget = position.clone();
    const desired = Math.max(radius * 7, radius + 1e-9);

    // Keep the current viewing direction where possible.
    const direction = this.scratch.copy(this.camera.position).sub(this.controls.target);
    if (direction.lengthSq() < 1e-24) direction.set(0.35, 0.32, 1);
    direction.normalize();
    if (direction.y < 0.12) {
      direction.y = 0.12;
      direction.normalize();
    }

    const toPosition = toTarget.clone().addScaledVector(direction, desired);

    if (instant) {
      this.camera.position.copy(toPosition);
      this.controls.target.copy(toTarget);
      this.lastTargetPosition.copy(toTarget);
      this.flight = null;
      return;
    }

    this.flight = {
      fromPosition: this.camera.position.clone(),
      toPosition,
      fromTarget: this.controls.target.clone(),
      toTarget,
      elapsed: 0,
      duration: 1.1,
    };
  }

  /** Pulls back to a full-system view. */
  overview(instant = false): void {
    this.following = null;
    const toPosition = new THREE.Vector3(0, 26, 62);
    const toTarget = new THREE.Vector3(0, 0, 0);

    if (instant) {
      this.camera.position.copy(toPosition);
      this.controls.target.copy(toTarget);
      this.flight = null;
      return;
    }

    this.flight = {
      fromPosition: this.camera.position.clone(),
      toPosition,
      fromTarget: this.controls.target.clone(),
      toTarget,
      elapsed: 0,
      duration: 1.2,
    };
  }

  /** Per-frame update. `targetPosition` is the followed body's world position. */
  update(dt: number, targetPosition: THREE.Vector3 | null, targetRadius: number): void {
    if (this.flight) {
      this.flight.elapsed += dt;
      const t = Math.min(1, this.flight.elapsed / this.flight.duration);
      const k = easeInOutCubic(t);
      this.camera.position.lerpVectors(this.flight.fromPosition, this.flight.toPosition, k);
      this.controls.target.lerpVectors(this.flight.fromTarget, this.flight.toTarget, k);
      if (t >= 1) {
        this.lastTargetPosition.copy(this.flight.toTarget);
        this.flight = null;
      }
    } else if (this.following && targetPosition) {
      // Translate target and camera together so the view is preserved while
      // the body moves along its orbit.
      const delta = this.scratch.copy(targetPosition).sub(this.controls.target);
      this.controls.target.add(delta);
      this.camera.position.add(delta);
      this.lastTargetPosition.copy(targetPosition);
    }

    this.controls.update();

    // Dynamic near plane: keep it just in front of the closest surface we
    // care about, so depth precision stays usable across all scales.
    const dist = this.distance;
    const surfaceGap = Math.max(dist - targetRadius, dist * 0.02);
    const near = THREE.MathUtils.clamp(surfaceGap * 5e-4, 1e-11, 1e-2);
    if (Math.abs(this.camera.near - near) / near > 0.05) {
      this.camera.near = near;
      this.camera.updateProjectionMatrix();
    }
  }

  resize(width: number, height: number): void {
    this.camera.aspect = width / height;
    this.camera.updateProjectionMatrix();
  }
}
