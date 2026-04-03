<div align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine-5.7-blue?style=for-the-badge&logo=unrealengine" alt="Unreal Engine 5.7">
  <img src="https://img.shields.io/badge/Platform-Win64-lightgrey?style=for-the-badge&logo=windows" alt="Platform Win64">
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" alt="License MIT">
</div>
 
<h1>TargetFrame Core</h1>

[Watch a video](https://www.youtube.com/watch?v=6ghqO5RCQz4)

[Read the manual](https://gregorigin.com/TargetFrame/)

[Discord support](https://discord.gg/nqYQ5mtmHb)

![TargetF0](https://github.com/user-attachments/assets/4bd4282b-8507-4aa6-b98b-12e8c9aeb7c2)



<p><strong>TargetFrame Core</strong> is an automatic, zero-configuration runtime scalability management system for Unreal Engine 5. Stop guessing what settings your players need. TargetFrame Core automatically benchmarks hardware, sets a stable baseline, and dynamically steps down resolution scale and overall quality during gameplay to defend your target frame rate.</p>
<p>Designed to eliminate &quot;stuttering&quot; reviews on launch day, this plugin handles the dirty work of performance scaling so you can focus on building your game.</p>
<hr>
<h2>🚀 Key Features</h2>

![TargetF2](https://github.com/user-attachments/assets/4cc28b45-58ae-49d5-8309-ace474ffea1a)

<h3>🛡️ Runtime Governor</h3>
<p>Never drop below your target frame rate during a boss fight again. The Runtime Governor constantly monitors the smoothed FPS. If the frame rate drops below your configured threshold for a sustained period, the Governor steps in to automatically lower the ResolutionScale and OverallQuality step-by-step until the target FPS is restored. When the action cools down, it smoothly scales back up to maximize visual fidelity.</p>
<h3>⚡ Auto-Benchmarking &amp; Tiering</h3>
<p>On boot, TargetFrame Core runs a fast, non-intrusive hardware scan. It assigns the machine to a hardware tier (Entry, Mainstream, Performance) based on CPU cores, RAM, and GPU capabilities. It automatically applies the recommended settings for that tier without the player ever having to open an Options menu.</p>
<h3>📦 Fire-and-Forget Shipping Capsule</h3>
<p>Hate writing custom save-game logic for graphics settings? The Fire-and-Forget Shipping feature completely abstracts scalability. The plugin automatically locks in the optimal settings after an initial &quot;stabilization period,&quot; giving players a consistently smooth experience out-of-the-box. </p>
<h3>🔲 Upscaler-Safe UI</h3>
<p>Upscaling a 1080p render to 4K is great for performance, but it can make your UI text look like a blurry mess. TargetFrame Core automatically separates your UI rendering from your 3D scene rendering. The 3D scene scales dynamically, while your Slate/UMG UI remains locked at crisp, native resolution.</p>

<img width="1024" height="572" alt="TargetF1" src="https://github.com/user-attachments/assets/d6cef925-b015-4a3a-b3da-7ee52b9ec4a2" />


<hr>
<h2>🛠️ Installation &amp; Setup</h2>
<ol>
<li>Clone or download this repository.</li>
<li>Drop the TargetFrameCore folder into your project&#39;s Plugins/ directory.</li>
<li>Right-click your .uproject file and select <strong>Generate Visual Studio project files</strong>.</li>
<li>Open the project and compile.</li>
<li>Enable the plugin via <strong>Edit &gt; Plugins &gt; Performance &gt; TargetFrame Core</strong>.</li>
</ol>
<h3>Configuration</h3>
<p>You can configure the plugin directly from the Project Settings!
Navigate to <strong>Project Settings &gt; Plugins &gt; TargetFrameCore</strong>.</p>
<p>Here you can adjust:</p>
<ul>
<li><strong>Default Target FPS:</strong> (e.g., 60)</li>
<li><strong>Runtime Governor:</strong> Adjust the threshold for how many dropped frames trigger a downgrade, and how quickly it recovers.</li>
<li><strong>Hardware Tiers:</strong> Define custom GPU score cutoffs for what you consider &quot;Entry&quot; vs &quot;Performance&quot;.</li>
</ul>
<hr>
<h2>💎 Upgrade to TargetFrame Pro</h2>
<p><strong>TargetFrame Core</strong> provides the essential foundation for automatic scalability. However, for teams pushing Unreal Engine 5 to its absolute limits, we offer <strong>TargetFrame Pro</strong> on Fab.</p>
<p>The Pro version includes advanced, granular controls designed specifically for modern UE5 rendering features and QA workflows:</p>
<table>
<thead>
<tr>
<th align="left">Feature</th>
<th align="center">TargetFrame Core</th>
<th align="center">TargetFrame Pro</th>
</tr>
</thead>
<tbody><tr>
<td align="left"><strong>Runtime Scalability Governor</strong></td>
<td align="center">✅</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>Auto-Benchmarking</strong></td>
<td align="center">✅</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>Upscaler Safe UI</strong></td>
<td align="center">✅</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>Dynamic Nanite Budgeting</strong></td>
<td align="center">❌</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>Hardware Ray Tracing Guards</strong></td>
<td align="center">❌</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>VRAM Exhaustion Protection</strong></td>
<td align="center">❌</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>Vendor-Specific Profiles</strong></td>
<td align="center">❌</td>
<td align="center">✅</td>
</tr>
<tr>
<td align="left"><strong>CSV Telemetry Export</strong></td>
<td align="center">❌</td>
<td align="center">✅</td>
</tr>
</tbody></table>
<h3>Why upgrade?</h3>
<ul>
<li><strong>Dynamic Nanite Budgeting:</strong> Instead of brutally dropping the global scalability level, the Pro version surgically relaxes Nanite&#39;s MaxPixelsPerEdge and time budgets during heavy scenes, keeping your shadows and post-processing intact while slightly reducing distant geometric density.</li>
<li><strong>Hardware Ray Tracing Guards:</strong> Lumen Hardware Ray Tracing is gorgeous but demands massive VRAM. The Pro version automatically intercepts the boot sequence, checks the physical VRAM and GPU capabilities, and gracefully falls back to Software Ray Tracing if the player&#39;s hardware would crash or stutter.</li>
<li><strong>VRAM Exhaustion Protection:</strong> Automatically clamps Texture Pool sizes and disables memory-hungry features like Nanite Tessellation on hardware with 8GB of VRAM or less to prevent catastrophic Out of Video Memory crashes.</li>
<li><strong>Vendor-Specific Profiles:</strong> Detects Intel ARC, AMD, and NVIDIA GPUs, applying specific capability maximums and dynamic resolution preferences to avoid known driver performance cliffs.</li>
<li><strong>CSV Telemetry Export:</strong> Automatically writes detailed frame-time data, intervention logs, and hardware specs to CSV files during playtests, making it incredibly easy for your QA team to identify performance bottlenecks.</li>
</ul>
<p><a href="https://fab.com/sellers/GregOrigin"><strong>👉 Get TargetFrame Pro on Fab</strong></a></p>
<hr>
<h2>📄 License</h2>
<p>This project is licensed under the MIT License - see the LICENSE file for details.</p>
<p><em>Copyright (c) 2026 GregOrigin. All Rights Reserved.</em></p>
