import { button, el, section, slider, toggleRow } from './dom';
import { formatDistanceAU } from './format';

const DIST_MIN_EXP = -8; // ~1.5 km
const DIST_MAX_EXP = Math.log10(600);

const sliderToDistance = (v: number) => Math.pow(10, DIST_MIN_EXP + (v / 1000) * (DIST_MAX_EXP - DIST_MIN_EXP));
const distanceToSlider = (d: number) =>
  ((Math.log10(Math.max(Math.pow(10, DIST_MIN_EXP), d)) - DIST_MIN_EXP) / (DIST_MAX_EXP - DIST_MIN_EXP)) * 1000;

export interface DisplayCallbacks {
  onToggleOrbits: (value: boolean) => void;
  onToggleLabels: (value: boolean) => void;
  onToggleBelts: (value: boolean) => void;
  onRadiusExaggeration: (value: number) => void;
  onDistance: (worldUnits: number) => void;
  onOverview: () => void;
}

/** Visibility toggles plus the two scale-related sliders. */
export class DisplayControls {
  readonly element: HTMLElement;

  private readonly distanceInput: HTMLInputElement;
  private readonly distanceSet: (v: number) => void;
  /** Suppresses the `input` handler while we write to the slider ourselves. */
  private syncing = false;
  private pending = 0;

  constructor(callbacks: DisplayCallbacks) {
    const { root, body } = section('显示与缩放', true);
    this.element = root;

    body.append(
      el('div', { class: 'toggle-list' }, [
        toggleRow('轨道线', true, callbacks.onToggleOrbits),
        toggleRow('天体标签', true, callbacks.onToggleLabels),
        toggleRow('小行星带 / 柯伊伯带', true, callbacks.onToggleBelts),
      ]),
    );

    const distance = slider({
      label: '相机距离（距焦点）',
      min: 0,
      max: 1000,
      step: 1,
      value: Math.round(distanceToSlider(55)),
      format: (v) => formatDistanceAU(sliderToDistance(v)),
      onInput: (v) => {
        if (!this.syncing) callbacks.onDistance(sliderToDistance(v));
      },
    });
    this.distanceInput = distance.input;
    this.distanceSet = distance.set;
    body.append(distance.root);

    body.append(
      slider({
        label: '天体半径视觉放大',
        min: 1,
        max: 1000,
        step: 1,
        value: 1,
        format: (v) => (v === 1 ? '1× (真实比例)' : `${v}× (仅视觉)`),
        onInput: (v) => callbacks.onRadiusExaggeration(v),
      }).root,
    );

    body.append(
      el('p', {
        class: 'note',
        text: '默认 1× 即完全真实比例。放大半径只改变球体大小，不改变轨道距离与位置。',
      }),
    );

    body.append(
      el('div', { class: 'button-row' }, [button('回到全景视角', () => callbacks.onOverview(), 'ghost')]),
    );
  }

  /** Keeps the distance slider in sync with wheel zoom (throttled by the app). */
  syncDistance(worldUnits: number): void {
    const next = Math.round(distanceToSlider(worldUnits));
    if (next === this.pending) return;
    this.pending = next;
    this.syncing = true;
    this.distanceSet(distanceToSlider(next));
    this.distanceInput.value = String(next);
    this.syncing = false;
  }
}
