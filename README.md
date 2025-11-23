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
```
```
