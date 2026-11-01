<p align="right">
  <a href="README.md"><b>English</b></a> | <a href="README.kr.md">한국어</a>
</p>

# Project Overview

This project is a 3D game engine developed using C++17.

The goal of this project is to design and implement the engine's core systems and rendering pipeline from the ground up.


# Screenshots

<p align="center">
  <img src="Docs/demo_optimized.gif" width="80%">
</p>
<p align="center">
  <img src="Docs/Screenshot.png" width="80%">
</p>

# Build Environment and Instructions

### 🛠️ Build Environment

- OS: Windows 10 / 11
- Compiler:
  - MSVC (`cl`)
  - Clang (`clang`, `clang++`)
- CMake: 3.23 or later
- C++ Standard: C++17

---

### 📦 Generate Build Files

- MSVC (Visual Studio 2022)
```bash
cmake -S . -B <build_directory> -G "Visual Studio 17 2022"
```

- Clang + Ninja (Debug)
```bash
cmake -S . -B <build_directory> -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
```

- Clang + Ninja (Release)
```bash
cmake -S . -B <build_directory> -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release
```

---

### 🔧 Build

- MSVC (Debug)
```bash
cmake --build <build_directory> --config Debug
```

- MSVC (Release)
```bash
cmake --build <build_directory> --config Release
```

- Clang + Ninja (Debug/Release)
```bash
cmake --build <build_directory>
```

# Engine Architecture

<p align="center">
  <img src="Docs/EngineArchitecture.svg" width="70%">
</p>

# Engine Execution Flow

The engine operates by having the Application submit commands to Subsystems for tasks required during execution. Each Subsystem processes the commands submitted to it.

The execution flow of the Application and Subsystems is centrally managed by the Engine class. The Engine exclusively controls the execution flow of the Application, as well as the timing and order in which each Subsystem processes its tasks.

This approach prevents dependency issues between Subsystems that could arise if their tasks were executed directly during Application execution, while ensuring consistent control over task execution order.

The engine's execution flow is illustrated below.

<p align="center">
  <img src="Docs/EngineExecutionFlow.en.svg" width="50%">
</p>

# Features

The following table summarizes the currently implemented features of each major module.

| Name | Features |
|---|---|
| **Reflection System** | Registration and retrieval of reflection metadata for built-in types, enums, structs, and classes<br>Management of type metadata, including Name, Size, Alignment, Enum Entries, Properties, Methods, and inheritance relationships<br>Editor UI customization through per-property Widget Type metadata<br>Static registration of reflection metadata using macros |
| **Object System** | Object creation and destruction management<br>Object Pooling-based memory management<br>Object traversal based on inheritance relationships<br>Dangling pointer prevention and safe type casting through ObjectHandle<br>Object Pool management through Object Manager |
| **Asset System** | Asset creation, registration, lookup, and destruction<br>Support for Static Mesh, Morph Target, Morph Animation, Material, Texture, and Shader assets<br>Model Import and Asset Import Workflows<br>Management of asset references and GPU Resource Upload states |
| **Rendering System** | **Rendering Infrastructure**<br>Rendering pipeline organization based on Render Passes and Render Stages<br>Render Queue Double Buffering and rendering task execution on a dedicated Rendering Thread<br>Rendering Backend abstraction and GPU Resource management<br><br>**Rendering Pipeline**<br>Forward Rendering pipeline<br>PBR (GGX-based BRDF) and Phong Shading<br>Material Alpha Mode support, including Opaque, Masked, and Blended<br>Draw Command processing through Instanced Rendering<br><br>**Shadow Mapping**<br>Dynamic Shadow Mapping for Directional, Point, and Spot Lights<br>Static Shadow Map Baking and runtime application for Point and Spot Lights<br>Cascaded Shadow Mapping (CSM) for Directional Lights<br><br>**Image Based Lighting**<br>Irradiance Map Baking<br>Prefiltered Environment Map Baking<br>Image Based Lighting using a BRDF Integration Map<br><br>**Post Processing**<br>ACES Filmic Tone Mapping<br>Gamma Correction |
| **Scene Manager** | Scene, GameObject, and Component lifecycle management, including creation and destruction<br>GameObject composition and management through Components<br>Scene and Object updates and Garbage Collection<br><br>Transform Component: Object transforms and hierarchy management<br>Camera Component: Perspective and Orthographic Projection support<br>Static Mesh Component: Static Mesh assignment and Shadow Casting Mode configuration<br>SkyBox Component: Sky Map, Irradiance Map, and Prefiltered Environment Map configuration<br>Directional Light Component: Cascade configuration and Shadow Mapping<br>Point Light Component: Attenuation and Shadow Mapping<br>Spot Light Component: Cutoff, Attenuation, and Shadow Mapping |
| **Editor** | **Main Window**: Main editor window and menu configuration<br>**Scene Hierarchy Window**: Viewing and managing the hierarchy of GameObjects in a Scene<br>**Inspector Window**: Inspecting and editing the properties of selected GameObjects and Components<br>**Scene View Window**: Displaying the current Scene and providing editing tools<br>**Asset Browser**: Browsing and managing engine assets<br>**Camera Preview Window**: Previewing the rendering output of Cameras in the Scene<br>**Console Window**: Viewing logs generated by the engine and editor |

# Reference