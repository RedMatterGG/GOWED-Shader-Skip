# Gears of War: E-Day — Startup Shader-Wait Skip

A standalone x64 Windows DLL that sets the game's native `TC.ShaderCompileBlock.Welcome` variable to zero after initialization, before the welcome screen checks it. No UE4SS runtime is required.

## What it does

- Loads as a local `dwmapi.dll` proxy and forwards Windows DWM exports to the genuine DLL in the system directory.
- Verifies the game executable's filename and SHA-256 before allowing any game-memory writes.
- Checks the recovered constructor and welcome-check instruction bytes, variable layout, vtable identities, and memory access permissions.
- Sets the variable's two integer value slots to `[0,0]` using atomic exchanges.
- Writes diagnostic results to `ShaderSkip.log` beside the loaded DLL.
- Does not patch executable code, modify the game executable on disk, disable the PSO cache, or alter anti-cheat components.

**This skips the blocking welcome-screen wait, not shader compilation itself.** Background precaching remains enabled. Runtime compilation can still cause hitches.

## Installation

Download `dwmapi.dll` from a published GitHub release, or build it using the instructions below. Close the game and copy the DLL beside `GoWEDay-Steam.exe`:

```text
<game installation>/GoWEDay/FairlightConcept/Binaries/Win64/dwmapi.dll
```

Do not overwrite an unrelated existing `dwmapi.dll` proxy without first preserving it and checking compatibility.

## Disable or uninstall

Close the game, then rename or remove the active `dwmapi.dll` in the game's `Win64` directory.

Alternatively, create an empty `ShaderSkip.disabled` file beside the active DLL, then restart the game. This leaves DWM forwarding active but disables game-memory changes.

## Supported executable

```text
Filename: GoWEDay-Steam.exe
SHA-256: 8947c46f97467ba64f4eb8bad7b600c660bb109f2890c431b444238ba6af8e72
Recovered engine version: 5.6.1-4894958++++fenix2+fairlight-omega-release
```

A different executable hash is refused. A game update requires renewed analysis and verification; changing the hash alone is not sufficient. RVAs are applied relative to the actual loaded module base.

Use offline. Online or anti-cheat compatibility has not been tested.

## Source files

- `shader_skip.cpp` — proxy initialization, executable hashing, guarded startup worker, and logging.
- `skip_core.h` — supported-build profile and guarded variable update.
- `dwmapi.asm` — x64 forwarding stubs, preserving register arguments and stack arguments.
- `dwmapi.def` — DLL export names and ordinals.
- `proxy_exports.h` — forwarding metadata.
- `proxy_manifest.json` — source Windows DLL hash and export inventory used during generation.
- `generate_proxy.py` — regenerates forwarding files from the local Windows system DLL; requires Python 3.
- `test_skip.cpp` — native initialized-variable behavior test using synthetic memory, not the game.
- `test_proxy.cpp` — compares a forwarded DWM call with the real Windows DLL.
- `build.bat` — builds the DLL and runs both native tests.
- `test.bat` — runs the variable behavior test alone.

## Build

Requires Visual Studio C++ x64 tools, the Windows SDK, and MASM (`ml64`). The supplied scripts use this installed toolchain:

```text
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat
```

If using a different installation, edit that path in `build.bat` and `test.bat`.

Run `build.bat` from Command Prompt. The generated forwarding files are included, so Python is not required for a normal rebuild. To regenerate those files on another Windows installation, first run `python generate_proxy.py`, then `build.bat`.

Building creates object files, test executables, the DLL, and a harness log in this folder. It does not automatically install the DLL into the game directory.

## Verification performed

- Observed the initialized-variable test fail before implementing the updater, then pass after implementation.
- Built the x64 DLL with MSVC and MASM.
- Verified `DwmIsCompositionEnabled` forwarding against the genuine Windows DLL.
- Verified the installed DLL hash matches the built DLL.
- Launched the actual game: the DLL logged success, independent read-only memory inspection returned `[0,0]`, and the startup compilation screen was bypassed. The user also confirmed the skip worked.
- Restored `Engine.ini` byte-for-byte to its pre-investigation backup, preserving its read-only attribute and existing tweaks.

This verifies the startup skip, not a full gameplay regression test or online compatibility. The proxy build emitted two linker advisories about making COM exports private; the DLL linked and the forwarding harness passed.
