/** Tiny DOM helpers — the UI deliberately avoids a framework. */

export function el<K extends keyof HTMLElementTagNameMap>(
  tag: K,
  attrs: Record<string, string> = {},
  children: Array<Node | string> = [],
): HTMLElementTagNameMap[K] {
  const node = document.createElement(tag);
  for (const [key, value] of Object.entries(attrs)) {
    if (key === 'class') node.className = value;
    else if (key === 'text') node.textContent = value;
    else if (key === 'html') node.innerHTML = value;
    else node.setAttribute(key, value);
  }
  for (const child of children) node.append(child);
  return node;
}

export function clear(node: HTMLElement): void {
  while (node.firstChild) node.removeChild(node.firstChild);
}

/** Collapsible section with a title, used throughout the sidebar. */
export function section(title: string, initiallyOpen = true): { root: HTMLElement; body: HTMLElement } {
  const body = el('div', { class: 'section-body' });
  const chevron = el('span', { class: 'chevron', text: '▾' });
  const header = el('button', { class: 'section-header', type: 'button' }, [
    el('span', { class: 'section-title', text: title }),
    chevron,
  ]);
  const root = el('section', { class: `panel-section${initiallyOpen ? '' : ' collapsed'}` }, [header, body]);
  header.addEventListener('click', () => {
    root.classList.toggle('collapsed');
    chevron.textContent = root.classList.contains('collapsed') ? '▸' : '▾';
  });
  return { root, body };
}

/** Labelled range slider with a live readout. */
export interface SliderOptions {
  label: string;
  min: number;
  max: number;
  step: number;
  value: number;
  /** Formats the raw value for display. */
  format: (value: number) => string;
  onInput: (value: number) => void;
}

export function slider(options: SliderOptions): { root: HTMLElement; input: HTMLInputElement; set: (v: number) => void } {
  const readout = el('span', { class: 'readout', text: options.format(options.value) });
  const input = el('input', {
    type: 'range',
    min: String(options.min),
    max: String(options.max),
    step: String(options.step),
    value: String(options.value),
    class: 'slider',
  }) as HTMLInputElement;

  input.addEventListener('input', () => {
    const v = Number(input.value);
    readout.textContent = options.format(v);
    options.onInput(v);
  });

  const root = el('div', { class: 'control-row' }, [
    el('div', { class: 'control-head' }, [el('span', { class: 'control-label', text: options.label }), readout]),
    input,
  ]);

  return {
    root,
    input,
    set: (v: number) => {
      input.value = String(v);
      readout.textContent = options.format(v);
    },
  };
}

export function toggle(label: string, checked: boolean, onChange: (value: boolean) => void): HTMLInputElement {
  const input = el('input', { type: 'checkbox', class: 'checkbox' }) as HTMLInputElement;
  input.checked = checked;
  input.addEventListener('change', () => onChange(input.checked));
  const wrap = el('label', { class: 'toggle' }, [input, el('span', { text: label })]);
  // The caller receives the input; wrap is attached by the caller.
  void wrap;
  return input;
}

/** Checkbox row that returns its own label wrapper. */
export function toggleRow(labelText: string, checked: boolean, onChange: (value: boolean) => void): HTMLElement {
  const input = el('input', { type: 'checkbox', class: 'checkbox' }) as HTMLInputElement;
  input.checked = checked;
  input.addEventListener('change', () => onChange(input.checked));
  return el('label', { class: 'toggle' }, [input, el('span', { text: labelText })]);
}

export function button(labelText: string, onClick: () => void, className = ''): HTMLButtonElement {
  const b = el('button', { class: `btn ${className}`.trim(), type: 'button', text: labelText });
  b.addEventListener('click', onClick);
  return b;
}
