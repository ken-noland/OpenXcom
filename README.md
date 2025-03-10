# OpenXcom

OpenXcom is an open-source reimplementation of the classic strategy games
"UFO: Enemy Unknown" ("X-COM: UFO Defense" in the USA release) and "X-COM: Terror From the Deep"
by Microprose, licensed under the GPL and written in C++.

More information can be found on the [official website](https://openxcom.org)
and the [wiki](https://www.ufopaedia.org/index.php/OpenXcom).

## About This Fork

This fork of OpenXcom is a **complete rewrite** of the original codebase, with a focus on **modernization**,
**performance**, and **modding extensibility**. Key improvements include:

- **Vulkan Rendering**: A new rendering backend using **Vulkan**, replacing the outdated SDL 1.0 renderer.
  This allows for better performance, cross-platform compatibility, and future-proofing.
- **Lua Scripting Integration**: A fully embedded **Lua scripting system**, enabling modders to dynamically
  modify gameplay mechanics, UI elements, and assets with far greater flexibility than before.
- **Entity-Component System (ECS)**: The game has been rewritten with an **ECS architecture** for improved
  scalability, maintainability, and modding support.
- **Custom Virtual Filesystem (VFS)**: A new filesystem abstraction that supports **physical files**,
  **ZIP-based storage**, and **embedded assets**, allowing seamless data access across different platforms.
- **Improved UI System**: The UI system has been reworked to **decouple input, rendering, and logic**,
  making it more flexible for future extensions.
- **Cross-Platform Support**: The game is designed to run on **Windows, Linux, and macOS**, with potential
  future support for **Android**.

This fork prioritizes **maintainability** and **extensibility**, ensuring that modders and developers can
continue to innovate and expand the game.

---

## Building OpenXcom

Building OpenXcom follows a standard CMake workflow. The project requires
modern C++ (C++20) and Vulkan support. Below are the general steps to build the project:

### Prerequisites

Ensure you have the following dependencies installed:

- **C++ Compiler**:
  - Windows: Microsoft Visual Studio
  - Linux/macOS: Clang or GCC (minimum version supporting C++20)
- **CMake** (version 3.20 or later)

### Build Steps

#### Windows (Visual Studio)

```sh
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

#### Linux/macOS (Ninja)

```sh
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
ninja
```

For debugging, replace `Release` with `Debug` in the build type.

After building, the executable will be located in the `bin/` directory.

---

## Installation

### Required Game Assets

OpenXcom requires a **vanilla** copy of the X-COM resources from one or both of the original games.
If you own the games on Steam, the Windows installer will automatically detect and copy
the required files.

To manually copy assets, locate your Steam game folders:

```
UFO: "Steam\SteamApps\common\XCom UFO Defense\XCOM"
TFTD: "Steam\SteamApps\common\X-COM Terror from the Deep\TFD"
```

Copy the `UFO` subfolders to the `UFO` directory in OpenXcom's **data** or **user folder**,
and/or copy the `TFTD` subfolders to the `TFTD` directory in OpenXcom's **data** or **user folder**.

#### Do Not Use Modded Versions

Using modded game files (e.g., those modified by **XcomUtil**) may cause **bugs and crashes**.
Ensure that your game assets are unmodified for the best experience.

---

## Modding & Lua Integration

This fork significantly expands **modding possibilities** through **Lua scripting**.
With Lua, modders can:

- Create **custom UI elements**
- Modify **game rules and mechanics**
- Add **procedurally generated content** (e.g., terrain, maps, weapons)
- Implement **custom events and interactions**
- Manage **resource loading and unloading dynamically**

Modders can define **custom surfaces and entities**, giving them unprecedented
control over how the game behaves. The goal is to provide a scripting interface that is
both **powerful** and **easy to use**, without requiring C++ modifications.

More details and documentation will be provided in future updates.

---

## Future Plans

This project is a **work in progress**, with ongoing development in the following areas:

- Expanding Vulkan rendering features (e.g., shader-based effects, better upscaling)
- Refining Lua APIs to improve modding capabilities
- Adding better debugging and profiling tools
- Supporting additional platforms like Android

---

## Contributions

Contributions are welcome! If you're interested in helping, check out the issues
on GitHub, submit pull requests, or join the OpenXcom community discussions.

### Reporting Bugs
If you find a bug, please provide the following details:

- Steps to reproduce the issue
- Your operating system and hardware specs
- Any relevant logs or screenshots

Bug reports can be submitted via the issue tracker.

---

## License

OpenXcom is licensed under the **GPL v3**. See the `LICENSE` file for details.

---

## Contact

For discussions and development updates, visit:

- [OpenXcom Forum](https://openxcom.org/forum/)
- [GitHub Issues](https://github.com/OpenXcom/OpenXcom/issues)

Stay tuned for updates, and happy modding!

