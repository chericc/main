/**
 * Data model for every celestial body in Solar.
 *
 * All values are real, sourced from NASA/JPL (see each data file for the
 * exact provenance, and `reference/` for the raw downloaded tables).
 */
import type { OrbitalElements } from '../core/kepler';

export type BodyKind = 'star' | 'planet' | 'dwarf-planet' | 'asteroid' | 'comet' | 'moon';

/** Reference plane in which a satellite's mean elements are expressed. */
export type OrbitFrame = 'ecliptic' | 'equator' | 'laplace';

export interface RingData {
  /** Inner radius, km (measured from the parent's centre). */
  innerRadiusKm: number;
  /** Outer radius, km. */
  outerRadiusKm: number;
  /** Radial alpha profile texture (x = radial direction). */
  texture?: string;
  color: number;
  opacity: number;
  /** Procedural radial bands `[start, end, alpha]` in 0..1 of the ring span. */
  bands?: Array<[number, number, number]>;
}

export interface PhysicalData {
  /** Volumetric mean radius, km. */
  radiusKm: number;
  /** Mass, kg. */
  massKg: number;
  /** Mean density, kg/m³. */
  densityKgM3?: number;
  /** Surface gravity at the equator, m/s². */
  gravityMs2?: number;
  /** Escape velocity, km/s. */
  escapeVelocityKms?: number;
  /**
   * Sidereal rotation period, hours. Always positive: retrograde rotation is
   * expressed through an obliquity greater than 90°.
   */
  rotationPeriodHours: number;
  /** Axial tilt relative to the orbital plane, degrees. */
  obliquityDeg: number;
  /** Mean surface (or cloud-top) temperature, °C. */
  meanTempC?: number;
  /** Geometric albedo, 0..1. */
  albedo?: number;
  /**
   * Polar flattening f = (r_eq - r_polar) / r_eq. Used for the oblate
   * spheroids (Jupiter and Saturn are visibly flattened).
   */
  flattening?: number;
  /** Dominant atmospheric composition. */
  atmosphere?: string;
  /** Discovery information. */
  discovery?: string;
  /** Number of known natural satellites. */
  satellitesKnown?: number;
  /**
   * IAU direction of the north pole in the ICRF/J2000 equatorial frame,
   * degrees. Source: IAU WGCCRE report (Archinal et al.), Table 1.
   * When present this gives the true orientation of the rotation axis
   * (and therefore the true season geometry); `obliquityDeg` is then only
   * used for display.
   */
  pole?: { raDeg: number; decDeg: number };
}

export interface TextureSet {
  /** Base colour map (equirectangular). */
  map?: string;
  /** Night-side emissive map. */
  nightMap?: string;
  /** Semi-transparent cloud layer. */
  cloudsMap?: string;
}

export interface BodyData {
  id: string;
  /** English name (IAU). */
  name: string;
  /** Chinese name. */
  nameZh: string;
  kind: BodyKind;
  /** Heliocentric orbit; omitted for the Sun. */
  orbit?: OrbitalElements;
  physical: PhysicalData;
  /** Fallback/flat colour used for procedural maps, UI swatches and lines. */
  color: number;
  textures?: TextureSet;
  ring?: RingData;
  /** Extra descriptive facts for the info panel: `[label, value]`. */
  facts?: Array<[string, string]>;
}

export interface MoonData {
  id: string;
  name: string;
  nameZh: string;
  radiusKm: number;
  massKg?: number;
  color: number;
  /** Parent body id. */
  parent: string;
  /** Reference plane of `orbit`. */
  frame: OrbitFrame;
  /** Satellite orbit; semi-major axis in AU. */
  orbit: OrbitalElements;
  /**
   * Sidereal rotation period in hours. Defaults to the orbital period
   * (all of these satellites are tidally locked).
   */
  rotationPeriodHours?: number;
  /** Use the dedicated lunar perturbation theory instead of the mean elements. */
  useLunarTheory?: boolean;
  facts?: Array<[string, string]>;
}

/** Kilometres -> AU. */
export function kmToAU(km: number): number {
  return km / 149_597_870.7;
}
