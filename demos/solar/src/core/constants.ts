/**
 * Physical and astronomical constants.
 *
 * Values follow the IAU 2015 / NASA "Planetary Fact Sheet" definitions.
 */

/** Astronomical unit in kilometres (IAU 2012 definition). */
export const AU_KM = 149_597_870.7;

/** Standard gravitational parameter of the Sun (km^3/s^2). */
export const GM_SUN_KM3_S2 = 1.32712440018e11;

/** Julian Date of the J2000.0 epoch: 2000 January 1.5 TDB. */
export const J2000 = 2_451_545.0;

/** Days in a Julian century. */
export const DAYS_PER_CENTURY = 36_525;

/** Seconds in a day. */
export const SECONDS_PER_DAY = 86_400;

/** Degrees -> radians. */
export const DEG = Math.PI / 180;

/** Radians -> degrees. */
export const RAD = 180 / Math.PI;

/**
 * Gaussian gravitational constant, in radians per day.
 * Mean motion n [rad/day] = GAUSSIAN_K / a^1.5, with a in AU.
 */
export const GAUSSIAN_K = 0.01720209895;

/** n in degrees per day for a semi-major axis a (AU): 0.9856076686 / a^1.5 */
export const MEAN_MOTION_DEG = (GAUSSIAN_K * RAD) as number;

/** Mean Earth radius in kilometres. */
export const EARTH_RADIUS_KM = 6_371.0;

/** Mean Earth mass in kilograms. */
export const EARTH_MASS_KG = 5.97237e24;

/** Obliquity of the ecliptic at J2000, in degrees. */
export const OBLIQUITY_J2000_DEG = 23.43928;
