# reference/

Raw, authoritative source data downloaded from NASA/JPL. Kept in the repository
so every number used by the simulation can be audited offline.

Regenerate with:

```bash
HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:reference   # Horizons + SBDB
HTTPS_PROXY=http://127.0.0.1:7897 npm run fetch:satellites  # satellite mean elements
npm run gen:moons                                           # regenerate src/data/moon-orbits.generated.ts
```

## Contents

| Path | Source | Used for |
| --- | --- | --- |
| `horizons-vectors.json` | [JPL Horizons](https://ssd.jpl.nasa.gov/horizons/) — heliocentric ecliptic-J2000 state vectors, `CENTER='500@10'`, `OUT_UNITS=AU-D`, at 2000-01-01, 2025-01-01 and 2050-01-01 | Ground truth for `tests/horizons.test.ts` (**not** used at runtime) |
| `sbdb-elements.json` | [JPL Small-Body Database API](https://ssd-api.jpl.nasa.gov/sbdb.api) — osculating elements with full precision | Orbital elements for the dwarf planets, large asteroids and comets |
| `satellite-elements.json` | [JPL Planetary Satellite Mean Elements](https://ssd.jpl.nasa.gov/sats/elem/) — parsed from the HTML below | Orbital elements for all 23 rendered satellites |
| `raw/horizons-*.txt` | JPL Horizons API responses | Raw evidence for `horizons-vectors.json` |
| `raw/sbdb-*.json` | JPL SBDB API responses | Raw evidence for `sbdb-elements.json` |
| `raw/satellite-elements.html` | `https://ssd.jpl.nasa.gov/sats/elem/` | Raw table of satellite mean elements |

## Conversions applied

The propagator works with the Standish & Williams element set
`(a, e, i, L, ϖ, Ω)`, while JPL's small-body and satellite tables publish
`(a, e, i, Ω, ω, M)`. The scripts convert between them with

```
ϖ = ω + Ω          (longitude of perihelion)
L = ω + Ω + M      (mean longitude at epoch)
```

Angles are left un-normalised (values above 360° are fine, the solver reduces
the mean anomaly modulo 2π).

## Frame conventions

| Source | Reference plane |
| --- | --- |
| Horizons vectors | Mean ecliptic and equinox of J2000 |
| SBDB osculating elements | Mean ecliptic and equinox of J2000 |
| Satellite mean elements | Per-satellite: `ecliptic`, `equatorial` or `Laplace` — recorded in the `frame` field and carried through to `src/data/moon-orbits.generated.ts` |

Solar converts everything into the mean ecliptic and equinox of J2000
rectangular frame (`+x` towards the vernal equinox, `+z` towards ecliptic
north). Planet and satellite pole orientations come from the IAU WGCCRE report
(Archinal et al.) as `(RA, Dec)` of the north pole in the ICRF, which is
converted with the J2000 obliquity ε = 23.43928°.
