# PowerShell script to build Dyscover for Windows locally

# Set error handling
$ErrorActionPreference = "Stop"

# Define variables
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build-windows"
$VcpkgRoot = "C:\vcpkg"  # Adjust this path to your vcpkg installation
$Generator = "Visual Studio 17 2022"
$Arch = "x64"
$Language = "nl"
$Licensing = "demo"

# Check if CMake is installed
function Test-CMake {
    try {
        $cmakeVersion = & cmake --version 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Host "CMake found: $cmakeVersion"
            return $true
        }
    } catch {
        # CMake not found
    }
    return $false
}

# Install CMake if not found
if (-not (Test-CMake)) {
    Write-Host "CMake not found. Installing CMake..."
    
    # Check if CMake installer already exists
    $cmakePath = "C:\Program Files\CMake\bin\cmake.exe"
    $installerPath = Join-Path $env:TEMP "cmake-installer.msi"
    
    if (Test-Path $cmakePath) {
        Write-Host "CMake is already installed at $cmakePath"
        # Add CMake to PATH for this session
        $env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
    } else {
        # Download CMake installer if not already downloaded
        if (-not (Test-Path $installerPath)) {
            $cmakeUrl = "https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-windows-x86_64.msi"
            try {
                Invoke-WebRequest -Uri $cmakeUrl -OutFile $installerPath
                Write-Host "Downloaded CMake installer to $installerPath"
            } catch {
                Write-Error "Failed to download CMake installer"
                exit 1
            }
        } else {
            Write-Host "CMake installer already downloaded at $installerPath"
        }
        
        # Install CMake silently
        $installArgs = "/i `"$installerPath`" /quiet /norestart"
        Start-Process -FilePath "msiexec.exe" -ArgumentList $installArgs -Wait
        Write-Host "CMake installed successfully"
        
        # Add CMake to PATH for this session
        $env:PATH = "C:\Program Files\CMake\bin;$env:PATH"
        Write-Host "Added CMake to PATH"
        
        # Clean up installer
        Remove-Item $installerPath -Force
    }
}

# Install required vcpkg packages (vcpkg will skip if already installed)
Write-Host "Ensuring required vcpkg packages are installed..."
$installArgs = @("wxwidgets", "portaudio", "gettext[tools]", "--recurse")
& "$VcpkgRoot\vcpkg.exe" install @installArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to install vcpkg packages"
    exit $LASTEXITCODE
}

# Set VCPKG_ROOT environment variable for CMake to find gettext tools
$env:VCPKG_ROOT = $VcpkgRoot
Write-Host "Set VCPKG_ROOT=$VcpkgRoot for CMake gettext detection"

# Create build directory
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

# Setup vcpkg binary caching
New-Item -ItemType Directory -Force -Path "$env:LOCALAPPDATA\vcpkg\archives" | Out-Null
& "$VcpkgRoot\vcpkg.exe" integrate install

# Configure CMake
Push-Location $BuildDir
try {
    $cmakeArgs = @(
        "-S", $ProjectRoot,
        "-G", $Generator, "-A", $Arch,
        "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot\scripts\buildsystems\vcpkg.cmake",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DLANGUAGE=$Language",
        "-DLICENSING=$Licensing",
        "-DPACKAGING_ENABLE=ON",
        "-DPACKAGING_DEBUG_SYMBOLS=ON",
        "-DBUILD_TESTS=OFF"
    )
    & cmake $cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
} finally {
    Pop-Location
}

# Build
cmake --build $BuildDir --config Release --target Dyscover -j $env:NUMBER_OF_PROCESSORS
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Copy dependencies
$DistDir = Join-Path $BuildDir "dist-windows-Release"
New-Item -Force -ItemType Directory $DistDir | Out-Null
Copy-Item (Join-Path $BuildDir "Release\Dyscover.exe") $DistDir

# Copy ALL DLLs from vcpkg installed bin directory (wxWidgets brings in many transitive dependencies)
Write-Host "Copying all DLLs from vcpkg..."
$VcpkgBinDir = "$VcpkgRoot\installed\x64-windows\bin"
$VcpkgToolsBinDir = "$VcpkgRoot\installed\x64-windows\tools\*\bin"

$allDLLs = @()
if (Test-Path $VcpkgBinDir) {
    $allDLLs += @(Get-ChildItem -Path $VcpkgBinDir -Filter "*.dll" -ErrorAction SilentlyContinue)
}
# Also check tools subdirectories
$allDLLs += @(Get-ChildItem -Path $VcpkgToolsBinDir -Filter "*.dll" -ErrorAction SilentlyContinue)

if ($allDLLs.Count -gt 0) {
    foreach ($dll in $allDLLs) {
        try {
            Copy-Item $dll.FullName $DistDir -ErrorAction SilentlyContinue
        } catch {
            # Silently skip files that can't be copied
        }
    }
    Write-Host "Copied $($allDLLs.Count) DLLs from vcpkg"
} else {
    Write-Host "Warning: No DLLs found in vcpkg bin directory"
}

# Copy librstts
$LibRsttsDll = Join-Path $ProjectRoot "lib\rstts\platforms\x86_64-pc-win64\librstts-2.dll"
if (Test-Path $LibRsttsDll) {
    Copy-Item $LibRsttsDll $DistDir
}

# Copy audio and TTS data
$AudioDir = Join-Path $BuildDir "Release\audio"
if (Test-Path $AudioDir) { 
    Copy-Item -Recurse $AudioDir $DistDir
}
$TtsDir = Join-Path $BuildDir "Release\tts"
if (Test-Path $TtsDir) { 
    Copy-Item -Recurse $TtsDir $DistDir
}

Write-Host "Build complete! Executable is at: $DistDir\Dyscover.exe"
