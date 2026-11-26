# 🤝 Contributing to MCCPE

Thank you for your interest in contributing to **MCCPE**! Reporting bugs and suggesting new ideas are always open to everyone, while direct code contributions follow a strict review and approval process.

---

## 🐛 Reporting Bugs

If you find a bug while playing on real hardware (DS/DSi) or an emulator, please open an **Issue** and include:

- **Game Version:** (e.g., `c0.11.0`)
- **Platform / Setup:** Real hardware (DS Lite, DSi, Flashcart model) or Emulator (melonDS, DeSmuME)
- **Description:** What happened and what you expected to happen
- **Steps to Reproduce:** How to trigger the bug step-by-step

---

## 💡 Suggesting Features

Have an idea for a new block, gameplay mechanic, or performance optimization?

- Open a post in the **Discussions** tab or create an **Issue** describing your idea.
- Keep in mind that **MCCPE** focuses on smooth core gameplay and hardware performance rather than replicating every modern Minecraft feature.

---

## 🔒 Submitting Code (Pull Requests)

To maintain the engine's architecture, performance standards, and vision, **unsolicited Pull Requests are not accepted**. Any code contribution requires **prior permission** and **documented supervision** by the project maintainer (**@Frostier-Dev**).

If you would like to contribute to the C++ codebase, you must follow these steps:

1. **Request Permission First:** Before writing any code, open an **Issue** or start a thread in **Discussions** detailing what you intend to build or fix. Wait for explicit approval from **@Frostier-Dev**.
2. **Documented Supervision:** Once approved, the development and implementation must remain documented within that Issue/Discussion (or linked thread) under the maintainer's guidance and supervision.
3. **Follow Hardware & Code Standards:** Keep your C++ code clean, lightweight, and optimized for the strict memory and CPU limits of target portable hardware (such as the Nintendo DS).
4. **Test Before Submitting:** Ensure the project compiles cleanly with `devkitARM` / `libnds` and runs properly on hardware or emulators.
5. **Submit the Pull Request:** Reference the approved Issue/Discussion in your Pull Request description so the supervision history is clearly linked.

> **Note:** Pull Requests opened without prior approval and documented supervision will be closed automatically.
