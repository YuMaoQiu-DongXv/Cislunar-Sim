# Cislunar-Sim · Earth–Moon Orbital 3D Simulator

[中文](./README.md)

A web-based spacecraft orbital-dynamics simulator for cislunar space: it numerically integrates and visualizes a spacecraft's position, velocity, and trajectory in the **Earth + Moon** two-body gravity field (the restricted three-body problem) using a **4th-order Runge–Kutta (RK4)** integrator.

## ✨ Features

- **Interactive 3D scene**: rotate / zoom / pan with the mouse, starfield background, and to-scale celestial bodies
- **Two initial-orbit input modes**, each with paired **slider + numeric** input and a live t=0 preview of position and velocity direction:
  - Position + velocity (coordinates, speed, azimuth / elevation)
  - Orbital elements (semi-major axis a, eccentricity e, inclination i, RAAN Ω, argument of periapsis ω, true anomaly ν)
- **Real-time telemetry panel**: simulation time, speed, altitude above the Earth / Moon surface, and Earth / Moon gravitational acceleration
- **Trajectory trail**: the spacecraft's flight path is drawn in real time
- **Adjustable simulation speed**: from 1× up to 1 day per second
- **Presets**: save / load any initial orbit parameter set (persisted in localStorage)
- **Camera presets**: overview / Earth / Moon, plus spacecraft follow

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

## 📁 Project Structure

```
Cislunar-Sim/
├── index.html     # main program (single file, contains all logic and UI)
├── README.md
├── README.en.md   # English introduction
└── LICENSE
```

## 📄 License

[MIT](./LICENSE) © 2026 一只羽毛球儿
