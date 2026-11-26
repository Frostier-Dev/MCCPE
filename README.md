# <img width="32" height="32" alt="icon" src="https://github.com/user-attachments/assets/357965f6-c8a4-4337-80c8-ae58510a81b9" /> Minecraft: Portable CXX Engine

```text
Current Version: c0.11.0
Current Version Group: Classic 'c'
```

**MCCPE** is a lightweight C++ Minecraft engine, built specifically for portable devices. The main goal of this project is to focus on smooth core gameplay mechanics rather than replicating every feature of the full game.

---

## ✨ Current Features

- **Flat World:** Clean and optimized flat terrain ready for exploration and building.
- **4 Distinct Block Types:** Includes 4 different types of blocks to build with.
- **Building System:** Functional block placing and breaking mechanics designed for handheld controls.

---

## 🎮 Game Manual

**How-To-Use**:
1. Place the game file inside your SD/Micro SD Card;
2. Insert the card into your flashcart or directly into the SD slot (on DSi);
3. Turn on your console and open the homebrew menu (DSi) or the standard menu (DS);
4. Start the game by tapping the cartridge icon in the menu (DS) or by finding the file in your homebrew menu and launching it (DSi);
5. *(Extra)* Press **Start** to close the game or return to the homebrew menu.

> **Tip:** Use the default bootstrap.

**Controls**:
- **A Button:** Jump;
- **Y Button:** Crouch (toggle);
- **L Trigger:** Break blocks;
- **R Trigger:** Place blocks;
- **D-Pad:** Move the player around the world;
- **Touch Screen:** Control the player's camera.

---

## 📂 Project Structure

```text
MCCPE/
├── audio/       # Sound effects and music assets
├── graphics/    # Textures and 3D assets
├── nitrofs/     # Filesystem assets bundled into the NDS ROM
└── source/      # Core C++ engine source code
```
