import './ui/styles.css';
import { createApp, type AppHosts } from './app';

function require(id: string): HTMLElement {
  const node = document.getElementById(id);
  if (!node) throw new Error(`Missing #${id} in index.html`);
  return node;
}

async function start(): Promise<void> {
  const bootFill = require('boot-fill');
  const bootStatus = require('boot-status');
  const boot = document.getElementById('boot');
  const app = require('app');

  const hosts: AppHosts = {
    canvasHost: require('canvas-host'),
    labelHost: require('label-host'),
    sidebar: require('sidebar'),
    infoHost: require('info-host'),
    hudHost: require('hud-host'),
  };

  try {
    await createApp(hosts, {
      onProgress: (loaded, total) => {
        const ratio = total ? loaded / total : 0;
        bootFill.style.width = `${Math.round(ratio * 100)}%`;
        bootStatus.textContent = `正在加载贴图 ${loaded}/${total} …`;
      },
    });
    app.hidden = false;
    boot?.classList.add('boot-done');
    window.setTimeout(() => boot?.remove(), 700);
  } catch (error) {
    console.error(error);
    bootStatus.textContent = `初始化失败：${(error as Error).message}`;
    bootStatus.classList.add('error');
  }
}

// The bundle may be inlined as a plain <script> (single-file build), so make
// sure the DOM exists before looking anything up.
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => void start(), { once: true });
} else {
  void start();
}
