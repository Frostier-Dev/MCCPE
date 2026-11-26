# Accessibility

**MCCPE** is built to be an enjoyable, lightweight voxel game for everyone on portable devices. Accessibility matters to us because all players and developers should be able to explore, build, and contribute to the project without unnecessary barriers.

This document outlines our current accessibility priorities, what contributors should keep in mind, supported hardware and input methods, and how to report any barriers you encounter.

## Priorities

Because **MCCPE** targets handheld consoles like the Nintendo DS, our primary focus is on clear visual feedback and comfortable physical controls:

- **Visual Clarity:** Designing block textures and UI elements with distinct shapes and contrast so they remain readable on small, low-resolution screens (`256x192` pixels).
- **Comfortable Controls:** Keeping core actions (moving, jumping, crouching, breaking, and placing blocks) simple, responsive, and easy to execute without complex button combinations.
- **Plain Documentation:** Keeping our `README.md`, manual, and repository guides clear, well-structured, and easy to read with screen readers.

## Contributor expectations

All contributions to **MCCPE** follow the supervision and approval process outlined in our `CONTRIBUTING.md`. When proposing or implementing user-facing changes (such as UI layouts, textures, or control schemes), contributors are expected to:

- Ensure new text and icons are legible at native Nintendo DS screen resolutions.
- Avoid relying solely on subtle color differences to distinguish interactive elements or block types.
- Test changes on an emulator (such as melonDS or DeSmuME) or real hardware and describe how controls and visuals behave in the linked Issue or Discussion.

## Reporting accessibility issues

If you encounter a barrier while playing **MCCPE** or navigating this repository, please let us know so we can work toward a solution:

1. Open an **Issue** in this repository or start a thread in **Discussions**.
2. Describe the task you were trying to complete and what blocked you.
3. Include helpful context such as your **game version** (e.g., `c0.11.0`), **device or emulator**, and **input method**.

Screenshots or recordings are always optional, and you never need to disclose personal medical or disability information to report a barrier.

### Severity

During triage, the maintainer assigns severity based on how strongly the barrier affects gameplay or repository access:

- **High (Blocker):** Prevents a player from starting the game, navigating menus, or performing core actions like moving and building.
- **Medium:** Makes an action difficult or tiring (such as hard-to-read text or awkward camera/button handling), though a workaround exists.
- **Low:** Minor visual or documentation friction that does not block core gameplay.

### How we respond

- **Acknowledgement:** We aim to acknowledge new accessibility reports within **7 days**.
- **Updates & Workarounds:** If a hardware limitation prevents an immediate fix, we will share available workarounds (such as emulator key-remapping tips) and keep the Issue updated as improvements are planned.

## Ownership and maintenance

Accessibility reviews and triage for **MCCPE** are maintained by **@Frostier-Dev**. This document is reviewed alongside major engine updates to reflect new features, control options, and hardware support.

## Supported environments

**MCCPE** has been tested and evaluated in the following environments:

- **Consoles:** Nintendo DS and Nintendo DSi (via flashcart or SD card homebrew menu).
- **Emulators:** melonDS and DeSmuME on desktop operating systems (which also allow custom keyboard/gamepad remapping and screen scaling).
- **Input Methods:** Physical Nintendo DS buttons (`D-Pad`, `A`, `Y`, `L`, `R`, `Start`) and resistive Touch Screen (stylus or finger).

## Known limitations

- **Fixed Button Mapping on Real Hardware:** The current version (`c0.11.0`) uses a fixed control scheme on physical DS/DSi consoles and does not yet include an in-game button remapping menu.
  - *Workaround:* Players using desktop emulators like melonDS or DeSmuME can freely remap buttons and touch controls to external controllers or assistive input devices.
- **Touch-Based Camera Control:** Camera movement currently relies on the Touch Screen, which may be difficult for one-handed play on original hardware.

## Feedback and improvements

We welcome suggestions on how to make **MCCPE** and its documentation more welcoming and easier to use. You can share general ideas in **Discussions**, or report active barriers through the **Issues** tab described above.
