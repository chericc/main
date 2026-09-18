/**
 * Scene assembly: renderer, lights, star background, body views, orbit traces,
 * belts, labels and screen-space picking.
 */
import * as THREE from 'three';
import { CSS2DRenderer } from 'three/addons/renderers/CSS2DRenderer.js';
import type { SolarBody } from '../data';
import { STAR_BACKGROUND } from '../data/textures';
import { SolarSystemModel } from '../core/model';
import { BodyView } from './body-view';
import { CameraController } from './camera-controller';
import { CelestialBelt, createAsteroidBelt, createKuiperBelt } from './belt';
import { OrbitLine } from './orbit-line';
import { eclipticToWorld, Scaling } from './scaling';
import { TextureLibrary } from './textures';

export interface SceneOptions {
  canvasHost: HTMLElement;
  labelHost: HTMLElement;
  /** Element that receives wheel gestures (the 3D viewport wrapper). */
  viewportHost: HTMLElement;
  textures: TextureLibrary;
  onSelect: (id: string) => void;
}

/** Camera distance (AU) inside which satellite labels appear. */
const MOON_LABEL_RANGE_AU = 0.06;
/** Belt refresh interval in seconds of real time. */
const BELT_INTERVAL = 0.25;

/** Normalises `WheelEvent.deltaY` across line/page scroll modes. */
function normalizeWheelDelta(event: WheelEvent): number {
  if (event.deltaMode === 1) return event.deltaY * 16; // lines
  if (event.deltaMode === 2) return event.deltaY * 100; // pages
  return event.deltaY;
}

export class SolarSystemScene {
  readonly renderer: THREE.WebGLRenderer;
  readonly labelRenderer: CSS2DRenderer;
  readonly scene = new THREE.Scene();
  readonly model = new SolarSystemModel();
  readonly camera: CameraController;
  readonly scaling = new Scaling();

  private readonly views = new Map<string, BodyView>();
  private readonly orbits = new Map<string, OrbitLine>();
  private readonly asteroidBelt: CelestialBelt;
  private readonly kuiperBelt: CelestialBelt;
  private readonly bodyList: SolarBody[];
  private beltTimer = 0;
  private lastTime = performance.now();
  private jdProvider?: () => number;
  private readonly onSelect: (id: string) => void;

  labelsVisible = true;
  orbitsVisible = true;
  beltsVisible = true;

  constructor(options: SceneOptions) {
    const { canvasHost, labelHost, viewportHost, textures, onSelect } = options;
    this.onSelect = onSelect;

    this.renderer = new THREE.WebGLRenderer({
      antialias: true,
      logarithmicDepthBuffer: true,
      powerPreference: 'high-performance',
    });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.renderer.setSize(canvasHost.clientWidth, canvasHost.clientHeight, false);
    this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
    this.renderer.toneMappingExposure = 1.05;
    canvasHost.appendChild(this.renderer.domElement);

    this.labelRenderer = new CSS2DRenderer({ element: document.createElement('div') });
    this.labelRenderer.domElement.className = 'label-layer';
    this.labelRenderer.setSize(canvasHost.clientWidth, canvasHost.clientHeight);
    labelHost.appendChild(this.labelRenderer.domElement);

    this.camera = new CameraController(this.renderer.domElement, canvasHost.clientWidth / canvasHost.clientHeight);

    // --- Background --------------------------------------------------------
    if (textures.has(STAR_BACKGROUND)) {
      const bg = textures.get(STAR_BACKGROUND)!;
      bg.mapping = THREE.EquirectangularReflectionMapping;
      this.scene.background = bg;
      this.scene.backgroundIntensity = 0.6;
    } else {
      this.scene.background = new THREE.Color(0x03040a);
    }

    // --- Lighting ----------------------------------------------------------
    // A point light at the Sun with decay 0: physically the flux falls off as
    // 1/r^2, which would leave Neptune 900x darker than Earth and effectively
    // invisible. Using no falloff keeps every body legible while preserving
    // the correct illumination *direction* (day/night terminator).
    const sunLight = new THREE.PointLight(0xfff4e0, 3.4, 0, 0);
    this.scene.add(sunLight);
    this.scene.add(new THREE.AmbientLight(0x2a3a55, 0.06));

    // --- Bodies ------------------------------------------------------------
    this.bodyList = [...this.model.bodies];
    for (const body of this.model.bodies) {
      const view = new BodyView({
        body,
        textures,
        scaling: this.scaling,
        onSelect,
      });
      this.scene.add(view.group);
      this.views.set(body.id, view);

      if (body.orbit) {
        const orbit = new OrbitLine(body, this.model, this.scaling);
        this.scene.add(orbit.group);
        this.orbits.set(body.id, orbit);
      }
    }

    // --- Belts -------------------------------------------------------------
    this.asteroidBelt = createAsteroidBelt();
    this.kuiperBelt = createKuiperBelt();
    this.scene.add(this.asteroidBelt.points, this.kuiperBelt.points);

    this.renderer.domElement.addEventListener('pointerdown', this.onPointerDown);
    viewportHost.addEventListener('wheel', this.onViewportWheel, { passive: false });
    this.viewportHost = viewportHost;
    this.resize();
  }

  private readonly viewportHost: HTMLElement;

  /**
   * Wheel handling at the viewport level.
   *
   * OrbitControls only calls `preventDefault()` while its internal state is
   * idle, and it never sees events that land on an HTML label overlay. Both
   * cases would otherwise let the browser zoom the whole page, so we always
   * consume the gesture here and dolly the camera ourselves when OrbitControls
   * was bypassed.
   */
  private onViewportWheel = (event: WheelEvent): void => {
    event.preventDefault();

    // The canvas is OrbitControls' own target — it already dollied for us.
    if (event.target === this.renderer.domElement) return;
    if (!this.camera.controls.enabled) return;

    const deltaY = normalizeWheelDelta(event);
    if (deltaY === 0) return;

    const scale = Math.pow(0.95, this.camera.controls.zoomSpeed * Math.abs(deltaY) * 0.01);
    const next = deltaY < 0 ? this.camera.distance * scale : this.camera.distance / scale;
    this.camera.setDistance(THREE.MathUtils.clamp(next, this.camera.controls.minDistance, this.camera.controls.maxDistance));
  };

  /** Screen-space picking: labels and tiny disks are much easier to hit. */
  private onPointerDown = (event: PointerEvent): void => {
    if (event.button !== 0) return;
    const rect = this.renderer.domElement.getBoundingClientRect();
    const hit = this.pick(event.clientX - rect.left, event.clientY - rect.top, 26);
    if (hit) this.onSelect(hit);
  };

  /** Returns the body whose projected centre is closest to the given pixel. */
  pick(x: number, y: number, maxPixels = 24): string | null {
    const rect = this.renderer.domElement.getBoundingClientRect();
    const width = rect.width;
    const height = rect.height;
    const projected = new THREE.Vector3();
    let best: string | null = null;
    let bestDistance = maxPixels;

    for (const [id, view] of this.views) {
      projected.copy(view.group.position).project(this.camera.camera);
      if (projected.z < -1 || projected.z > 1) continue;
      const px = ((projected.x + 1) / 2) * width;
      const py = ((1 - projected.y) / 2) * height;
      const d = Math.hypot(px - x, py - y);
      if (d < bestDistance) {
        bestDistance = d;
        best = id;
      }
    }
    return best;
  }

  focus(id: string, instant = false): void {
    const view = this.views.get(id);
    if (!view) return;
    const [eq] = this.scaling.radii(view.body.physical.radiusKm, view.body.physical.flattening ?? 0);
    // Use the model position: it is always current, even before the first
    // render pass has moved the view groups.
    const position = this.worldPosition(id, new THREE.Vector3());
    this.camera.focus(id, position, eq, instant);
  }

  overview(instant = false): void {
    this.camera.overview(instant);
  }

  setLabelsVisible(visible: boolean): void {
    this.labelsVisible = visible;
  }

  setOrbitsVisible(visible: boolean): void {
    this.orbitsVisible = visible;
    for (const o of this.orbits.values()) o.setVisible(visible);
  }

  setBeltsVisible(visible: boolean): void {
    this.beltsVisible = visible;
    this.asteroidBelt.setVisible(visible);
    this.kuiperBelt.setVisible(visible);
  }

  setRadiusExaggeration(factor: number): void {
    this.scaling.radiusExaggeration = factor;
    for (const view of this.views.values()) view.refreshScale();
  }

  /** World position of a body (three.js coordinates, AU). */
  worldPosition(id: string, target = new THREE.Vector3()): THREE.Vector3 {
    const state = this.model.state(id);
    return eclipticToWorld(state.position, target);
  }

  private updateLabels(): void {
    const height = this.renderer.domElement.clientHeight;
    const camera = this.camera.camera;
    const projected = new THREE.Vector3();

    for (const view of this.views.values()) {
      if (!this.labelsVisible) {
        view.updateLabel(false, 0);
        continue;
      }

      // Hide anchors that project outside the viewport: CSS2D has no clipping
      // of its own and would otherwise park them on the edges.
      projected.copy(view.group.position).project(camera);
      if (
        projected.z < -1 ||
        projected.z > 1 ||
        Math.abs(projected.x) > 1.06 ||
        Math.abs(projected.y) > 1.1
      ) {
        view.updateLabel(false, 0);
        continue;
      }

      const body = view.body;
      if (body.kind === 'moon') {
        const d = camera.position.distanceTo(view.group.position);
        if (d > MOON_LABEL_RANGE_AU) {
          view.updateLabel(false, 0);
          continue;
        }
      }

      view.updateLabel(true, view.screenRadius(camera, height));
    }
  }

  /**
   * Advance the simulation and redraw.
   * Returns true while the camera is still moving, so the app can keep the
   * frame rate high until the view settles.
   */
  render(): boolean {
    const now = performance.now();
    const dt = Math.min(0.1, (now - this.lastTime) / 1000);
    this.lastTime = now;

    const jd = this.jdProvider?.() ?? this.model.epochJD;
    this.model.update(jd);

    for (const [id, view] of this.views) {
      view.update(this.model.state(id));
    }
    for (const orbit of this.orbits.values()) {
      orbit.update(this.model, jd);
    }

    this.beltTimer += dt;
    if (this.beltTimer >= BELT_INTERVAL) {
      this.beltTimer = 0;
      this.asteroidBelt.update(jd);
      this.kuiperBelt.update(jd);
    }

    const followed = this.camera.following ? this.views.get(this.camera.following) : undefined;
    const targetPosition = followed ? followed.group.position : null;
    const targetRadius = followed
      ? this.scaling.radii(followed.body.physical.radiusKm, followed.body.physical.flattening ?? 0)[0]
      : 0;
    const cameraMoving = this.camera.update(dt, targetPosition, targetRadius);

    this.updateLabels();

    this.renderer.render(this.scene, this.camera.camera);
    this.labelRenderer.render(this.scene, this.camera.camera);
    return cameraMoving;
  }

  /** Supplies the current simulation epoch each frame. */
  setJDProvider(provider: () => number): void {
    this.jdProvider = provider;
  }

  resize(): void {
    const host = this.renderer.domElement.parentElement;
    if (!host) return;
    const width = host.clientWidth;
    const height = host.clientHeight;
    if (width === 0 || height === 0) return;
    this.renderer.setSize(width, height, false);
    this.labelRenderer.setSize(width, height);
    this.camera.resize(width, height);
  }

  get bodyCount(): number {
    return this.bodyList.length;
  }

  dispose(): void {
    this.renderer.domElement.removeEventListener('pointerdown', this.onPointerDown);
    this.viewportHost.removeEventListener('wheel', this.onViewportWheel);
    for (const view of this.views.values()) view.dispose();
    for (const orbit of this.orbits.values()) orbit.dispose();
    this.asteroidBelt.dispose();
    this.kuiperBelt.dispose();
    this.renderer.dispose();
    this.labelRenderer.domElement.remove();
  }
}
