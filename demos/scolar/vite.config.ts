/// <reference types="vitest/config" />
import { defineConfig, type Plugin } from 'vite';

function escapeRegExp(text: string): string {
  return text.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

/**
 * Emits a single self-contained `index.html` (JS and CSS inlined) so the build
 * output can be opened directly from the file system — no web server needed.
 *
 * Public assets (the textures) stay external and are referenced relatively,
 * which browsers allow for `<img>`/texture loads from `file://`.
 */
function singleFilePlugin(): Plugin {
  return {
    name: 'scolar:single-file',
    apply: 'build',
    enforce: 'post',
    generateBundle(_options, bundle) {
      const htmlName = Object.keys(bundle).find((name) => name.endsWith('.html'));
      if (!htmlName) return;
      const htmlAsset = bundle[htmlName];
      if (htmlAsset.type !== 'asset') return;

      let html = String(htmlAsset.source);

      for (const [name, output] of Object.entries(bundle)) {
        if (output.type !== 'asset' || !name.endsWith('.css')) continue;
        const css = String(output.source);
        const pattern = new RegExp(`<link[^>]*href="[^"]*${escapeRegExp(name)}"[^>]*>`);
        if (pattern.test(html)) {
          // Function replacement: the CSS/JS may contain `$&`, `$1`, …
          html = html.replace(pattern, () => `<style>\n${css}\n</style>`);
          delete bundle[name];
        }
      }

      let inlineScript = '';
      for (const [name, output] of Object.entries(bundle)) {
        if (output.type !== 'chunk' || !name.endsWith('.js')) continue;
        // Guard against a literal `</script` inside the bundle.
        const code = output.code.replace(/<\/script/gi, '<\\/script');
        const pattern = new RegExp(`<script[^>]*src="[^"]*${escapeRegExp(name)}"[^>]*></script>`);
        if (pattern.test(html)) {
          // Drop the tag from its original position (usually <head>) — an
          // inline script has no `defer`, so it must run after the DOM exists.
          html = html.replace(pattern, '');
          inlineScript += `<script>\n${code}\n</script>\n`;
          delete bundle[name];
        }
      }

      if (inlineScript) {
        html = html.includes('</body>')
          ? html.replace('</body>', () => `${inlineScript}</body>`)
          : html + inlineScript;
      }

      // Drop now-orphaned source maps.
      for (const name of Object.keys(bundle)) {
        if (name.endsWith('.map')) delete bundle[name];
      }

      htmlAsset.source = html;
    },
  };
}

const standalone = process.env.SCOLAR_STANDALONE === '1';

export default defineConfig(({ mode }) => {
  // `npm run build` (mode=standalone) emits one self-contained index.html that
  // can be opened straight from disk; `npm run build:server` keeps the classic
  // multi-file output for a web server.
  const single = standalone || mode === 'standalone';

  return {
    base: './',
    // No runtime assets live in `public/` any more: textures are imported
    // through the bundler from src/assets and inlined in the single-file build.
    publicDir: false,
    server: {
      host: '127.0.0.1',
      port: 5173,
      open: false,
    },
    build: {
      target: 'es2022',
      outDir: 'dist',
      emptyOutDir: true,
      sourcemap: !single,
      cssCodeSplit: !single,
      modulePreload: !single,
      // The single-file build inlines every texture as a data URL so that the
      // page also works from file:// (where CORS-checked image loads fail).
      assetsInlineLimit: single ? Number.MAX_SAFE_INTEGER : 4096,
      chunkSizeWarningLimit: 6000,
      ...(single
        ? {
            rollupOptions: {
              output: {
                format: 'iife' as const,
                inlineDynamicImports: true,
                entryFileNames: 'assets/[name].js',
                assetFileNames: 'assets/[name][extname]',
              },
            },
          }
        : {}),
    },
    plugins: single ? [singleFilePlugin()] : [],
    test: {
      environment: 'node',
      include: ['tests/**/*.test.ts'],
    },
  };
});
