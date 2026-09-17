import type { BodyData } from './types';

/**
 * Large main-belt asteroids and two well-known periodic comets.
 *
 * ORBITAL ELEMENTS: JPL Small-Body Database (reference/sbdb-elements.json),
 * converted from (Ω, ω, M) to (Ω, ϖ, L).
 *   epoch 2 461 200.5 = 2023-Feb-25 for the asteroids,
 *   epoch 2 439 875.5 = 1962-Oct-15 for 1P/Halley,
 *   epoch 2 459 993.5 = 2023-Feb-25 for 2P/Encke.
 *
 * PHYSICAL DATA: NASA/JPL, Dawn / Rosetta / ESO VLT measurements.
 */
const EPOCH_2023 = 2_461_200.5;

export const ASTEROIDS: BodyData[] = [
  {
    id: 'vesta',
    name: 'Vesta',
    nameZh: '灶神星',
    kind: 'asteroid',
    color: 0xa8a094,
    orbit: {
      epoch: EPOCH_2023,
      a: 2.361365965127599,
      e: 0.09020374382834395,
      i: 7.143925545058711,
      L: 336.3600971640574,
      peri: 255.16994108718842,
      node: 103.701293265032,
    },
    physical: {
      radiusKm: 262.7,
      massKg: 2.589e20,
      densityKgM3: 3_456,
      gravityMs2: 0.22,
      escapeVelocityKms: 0.36,
      rotationPeriodHours: 5.342,
      obliquityDeg: 29,
      meanTempC: -108,
      albedo: 0.42,
      discovery: '1807 年 Heinrich Olbers',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '3.63 年'],
      ['小行星带质量占比', '约 9%'],
      ['直径', '525 km，小行星带第二大天体'],
      ['南极撞击坑', 'Rheasilvia，直径 500 km'],
      ['探测', 'NASA Dawn 号，2011–2012 年环绕'],
    ],
  },
  {
    id: 'pallas',
    name: 'Pallas',
    nameZh: '智神星',
    kind: 'asteroid',
    color: 0x9b968c,
    orbit: {
      epoch: EPOCH_2023,
      a: 2.769559010737709,
      e: 0.2307000995648547,
      i: 34.93279321851542,
      L: 18.106187675256365,
      peri: 483.856535500983,
      node: 172.8866193357694,
    },
    physical: {
      radiusKm: 256,
      massKg: 2.04e20,
      densityKgM3: 2_800,
      gravityMs2: 0.21,
      escapeVelocityKms: 0.32,
      rotationPeriodHours: 7.813,
      obliquityDeg: 84,
      meanTempC: -108,
      albedo: 0.155,
      discovery: '1802 年 Heinrich Olbers',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '4.61 年'],
      ['轨道倾角', '34.9°，主带中最倾斜的大天体'],
      ['直径', '约 512 km'],
      ['表面', '富碳，布满撞击坑'],
    ],
  },
  {
    id: 'juno',
    name: 'Juno',
    nameZh: '婚神星',
    kind: 'asteroid',
    color: 0xa39a8e,
    orbit: {
      epoch: EPOCH_2023,
      a: 2.670989527103278,
      e: 0.2556999836681878,
      i: 12.98659236598085,
      L: 320.4389641451886,
      peri: 417.7066696568031,
      node: 169.8115953492418,
    },
    physical: {
      radiusKm: 127,
      massKg: 2.67e19,
      densityKgM3: 3_200,
      gravityMs2: 0.12,
      escapeVelocityKms: 0.18,
      rotationPeriodHours: 7.21,
      obliquityDeg: 50,
      meanTempC: -108,
      albedo: 0.238,
      discovery: '1804 年 Karl Harding',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '4.36 年'],
      ['直径', '约 254 km'],
      ['类型', 'S 型，表面为硅酸盐岩石'],
    ],
  },
  {
    id: 'hygiea',
    name: 'Hygiea',
    nameZh: '健神星',
    kind: 'asteroid',
    color: 0x8c8880,
    orbit: {
      epoch: EPOCH_2023,
      a: 3.150974033963701,
      e: 0.1067092741240963,
      i: 3.829529946447122,
      L: 127.57855572129472,
      peri: 595.5441314853298,
      node: 283.1198927508594,
    },
    physical: {
      radiusKm: 216.5,
      massKg: 8.32e19,
      densityKgM3: 1_940,
      gravityMs2: 0.09,
      escapeVelocityKms: 0.21,
      rotationPeriodHours: 13.83,
      obliquityDeg: 0,
      meanTempC: -120,
      albedo: 0.072,
      discovery: '1849 年 Annibale de Gasparis',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '5.59 年'],
      ['直径', '约 433 km，主带第四大天体'],
      ['形态', '2019 年 VLT 观测显示接近球形，可能符合矮行星定义'],
    ],
  },
];

export const COMETS: BodyData[] = [
  {
    id: 'halley',
    name: '1P/Halley',
    nameZh: '哈雷彗星',
    kind: 'comet',
    color: 0x9fd8e8,
    orbit: {
      epoch: 2_439_875.5, // 1962-Oct-15
      a: 17.92863504856923,
      e: 0.9679359956953211,
      i: 162.1905300439129,
      L: 85.72271580657997,
      peri: 171.34037866990076,
      node: 59.09894720612437,
    },
    physical: {
      radiusKm: 5.5, // 15 × 8 × 8 km nucleus -> equivalent radius
      massKg: 2.2e14,
      densityKgM3: 600,
      rotationPeriodHours: 52.8,
      obliquityDeg: 0,
      albedo: 0.04,
      discovery: '1705 年 Edmond Halley 预言回归',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '75.3 年'],
      ['轨道倾角', '162.19°（逆行）'],
      ['近日点', '0.586 AU'],
      ['远日点', '35.08 AU'],
      ['上次回归', '1986 年（Giotto 号近距离探测）'],
      ['下次回归', '2061 年 7 月 28 日'],
    ],
  },
  {
    id: 'encke',
    name: '2P/Encke',
    nameZh: '恩克彗星',
    kind: 'comet',
    color: 0xa8dcc8,
    orbit: {
      epoch: 2_459_993.5,
      a: 2.219495497912434,
      e: 0.8471460582838967,
      i: 11.35271628498256,
      L: 87.84779912104727,
      peri: 521.3129347728809,
      node: 334.0763209232213,
    },
    physical: {
      radiusKm: 2.4,
      massKg: 7.0e13,
      densityKgM3: 500,
      rotationPeriodHours: 11.08,
      obliquityDeg: 0,
      albedo: 0.046,
      discovery: '1786 年 Pierre Méchain，1819 年 Encke 确定周期',
      satellitesKnown: 0,
    },
    facts: [
      ['公转周期', '3.30 年（最短的已知彗星周期）'],
      ['近日点', '0.336 AU'],
      ['远日点', '4.10 AU'],
      ['命名', '以确定其周期性的 Johann Encke 命名'],
    ],
  },
];

/** All small bodies that orbit the Sun directly. */
export const SMALL_BODIES: BodyData[] = [...ASTEROIDS, ...COMETS];
