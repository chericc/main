import type { BodyData } from './types';
import { TEX } from './textures';

/**
 * The Sun.
 *
 * Physical data: NASA Planetary Fact Sheet / IAU 2015 nominal solar values.
 *   radius 695 700 km, mass 1.9885e30 kg, sidereal rotation 25.38 d,
 *   obliquity 7.25° (to the ecliptic), photosphere ~5 505 °C.
 */
export const SUN: BodyData = {
  id: 'sun',
  name: 'Sun',
  nameZh: '太阳',
  kind: 'star',
  color: 0xffd27a,
  physical: {
    radiusKm: 695_700,
    massKg: 1.9885e30,
    densityKgM3: 1_408,
    gravityMs2: 274.0,
    escapeVelocityKms: 617.7,
    rotationPeriodHours: 25.38 * 24,
    obliquityDeg: 7.25,
    meanTempC: 5_505,
    satellitesKnown: 8,
    pole: { raDeg: 286.13, decDeg: 63.87 },
    flattening: 0.000009,
  },
  textures: { map: TEX.sun },
  facts: [
    ['光谱型', 'G2V 主序星'],
    ['核心温度', '约 1 500 万 °C'],
    ['光度', '3.828 × 10²⁶ W'],
    ['年龄', '约 46 亿年'],
    ['质量占比', '太阳系总质量的 99.86%'],
    ['组成', '氢 73.5%、氦 24.9%、其他 1.6%'],
    ['自转（赤道）', '25.38 天'],
    ['日地距离', '1 AU = 149 597 870.7 km'],
  ],
};
