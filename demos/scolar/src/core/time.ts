import { J2000, SECONDS_PER_DAY } from './constants';

/**
 * Julian Date helpers and the simulation clock.
 *
 * All ephemeris maths in Scolar is driven by a Julian Date (JD, TT/TDB).
 * The tiny difference between UTC and TDB (< 2 ms) is irrelevant at the
 * accuracy of the mean-element model used here.
 */

const UNIX_EPOCH_JD = 2_440_587.5;
const MS_PER_DAY = 86_400_000;

/** JavaScript `Date` (interpreted as UTC) -> Julian Date. */
export function dateToJD(date: Date): number {
  return date.getTime() / MS_PER_DAY + UNIX_EPOCH_JD;
}

/** Julian Date -> JavaScript `Date`. */
export function jdToDate(jd: number): Date {
  return new Date((jd - UNIX_EPOCH_JD) * MS_PER_DAY);
}

/** Julian centuries elapsed since J2000.0. */
export function centuriesSinceJ2000(jd: number): number {
  return (jd - J2000) / 36_525;
}

/** Human readable UTC string, e.g. `2025-01-01 00:00 UTC`. */
export function formatJD(jd: number, withTime = true): string {
  const d = jdToDate(jd);
  const iso = d.toISOString();
  return withTime ? `${iso.slice(0, 10)} ${iso.slice(11, 16)} UTC` : iso.slice(0, 10);
}

/** `<input type="datetime-local">` value for a Julian Date. */
export function jdToLocalInputValue(jd: number): string {
  return jdToDate(jd).toISOString().slice(0, 16);
}

/** Parse an `<input type="datetime-local">` value as UTC. */
export function localInputValueToJD(value: string): number {
  return dateToJD(new Date(`${value}:00Z`));
}

/** Formats a duration in days as a human string (e.g. `165.2 y`). */
export function formatDurationDays(days: number): string {
  const abs = Math.abs(days);
  if (abs < 1 / 24) return `${(days * 24 * 60).toFixed(1)} min`;
  if (abs < 2) return `${(days * 24).toFixed(1)} h`;
  if (abs < 700) return `${days.toFixed(2)} d`;
  if (abs < 365_250) return `${(days / 365.25).toFixed(2)} y`;
  return `${(days / 365.25 / 1000).toFixed(2)} ky`;
}

/**
 * The simulation clock.
 *
 * `speed` is expressed as *simulated seconds per real second*, so
 * `speed = 1` is real time and `speed = 86_400` advances one day per second.
 */
export class SimulationClock {
  private _jd: number;
  private _paused = false;
  private _speed = 60; // 1 simulated minute per real second

  constructor(jd: number = dateToJD(new Date())) {
    this._jd = jd;
  }

  get jd(): number {
    return this._jd;
  }

  set jd(value: number) {
    this._jd = value;
  }

  get paused(): boolean {
    return this._paused;
  }

  set paused(value: boolean) {
    this._paused = value;
  }

  /** Simulated seconds per real second. May be negative (time runs backwards). */
  get speed(): number {
    return this._speed;
  }

  set speed(value: number) {
    this._speed = value;
  }

  /** Advances the clock by a real elapsed time in seconds. */
  advance(realSeconds: number): void {
    if (this._paused || this._speed === 0) return;
    this._jd += (realSeconds * this._speed) / SECONDS_PER_DAY;
  }
}

/** Common speed presets, in simulated seconds per real second. */
export const SPEED_PRESETS: ReadonlyArray<{ label: string; seconds: number }> = [
  { label: '实时', seconds: 1 },
  { label: '1 分/秒', seconds: 60 },
  { label: '1 时/秒', seconds: 3_600 },
  { label: '1 天/秒', seconds: SECONDS_PER_DAY },
  { label: '1 周/秒', seconds: 7 * SECONDS_PER_DAY },
  { label: '1 月/秒', seconds: 30 * SECONDS_PER_DAY },
  { label: '1 年/秒', seconds: 365.25 * SECONDS_PER_DAY },
  { label: '10 年/秒', seconds: 10 * 365.25 * SECONDS_PER_DAY },
];
