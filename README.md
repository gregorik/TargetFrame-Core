<div align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine-5.7-blue?style=for-the-badge&logo=unrealengine" alt="Unreal Engine 5.7">
  <img src="https://img.shields.io/badge/Platform-Win64-lightgrey?style=for-the-badge&logo=windows" alt="Platform Win64">
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" alt="License MIT">
</div>

# Target Frame Core

**Target Frame Core** is an automatic, zero-configuration runtime scalability management system for Unreal Engine 5. Stop guessing what settings your players need. Target Frame Core automatically benchmarks hardware, sets a stable baseline, and dynamically steps down resolution scale and overall quality during gameplay to defend your target frame rate.

Designed to eliminate "stuttering" reviews on launch day, this plugin handles the dirty work of performance scaling so you can focus on building your game.

---

## 🚀 Key Features

### 🛡️ Runtime Governor
Never drop below your target frame rate during a boss fight again. The Runtime Governor constantly monitors the smoothed FPS. If the frame rate drops below your configured threshold for a sustained period, the Governor steps in to automatically lower the ResolutionScale and OverallQuality step-by-step until the target FPS is restored. When the action cools down, it smoothly scales back up to maximize visual fidelity.

### ⚡ Auto-Benchmarking & Tiering
On boot, Target Frame Core runs a fast, non-intrusive hardware scan. It assigns the machine to a hardware tier (Entry, Mainstream, Performance) based on CPU cores, RAM, and GPU capabilities. It automatically applies the recommended settings for that tier without the player ever having to open an Options menu.

### 📦 Fire-and-Forget Shipping Capsule
Hate writing custom save-game logic for graphics settings? The Fire-and-Forget Shipping feature completely abstracts scalability. The plugin automatically locks in the optimal settings after an initial "stabilization period," giving players a consistently smooth experience out-of-the-box. 

### 🔲 Upscaler-Safe UI
Upscaling a 1080p render to 4K is great for performance, but it can make your UI text look like a blurry mess. Target Frame Core automatically separates your UI rendering from your 3D scene rendering. The 3D scene scales dynamically, while your Slate/UMG UI remains locked at crisp, native resolution.

---

## 🛠️ Installation & Setup

1. Clone or download this repository.
2. Drop the TargetFrameCore folder into your project's Plugins/ directory.
3. Right-click your .uproject file and select **Generate Visual Studio project files**.
4. Open the project and compile.
5. Enable the plugin via **Edit > Plugins > Performance > Target Frame Core**.

### Configuration

You can configure the plugin directly from the Project Settings!
Navigate to **Project Settings > Plugins > TargetFrameCore**.

Here you can adjust:
* **Default Target FPS:** (e.g., 60)
* **Runtime Governor:** Adjust the threshold for how many dropped frames trigger a downgrade, and how quickly it recovers.
* **Hardware Tiers:** Define custom GPU score cutoffs for what you consider "Entry" vs "Performance".

---

## 💎 Upgrade to Target Frame Pro

**Target Frame Core** provides the essential foundation for automatic scalability. However, for teams pushing Unreal Engine 5 to its absolute limits, we offer **Target Frame Pro** on Fab.

The Pro version includes advanced, granular controls designed specifically for modern UE5 rendering features and QA workflows:

| Feature | Target Frame Core | Target Frame Pro |
| :--- | :---: | :---: |
| **Runtime Scalability Governor** | ✅ | ✅ |
| **Auto-Benchmarking** | ✅ | ✅ |
| **Upscaler Safe UI** | ✅ | ✅ |
| **Dynamic Nanite Budgeting** | ❌ | ✅ |
| **Hardware Ray Tracing Guards** | ❌ | ✅ |
| **VRAM Exhaustion Protection** | ❌ | ✅ |
| **Vendor-Specific Profiles** | ❌ | ✅ |
| **CSV Telemetry Export** | ❌ | ✅ |

### Why upgrade?

* **Dynamic Nanite Budgeting:** Instead of brutally dropping the global scalability level, the Pro version surgically relaxes Nanite's MaxPixelsPerEdge and time budgets during heavy scenes, keeping your shadows and post-processing intact while slightly reducing distant geometric density.
* **Hardware Ray Tracing Guards:** Lumen Hardware Ray Tracing is gorgeous but demands massive VRAM. The Pro version automatically intercepts the boot sequence, checks the physical VRAM and GPU capabilities, and gracefully falls back to Software Ray Tracing if the player's hardware would crash or stutter.
* **VRAM Exhaustion Protection:** Automatically clamps Texture Pool sizes and disables memory-hungry features like Nanite Tessellation on hardware with 8GB of VRAM or less to prevent catastrophic Out of Video Memory crashes.
* **Vendor-Specific Profiles:** Detects Intel ARC, AMD, and NVIDIA GPUs, applying specific capability maximums and dynamic resolution preferences to avoid known driver performance cliffs.
* **CSV Telemetry Export:** Automatically writes detailed frame-time data, intervention logs, and hardware specs to CSV files during playtests, making it incredibly easy for your QA team to identify performance bottlenecks.

[**👉 Get Target Frame Pro on Fab**](https://fab.com/sellers/GregOrigin)

---

## 📄 License
This project is licensed under the MIT License - see the LICENSE file for details.

*Copyright (c) 2026 GregOrigin. All Rights Reserved.*
