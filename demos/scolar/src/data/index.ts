import type { OrbitalElements } from '../core/kepler';
import { DWARF_PLANETS } from './dwarf-planets';
import { MOONS } from './moons';
import { PLANETS } from './planets';
import { SMALL_BODIES } from './small-bodies';
import { SUN } from './sun';
import type { BodyData, BodyKind, MoonData, OrbitFrame, PhysicalData, RingData, TextureSet } from './types';

export * from './types';
export { SUN } from './sun';
export { PLANETS } from './planets';
export { DWARF_PLANETS } from './dwarf-planets';
export { ASTEROIDS, COMETS, SMALL_BODIES } from './small-bodies';
export { MOONS } from './moons';

/**
 * A unified, hierarchical body record: a Sun-orbiting body or a satellite.
 * Satellites carry a `parentId` and a `frame` describing the reference plane
 * of their elements.
 */
export interface SolarBody {
  id: string;
  name: string;
  nameZh: string;
  kind: BodyKind;
  /** Set for satellites. */
  parentId?: string;
  /** Orbit around the Sun (for planets/small bodies) or around the parent. */
  orbit: OrbitalElements | null;
  /** Reference plane for satellite elements. */
  frame?: OrbitFrame;
  /** Use the perturbed lunar theory (Moon only). */
  useLunarTheory?: boolean;
  physical: PhysicalData;
  color: number;
  textures?: TextureSet;
  ring?: RingData;
  facts?: Array<[string, string]>;
  /** Rendered satellites of this body. */
  children: SolarBody[];
  /** Direct parent record, if this is a satellite. */
  parent?: SolarBody;
}

/** Chinese labels for the body kinds, used by the UI. */
export const KIND_LABELS: Record<BodyKind, string> = {
  star: '恒星',
  planet: '行星',
  'dwarf-planet': '矮行星',
  asteroid: '小行星',
  comet: '彗星',
  moon: '卫星',
};

function moonToBody(moon: MoonData): SolarBody {
  const rotationPeriodHours = moon.rotationPeriodHours ?? (moon.orbit.period ?? 0) * 24;
  return {
    id: moon.id,
    name: moon.name,
    nameZh: moon.nameZh,
    kind: 'moon',
    parentId: moon.parent,
    orbit: moon.orbit,
    frame: moon.frame,
    ...(moon.useLunarTheory ? { useLunarTheory: true } : {}),
    physical: {
      radiusKm: moon.radiusKm,
      massKg: moon.massKg ?? 0,
      // Satellites orbit in (or very near) their parent's equatorial plane.
      obliquityDeg: 0,
      rotationPeriodHours,
    },
    color: moon.color,
    ...(moon.facts ? { facts: moon.facts } : {}),
    children: [],
  };
}

function bodyToSolarBody(body: BodyData, moons: SolarBody[]): SolarBody {
  return {
    id: body.id,
    name: body.name,
    nameZh: body.nameZh,
    kind: body.kind,
    orbit: body.orbit ?? null,
    physical: body.physical,
    color: body.color,
    ...(body.textures ? { textures: body.textures } : {}),
    ...(body.ring ? { ring: body.ring } : {}),
    ...(body.facts ? { facts: body.facts } : {}),
    children: moons,
  };
}

function buildSystem(): SolarBody[] {
  const moonBodies = MOONS.map(moonToBody);
  const byParent = new Map<string, SolarBody[]>();
  for (const m of moonBodies) {
    const list = byParent.get(m.parentId!) ?? [];
    list.push(m);
    byParent.set(m.parentId!, list);
  }

  const roots = [SUN, ...PLANETS, ...DWARF_PLANETS, ...SMALL_BODIES].map((b) =>
    bodyToSolarBody(b, byParent.get(b.id) ?? []),
  );

  // Link parents (satellites are attached to their parent record).
  const byId = new Map<string, SolarBody>();
  for (const r of roots) {
    byId.set(r.id, r);
    for (const c of r.children) byId.set(c.id, c);
  }
  for (const r of roots) {
    for (const c of r.children) c.parent = r;
  }
  return roots;
}

/** The full hierarchy: the Sun plus every Sun-orbiting body. */
export const SOLAR_SYSTEM: SolarBody[] = buildSystem();

/** Flat list of every body, root-first. */
export const ALL_BODIES: SolarBody[] = (() => {
  const out: SolarBody[] = [];
  const walk = (b: SolarBody) => {
    out.push(b);
    for (const c of b.children) walk(c);
  };
  for (const r of SOLAR_SYSTEM) walk(r);
  return out;
})();

const BODY_INDEX = new Map(ALL_BODIES.map((b) => [b.id, b]));

export function findBody(id: string): SolarBody | undefined {
  return BODY_INDEX.get(id);
}

export function requireBody(id: string): SolarBody {
  const b = BODY_INDEX.get(id);
  if (!b) throw new Error(`Unknown body id: ${id}`);
  return b;
}

/** The Sun record. */
export const SUN_BODY: SolarBody = requireBody('sun');

/** Ordering used by the body list in the UI. */
export const BODY_ORDER: string[] = ALL_BODIES.map((b) => b.id);
