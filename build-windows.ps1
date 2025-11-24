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
    cmake -S $ProjectRoot `
        -G $Generator -A $Arch `
        -DCMAKE_TOOLCHAIN_FILE="$VcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
        -DCMAKE_BUILD_TYPE=Release `
        -DLANGUAGE=$Language `
        -DLICENSING=$Licensing `
        -DPACKAGING_ENABLE=ON `
        -DPACKAGING_DEBUG_SYMBOLS=ON `
        -DBUILD_TESTS=OFF
} finally {
    Pop-Location
}

# Build
cmake --build $BuildDir --config Release --target Dyscover -j $env:NUMBER_OF_PROCESSORS

# Copy dependencies
$DistDir = Join-Path $BuildDir "dist-windows-Release"
New-Item -Force -ItemType Directory $DistDir | Out-Null
Copy-Item (Join-Path $BuildDir "Release\Dyscover.exe") $DistDir

# Copy wxWidgets DLLs
$WxDlls = Get-ChildItem "$VcpkgRoot\installed\x64-windows\bin\wx*.dll"
foreach ($dll in $WxDlls) { 
    Copy-Item $dll.FullName $DistDir
}

# Copy PortAudio if present
$PortAudioDll = "$VcpkgRoot\installed\x64-windows\bin\portaudio*.dll"
if (Test-Path $PortAudioDll) {
    Copy-Item $PortAudioDll $DistDir
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
