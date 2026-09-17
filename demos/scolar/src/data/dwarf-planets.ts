import type { BodyData, PhysicalData } from './types';
import { TEX } from './textures';

/**
 * Dwarf planets.
 *
 * ORBITAL ELEMENTS: JPL Small-Body Database, osculating elements with full
 * precision, fetched by `npm run fetch:reference` (see
 * reference/sbdb-elements.json and reference/raw/sbdb-*.json).
 * The tabulated (Ω, ω, M) triples are converted here to the (Ω, ϖ, L) form
 * used by the propagator:  ϖ = ω + Ω,  L = ω + Ω + M.
 *
 * PHYSICAL DATA: NASA/JPL fact sheets and the New Horizons / Herschel /
 * Spitzer measurement literature.
 *
 * NOTE ON TEXTURES: Solar System Scope publishes no Pluto map, so Pluto and
 * all satellites use the procedurally generated maps from `scene/textures.ts`.
 * The Ceres/Haumea/Makemake/Eris maps are the artist's "fictional"
 * interpretations published by the same source.
 */
const EPOCH_SBDB_2023 = 2_461_200.5; // 2023-Feb-25 (SBDB osculation epoch)
const EPOCH_PLUTO_2016 = 2_457_588.5; // 2016-Jul-14 (SBDB osculation epoch)

function physical(p: Partial<PhysicalData> & Pick<PhysicalData, 'radiusKm' | 'massKg' | 'rotationPeriodHours' | 'obliquityDeg'>): PhysicalData {
  return p as PhysicalData;
}

export const DWARF_PLANETS: BodyData[] = [
  {
    id: 'ceres',
    name: 'Ceres',
    nameZh: '谷神星',
    kind: 'dwarf-planet',
    color: 0x8f8a82,
    orbit: {
      epoch: EPOCH_SBDB_2023,
      a: 2.765552595034094,
      e: 0.07969229514816586,
      i: 10.58802780183462,
      L: 67.96218772678225,
      peri: 153.54284135064808,
      node: 80.24862682043221,
    },
    physical: physical({
      radiusKm: 469.7,
      massKg: 9.3835e20,
      densityKgM3: 2_162,
      gravityMs2: 0.28,
      escapeVelocityKms: 0.51,
      rotationPeriodHours: 9.074,
      obliquityDeg: 4,
      meanTempC: -105,
      albedo: 0.09,
      discovery: '1801 年 Giuseppe Piazzi',
      satellitesKnown: 0,
      pole: { raDeg: 291.418, decDeg: 66.764 },
    }),
    textures: { map: TEX.ceres },
    facts: [
      ['公转周期', '4.60 年'],
      ['小行星带质量占比', '约 25%'],
      ['成分', '岩石-冰混合，地下可能有水冰'],
      ['身份', '小行星带中唯一的矮行星'],
      ['谷神星亮点', 'Occator 撞击坑的盐类沉积'],
    ],
  },
  {
    id: 'pluto',
    name: 'Pluto',
    nameZh: '冥王星',
    kind: 'dwarf-planet',
    color: 0xc9b49f,
    orbit: {
      epoch: EPOCH_PLUTO_2016,
      a: 39.58862938517124,
      e: 0.2518378778576892,
      i: 17.14771140999114,
      L: 262.68504904334407,
      peri: 224.0013855701622,
      node: 110.2923840543057,
    },
    physical: physical({
      radiusKm: 1_188.3,
      massKg: 1.303e22,
      densityKgM3: 1_854,
      gravityMs2: 0.62,
      escapeVelocityKms: 1.21,
      rotationPeriodHours: 153.2928,
      obliquityDeg: 122.53,
      meanTempC: -229,
      albedo: 0.52,
      atmosphere: '稀薄的 N₂、CH₄、CO（表面气压约 1 Pa）',
      discovery: '1930 年 Clyde Tombaugh',
      satellitesKnown: 5,
      pole: { raDeg: 132.993, decDeg: -6.163 },
    }),
    facts: [
      ['公转周期', '247.94 年'],
      ['自转周期', '6.39 天（逆行，与 Charon 潮汐锁定）'],
      ['轨道共振', '与海王星 2:3 平均运动共振'],
      ['轨道倾角', '17.16°，偏心率 0.249'],
      ['冥王星-卡戎', '质心位于冥王星之外的双行星系统'],
      ['斯普特尼克高原', '新视野号 2015 年发现的巨大氮冰平原'],
    ],
  },
  {
    id: 'haumea',
    name: 'Haumea',
    nameZh: '妊神星',
    kind: 'dwarf-planet',
    color: 0xd8d4cf,
    orbit: {
      epoch: EPOCH_SBDB_2023,
      a: 43.06029023650952,
      e: 0.1944430148898797,
      i: 28.20847393040364,
      L: 225.68701526503855,
      peri: 362.4766033838086,
      node: 121.7860561329425,
    },
    physical: physical({
      radiusKm: 780,
      massKg: 4.006e21,
      densityKgM3: 2_600,
      gravityMs2: 0.4,
      escapeVelocityKms: 0.9,
      rotationPeriodHours: 3.915,
      obliquityDeg: 126,
      meanTempC: -241,
      albedo: 0.51,
      discovery: '2004 年（Caltech 团队 / Sierra Nevada 天文台）',
      satellitesKnown: 2,
    }),
    textures: { map: TEX.haumea },
    facts: [
      ['公转周期', '283.8 年'],
      ['自转周期', '3.92 小时（太阳系大天体中最快）'],
      ['形状', '1050 × 840 × 537 km 的椭球，因自转而拉长'],
      ['环', '2017 年发现一条环'],
      ['卫星', 'Hiʻiaka、Namaka'],
    ],
  },
  {
    id: 'makemake',
    name: 'Makemake',
    nameZh: '鸟神星',
    kind: 'dwarf-planet',
    color: 0xc8a48c,
    orbit: {
      epoch: EPOCH_SBDB_2023,
      a: 45.57093317300052,
      e: 0.1588889953992523,
      i: 29.02785603743067,
      L: 186.32510336548444,
      peri: 376.3871071606613,
      node: 79.2948338209406,
    },
    physical: physical({
      radiusKm: 715,
      massKg: 3.1e21,
      densityKgM3: 1_700,
      gravityMs2: 0.5,
      escapeVelocityKms: 0.8,
      rotationPeriodHours: 22.83,
      obliquityDeg: 0, // unknown; the pole orientation is poorly constrained
      meanTempC: -239,
      albedo: 0.81,
      discovery: '2005 年 Michael Brown 团队',
      satellitesKnown: 1,
    }),
    textures: { map: TEX.makemake },
    facts: [
      ['公转周期', '306.2 年'],
      ['自转周期', '22.83 小时'],
      ['表面', '甲烷与乙烷冰，呈红褐色'],
      ['卫星', 'MK 2（2016 年发现）'],
      ['转轴方向', '尚未精确测定'],
    ],
  },
  {
    id: 'eris',
    name: 'Eris',
    nameZh: '阋神星',
    kind: 'dwarf-planet',
    color: 0xd9d9dc,
    orbit: {
      epoch: EPOCH_SBDB_2023,
      a: 67.93394687853566,
      e: 0.4382385347971672,
      i: 43.9258279471791,
      L: 38.57412830321073,
      peri: 186.7996940282037,
      node: 36.00477044417249,
    },
    physical: physical({
      radiusKm: 1_163,
      massKg: 1.6466e22,
      densityKgM3: 2_340,
      gravityMs2: 0.82,
      escapeVelocityKms: 1.38,
      rotationPeriodHours: 378.96,
      obliquityDeg: 78,
      meanTempC: -231,
      albedo: 0.96,
      discovery: '2005 年 Michael Brown 团队',
      satellitesKnown: 1,
    }),
    textures: { map: TEX.eris },
    facts: [
      ['公转周期', '557.4 年'],
      ['自转周期', '15.79 天'],
      ['质量', '比冥王星重约 27%'],
      ['轨道半径', '平均 67.9 AU，远日点达 97.5 AU'],
      ['发现影响', '直接导致 IAU 2006 年重新定义"行星"'],
      ['卫星', 'Dysnomia'],
    ],
  },
];
