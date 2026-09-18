import * as THREE from 'three';
import { ALL_BODIES, findBody, KIND_LABELS, type SolarBody } from './data';
import { RAD } from './core/constants';
import { dateToJD, SimulationClock } from './core/time';
import { SolarSystemScene } from './scene/solar-system-scene';
import { TextureLibrary } from './scene/textures';
import { BodyList } from './ui/body-list';
import { DisplayControls } from './ui/display-controls';
import { Hud } from './ui/hud';
import { InfoPanel } from './ui/info-panel';
import { TimeControls } from './ui/time-controls';
import { el, section } from './ui/dom';

export interface AppHosts {
  canvasHost: HTMLElement;
  labelHost: HTMLElement;
  sidebar: HTMLElement;
  infoHost: HTMLElement;
  hudHost: HTMLElement;
}

export interface AppCallbacks {
  onProgress?: (loaded: number, total: number) => void;
}

/**
 * Refresh interval (ms) for a fully static scene — clock paused and camera at
 * rest. At true scale a still frame is just a still frame, so there is nothing
 * to gain from repainting at the display rate; ~1 fps keeps the DOM readouts
 * alive (and picks up late layout) at a tiny fraction of the cost.
 */
const IDLE_RENDER_MS = 1000;

/** Every texture the scene may reference (star background included). */
function collectTexturePaths(): string[] {
  const paths = new Set<string>(['textures/stars-milkyway.jpg']);
  for (const body of ALL_BODIES) {
    const set = body.textures;
    if (set) {
      for (const p of [set.map, set.nightMap, set.cloudsMap]) if (p) paths.add(p);
    }
    if (body.ring?.texture) paths.add(body.ring.texture);
  }
  return [...paths];
}

export class App {
  private readonly scene: SolarSystemScene;
  private readonly clock = new SimulationClock();
  private readonly bodyList: BodyList;
  private readonly timeControls: TimeControls;
  private readonly displayControls: DisplayControls;
  private readonly infoPanel: InfoPanel;
  private readonly hud: Hud;

  private selected: string | null = null;
  private infoTimer = 0;
  private syncTimer = 0;
  private raf = 0;
  private readonly viewport: HTMLElement;
  /** Set when something outside the render loop changes the scene. */
  private dirty = true;
  /** True while the camera is still moving (flight / damping / follow). */
  private animating = false;
  private lastRender = 0;

  constructor(hosts: AppHosts, textures: TextureLibrary) {
    this.scene = new SolarSystemScene({
      canvasHost: hosts.canvasHost,
      labelHost: hosts.labelHost,
      viewportHost: hosts.canvasHost.parentElement ?? hosts.canvasHost,
      textures,
      onSelect: (id) => this.select(id),
    });
    this.scene.setJDProvider(() => this.clock.jd);

    // --- UI ---------------------------------------------------------------
    this.bodyList = new BodyList((id) => this.select(id));
    this.timeControls = new TimeControls({
      onTogglePause: () => {
        this.clock.paused = !this.clock.paused;
        this.markDirty();
      },
      onSpeed: (value) => {
        this.clock.speed = value;
        this.markDirty();
      },
      onDate: (jd) => {
        this.clock.jd = jd;
        this.scene.model.update(jd);
        this.refreshInfo(true);
        this.markDirty();
      },
      onNow: () => {
        this.clock.jd = dateToJD(new Date());
        this.refreshInfo(true);
        this.markDirty();
      },
      onStep: (days) => {
        this.clock.jd += days;
        this.refreshInfo(true);
        this.markDirty();
      },
    });

    this.displayControls = new DisplayControls({
      onToggleOrbits: (v) => {
        this.scene.setOrbitsVisible(v);
        this.markDirty();
      },
      onToggleLabels: (v) => {
        this.scene.setLabelsVisible(v);
        this.markDirty();
      },
      onToggleBelts: (v) => {
        this.scene.setBeltsVisible(v);
        this.markDirty();
      },
      onRadiusExaggeration: (v) => {
        this.scene.setRadiusExaggeration(v);
        this.markDirty();
      },
      onDistance: (v) => {
        this.scene.camera.setDistance(v);
        this.markDirty();
      },
      onOverview: () => this.overview(),
    });

    this.infoPanel = new InfoPanel();
    this.hud = new Hud();

    hosts.sidebar.append(this.buildBrand(), this.timeControls.element, this.bodyList.element, this.displayControls.element, this.buildAbout());
    hosts.infoHost.append(this.infoPanel.element);
    hosts.hudHost.append(this.hud.element);

    this.infoPanel.show(null);
    this.scene.overview(true);
    this.clock.jd = dateToJD(new Date());
    this.scene.model.update(this.clock.jd);

    // Deep link: `#saturn` (or any body id) starts focused on that body.
    const hash = decodeURIComponent(location.hash.replace(/^#/, ''));
    if (hash && findBody(hash)) this.select(hash, true);

    window.addEventListener('keydown', this.onKeyDown);
    window.addEventListener('resize', this.onResize);
    window.addEventListener('hashchange', this.onHashChange);

    // Pointer and wheel activity must wake the idle renderer immediately,
    // otherwise a drag would not reach the screen until the next 1 fps tick.
    this.viewport = hosts.canvasHost.parentElement ?? hosts.canvasHost;
    this.viewport.addEventListener('pointerdown', this.onActivity, { passive: true });
    this.viewport.addEventListener('pointermove', this.onPointerMoveActivity, { passive: true });
    this.viewport.addEventListener('wheel', this.onActivity, { passive: true });
    this.onResize();

    this.raf = requestAnimationFrame(this.loop);
  }

  private buildBrand(): HTMLElement {
    return el('header', { class: 'brand' }, [
      el('h1', { class: 'brand-title', html: 'Solar<span>.</span>' }),
      el('p', { class: 'brand-sub', text: '真实比例太阳系模拟 · Real-scale Solar System' }),
    ]);
  }

  private buildAbout(): HTMLElement {
    const { root, body } = section('关于数据', false);
    body.append(
      Object.assign(document.createElement('p'), {
        className: 'note',
        textContent:
          '轨道要素来自 JPL（行星：Standish & Williams 1992；矮行星/小行星/彗星：Small-Body Database；卫星：Planetary Satellite Mean Elements）；' +
          '物理数据来自 NASA Planetary Fact Sheet 与 IAU WGCCRE 报告。原始下载数据保存在 reference/ 目录。',
      }),
    );
    return root;
  }

  /** Focus a body and update the side panels. */
  select(id: string, instant = false): void {
    this.selected = id;
    this.scene.focus(id, instant);
    this.bodyList.setActive(id);
    this.refreshInfo(true);
    this.markDirty();
    if (decodeURIComponent(location.hash.replace(/^#/, '')) !== id) {
      history.replaceState(null, '', `#${id}`);
    }
  }

  private onHashChange = (): void => {
    const id = decodeURIComponent(location.hash.replace(/^#/, ''));
    if (id && findBody(id) && id !== this.selected) this.select(id);
  };

  overview(): void {
    this.selected = null;
    this.scene.overview();
    this.bodyList.setActive(null);
    this.infoPanel.show(null);
    this.markDirty();
    if (location.hash) history.replaceState(null, '', location.pathname + location.search);
  }

  private onKeyDown = (event: KeyboardEvent): void => {
    const target = event.target as HTMLElement | null;
    if (target && (target.tagName === 'INPUT' || target.tagName === 'TEXTAREA')) return;
    // Never shadow browser shortcuts such as Ctrl/Cmd + "+/-" (page zoom).
    if (event.ctrlKey || event.metaKey || event.altKey) return;

    switch (event.key) {
      case ' ':
        event.preventDefault();
        this.clock.paused = !this.clock.paused;
        this.markDirty();
        break;
      case 'Escape':
      case '0':
        this.overview();
        break;
      case '[':
      case ']': {
        const index = ALL_BODIES.findIndex((b) => b.id === this.selected);
        const delta = event.key === ']' ? 1 : -1;
        const next = ALL_BODIES[(index + delta + ALL_BODIES.length) % ALL_BODIES.length];
        this.select(next.id);
        break;
      }
      case '+':
      case '=':
        this.scene.camera.setDistance(this.scene.camera.distance / 1.6);
        this.markDirty();
        break;
      case '-':
      case '_':
        this.scene.camera.setDistance(this.scene.camera.distance * 1.6);
        this.markDirty();
        break;
      default:
        break;
    }
  };

  private onResize = (): void => {
    this.scene.resize();
    this.markDirty();
  };

  private markDirty(): void {
    this.dirty = true;
  }

  private onActivity = (): void => {
    this.dirty = true;
  };

  private onPointerMoveActivity = (event: PointerEvent): void => {
    // Hovering alone must not keep the renderer awake — only drags.
    if (event.buttons !== 0) this.dirty = true;
  };

  private refreshInfo(immediate = false): void {
    if (immediate) this.infoTimer = 1;
  }

  private updateInfoPanel(): void {
    const body: SolarBody | null = this.selected ? findBody(this.selected) ?? null : null;
    if (!body) return;

    const state = this.scene.model.state(body.id);
    const p = state.position;
    const r = Math.hypot(p.x, p.y, p.z);
    const earth = this.scene.model.state('earth').position;
    const fromEarth = Math.hypot(p.x - earth.x, p.y - earth.y, p.z - earth.z);

    this.infoPanel.setLive({
      heliocentricAU: r,
      longitudeDeg: ((Math.atan2(p.y, p.x) * RAD + 360) % 360),
      latitudeDeg: r === 0 ? 0 : Math.asin(p.z / r) * RAD,
      fromEarthAU: body.id === 'earth' ? 0 : fromEarth,
      fromCameraAU: this.scene.camera.camera.position.distanceTo(this.scene.worldPosition(body.id)),
      speedKms: this.scene.model.velocityKms(body, this.clock.jd),
      spinDeg: (state.spinRad * RAD + 360) % 360,
    });
  }

  private loop = (): void => {
    this.raf = requestAnimationFrame(this.loop);

    const now = performance.now();
    const dt = Math.min(0.1, (now - this.lastFrame) / 1000 || 0);
    this.lastFrame = now;

    // A paused clock with a settled camera is a static image: there is no
    // reason to repaint at the display rate. Idle drops to ~1 fps; any input
    // (`dirty`), camera motion (`animating`) or a running clock resumes full
    // rate on the very next frame.
    const running = !this.clock.paused && this.clock.speed !== 0;
    if (!running && !this.dirty && !this.animating && now - this.lastRender < IDLE_RENDER_MS) {
      return;
    }

    this.lastRender = now;
    this.dirty = false;
    this.clock.advance(dt);
    this.animating = this.scene.render();

    this.syncTimer += dt;
    if (this.syncTimer >= 0.1) {
      this.syncTimer = 0;
      this.timeControls.sync(this.clock.jd, this.clock.paused, this.clock.speed);
      this.displayControls.syncDistance(this.scene.camera.distance);
    }

    this.infoTimer += dt;
    if (this.infoTimer >= 0.25) {
      this.infoTimer = 0;
      this.updateInfoPanel();
    }

    const focusBody = this.selected ? findBody(this.selected) : undefined;
    this.hud.update(
      this.clock.jd,
      this.clock.speed,
      this.clock.paused,
      focusBody ? `${focusBody.nameZh} ${focusBody.name} · ${KIND_LABELS[focusBody.kind]}` : '太阳系全景',
      this.scene.camera.distance,
    );
  }

  private lastFrame = performance.now();

  dispose(): void {
    cancelAnimationFrame(this.raf);
    window.removeEventListener('keydown', this.onKeyDown);
    window.removeEventListener('resize', this.onResize);
    window.removeEventListener('hashchange', this.onHashChange);
    this.viewport.removeEventListener('pointerdown', this.onActivity);
    this.viewport.removeEventListener('pointermove', this.onPointerMoveActivity);
    this.viewport.removeEventListener('wheel', this.onActivity);
    this.scene.dispose();
  }
}

/** Boots the whole application. */
export async function createApp(hosts: AppHosts, callbacks: AppCallbacks = {}): Promise<App> {
  const textures = new TextureLibrary();
  const paths = collectTexturePaths();

  let loaded = 0;
  const manager = new THREE.LoadingManager();
  manager.onProgress = () => {
    loaded = Math.min(paths.length, loaded + 1);
    callbacks.onProgress?.(loaded, paths.length);
  };

  await textures.preload(paths, manager);
  callbacks.onProgress?.(paths.length, paths.length);

  const app = new App(hosts, textures);
  return app;
}
