Clevy
=====

This repository contains source code of Clevy Dyscover.

Dependencies
------------

 - C++ compiler that supports at least C++17 (recommended: GCC >= 9, Clang >= 10, MSVC/Visual Studio 2019 16.8+)
 - CMake (version 3.15 or above)
- wxWidgets (version 3.0 or above)

Building
--------

```
cmake -S . -B build
cmake --build build

Optional build toggles (CMake cache options):

- ENABLE_CXX20=ON — allow building the project with C++20 instead of the default C++17. The build will still assert a minimum of C++17 and will fail if the compiler does not support the requested standard.
- BUILD_WITH_LIBRSTTS=OFF — skip linking and packaging the librstts runtime (useful for CI images or platforms that don't include the prebuilt librstts binary). Default: ON.

CI
--
This repository now includes a GitHub Actions CI workflow which verifies C++17 compiler support on Linux, macOS and Windows runners. The CI currently performs a lightweight C++17 smoke compile test so it can validate toolchain support without requiring heavy binary dependencies.

CI caching and faster builds
--------------------------

To speed up CI runs and reduce cross-run build time, our GitHub Actions workflows now cache compiler caches (ccache) and build directories for full product builds where possible:

- Linux and macOS jobs attempt to install and use ccache and cache ~/.ccache across runs.
- Full product builds on Linux/macOS/Windows cache the CMake build directories (e.g., build-full, build-mac, build-win) to speed up incremental builds.

If you operate self-hosted runners, consider configuring a persistent ccache directory and a clean caching policy suitable for your runners.

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
