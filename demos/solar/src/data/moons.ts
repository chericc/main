import { MOON_ORBITS } from './moon-orbits.generated';
import { MOON_PHYSICAL } from './moon-physical';
import type { MoonData } from './types';

/**
 * The rendered natural satellites.
 *
 * Orbital elements come from the auto-generated JPL mean-element table
 * (`moon-orbits.generated.ts`); physical data from `moon-physical.ts`.
 * Only a curated selection of the most significant satellites is included —
 * there are 459 satellites in the JPL table, 23 of which are rendered here.
 */
export const MOONS: MoonData[] = MOON_ORBITS.map((record) => {
  const phys = MOON_PHYSICAL[record.name];
  if (!phys) throw new Error(`Missing physical data for satellite ${record.name}`);
  return {
    id: record.name.toLowerCase(),
    name: record.name,
    nameZh: record.nameZh,
    parent: record.parent,
    frame: record.frame,
    orbit: record.orbit,
    radiusKm: phys.radiusKm,
    massKg: phys.massKg,
    color: phys.color,
    ...(phys.facts ? { facts: phys.facts } : {}),
    ...(record.useLunarTheory ? { useLunarTheory: true } : {}),
  };
});

/** Number of satellites rendered by Solar, per parent id. */
export const MOON_COUNT_BY_PARENT: Record<string, number> = MOONS.reduce<Record<string, number>>((acc, m) => {
  acc[m.parent] = (acc[m.parent] ?? 0) + 1;
  return acc;
}, {});
