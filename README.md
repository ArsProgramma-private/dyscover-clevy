Clevy
=====

This repository contains source code of Clevy Dyscover.

Dependencies
------------

### Cross-Platform Requirements
 - C++ compiler that supports at least C++17 (recommended: GCC >= 9, Clang >= 10, MSVC/Visual Studio 2019 16.8+)
 - CMake (version 3.15 or above)
 - wxWidgets (version 3.0 or above)

### Platform-Specific Dependencies

#### Windows (MSVC)
- wxWidgets development libraries
- PortAudio (optional, can disable with `-DBUILD_WITH_PORTAUDIO=OFF`)
- Windows SDK (for Config Manager and multimedia APIs)

#### Linux (GCC/Clang)
Install via package manager (Debian/Ubuntu example):
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake pkg-config \
    libwxgtk3.0-gtk3-dev \
    libudev-dev \
    libasound2-dev \
    libpulse-dev \
    portaudio19-dev \
    gettext
```

**Optional audio backends:**
- `libpulse-dev` — PulseAudio support (recommended for modern Linux desktops)
- `libasound2-dev` — ALSA support (fallback for systems without PulseAudio)

If neither is installed, audio volume control will report as unsupported.

#### macOS (Clang/Xcode)
Install Homebrew packages:
```bash
brew install wxwidgets portaudio gettext cmake
```

**System frameworks** (automatically linked):
- CoreAudio — Audio volume control
- AudioToolbox — Audio device management
- IOKit — USB device detection
- CoreFoundation — System utilities

#### ChromeOS (Crostini)
Same as Linux dependencies. Note: Some APIs may be restricted in Crostini container:
- Device detection may require additional permissions
- Audio control may fall back to unsupported in sandboxed environments

Building
--------

### Quick Start (All Platforms)
```bash
cmake -S . -B build
cmake --build build
```

### Platform-Specific Build Examples

#### Windows (Visual Studio 2019+)
```cmd
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```

#### Linux
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### macOS
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Build Options (CMake cache options)

- `ENABLE_CXX20=ON` — Allow building with C++20 instead of C++17 (default: OFF)
- `BUILD_WITH_LIBRSTTS=OFF` — Skip linking librstts TTS library (default: ON)
- `BUILD_WITH_PORTAUDIO=OFF` — Disable PortAudio support (default: ON)
- `BUILD_TESTS=ON` — Enable unit tests (default: ON)
- `BUILD_INTEGRATION_TESTS=ON` — Enable hardware-dependent integration tests (default: OFF)
- `LANGUAGE=nl` — Set UI language (options: `nl`, `nl_be`)
- `LICENSING=demo` — Set licensing mode (options: `demo`, `full`, `none`)

Example with options:
```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DLANGUAGE=nl \
    -DLICENSING=demo
cmake --build build
```

CI
--
This repository now includes a GitHub Actions CI workflow which verifies C++17 compiler support on Linux, macOS and Windows runners. The CI currently performs a lightweight C++17 smoke compile test so it can validate toolchain support without requiring heavy binary dependencies.

### CI Build Matrix
| Platform | Runner | Compiler | Build Type | Audio Backend | Notes |
|----------|--------|----------|------------|---------------|-------|
| Linux (Ubuntu) | `ubuntu-latest` | GCC 9+ | Release | ALSA (PulseAudio unavailable in CI) | Full platform abstraction |
| macOS (x86_64) | `macos-latest` | Apple Clang | Release | CoreAudio | IOKit device detection |
| Windows (x64) | `windows-latest` | MSVC 19.28+ | Release | waveOut | Config Manager device detection |

**Integration Tests:** Run on self-hosted runners with `hardware` label (requires physical USB devices). Manually triggered via workflow dispatch with `run_integration=true` and `INTEGRATION_RUN_TOKEN` secret configured.

### CI Security & Compliance Gates
Full product build jobs now include automated quality checks:

- **Hardening Verification:** Each platform runs `verify-hardening.sh` / `verify-hardening.ps1` to confirm PIE/ASLR, NX/DEP, RELRO, and code signing status.
- **Dependency Audit:** Extended audit scripts (`audit-deps.sh` / `audit-deps.ps1`) output SHA256 hashes of all linked libraries plus RPATH/LC_RPATH inspection. Results uploaded as CI artifacts.
- **SBOM Generation:** Linux and macOS jobs generate CycloneDX SBOM + license matrix using `syft` and upload to artifacts (`sbom-linux`, `sbom-macos`).

Artifacts available after each CI run:
- `sbom-linux` / `sbom-macos`: CycloneDX JSON, license CSV, audit logs
- `audit-windows`: Windows dependency audit with SHA256 hashes

Future: Automated signature chain verification and SBOM diff analysis on PRs.

CI caching and faster builds
--------------------------

To speed up CI runs and reduce cross-run build time, our GitHub Actions workflows now cache compiler caches (ccache) and build directories for full product builds where possible:

- Linux and macOS jobs attempt to install and use ccache and cache ~/.ccache across runs.
- Full product builds on Linux/macOS/Windows cache the CMake build directories (e.g., build-full, build-mac, build-win) to speed up incremental builds.

If you operate self-hosted runners, consider configuring a persistent ccache directory and a clean caching policy suitable for your runners.

Automated ccache cleanup for self-hosted runners
------------------------------------------------

To prevent unbounded growth of ccache directories on self-hosted runners, use the provided cleanup script at `scripts/cleanup-ccache.sh`. This script:

- Removes cache files older than a specified age (default: 30 days)
- Enforces a maximum cache size limit (default: 10GB)
- Removes empty directories
- Uses ccache's built-in cleanup if available

Usage:

```bash
# Use defaults (10GB, 30 days, using $CCACHE_DIR or ~/.ccache)
./scripts/cleanup-ccache.sh

# Custom location, size, and age
./scripts/cleanup-ccache.sh /var/cache/ccache 20 14  # 20GB max, 14 days retention
```

**Automated cleanup via cron** (example — runs daily at 2am):

```bash
# Add to crontab (crontab -e)
0 2 * * * /path/to/dyscover-clevy/scripts/cleanup-ccache.sh /var/cache/ccache 10 30 >> /var/log/ccache-cleanup.log 2>&1
```

**Automated cleanup via systemd timer** (recommended for production):

Create `/etc/systemd/system/ccache-cleanup.service`:

```
[Unit]
Description=Clean up ccache directory
After=network.target

[Service]
Type=oneshot
User=runner
ExecStart=/path/to/dyscover-clevy/scripts/cleanup-ccache.sh /var/cache/ccache 10 30
StandardOutput=journal
StandardError=journal
```

Create `/etc/systemd/system/ccache-cleanup.timer`:

```
[Unit]
Description=Daily ccache cleanup timer

[Timer]
OnCalendar=daily
Persistent=true

[Install]
WantedBy=timers.target
```

Enable and start the timer:

```bash
sudo systemctl daemon-reload
sudo systemctl enable ccache-cleanup.timer
sudo systemctl start ccache-cleanup.timer
# Check status: sudo systemctl status ccache-cleanup.timer
```

Cache tuning in CI
------------------

The CI cache keys are tuned to include both top-level `CMakeLists.txt` and source directory contents where possible, and cache entries use a short retention window (14 days) so that caches don't grow without bound. The workflows also use restore-keys to allow fallbacks when exact cache keys are not found. This improves cache hit rates and reduces rebuild time across small changes while keeping cache storage manageable.

Self-hosted runners: persistent ccache (recommended)
--------------------------------------------------

If you run these workflows on a self-hosted runner (especially for integration/hardware runs), it's recommended to configure a persistent ccache directory to improve build performance across jobs. Below is a minimal example that configures a shared cache at `/var/cache/ccache` and ensures the service user can access it.

1. Install and configure ccache globally (example for Debian/Ubuntu):

```bash
sudo apt-get update
sudo apt-get install -y ccache
sudo mkdir -p /var/cache/ccache
sudo chown -R $(whoami):$(whoami) /var/cache/ccache
# Optionally set system-wide env for runner service: /etc/environment
echo 'CCACHE_DIR=/var/cache/ccache' | sudo tee -a /etc/environment
```

2. If using a runner service account (systemd), ensure the runner process inherits CCACHE_DIR. Example systemd snippet for a runner service unit (adjust paths and user):

```
[Unit]
Description=GitHub Actions Runner (example)
After=network.target

[Service]
User=runner
Environment=CCACHE_DIR=/var/cache/ccache
ExecStart=/home/runner/actions-runner/run.sh
Restart=always

[Install]
WantedBy=multi-user.target
```

3. Verify cache availability on the runner by confirming ccache --show-config and that the CCACHE_DIR contains cached objects. If you want exclusive caches for certain repositories, use per-repo sub-directories and set CCACHE_DIR accordingly in your runner configuration.

Note: When using shared caches, consider eviction and sizing policies (e.g., automated cron jobs to prune older items) and set appropriate filesystem permissions for the runner user.

Running tests locally
---------------------

To build and run the unit tests (example uses DeviceStaticListTest):

```bash
cmake -S . -B build-tests -DBUILD_TESTS=ON
cmake --build build-tests --target DeviceStaticListTest
ctest --test-dir build-tests --output-on-failure
```

Integration tests (manual)
-------------------------

Integration tests are hardware-dependent and run on a manual CI trigger which is intentionally gated for safety. The workflow requires all of the following to run:

- You must use the workflow's "Run workflow" button and set the input `run_integration=true`.
- The repository must have a repository secret named `INTEGRATION_RUN_TOKEN` configured (non-empty). This is a safety gate to prevent accidental public runs.
- The job is configured to run on a self-hosted runner with the label `hardware` (i.e. `runs-on: [self-hosted, linux, hardware]`). This ensures the integration tests are executed on machines with attached devices.

If any of the above conditions are not met, the job will not run. This is deliberate — integration tests often require physical hardware and special access that public runners don't have.

Locally you can enable integration tests with:

```bash
cmake -S . -B build-integration -DBUILD_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON
cmake --build build-integration --target Integration-DeviceDetectionStaticList
ctest --test-dir build-integration --output-on-failure

Cross-Platform Build Guide
--------------------------
This section provides a production-oriented matrix of build options for each supported platform, including prerequisites, alternative toolchains, and canonical commands. Use the provided helper scripts in `scripts/` for reproducibility.

Windows
-------
Options:
1. MSVC Build Tools + Visual Studio generator (recommended)
2. MSVC Build Tools + Ninja generator (fast single-config)
3. MinGW-w64 (native or cross) via `-Generator MinGW`
4. Cross-compile from Linux using MinGW-w64 toolchain file

Prerequisites:
- Common: CMake ≥3.15, wxWidgets 3.2 (built for chosen toolchain), librstts DLL, optional PortAudio, gettext.
- MSVC: Install "Visual Studio Build Tools" with Windows 11 SDK + C++ core features. Optional: Ninja component.
- MinGW native: MSYS2 with packages: `mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-wxwidgets mingw-w64-x86_64-gettext` (and PortAudio if needed).
- Cross (Linux→Windows): `mingw-w64`, custom-built wxWidgets for target or prebuilt distribution.

Commands:
MSVC (multi-config):
```powershell
pwsh ./scripts/build-windows.ps1 -Config Release -Generator MSVC -Language nl -Licensing demo -Tests On -Package On -CopyDeps On
```
MSVC + Ninja (single-config Release):
```powershell
pwsh ./scripts/build-windows.ps1 -Config Release -Generator Ninja -Language nl -Licensing demo -Tests On -Package On -CopyDeps On
```
MinGW (MSYS2 shell):
```bash
./scripts/build-windows.ps1 -Config Release -Generator MinGW -Language nl -Licensing demo -Package On -CopyDeps On
```
Cross (Linux):
```bash
./scripts/build-windows-cross-mingw.sh -c Release -l nl -L demo --package
```
Manual cross invocation:
```bash
cmake -S . -B build-cross -DCMAKE_TOOLCHAIN_FILE=scripts/toolchains/mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release -DLANGUAGE=nl -DLICENSING=demo
cmake --build build-cross --target Dyscover -j$(nproc)
```

macOS
-----
Options:
1. Clang + CMake (Intel only)
2. Universal build (x86_64 + arm64) via `--universal`
3. Ninja generator (optional for speed)

Prerequisites:
- Xcode command line tools (provides clang & SDKs)
- Homebrew: `brew install wxwidgets portaudio gettext cmake ninja`
- librstts macOS binary in `lib/rstts/platforms/x86_64-apple-darwin/` (and arm64 if universal later)

Commands:
Standard Intel Release:
```bash
./scripts/build-macos.sh -c Release -l nl -L demo --package
```
Universal:
```bash
./scripts/build-macos.sh -c Release --universal -l nl -L demo --package
```
Ninja single-config (manual):
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DLANGUAGE=nl -DLICENSING=demo
cmake --build build -j$(sysctl -n hw.ncpu)
```

Linux
-----
Options:
1. GCC or Clang native build
2. With/without PulseAudio, ALSA fallback
3. Optional benchmarks (`--bench`)

Prerequisites (Debian/Ubuntu example): see earlier dependency list.

Commands:
Release build with tests + benchmarks + packaging:
```bash
./scripts/build-linux.sh -c Release -l nl -L demo --bench --package
```
Minimal no-tests debug:
```bash
./scripts/build-linux.sh --no-tests
```

ChromeOS (Crostini)
-------------------
Options mirror Linux. Some hardware APIs may degrade gracefully.

Prerequisites:
- Same as Linux; ensure `libudev-dev` accessible in container
- Optional audio libs for volume control (PulseAudio/ALSA)

Commands:
```bash
./scripts/build-chromeos.sh -c Release -l nl -L demo --package
```
If sandbox restricts device/audio APIs the app logs capability fallback.

Common Flags & Modes
--------------------
Licensing:
- `-L demo` or `-DLICENSING=demo` sets demo mode.
- `full` enables full licensing paths.
- `none` for development.

Language:
- `-l nl` (Dutch), `-l nl_be` (Flemish).

Tests & Integration:
- Disable tests: `--no-tests` or `-DBUILD_TESTS=OFF`
- Enable integration tests: `--integration` or `-DBUILD_INTEGRATION_TESTS=ON`

Benchmark Targets:
- `DeviceDetectorBenchmark` and `PerformanceSmokeTest` built when tests enabled; invoked automatically with `--bench` on Linux script.

Packaging Notes
---------------
Windows:
- `-Package On -CopyDeps On` collects `Dyscover.exe` + common runtime DLLs into `dist-windows-<Config>`.
- For installer creation consider WiX Toolset or NSIS (future enhancement).

macOS:
- Current packaging step copies binary only; for distribution create an app bundle (`Dyscover.app`) with Info.plist, resources, and codesign.

Linux / ChromeOS:
- Packaging produces `dist-linux-<Config>/Dyscover`. For distribution create `.deb` or `.rpm` using CPack or custom scripts.

Cross-Compilation Caveats
--------------------------
- Cross-building Windows from Linux lacks automatic dependency resolution for wxWidgets; prefer native build for release artifacts.
- Ensure architecture consistency (no mixing x86 DLLs into x64 builds).
- Test runtime on target OS before release.

Verification Checklist
----------------------
1. Binary starts without missing DLL errors.
2. Device detection reports capabilities flags correctly.
3. Keyboard translation tests pass (`KeyboardHandlerTranslateTest`).
4. Audio controller selects backend (or reports unsupported gracefully).
5. Resource locator returns platform-specific icon paths.
6. Manifest embedded (Windows MSVC): extract using `mt.exe -inputresource:Dyscover.exe;#1 -out:manifest.xml`.

Troubleshooting Quick Reference
-------------------------------
| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Missing wx DLL on Windows | Not copied by `-CopyDeps` | Manually copy from wx build lib dir |
| No audio volume control | Missing PulseAudio/ALSA/CoreAudio | Install dev packages / frameworks |
| Unsupported device detection on ChromeOS | Sandbox restrictions | Polling fallback active (expected) |
| Build fails finding librstts | Binary absent for platform | Place DLL/SO in `lib/rstts/platforms/<platform>` |
| Tests crash on headless Linux | xkbcommon unavailable | Install `libxkbcommon-dev` or skip tests |

```

Packaging Extensions
--------------------
New packaging & audit assets have been added:

- CPack toggle: enable/disable via `-DPACKAGING_ENABLE=ON|OFF` (default ON). Generators: WiX (Windows), DragNDrop DMG (macOS), TGZ/ZIP (all), DEB/RPM (Linux if tools present).
- macOS bundle script: `./scripts/package-macos.sh` builds a `.app` + signed DMG (when `--sign` used). Optional universal binary with `--universal`.
- Codesign helper: `./scripts/macos-codesign-example.sh <path/to/Dyscover.app> "Developer ID Application: Corp (TEAMID)"` performs deep signing.
- Dependency audit (Linux/macOS): `./scripts/audit-deps.sh Dyscover build` lists dynamic libs & missing entries.
- Dependency audit (Windows): `pwsh ./scripts/audit-deps.ps1 -Target Dyscover.exe -BuildDir build` (requires VS dev tools in PATH for `dumpbin`).
- Windows Inno Setup template: `scripts/package-windows-inno.iss` (compile with `iscc`). Use `/DSourceDir=dist-windows-Release` pointing at staging folder produced by existing build scripts.
- Windows Squirrel (experimental): `pwsh ./scripts/package-windows-squirrel.ps1` produces a NuGet-based update package (requires `dotnet tool install --global SquirrelCli`). Prefer WiX or Inno for stable native distribution.

Recommended Flow:
1. Build product artifacts (e.g. `pwsh ./scripts/build-windows.ps1 -Config Release -Package On -CopyDeps On`).
2. Audit dependencies before packaging.
3. Run chosen packager: WiX (default CPack), Inno (`iscc`), or Squirrel (experimental).
4. macOS: create & sign DMG via `package-macos.sh --sign "Developer ID Application: ..."` then notarize externally.

Notes:
- Squirrel.Windows is primarily designed for managed/Electron apps; native C++ usage is supported but less common. Treat its artifacts as experimental until validated.
- Inno Setup template generates a lightweight installer; adjust icon, license path, and add registry entries as needed.
- For Linux DEB/RPM output ensure `fakeroot`, `rpmbuild`, and requisite tooling are installed; otherwise CPack silently skips those generators.
- Disable packaging during inner-loop development to speed CMake config: `cmake -S . -B build -DPACKAGING_ENABLE=OFF`.

Release Hardening & Compliance
------------------------------
New security and provenance tooling:

- Debug Symbols: enable `-DPACKAGING_DEBUG_SYMBOLS=ON` to package PDB (Windows), dSYM (macOS), or unstripped debug copy (Linux).
- Notarization (macOS): after signing DMG run `./scripts/notarize-macos.sh dist-macos-Release/Dyscover-<...>.dmg` with `APPLE_NOTARY_KEY_ID`, `APPLE_NOTARY_ISSUER`, `APPLE_NOTARY_KEY_FILE` env vars.
- SBOM & Licenses: `./scripts/generate-sbom.sh build/Dyscover` outputs `sbom/cyclonedx.json` + `sbom/licenses.csv` (requires `syft`).
- Dependency Audit (extended): now prints SHA256 + RPATH (`audit-deps.sh` / `audit-deps.ps1`).
- Hardening Verification: `./scripts/verify-hardening.sh build/Dyscover` or `pwsh ./scripts/verify-hardening.ps1 -Binary build\Dyscover.exe` checks PIE/ASLR/NX/RELRO and code signing.
- Authenticode Signing: `pwsh ./scripts/sign-windows.ps1 -Binary build\Dyscover.exe -CertFile cert.pfx -Password '...'` adds SHA256 + timestamp.

Recommended Release Checklist:
1. Build release artifacts with packaging + symbols.
2. Run hardening + audit scripts; fix any warnings.
3. Generate SBOM; archive with build outputs.
4. Sign Windows binary; create installer (WiX/Inno); verify signature.
5. macOS: sign bundle & DMG, notarize, staple.
6. Publish artifacts with SBOM, checksums, and separate symbols directory.

Future Enhancements:
- Optional delta updates (Squirrel) once native path validated.
- Automated CI gates for SBOM diff & signing chain verification.
```
