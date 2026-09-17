import { SOLAR_SYSTEM, type SolarBody } from '../data';
import { formatDistanceAU } from './format';
import { el } from './dom';

const GROUPS: Array<[string, (body: SolarBody) => boolean]> = [
  ['恒星', (b) => b.kind === 'star'],
  ['行星', (b) => b.kind === 'planet'],
  ['矮行星', (b) => b.kind === 'dwarf-planet'],
  ['小行星', (b) => b.kind === 'asteroid'],
  ['彗星', (b) => b.kind === 'comet'],
];

/**
 * Sidebar navigator. Clicking a row flies the camera to the body; this is the
 * primary way to locate objects at true scale, where most of the system is
 * sub-pixel.
 */
export class BodyList {
  readonly element: HTMLElement;
  private readonly rows = new Map<string, HTMLElement>();
  private readonly filterInput: HTMLInputElement;

  constructor(private readonly onSelect: (id: string) => void) {
    this.filterInput = el('input', {
      type: 'search',
      class: 'filter-input',
      placeholder: '搜索天体…',
    }) as HTMLInputElement;
    this.filterInput.addEventListener('input', () => this.applyFilter());

    this.element = el('div', { class: 'body-list' }, [
      this.filterInput,
      el('div', { class: 'hint', text: '点击任意天体即可飞抵并跟随；也可直接点击视图中的标签。' }),
    ]);

    for (const [title, predicate] of GROUPS) {
      const members = SOLAR_SYSTEM.filter(predicate);
      if (!members.length) continue;
      const group = el('div', { class: 'list-group' }, [el('div', { class: 'list-group-title', text: title })]);
      for (const body of members) {
        group.append(this.buildRow(body, 0));
        for (const moon of body.children) group.append(this.buildRow(moon, 1));
      }
      this.element.append(group);
    }
  }

  private buildRow(body: SolarBody, depth: number): HTMLElement {
    const dot = el('span', { class: 'list-dot', style: `--dot:#${body.color.toString(16).padStart(6, '0')}` });
    const meta =
      body.kind === 'moon'
        ? `${body.physical.radiusKm.toFixed(0)} km`
        : body.orbit
          ? formatDistanceAU(body.orbit.a)
          : '';

    const row = el('button', {
      class: `list-row depth-${depth}`,
      type: 'button',
      'data-id': body.id,
      'data-search': `${body.name} ${body.nameZh} ${body.id}`.toLowerCase(),
    }, [
      dot,
      el('span', { class: 'list-name', text: body.nameZh }),
      el('span', { class: 'list-sub', text: body.name }),
      el('span', { class: 'list-meta', text: meta }),
    ]);

    row.addEventListener('click', () => this.onSelect(body.id));
    this.rows.set(body.id, row);
    return row;
  }

  private applyFilter(): void {
    const needle = this.filterInput.value.trim().toLowerCase();
    const visibleRoots = new Set<string>();

    for (const body of SOLAR_SYSTEM) {
      const row = this.rows.get(body.id);
      if (!row) continue;
      const matches = !needle || (row.dataset.search ?? '').includes(needle);
      row.classList.toggle('filtered-out', !matches);
      if (matches) visibleRoots.add(body.id);
    }

    // Keep a parent visible when one of its satellites matches.
    for (const body of SOLAR_SYSTEM) {
      for (const moon of body.children) {
        const moonRow = this.rows.get(moon.id);
        if (moonRow && !moonRow.classList.contains('filtered-out')) {
          this.rows.get(body.id)?.classList.remove('filtered-out');
        }
      }
    }
    void visibleRoots;
  }

  setActive(id: string | null): void {
    for (const [rowId, row] of this.rows) {
      row.classList.toggle('active', rowId === id);
    }
    if (id) this.rows.get(id)?.scrollIntoView({ block: 'nearest' });
  }
}
