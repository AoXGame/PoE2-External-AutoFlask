# POE2 External Auto Flask

External automation tool for Path of Exile 2 that monitors player health and mana, automatically triggering flask usage when thresholds are reached.

## Architecture

### Core Components

- **MemoryReader** (`memory.h/cpp`): Handles process attachment, memory reading/writing, and pattern scanning using AOB (Array of Bytes) signatures
- **GameDataReader** (`game.h/cpp`): Locates and reads player character data structures from game memory
- **FlaskManager** (`flask.h/cpp`): Implements flask activation logic with configurable thresholds and randomization
- **MemoryPatcher** (`patcher.h/cpp`): Applies runtime memory patches for game modifications
- **GUI** (`gui.h/cpp`): ImGui-based interface for real-time monitoring and configuration
- **Config** (`config.h`): Centralized configuration structure

### Memory Scanning

The tool uses dynamic memory scanning to locate the player character structure:

1. Scans process memory regions with `MEM_COMMIT` and `PAGE_READWRITE` permissions
2. Filters candidates based on HP/MP value ranges (50-5000 for max values)
3. Validates by checking if current HP/MP are within reasonable bounds
4. Re-scans every 2 seconds to handle address changes

### Process Detection

Supports both Steam and Epic Games Store versions:
- `PathOfExile2_x64Steam.exe`
- `PathOfExile2_x64EGS.exe`

Requires `SE_DEBUG_PRIVILEGE` for process memory access.

## Build Requirements

- **Compiler**: MSVC 2019 or later (C++17)
- **Platform**: Windows x64
- **Dependencies**:
  - DirectX 11 (`d3d11.lib`, `d3dcompiler.lib`)
  - ImGui (included as submodule)
  - Windows SDK 10.0+

## Compilation

```bash
# Open POE2_AutoFlask.vcxproj in Visual Studio
# Select Release configuration
# Build Solution (Ctrl+Shift+B)
```

Output: `x64/Release/POE2_AutoFlask.exe`

## Configuration

### Runtime Configuration (GUI)

- **HP Threshold**: 0.0 - 1.0 (percentage of max HP)
- **MP Threshold**: 0.0 - 1.0 (percentage of max MP)
- **Auto HP/MP**: Enable/disable flask automation
- **Memory Patches**: Toggle runtime patches

### Code Configuration (`config.h`)

```cpp
struct Config {
    float hpThreshold = 0.6f;
    float mpThreshold = 0.6f;
    bool autoHP = true;
    bool autoMP = true;
    bool patch1Enabled = false;
    bool patch2Enabled = false;
};
```

### Flask Keys

Default key bindings are defined in `flask.h`:
- HP Flask: `VK_NUMPAD1` (Numpad 1)
- MP Flask: `VK_NUMPAD2` (Numpad 2)

## Memory Patches

### Patch 1: Zoomhack [check unknowncheats.me\pathOfexile reverse thread for update]
- **Address**: `PathOfExileSteam.exe+10F183`
- **Original**: `F3 0F 5D 05 4D F9 02 03` (minss xmm0, [offset])
- **Patch**: NOPs the instruction (5 bytes)

### Patch 2: Map Reveal [check unknowncheats.me\pathOfexile reverse thread for update]
- **Address**: `PathOfExileSteam.exe+EBA847+4`
- **Original**: `00`
- **Patch**: `01`

**Note**: Patches may require signature updates after game updates.

## Technical Details

### Memory Access

- Uses `OpenProcess` with `PROCESS_ALL_ACCESS`
- `ReadProcessMemory` / `WriteProcessMemory` for data operations
- `VirtualQueryEx` for memory region enumeration

### Pattern Scanning

AOB scanning implementation:
- Wildcard support (??) for variable bytes
- Alignment-aware searching
- Memory protection filtering

### Anti-Debug

Basic PEB-based debugger detection via `obfuscation.h`.

## Project Structure

```
POE2_AutoFlask_CPP/
├── main.cpp          # Entry point, main loop
├── memory.h/cpp      # Memory operations
├── game.h/cpp        # Game data reading
├── flask.h/cpp       # Flask automation
├── patcher.h/cpp     # Memory patching
├── gui.h/cpp         # ImGui interface
├── config.h          # Configuration
├── obfuscation.h     # Anti-analysis
├── resource.rc       # Resources (icon)
└── imgui/            # ImGui library
```
