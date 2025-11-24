# PowerShell script to install vcpkg on Windows

# Set error handling
$ErrorActionPreference = "Stop"

# Define vcpkg installation directory
$VcpkgRoot = "C:\vcpkg"

# Check if vcpkg is already installed
if (Test-Path $VcpkgRoot) {
    Write-Host "vcpkg is already installed at $VcpkgRoot"
    exit 0
}

# Clone vcpkg repository
Write-Host "Cloning vcpkg repository..."
git clone https://github.com/Microsoft/vcpkg.git $VcpkgRoot

# Change to vcpkg directory
Push-Location $VcpkgRoot

try {
    # Bootstrap vcpkg
    Write-Host "Bootstrapping vcpkg..."
    .\bootstrap-vcpkg.bat
    
    # Integrate with system
    Write-Host "Integrating vcpkg with Visual Studio..."
    .\vcpkg integrate install
    
    # Install required packages
    Write-Host "Installing required packages..."
    .\vcpkg install wxwidgets portaudio
    
    Write-Host "vcpkg installation complete!"
    Write-Host "vcpkg root: $VcpkgRoot"
    Write-Host ""
    Write-Host "To use vcpkg in your projects, add this to your CMake command:"
    Write-Host "-DCMAKE_TOOLCHAIN_FILE=""$VcpkgRoot\scripts\buildsystems\vcpkg.cmake"""
    
} finally {
    Pop-Location
}
