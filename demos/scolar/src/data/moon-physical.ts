/**
 * Physical and visual data for the rendered natural satellites.
 *
 * RADII / MASSES: NASA/JPL planetary satellite physical parameters
 *   https://ssd.jpl.nasa.gov/sats/phys_par/
 * COLOURS: representative disk-integrated colours from the published imagery
 *   (used to tint the procedurally generated maps).
 */
export interface MoonPhysical {
  /** Volumetric mean radius, km. */
  radiusKm: number;
  /** Mass, kg. */
  massKg: number;
  /** Representative colour. */
  color: number;
  facts?: Array<[string, string]>;
}

export const MOON_PHYSICAL: Record<string, MoonPhysical> = {
  Moon: {
    radiusKm: 1_737.4,
    massKg: 7.342e22,
    color: 0x9a9a9a,
    facts: [
      ['公转周期', '27.32 天（恒星月）'],
      ['半长轴', '384 400 km'],
      ['潮汐锁定', '始终以同一面朝向地球'],
      ['远离速率', '3.8 cm/年'],
      ['形成', '约 45 亿年前的大碰撞'],
    ],
  },
  Phobos: {
    radiusKm: 11.267,
    massKg: 1.0659e16,
    color: 0x8a7f72,
    facts: [
      ['公转周期', '7 小时 39 分'],
      ['轨道高度', '距火星表面约 6 000 km'],
      ['轨道衰减', '每 100 年下降 1.8 m，约 5 000 万年后解体'],
      ['形状', '不规则，最大直径 27 km'],
    ],
  },
  Deimos: { radiusKm: 6.2, massKg: 1.4762e15, color: 0x9c9184 },
  Io: {
    radiusKm: 1_821.6,
    massKg: 8.9319e22,
    color: 0xd9c46a,
    facts: [
      ['公转周期', '1.77 天'],
      ['火山', '太阳系火山活动最剧烈的天体，400 余座活火山'],
      ['潮汐加热', '与木星及伽利略卫星的轨道共振产生'],
      ['表面', '硫与二氧化硫霜覆盖'],
    ],
  },
  Europa: {
    radiusKm: 1_560.8,
    massKg: 4.7998e22,
    color: 0xd8cfc0,
    facts: [
      ['公转周期', '3.55 天'],
      ['地下海洋', '冰壳下 60–150 km 深处存在液态水海洋'],
      ['表面', '最光滑的固态天体，冰壳厚 15–25 km'],
      ['探测', 'NASA Europa Clipper（2024 年发射）'],
    ],
  },
  Ganymede: {
    radiusKm: 2_634.1,
    massKg: 1.4819e23,
    color: 0x9a8f80,
    facts: [
      ['公转周期', '7.15 天'],
      ['太阳系最大卫星', '直径 5 268 km，比水星还大'],
      ['磁场', '唯一拥有自身磁场的卫星'],
    ],
  },
  Callisto: {
    radiusKm: 2_410.3,
    massKg: 1.0759e23,
    color: 0x6f6a60,
    facts: [
      ['公转周期', '16.69 天'],
      ['表面', '太阳系撞击坑最密集的地表'],
      ['轨道', '位于木星强辐射带之外，适合未来载人基地'],
    ],
  },
  Mimas: { radiusKm: 198.2, massKg: 3.7493e19, color: 0xbdbdbd },
  Enceladus: {
    radiusKm: 252.1,
    massKg: 1.08022e20,
    color: 0xe8eef0,
    facts: [
      ['公转周期', '1.37 天'],
      ['冰喷泉', '南极虎纹区喷出含有机物的水汽羽流'],
      ['反照率', '0.99，太阳系最亮的天体之一'],
      ['E 环', '由 Enceladus 喷发出的物质形成'],
    ],
  },
  Tethys: { radiusKm: 531.1, massKg: 6.17449e20, color: 0xc9cfd2 },
  Dione: { radiusKm: 561.4, massKg: 1.095452e21, color: 0xbfc4c7 },
  Rhea: { radiusKm: 763.8, massKg: 2.306518e21, color: 0xb4b8bb },
  Titan: {
    radiusKm: 2_574.7,
    massKg: 1.3452e23,
    color: 0xd9a34e,
    facts: [
      ['公转周期', '15.95 天'],
      ['大气', '1.45 bar 的氮气大气，太阳系卫星中唯一'],
      ['液体', '极区存在甲烷-乙烷湖泊与河流'],
      ['探测', 'Cassini-Huygens（2005 年着陆）'],
    ],
  },
  Hyperion: { radiusKm: 135.0, massKg: 5.6199e18, color: 0xa89f8e },
  Iapetus: {
    radiusKm: 734.5,
    massKg: 1.805635e21,
    color: 0x8a8478,
    facts: [
      ['公转周期', '79.33 天'],
      ['双色表面', '前导半球暗如煤（反照率 0.05），后随半球亮如雪（0.6）'],
      ['赤道脊', '高 13 km、长 1 300 km 的奇特山脊'],
    ],
  },
  Miranda: {
    radiusKm: 235.8,
    massKg: 6.59e19,
    color: 0xa9a9a9,
    facts: [
      ['公转周期', '1.41 天'],
      ['地貌', 'Verona Rupes 断崖高达 20 km，太阳系最高悬崖'],
    ],
  },
  Ariel: { radiusKm: 578.9, massKg: 1.353e21, color: 0xb5b5b5 },
  Umbriel: { radiusKm: 584.7, massKg: 1.172e21, color: 0x8c8c8c },
  Titania: { radiusKm: 788.4, massKg: 3.527e21, color: 0xa8a8a8 },
  Oberon: { radiusKm: 761.4, massKg: 3.014e21, color: 0x9a9a9a },
  Triton: {
    radiusKm: 1_353.4,
    massKg: 2.139e22,
    color: 0xd6d3cc,
    facts: [
      ['公转周期', '5.88 天（逆行）'],
      ['轨道倾角', '157°，太阳系唯一大逆行卫星'],
      ['氮气喷泉', '表面温度 -235 °C，存在氮气间歇泉'],
      ['潮汐演化', '正在向海王星螺旋靠近，最终将被撕裂'],
    ],
  },
  Proteus: { radiusKm: 210, massKg: 4.4e19, color: 0x8b8b8b },
  Charon: {
    radiusKm: 606.0,
    massKg: 1.586e21,
    color: 0xa8a8a8,
    facts: [
      ['公转周期', '6.39 天（与冥王星互相潮汐锁定）'],
      ['质心', '位于冥王星表面之外，构成双矮行星系统'],
      ['北极红斑', 'Mordor Macula，红色的托林沉积'],
    ],
  },
};
