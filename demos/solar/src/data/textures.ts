/**
 * Texture assets.
 *
 * Every map is imported through Vite so that it is:
 *   - served as a plain URL in dev and in the multi-file build, and
 *   - inlined as a `data:` URL in the single-file build (`npm run build`),
 *     which is what makes the built page work when opened directly from disk
 *     (`file://` blocks CORS-checked image loads, but `data:` URLs are fine).
 *
 * Source: Solar System Scope (https://www.solarsystemscope.com/textures/),
 * CC BY 4.0. Masters live in `public/textures/`; the bundled, down-scaled
 * copies in `src/assets/textures/` are produced by `npm run optimize:textures`.
 */
import ceres from '../assets/textures/ceres.jpg';
import earthClouds from '../assets/textures/earth-clouds.jpg';
import earthDay from '../assets/textures/earth-day.jpg';
import earthNight from '../assets/textures/earth-night.jpg';
import eris from '../assets/textures/eris.jpg';
import haumea from '../assets/textures/haumea.jpg';
import jupiter from '../assets/textures/jupiter.jpg';
import makemake from '../assets/textures/makemake.jpg';
import mars from '../assets/textures/mars.jpg';
import mercury from '../assets/textures/mercury.jpg';
import moon from '../assets/textures/moon.jpg';
import neptune from '../assets/textures/neptune.jpg';
import saturn from '../assets/textures/saturn.jpg';
import saturnRing from '../assets/textures/saturn-ring.png';
import starsMilkyWay from '../assets/textures/stars-milkyway.jpg';
import sun from '../assets/textures/sun.jpg';
import uranus from '../assets/textures/uranus.jpg';
import venusAtmosphere from '../assets/textures/venus-atmosphere.jpg';
import venusSurface from '../assets/textures/venus.jpg';

export const TEX = {
  sun,
  mercury,
  /** Radar-derived surface map of Venus (not used by default). */
  venusSurface,
  /** Visible cloud deck — what Venus actually looks like from outside. */
  venusAtmosphere,
  earthDay,
  earthNight,
  earthClouds,
  moon,
  mars,
  jupiter,
  saturn,
  saturnRing,
  uranus,
  neptune,
  ceres,
  eris,
  haumea,
  makemake,
} as const;

/** Equirectangular Milky Way panorama used as the sky background. */
export const STAR_BACKGROUND = starsMilkyWay;
