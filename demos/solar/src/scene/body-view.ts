/**
 * One rendered body: an oblate spheroid with the real axial tilt and spin,
 * optional cloud layer, ring system, corona and a DOM label/marker.
 */
import * as THREE from 'three';
import { CSS2DObject } from 'three/addons/renderers/CSS2DRenderer.js';
import type { SolarBody } from '../data';
import type { BodyState } from '../core/model';
import { eclipticToWorld, Scaling, writeEclipticToWorld } from './scaling';
import { glowTexture, proceduralPlanetMaps, proceduralRingTexture, type SurfaceStyle, TextureLibrary } from './textures';

const UP = new THREE.Vector3(0, 1, 0);

/**
 * Marker diameter is exactly `2 × projected radius` — there is deliberately
 * no minimum (a distant body must be able to vanish completely) and no
 * maximum (a nearby body must not get a fixed-size blob).
 * The marker only fades out once the sphere itself is clearly resolved.
 */
const MARKER_FADE_IN_PX = 1.2;
const MARKER_FADE_OUT_FROM_PX = 2;
const MARKER_FADE_OUT_TO_PX = 4;
/** Above this projected radius the name plate is redundant. */
const NAME_HIDE_PX = 150;

/** Sphere tessellation by body class — keeps the moon count cheap. */
function segmentsFor(body: SolarBody): [number, number] {
  switch (body.kind) {
    case 'star':
      return [72, 48];
    case 'planet':
      return [64, 48];
    case 'dwarf-planet':
      return [48, 32];
    case 'moon':
      return [28, 20];
    default:
      return [24, 16];
  }
}

function styleFor(body: SolarBody): SurfaceStyle {
  switch (body.kind) {
    case 'star':
      return 'gas';
    case 'comet':
      return 'icy';
    case 'moon':
      return body.id === 'io' ? 'rocky' : 'cratered';
    default:
      return body.physical.radiusKm > 20_000 ? 'gas' : 'rocky';
  }
}

export interface BodyViewOptions {
  body: SolarBody;
  textures: TextureLibrary;
  scaling: Scaling;
  onSelect: (id: string) => void;
}

export class BodyView {
  readonly body: SolarBody;
  readonly group = new THREE.Group();
  /** Anchored exactly on the body centre — never moves when labels change. */
  readonly markerObject: CSS2DObject;
  /** Name plate, offset to the right of the marker. */
  readonly nameObject: CSS2DObject;
  readonly markerElement: HTMLDivElement;
  readonly nameElement: HTMLDivElement;

  private readonly tiltGroup = new THREE.Group();
  private readonly mesh: THREE.Mesh;
  private clouds?: THREE.Mesh;
  private ring?: THREE.Mesh;
  private glow?: THREE.Sprite;
  private readonly scaling: Scaling;
  private readonly baseRadius: number;
  private readonly flattening: number;

  constructor(options: BodyViewOptions) {
    const { body, textures, scaling, onSelect } = options;
    this.body = body;
    this.scaling = scaling;
    this.baseRadius = body.physical.radiusKm;
    this.flattening = body.physical.flattening ?? 0;

    this.group.name = `body:${body.id}`;
    this.group.add(this.tiltGroup);

    const [wSeg, hSeg] = segmentsFor(body);
    const geometry = new THREE.SphereGeometry(1, wSeg, hSeg);

    const material = this.buildMaterial(textures);
    this.mesh = new THREE.Mesh(geometry, material);
    this.mesh.name = `mesh:${body.id}`;
    this.mesh.userData.bodyId = body.id;
    this.tiltGroup.add(this.mesh);

    // Venus' visible disk is its cloud deck, so the atmospheric map is used
    // as the base map there (the radar surface map stays in public/textures).
    if (body.textures?.cloudsMap && textures.has(body.textures.cloudsMap)) {
      const cloudTex = textures.get(body.textures.cloudsMap)!;
      this.clouds = new THREE.Mesh(
        new THREE.SphereGeometry(1, Math.max(24, wSeg - 8), Math.max(16, hSeg - 8)),
        new THREE.MeshStandardMaterial({
          color: 0xffffff,
          alphaMap: cloudTex,
          transparent: true,
          opacity: 0.85,
          depthWrite: false,
          roughness: 1,
          metalness: 0,
        }),
      );
      this.tiltGroup.add(this.clouds);
    }

    if (body.ring && body.ring.outerRadiusKm > body.ring.innerRadiusKm) {
      this.ring = this.buildRing(body, textures);
      this.tiltGroup.add(this.ring);
    }

    if (body.kind === 'star') {
      this.glow = new THREE.Sprite(
        new THREE.SpriteMaterial({
          map: glowTexture(),
          color: 0xffffff,
          transparent: true,
          blending: THREE.AdditiveBlending,
          depthWrite: false,
          depthTest: false,
        }),
      );
      this.group.add(this.glow);
    }

    this.applyScale();

    // --- DOM marker + name plate -------------------------------------------
    // Two separate CSS2D objects: the marker is centred on the body, the name
    // is left-aligned to the body and pushed right by padding. Hiding the name
    // therefore never shifts the marker.
    const cssColor = colorToCss(body.color);
    const onPointerDown = (event: PointerEvent) => {
      event.stopPropagation();
      onSelect(body.id);
    };

    this.markerElement = document.createElement('div');
    this.markerElement.className = `body-marker kind-${body.kind}`;
    this.markerElement.dataset.bodyId = body.id;
    this.markerElement.style.setProperty('--dot', cssColor);
    this.markerElement.addEventListener('pointerdown', onPointerDown);
    this.markerObject = new CSS2DObject(this.markerElement);
    this.markerObject.center.set(0.5, 0.5);
    this.group.add(this.markerObject);

    this.nameElement = document.createElement('div');
    this.nameElement.className = `body-name kind-${body.kind}`;
    this.nameElement.dataset.bodyId = body.id;
    this.nameElement.textContent = body.nameZh;
    this.nameElement.addEventListener('pointerdown', onPointerDown);
    this.nameObject = new CSS2DObject(this.nameElement);
    this.nameObject.center.set(0, 0.5);
    this.group.add(this.nameObject);
  }

  private buildMaterial(textures: TextureLibrary): THREE.Material {
    const { body } = this;
    if (body.kind === 'star') {
      const map = textures.get(body.textures?.map ?? '');
      return new THREE.MeshBasicMaterial({
        map: map ?? undefined,
        color: map ? 0xffffff : body.color,
        toneMapped: false,
      });
    }

    const mapPath = body.textures?.map;
    const material = new THREE.MeshStandardMaterial({
      color: mapPath && textures.has(mapPath) ? 0xffffff : body.color,
      roughness: 1,
      metalness: 0,
    });

    if (mapPath && textures.has(mapPath)) {
      material.map = textures.get(mapPath)!;
    } else {
      // Procedural fallback, deterministic per body.
      const maps = proceduralPlanetMaps({
        seed: hashSeed(body.id),
        color: body.color,
        style: styleFor(body),
      });
      material.map = maps.map;
      material.bumpMap = maps.bumpMap;
      material.bumpScale = body.physical.radiusKm > 2_000 ? 0.02 : 0.05;
    }

    const nightPath = body.textures?.nightMap;
    if (nightPath && textures.has(nightPath)) {
      material.emissiveMap = textures.get(nightPath)!;
      material.emissive = new THREE.Color(0xffd9a0);
      material.emissiveIntensity = 0.9;
    }
    return material;
  }

  private buildRing(body: SolarBody, textures: TextureLibrary): THREE.Mesh {
    const ring = body.ring!;
    const inner = ring.innerRadiusKm / 149_597_870.7;
    const outer = ring.outerRadiusKm / 149_597_870.7;
    const geometry = new THREE.RingGeometry(inner, outer, 256, 1);

    // The published Saturn profile is a horizontal strip: x = radial position.
    const pos = geometry.attributes.position;
    const uv = geometry.attributes.uv;
    for (let i = 0; i < pos.count; i++) {
      const r = Math.hypot(pos.getX(i), pos.getY(i));
      uv.setXY(i, (r - inner) / (outer - inner), 0.5);
    }
    uv.needsUpdate = true;

    const texture =
      ring.texture && textures.has(ring.texture)
        ? textures.get(ring.texture)!
        : proceduralRingTexture(ring.bands ?? [[0, 1, 0.4]], ring.color);

    const material = new THREE.MeshBasicMaterial({
      map: texture,
      color: 0xffffff,
      transparent: true,
      opacity: ring.opacity,
      side: THREE.DoubleSide,
      depthWrite: false,
    });

    const mesh = new THREE.Mesh(geometry, material);
    mesh.rotation.x = -Math.PI / 2; // RingGeometry is in XY; the equator is XZ.
    mesh.name = `ring:${body.id}`;
    return mesh;
  }

  private applyScale(): void {
    const [eq, polar] = this.scaling.radii(this.baseRadius, this.flattening);
    this.mesh.scale.set(eq, polar, eq);
    if (this.clouds) this.clouds.scale.set(eq * 1.008, polar * 1.008, eq * 1.008);
    if (this.glow) {
      const s = eq * 6;
      this.glow.scale.set(s, s, 1);
    }
  }

  /** Applies the radius exaggeration (visual aid; 1 = true scale). */
  refreshScale(): void {
    this.applyScale();
  }

  /** Positions the body and applies its true orientation for this epoch. */
  update(state: BodyState): void {
    writeEclipticToWorld(state.position, this.group.position);

    const pole = eclipticToWorld(state.pole).normalize();
    this.tiltGroup.quaternion.setFromUnitVectors(UP, pole);

    this.mesh.rotation.y = state.spinRad;
    if (this.clouds) this.clouds.rotation.y = state.spinRad * 1.08;
  }

  /**
   * Screen-space radius of the body in pixels, used for label logic.
   */
  screenRadius(camera: THREE.PerspectiveCamera, viewportHeight: number): number {
    const distance = camera.position.distanceTo(this.group.position);
    if (distance <= 0) return Infinity;
    const [eq] = this.scaling.radii(this.baseRadius, this.flattening);
    const fovRad = (camera.fov * Math.PI) / 180;
    return ((eq / distance) * (viewportHeight / 2)) / Math.tan(fovRad / 2);
  }

  /**
   * Updates the DOM overlays.
   *
   * The marker diameter is exactly twice the body's projected radius, so
   * zooming keeps scaling it all the way down to nothing — relative sizes are
   * always truthful. The name plate is what keeps distant bodies findable.
   */
  updateLabel(visible: boolean, screenRadiusPx: number): void {
    // Visibility must be toggled on the CSS2DObjects, never via
    // `element.style.display`: CSS2DRenderer rewrites `display` every frame
    // from `object.visible`, so a `display: none` set here is immediately
    // undone and the node reappears at its stylesheet fallback size (a 6px
    // dot). That fallback was the phantom "minimum size" seen on satellites
    // until they were visited once and got a real `--dot-size`.
    this.markerObject.visible = visible;
    this.nameObject.visible = visible;

    if (!visible) {
      // Defensive: should anything re-show the node before the next update it
      // must render at zero size, not at the CSS default.
      this.markerElement.style.setProperty('--dot-size', '0px');
      this.markerElement.style.opacity = '0';
      return;
    }

    // --- marker: true projected size, no floor and no ceiling -------------- 
    const markerPx = screenRadiusPx * 2;
    this.markerElement.style.setProperty('--dot-size', `${markerPx.toFixed(4)}px`);
    const fadeIn = Math.min(1, screenRadiusPx / MARKER_FADE_IN_PX);
    const fadeOut = Math.max(
      0,
      Math.min(1, (MARKER_FADE_OUT_TO_PX - screenRadiusPx) / (MARKER_FADE_OUT_TO_PX - MARKER_FADE_OUT_FROM_PX)),
    );
    this.markerElement.style.opacity = String(fadeIn * fadeOut);

    // --- name plate: anchored on the body centre, pushed clear of the disc --
    const showName = screenRadiusPx <= NAME_HIDE_PX;
    this.nameObject.visible = showName;
    if (showName) {
      const padX = Math.max(11, screenRadiusPx + 9);
      const padBottom = Math.min(46, screenRadiusPx * 0.95);
      this.nameElement.style.paddingLeft = `${padX.toFixed(1)}px`;
      this.nameElement.style.paddingBottom = `${padBottom.toFixed(1)}px`;
    }
  }

  dispose(): void {
    this.mesh.geometry.dispose();
    (this.mesh.material as THREE.Material).dispose();
    this.clouds?.geometry.dispose();
    this.ring?.geometry.dispose();
    this.markerElement.remove();
    this.nameElement.remove();
  }
}

function colorToCss(color: number): string {
  return `#${color.toString(16).padStart(6, '0')}`;
}

function hashSeed(text: string): number {
  let h = 2166136261;
  for (let i = 0; i < text.length; i++) {
    h ^= text.charCodeAt(i);
    h = Math.imul(h, 16777619);
  }
  return h >>> 0;
}
