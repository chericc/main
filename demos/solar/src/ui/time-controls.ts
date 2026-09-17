import {
  SPEED_PRESETS,
  formatJD,
  jdToLocalInputValue,
  localInputValueToJD,
} from '../core/time';
import { button, el, section } from './dom';
import { formatSpeed } from './format';

const SLIDER_MAX = 1000;
const SPEED_MIN = 1; // real time
const SPEED_MAX = 1e9; // ~31.7 years per second

const sliderToSpeed = (v: number): number => Math.pow(10, (v / SLIDER_MAX) * 9);
const speedToSlider = (s: number): number => (Math.log10(Math.min(SPEED_MAX, Math.max(SPEED_MIN, s))) / 9) * SLIDER_MAX;

export interface TimeCallbacks {
  onTogglePause: () => void;
  onSpeed: (secondsPerSecond: number) => void;
  onDate: (jd: number) => void;
  onNow: () => void;
  onStep: (days: number) => void;
}

/** Playback controls: date, direction, speed and jump buttons. */
export class TimeControls {
  readonly element: HTMLElement;

  private readonly dateInput: HTMLInputElement;
  private readonly playButton: HTMLButtonElement;
  private readonly directionButton: HTMLButtonElement;
  private readonly speedInput: HTMLInputElement;
  private readonly speedReadout: HTMLElement;
  private readonly speedValue: HTMLElement;
  private readonly presetRow: HTMLElement;
  private reverse = false;
  private speed = 60;

  constructor(private readonly callbacks: TimeCallbacks) {
    const { root, body } = section('时间', true);
    this.element = root;

    // --- Playback row ------------------------------------------------------
    this.playButton = button('暂停', () => this.callbacks.onTogglePause(), 'primary');
    this.directionButton = button('正向 ▸', () => this.setReverse(!this.reverse), 'ghost');

    const nowButton = button('回到现在', () => this.callbacks.onNow(), 'ghost');

    body.append(
      el('div', { class: 'button-row' }, [this.playButton, this.directionButton, nowButton]),
    );

    // --- Date --------------------------------------------------------------
    this.dateInput = el('input', { type: 'datetime-local', class: 'date-input' }) as HTMLInputElement;
    this.dateInput.addEventListener('change', () => {
      if (this.dateInput.value) this.callbacks.onDate(localInputValueToJD(this.dateInput.value));
    });
    body.append(
      el('div', { class: 'control-row' }, [
        el('div', { class: 'control-head' }, [el('span', { class: 'control-label', text: '仿真时刻 (UTC)' })]),
        this.dateInput,
      ]),
    );

    // --- Steps -------------------------------------------------------------
    body.append(
      el('div', { class: 'button-row steps' }, [
        button('−1 年', () => this.callbacks.onStep(-365.25), 'ghost small'),
        button('−1 月', () => this.callbacks.onStep(-30.44), 'ghost small'),
        button('−1 天', () => this.callbacks.onStep(-1), 'ghost small'),
        button('+1 天', () => this.callbacks.onStep(1), 'ghost small'),
        button('+1 月', () => this.callbacks.onStep(30.44), 'ghost small'),
        button('+1 年', () => this.callbacks.onStep(365.25), 'ghost small'),
      ]),
    );

    // --- Speed -------------------------------------------------------------
    this.speedReadout = el('span', { class: 'readout' });
    this.speedValue = el('span', { class: 'speed-value', text: formatSpeed(this.speed) });
    this.speedInput = el('input', {
      type: 'range',
      min: '0',
      max: String(SLIDER_MAX),
      step: '1',
      value: String(speedToSlider(this.speed)),
      class: 'slider',
    }) as HTMLInputElement;
    this.speedInput.addEventListener('input', () => {
      this.speed = sliderToSpeed(Number(this.speedInput.value));
      this.emitSpeed();
    });

    body.append(
      el('div', { class: 'control-row' }, [
        el('div', { class: 'control-head' }, [
          el('span', { class: 'control-label', text: '时间流速' }),
          this.speedReadout,
        ]),
        this.speedInput,
        this.speedValue,
      ]),
    );

    this.presetRow = el('div', { class: 'chip-row' });
    for (const preset of SPEED_PRESETS) {
      const chip = button(preset.label, () => {
        this.speed = preset.seconds;
        this.speedInput.value = String(speedToSlider(this.speed));
        this.emitSpeed();
      }, 'chip');
      this.presetRow.append(chip);
    }
    body.append(this.presetRow);

    this.updateSpeedReadout();
  }

  private setReverse(reverse: boolean): void {
    this.reverse = reverse;
    this.directionButton.textContent = reverse ? '倒放 ◂' : '正向 ▸';
    this.emitSpeed();
  }

  private emitSpeed(): void {
    this.updateSpeedReadout();
    this.callbacks.onSpeed(this.reverse ? -this.speed : this.speed);
  }

  private updateSpeedReadout(): void {
    this.speedReadout.textContent = `${this.speed.toExponential(1)}×`;
    this.speedValue.textContent = formatSpeed(this.reverse ? -this.speed : this.speed);
    for (const chip of Array.from(this.presetRow.children) as HTMLElement[]) {
      void chip;
    }
  }

  /** Called by the app every frame (cheaply) to reflect external changes. */
  sync(jd: number, paused: boolean, speed: number): void {
    this.playButton.textContent = paused ? '播放' : '暂停';
    this.playButton.classList.toggle('is-paused', paused);

    const magnitude = Math.abs(speed);
    if (Math.abs(magnitude - this.speed) / this.speed > 1e-6) {
      this.speed = magnitude;
      this.speedInput.value = String(speedToSlider(magnitude));
      this.updateSpeedReadout();
    }

    const reverse = speed < 0;
    if (reverse !== this.reverse) {
      this.reverse = reverse;
      this.directionButton.textContent = reverse ? '倒放 ◂' : '正向 ▸';
      this.updateSpeedReadout();
    }

    if (document.activeElement !== this.dateInput) {
      const value = jdToLocalInputValue(jd);
      if (this.dateInput.value !== value) this.dateInput.value = value;
      this.dateInput.title = formatJD(jd);
    }
  }
}
