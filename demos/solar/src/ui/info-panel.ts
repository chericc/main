import { EARTH_MASS_KG, EARTH_RADIUS_KM } from '../core/constants';
import { orbitalPeriodDays } from '../core/kepler';
import { formatDurationDays } from '../core/time';
import { ALL_BODIES, KIND_LABELS, type SolarBody } from '../data';
import { el } from './dom';
import {
  formatAU,
  formatDeg,
  formatDistanceAU,
  formatEarthRatio,
  formatKm,
  formatMass,
  formatRotation,
  formatTemp,
} from './format';

export interface LiveInfo {
  heliocentricAU: number;
  longitudeDeg: number;
  latitudeDeg: number;
  fromEarthAU: number;
  fromCameraAU: number;
  /** Heliocentric (or parent-relative) speed in km/s. */
  speedKms: number;
  /** Rotation phase is not modelled — reported for transparency. */
  spinDeg: number;
}

function stat(label: string, value: string, extra?: string): HTMLElement {
  return el('div', { class: 'stat' }, [
    el('span', { class: 'stat-label', text: label }),
    el('span', { class: 'stat-value', text: value }),
    ...(extra ? [el('span', { class: 'stat-extra', text: extra })] : []),
  ]);
}

/** Right-hand panel with the real, sourced data for the selected body. */
export class InfoPanel {
  readonly element: HTMLElement;
  private readonly titleEl: HTMLElement;
  private readonly subtitleEl: HTMLElement;
  private readonly bodyEl: HTMLElement;
  private live: LiveInfo | null = null;

  constructor() {
    this.titleEl = el('h2', { class: 'info-title', text: '太阳系' });
    this.subtitleEl = el('div', { class: 'info-subtitle', text: '真实比例 · 真实轨道要素' });
    this.bodyEl = el('div', { class: 'info-body' });
    this.element = el('aside', { class: 'info-panel' }, [
      el('header', { class: 'info-header' }, [
        this.titleEl,
        this.subtitleEl,
      ]),
      this.bodyEl,
    ]);
  }

  setLive(live: LiveInfo | null): void {
    this.live = live;
    const liveSection = this.bodyEl.querySelector('.live-section');
    if (liveSection && live) this.renderLive(liveSection as HTMLElement, live);
  }

  show(body: SolarBody | null): void {
    if (!body) {
      this.titleEl.textContent = '太阳系';
      this.subtitleEl.textContent = '真实比例 · 真实轨道要素';
      this.bodyEl.replaceChildren(this.renderOverview());
      return;
    }

    this.titleEl.textContent = `${body.nameZh} ${body.name}`;
    this.subtitleEl.textContent = body.parent ? `${KIND_LABELS[body.kind]} · 环绕${body.parent.nameZh}` : KIND_LABELS[body.kind];

    const phys = body.physical;
    const fragments: HTMLElement[] = [];

    fragments.push(
      el('div', { class: 'stat-grid' }, [
        stat('平均半径', formatKm(phys.radiusKm), `${formatEarthRatio(phys.radiusKm, EARTH_RADIUS_KM)}地球`),
        stat('质量', formatMass(phys.massKg), phys.massKg ? `${formatEarthRatio(phys.massKg, EARTH_MASS_KG)}地球` : ''),
        stat('平均密度', phys.densityKgM3 ? `${phys.densityKgM3} kg/m³` : '—', phys.densityKgM3 ? `水 ${(phys.densityKgM3 / 1000).toFixed(2)} 倍` : ''),
        stat('表面重力', phys.gravityMs2 ? `${phys.gravityMs2} m/s²` : '—'),
        stat('逃逸速度', phys.escapeVelocityKms ? `${phys.escapeVelocityKms} km/s` : '—'),
        stat('反照率', phys.albedo !== undefined ? phys.albedo.toFixed(3) : '—'),
        stat('自转周期', formatRotation(phys.rotationPeriodHours), phys.obliquityDeg >= 90 ? '逆行' : ''),
        stat('转轴倾角', formatDeg(phys.obliquityDeg, 2)),
        stat('平均温度', formatTemp(phys.meanTempC)),
      ]),
    );

    if (body.orbit) {
      const period = orbitalPeriodDays(body.orbit);
      const rows: HTMLElement[] = [
        stat('半长轴 a', formatAU(body.orbit.a)),
        stat('偏心率 e', body.orbit.e.toFixed(5)),
        stat('轨道倾角 i', formatDeg(body.orbit.i, 3)),
        stat('升交点 Ω', formatDeg(body.orbit.node, 2), body.parent ? '相对母星赤道' : '相对 J2000 黄道'),
        stat('近日点', formatAU(body.orbit.a * (1 - body.orbit.e))),
        stat('远日点', formatAU(body.orbit.a * (1 + body.orbit.e))),
        stat('轨道周期', formatDurationDays(period)),
        ...(body.orbit.rates
          ? [stat('要素历元', 'J2000.0 (JD 2451545.0)', '含世纪变率')]
          : [stat('要素历元', `JD ${body.orbit.epoch}`, '瞬时要素')]),
      ];
      fragments.push(el('div', { class: 'subsection' }, [el('h3', { text: '轨道' }), el('div', { class: 'stat-grid' }, rows)]));
    }

    if (phys.atmosphere) {
      fragments.push(el('div', { class: 'subsection' }, [el('h3', { text: '大气' }), el('p', { class: 'prose', text: phys.atmosphere })]));
    }

    if (phys.discovery) {
      fragments.push(el('div', { class: 'subsection' }, [el('h3', { text: '发现' }), el('p', { class: 'prose', text: phys.discovery })]));
    }

    if (phys.satellitesKnown !== undefined) {
      const rendered = body.children.length;
      fragments.push(
        el('div', { class: 'subsection' }, [
          el('h3', { text: '卫星' }),
          el('p', {
            class: 'prose',
            text:
              body.kind === 'star'
                ? `已确认行星 ${phys.satellitesKnown} 颗`
                : `已确认 ${phys.satellitesKnown} 颗${rendered ? `，本模拟渲染其中 ${rendered} 颗` : ''}`,
          }),
        ]),
      );
    }

    if (body.facts?.length) {
      fragments.push(
        el('div', { class: 'subsection' }, [
          el('h3', { text: '要点' }),
          el('div', { class: 'fact-list' }, body.facts.map(([label, value]) =>
            el('div', { class: 'fact' }, [
              el('span', { class: 'fact-label', text: label }),
              el('span', { class: 'fact-value', text: value || '—' }),
            ]),
          )),
        ]),
      );
    }

    const live = el('div', { class: 'subsection live-section' });
    fragments.push(live);

    this.bodyEl.replaceChildren(...fragments);
    if (this.live) this.renderLive(live, this.live);
    else this.renderLive(live, null);
  }

  private renderLive(container: HTMLElement, live: LiveInfo | null): void {
    if (!live) {
      container.replaceChildren(el('h3', { text: '当前状态' }), el('p', { class: 'prose muted', text: '计算中…' }));
      return;
    }
    container.replaceChildren(
      el('h3', { text: '当前状态' }),
      el('div', { class: 'stat-grid' }, [
        stat('日心距', formatDistanceAU(live.heliocentricAU), formatAU(live.heliocentricAU)),
        stat('黄经 / 黄纬', `${live.longitudeDeg.toFixed(3)}° / ${live.latitudeDeg.toFixed(3)}°`),
        stat('距地球', formatDistanceAU(live.fromEarthAU)),
        stat('轨道速度', `${live.speedKms.toFixed(3)} km/s`),
        stat('距相机', formatDistanceAU(live.fromCameraAU)),
      ]),
    );
  }

  private renderOverview(): HTMLElement {
    return el('div', { class: 'overview' }, [
      el('p', {
        class: 'prose',
        text:
          '本模拟按真实比例渲染：1 个世界单位 = 1 AU，天体半径与轨道距离均为真实值，' +
          '位置由 J2000 轨道要素通过开普勒方程实时求解。',
      }),
      el('p', {
        class: 'prose muted',
        text:
          '由于真实比例下外行星在屏幕上往往不足一个像素，请使用左侧列表或视图中的标签定位天体，' +
          '再用滚轮或右下角的距离滑块缩放。',
      }),
      el('div', { class: 'stat-grid' }, [
        stat('渲染天体总数', String(ALL_BODIES.length), `含 ${ALL_BODIES.filter((b) => b.kind === 'moon').length} 颗卫星`),
        stat('参考历元', 'J2000.0', 'JD 2451545.0'),
        stat('行星要素来源', 'JPL Standish & Williams (1992)', '1800–2050 AD'),
        stat('卫星要素来源', 'JPL Planetary Satellite Mean Elements', ''),
        stat('小行星 / 彗星', 'JPL Small-Body Database', '瞬时要素'),
      ]),
    ]);
  }
}
