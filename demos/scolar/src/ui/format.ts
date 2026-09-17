import { AU_KM } from '../core/constants';
import { formatDurationDays } from '../core/time';

const nf = (digits: number) => new Intl.NumberFormat('zh-CN', { maximumFractionDigits: digits, minimumFractionDigits: 0 });

/** `12 345 km` */
export function formatKm(km: number): string {
  const abs = Math.abs(km);
  if (abs < 1) return `${km.toFixed(3)} km`;
  if (abs < 1000) return `${km.toFixed(1)} km`;
  return `${nf(abs < 100_000 ? 0 : 0).format(km)} km`;
}

/** `1.5237 AU` */
export function formatAU(au: number): string {
  const abs = Math.abs(au);
  if (abs < 0.001) return `${(au * AU_KM).toFixed(0)} km`;
  if (abs < 1) return `${au.toFixed(4)} AU`;
  if (abs < 100) return `${au.toFixed(3)} AU`;
  return `${au.toFixed(1)} AU`;
}

/** Chooses the most readable unit for a distance in AU. */
export function formatDistanceAU(au: number): string {
  const km = au * AU_KM;
  if (Math.abs(km) < 1e6) return `${nf(0).format(km)} km`;
  return formatAU(au);
}

export function formatMass(kg: number): string {
  if (!kg) return '—';
  if (kg >= 1e27) return `${(kg / 1e27).toFixed(3)} × 10²⁷ kg`;
  if (kg >= 1e24) return `${(kg / 1e24).toFixed(3)} × 10²⁴ kg`;
  if (kg >= 1e21) return `${(kg / 1e21).toFixed(3)} × 10²¹ kg`;
  if (kg >= 1e18) return `${(kg / 1e18).toFixed(3)} × 10¹⁸ kg`;
  if (kg >= 1e15) return `${(kg / 1e15).toFixed(3)} × 10¹⁵ kg`;
  return `${kg.toExponential(3)} kg`;
}

export function formatEarthRatio(value: number, reference: number): string {
  if (!reference) return '';
  const r = value / reference;
  if (r >= 1000) return `${r.toExponential(2)} ×`;
  if (r >= 10) return `${r.toFixed(1)} ×`;
  if (r >= 0.1) return `${r.toFixed(3)} ×`;
  return `${r.toExponential(2)} ×`;
}

/** Simulated seconds per real second -> `1 天/秒` style label. */
export function formatSpeed(secondsPerSecond: number): string {
  const sign = secondsPerSecond < 0 ? '−' : '';
  const s = Math.abs(secondsPerSecond);
  if (s < 60) return `${sign}实时 (${s.toFixed(2)}×)`;
  if (s < 3600) return `${sign}${(s / 60).toFixed(1)} 分/秒`;
  if (s < 86_400) return `${sign}${(s / 3600).toFixed(2)} 时/秒`;
  if (s < 2_592_000) return `${sign}${(s / 86_400).toFixed(2)} 天/秒`;
  if (s < 31_557_600) return `${sign}${(s / 2_592_000).toFixed(2)} 月/秒`;
  return `${sign}${(s / 31_557_600).toFixed(2)} 年/秒`;
}

/** Rotation period in hours -> readable text. */
export function formatRotation(hours: number): string {
  if (!hours) return '—';
  if (Math.abs(hours) < 48) return `${hours.toFixed(4)} 小时`;
  return `${(hours / 24).toFixed(2)} 天 (${formatDurationDays(hours / 24)})`;
}

export function formatDeg(deg: number, digits = 4): string {
  return `${deg.toFixed(digits)}°`;
}

export function formatTemp(celsius: number | undefined): string {
  if (celsius === undefined) return '—';
  return `${celsius} °C (${celsius + 273.15} K)`;
}
