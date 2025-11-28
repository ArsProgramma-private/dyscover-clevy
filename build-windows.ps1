#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Build Dyscover (Clevy) on Windows with automatic dependency management.
.DESCRIPTION
  Automatically installs CMake and vcpkg packages, then builds Dyscover with configurable options.
  Uses vcpkg for dependency management (wxWidgets, PortAudio, gettext).
.PARAMETER Config
  Build configuration: Debug or Release (default: Release)
.PARAMETER Arch
  Target architecture: x64 (default: x64)
.PARAMETER Language
  UI language code (e.g., nl, nl_be, en) (default: nl)
.PARAMETER Licensing
  License mode: demo or full (default: demo)
.PARAMETER Tests
  Build and run tests: On or Off (default: Off)
.PARAMETER Integration
  Build integration tests: On or Off (default: Off)
.PARAMETER Package
  Create distribution package: On or Off (default: On)
.PARAMETER Generator
  CMake generator: MSVC, MinGW, or Ninja (default: MSVC)
.PARAMETER VcpkgRoot
  Path to vcpkg installation (default: C:\vcpkg)
.EXAMPLE
  .\build-windows.ps1
  Build with defaults (Release, nl, demo, no tests)
.EXAMPLE
  .\build-windows.ps1 -Config Debug -Language nl_be -Licensing full -Tests On
  Build debug version with Belgian Dutch UI, full licensing, and tests
.EXAMPLE
  .\build-windows.ps1 -Generator Ninja -Tests On
  Build with Ninja generator and run tests
.NOTES
  Automatically installs CMake if not found and manages vcpkg dependencies.
  Requires: Visual Studio 2022 (for MSVC generator) or appropriate compiler for MinGW/Ninja.
#>
param(
    [ValidateSet('Debug','Release')] 
    [string]$Config = 'Release',
  
    [ValidateSet('x64')] 
    [string]$Arch = 'x64',
  
    # Default language updated to full locale code to match layout directories (res/layouts/*/nl_nl)
    # Previous default 'nl' caused zero layouts discovered, preventing TTS resource copy and failing Speech init.
    [string]$Language = 'nl_nl',
  
  [ValidateSet('demo','full')] 
  #[string]$Licensing = 'demo',
  [string]$Licensing = 'full',
  
  [ValidateSet('On','Off')] 
  [string]$Tests = 'Off',
  
  [ValidateSet('On','Off')] 
  [string]$Integration = 'Off',
  
  [ValidateSet('On','Off')] 
  [string]$Package = 'On',
  
  [ValidateSet('MSVC','MinGW','Ninja')] 
  [string]$Generator = 'MSVC',
  
  [string]$VcpkgRoot = 'C:\vcpkg'
)

# Set error handling
$ErrorActionPreference = "Stop"

# Define variables
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build-windows-$Config"

# Functions
function Test-CMake {
    try {
        $cmakeVersion = & cmake --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[Info] CMake found: $($cmakeVersion -split "`n" | Select-Object -First 1)" -ForegroundColor Green
            return $true
        }
    } catch {
        # CMake not found
    }
    return $false
}

# Check and install CMake if needed
Write-Host "[Setup] Checking for CMake..." -ForegroundColor Cyan
if (-not (Test-CMake)) {
    Write-Host "[Setup] CMake not found. Installing CMake..." -ForegroundColor Yellow
    
    $cmakePath = "C:\Program Files\CMake\bin\cmake.exe"
    $installerPath = Join-Path $env:TEMP "cmake-installer.msi"
    
    if (Test-Path $cmakePath) {
        Write-Host "[Setup] CMake is already installed at $cmakePath" -ForegroundColor Green
        $env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
    } else {
        if (-not (Test-Path $installerPath)) {
            $cmakeUrl = "https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-windows-x86_64.msi"
            Write-Host "[Setup] Downloading CMake from $cmakeUrl..." -ForegroundColor Yellow
            try {
                Invoke-WebRequest -Uri $cmakeUrl -OutFile $installerPath
                Write-Host "[Setup] Downloaded CMake installer" -ForegroundColor Green
            } catch {
                Write-Error "Failed to download CMake installer: $_"
                exit 1
            }
        }
        
        Write-Host "[Setup] Installing CMake silently..." -ForegroundColor Yellow
        $installArgs = "/i `"$installerPath`" /quiet /norestart"
        Start-Process -FilePath "msiexec.exe" -ArgumentList $installArgs -Wait
        Write-Host "[Setup] CMake installed successfully" -ForegroundColor Green
        
        $env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
        Remove-Item $installerPath -Force
    }
}

# Install vcpkg packages
Write-Host "[Setup] Ensuring vcpkg packages are installed..." -ForegroundColor Cyan
$vcpkgExe = Join-Path $VcpkgRoot "vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    Write-Error "vcpkg not found at $vcpkgExe. Please install vcpkg first or specify correct path with -VcpkgRoot"
    exit 1
}

$installArgs = @("wxwidgets", "portaudio", "gettext[tools]", "--recurse")
& $vcpkgExe install @installArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to install vcpkg packages"
    exit $LASTEXITCODE
}
Write-Host "[Setup] vcpkg packages ready" -ForegroundColor Green

# Set VCPKG_ROOT environment variable
$env:VCPKG_ROOT = $VcpkgRoot
Write-Host "[Setup] Set VCPKG_ROOT=$VcpkgRoot" -ForegroundColor Green

# Create build directory
if (Test-Path $BuildDir) {
    Write-Host "[Build] Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

# Setup vcpkg binary caching
New-Item -ItemType Directory -Force -Path "$env:LOCALAPPDATA\vcpkg\archives" | Out-Null
& $vcpkgExe integrate install | Out-Null

# Configure CMake
Write-Host "[Build] Configuring CMake project ($Config, $Arch, Language=$Language, Licensing=$Licensing)..." -ForegroundColor Cyan

# Select generator
switch ($Generator) {
    'MSVC'  { $generatorArgs = @('-G', 'Visual Studio 17 2022', '-A', $Arch) }
    'MinGW' { $generatorArgs = @('-G', 'MinGW Makefiles') }
    'Ninja' { $generatorArgs = @('-G', 'Ninja') }
}

$cmakeArgs = @(
    '-S', $ProjectRoot,
    '-B', $BuildDir
) + $generatorArgs + @(
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot\scripts\buildsystems\vcpkg.cmake",
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DLANGUAGE=$Language",
    "-DLICENSING=$Licensing",
    "-DBUILD_TESTS=$Tests",
    "-DBUILD_INTEGRATION_TESTS=$Integration"
)

if ($Package -eq 'On') {
    $cmakeArgs += @(
        "-DPACKAGING_ENABLE=ON",
        "-DPACKAGING_DEBUG_SYMBOLS=ON"
    )
}

& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
    exit $LASTEXITCODE
}

# Build
Write-Host "[Build] Building target Dyscover..." -ForegroundColor Cyan
$buildArgs = @('--build', $BuildDir, '--target', 'Dyscover')
if ($Generator -eq 'MSVC') {
    $buildArgs += @('--config', $Config)
}
if ($env:NUMBER_OF_PROCESSORS) {
    $buildArgs += @('-j', $env:NUMBER_OF_PROCESSORS)
}

& cmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit $LASTEXITCODE
}
Write-Host "[Build] Build completed successfully" -ForegroundColor Green

# Run tests if enabled
if ($Tests -eq 'On') {
    Write-Host "[Test] Running unit tests..." -ForegroundColor Cyan
    $testArgs = @('--test-dir', $BuildDir, '--output-on-failure')
    if ($Generator -eq 'MSVC') {
        $testArgs += @('-C', $Config)
    }
    & ctest @testArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Some tests failed"
    } else {
        Write-Host "[Test] All tests passed" -ForegroundColor Green
    }
}

# Package if enabled
if ($Package -eq 'On') {
    Write-Host "[Package] Creating distribution package..." -ForegroundColor Cyan
    
    $DistDir = Join-Path $BuildDir "dist-windows-$Config"
    if (Test-Path $DistDir) {
        Remove-Item -Recurse -Force $DistDir
    }
    New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
    
    # Determine exe location based on generator
    if ($Generator -eq 'MSVC') {
        $exePath = Join-Path $BuildDir "$Config\Dyscover.exe"
        $buildOutputDir = Join-Path $BuildDir $Config
    } else {
        $exePath = Join-Path $BuildDir "Dyscover.exe"
        $buildOutputDir = $BuildDir
    }
    
    if (-not (Test-Path $exePath)) {
        Write-Error "Built executable not found at $exePath"
        exit 1
    }
    
    Copy-Item $exePath $DistDir
    Write-Host "[Package] Copied Dyscover.exe" -ForegroundColor Green
    
    # Copy ALL DLLs from vcpkg (wxWidgets has many transitive dependencies)
    Write-Host "[Package] Copying vcpkg DLLs..." -ForegroundColor Cyan
    $VcpkgBinDir = Join-Path $VcpkgRoot "installed\x64-windows\bin"
    $VcpkgToolsBinDir = Join-Path $VcpkgRoot "installed\x64-windows\tools\*\bin"
    
    $copiedDLLs = 0
    if (Test-Path $VcpkgBinDir) {
        Get-ChildItem -Path $VcpkgBinDir -Filter "*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
            try {
                Copy-Item $_.FullName $DistDir -ErrorAction SilentlyContinue
                $copiedDLLs++
            } catch {
                # Silently skip files that can't be copied
            }
        }
    }
    
    # Also check tools subdirectories
    Get-ChildItem -Path $VcpkgToolsBinDir -Filter "*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
        try {
            Copy-Item $_.FullName $DistDir -ErrorAction SilentlyContinue
            $copiedDLLs++
        } catch {
            # Silently skip
        }
    }
    
    if ($copiedDLLs -gt 0) {
        Write-Host "[Package] Copied $copiedDLLs DLLs from vcpkg" -ForegroundColor Green
    } else {
        Write-Warning "No DLLs found in vcpkg directories"
    }
    
    # Copy librstts (correct path for Windows)
    $LibRsttsDll = Join-Path $ProjectRoot "lib\rstts\platforms\x86_64-pc-win64\librstts-2.dll"
    if (Test-Path $LibRsttsDll) {
        Copy-Item $LibRsttsDll $DistDir
        Write-Host "[Package] Copied librstts-2.dll" -ForegroundColor Green
    } else {
        Write-Warning "librstts-2.dll not found at $LibRsttsDll"
    }
    
    # Copy audio and TTS data directories
    $AudioDir = Join-Path $buildOutputDir "audio"
    if (Test-Path $AudioDir) { 
        Copy-Item -Recurse $AudioDir $DistDir
        Write-Host "[Package] Copied audio directory" -ForegroundColor Green
    }
    
    $TtsDir = Join-Path $buildOutputDir "tts"
    if (Test-Path $TtsDir) { 
        Copy-Item -Recurse $TtsDir $DistDir
        Write-Host "[Package] Copied TTS directory" -ForegroundColor Green
    }
    
    Write-Host "[Done] Package created at: $DistDir" -ForegroundColor Green
    Write-Host "[Done] Executable: $DistDir\Dyscover.exe" -ForegroundColor Green
} else {
    Write-Host "[Done] Build complete (packaging skipped)" -ForegroundColor Green
    if ($Generator -eq 'MSVC') {
        Write-Host "[Done] Executable: $BuildDir\$Config\Dyscover.exe" -ForegroundColor Green
    } else {
        Write-Host "[Done] Executable: $BuildDir\Dyscover.exe" -ForegroundColor Green
    }
}