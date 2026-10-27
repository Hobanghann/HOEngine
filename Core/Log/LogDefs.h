#pragma once

namespace ho
{
using LogLevel = int;

enum eLogLevel
{
    LogLevel_Trace = 1 << 0,
    LogLevel_Debug = 1 << 1,
    LogLevel_Info = 1 << 2,
    LogLevel_Warn = 1 << 3,
    LogLevel_Error = 1 << 4,
    LogLevel_Critical = 1 << 5,
};

using LogCategory = int;

enum eLogCategory
{
    LogCategory_None = 1 << 0,
    // 1. Common & Engine Core
    LogCategory_System = 1 << 1, // Engine initialization, setup, main loop, OS-related
    LogCategory_Memory = 1 << 2, // Memory allocator, pooling, leak tracking
    LogCategory_IO = 1 << 3,     // File I/O, async I/O, packed files
    LogCategory_Asset = 1 << 4,  // Texture/mesh/shader loaders, resource manager

    // 2. Pipeline & Subsystems
    LogCategory_Render = 1 << 5,  // Rendering pipeline, bindings, draw calls, shaders
    LogCategory_Window = 1 << 6,  // Window creation, events, keyboard/mouse/gamepad input
    LogCategory_Audio = 1 << 7,   // Sound playback, BGM, SFX, FMOD/Wwise
    LogCategory_Physics = 1 << 8, // Collision handling, rigid body physics, raycasting

    // 3. Gameplay & Scripting
    LogCategory_Scene = 1 << 9,   // Scene transitions, scene graph, entity/component (ECS)
    LogCategory_Script = 1 << 10, // Lua/C# bindings, scripting integration
    LogCategory_UI = 1 << 11,     // Engine editor UI (ImGui) & viewport UI
};
} // namespace ho