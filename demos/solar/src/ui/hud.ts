import { formatJD } from '../core/time';
import { el } from './dom';
import { formatDistanceAU, formatSpeed } from './format';

/** Translucent overlay with the epoch, playback speed and camera readout. */
export class Hud {
  readonly element: HTMLElement;
  private readonly dateEl: HTMLElement;
  private readonly speedEl: HTMLElement;
  private readonly focusEl: HTMLElement;
  private readonly distanceEl: HTMLElement;

  constructor() {
    this.dateEl = el('span', { class: 'hud-value', text: '—' });
    this.speedEl = el('span', { class: 'hud-value', text: '—' });
    this.focusEl = el('span', { class: 'hud-value', text: '太阳系全景' });
    this.distanceEl = el('span', { class: 'hud-value', text: '—' });

    this.element = el('div', { class: 'hud' }, [
      el('div', { class: 'hud-item' }, [el('span', { class: 'hud-label', text: '时刻' }), this.dateEl]),
      el('div', { class: 'hud-item' }, [el('span', { class: 'hud-label', text: '流速' }), this.speedEl]),
      el('div', { class: 'hud-item' }, [el('span', { class: 'hud-label', text: '焦点' }), this.focusEl]),
      el('div', { class: 'hud-item' }, [el('span', { class: 'hud-label', text: '视距' }), this.distanceEl]),
      el('div', { class: 'hud-hint', text: '拖拽旋转 · 滚轮缩放 · 点击天体聚焦 · 空格暂停 · [ ] 切换天体' }),
    ]);
  }

  update(jd: number, speed: number, paused: boolean, focusLabel: string, cameraDistance: number): void {
    this.dateEl.textContent = formatJD(jd);
    this.speedEl.textContent = paused ? '已暂停' : formatSpeed(speed);
    this.focusEl.textContent = focusLabel;
    this.distanceEl.textContent = formatDistanceAU(cameraDistance);
  }
}
