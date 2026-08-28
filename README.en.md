# Cislunar-Sim · Earth–Moon Orbital 3D Simulator

[中文](./README.md)

A web-based spacecraft orbital-dynamics simulator for cislunar space: it numerically integrates and visualizes a spacecraft's position, velocity, and trajectory in the **Earth + Moon** two-body gravity field (the restricted three-body problem) using a **4th-order Runge–Kutta (RK4)** integrator.

> 🖥️ **Desktop edition (Windows)**: see [`desktop/`](./desktop/README.md), a native rewrite with Qt 6 + OpenGL that builds into a standalone exe.

## ✨ Features

- **Interactive 3D scene**: rotate / zoom / pan with the mouse, starfield background, and to-scale celestial bodies
- **Two initial-orbit input modes**, each with paired **slider + numeric** input and a live t=0 preview of position and velocity direction:
  - Position + velocity (coordinates, speed, azimuth / elevation)
  - Orbital elements (semi-major axis a, eccentricity e, inclination i, RAAN Ω, argument of periapsis ω, true anomaly ν)
- **Real-time telemetry panel**: simulation time, speed, altitude above the Earth / Moon surface, and Earth / Moon gravitational acceleration
- **Trajectory display**: the flight path is drawn in real time, with a toggle between "last 30 days" (default) and "always show"
- **Speed visualization**: the trajectory can be color-coded by speed (red = slow → blue = fast), or overlaid with vertical speed bars (length = speed) perpendicular to the orbital plane
- **Adjustable simulation speed**: from 1× up to 1 day per second
- **Presets**: save / load any initial orbit parameter set (persisted in localStorage)
- **Camera presets**: overview / Earth / Moon, plus spacecraft follow
- **Kepler orbit preview (Earth gravity only)**: in the initial-orbit preview state (t=0), draw an extra **green analytic orbit** (two-body problem: ellipse / parabola / hyperbola) so you can directly compare how the Moon's gravity pulls the real trajectory away from the pure Kepler path; toggleable anytime and auto-hidden once the simulation starts

## 🚀 Quick Start

No build step required — it's a single file:

```bash
# Option 1: open index.html directly in a browser

# Option 2: serve it locally (recommended)
python -m http.server 8000
# then visit http://localhost:8000
```

> The app loads [Three.js](https://threejs.org/) from a CDN, so an internet connection is required on first run.

### Deploying to GitHub Pages

1. Repository Settings → Pages → set Source to the `main` branch (root) → Save
2. After deployment finishes, visit `https://<your-username>.github.io/Cislunar-Sim/`

## 🔬 Physics Model

- **Gravity field**: Earth + Moon two-body system (circular restricted three-body approximation: Earth fixed at the origin, Moon on a circular orbit with a 27.32-day period)
- **Numerical integration**: 4th-order Runge–Kutta (RK4) over the six state variables `[x, y, z, vx, vy, vz]`
- **Units**: km / s; display scale 1 scene unit = 1000 km (true scale)
- **Kepler orbit preview**: from the current position + velocity, analytically derive the angular momentum **h** and eccentricity vector **e**, build the orbital plane and sample the whole conic — a closed ellipse for e<1, or an open parabola / hyperbola (e≥1) clamped to a 2,000,000 km radius asymptote limit for ellipses; the green orbit passes exactly through the spacecraft's current position for direct comparison against the RK4 trajectory

## 📁 Project Structure

```
Cislunar-Sim/
├── index.html     # web edition (single file, contains all logic and UI)
├── desktop/       # desktop edition (Qt6 + OpenGL, see desktop/README.md)
├── README.md
├── README.en.md   # English introduction
└── LICENSE
```

## 📄 License

[MIT](./LICENSE) © 2026 一只羽毛球儿
