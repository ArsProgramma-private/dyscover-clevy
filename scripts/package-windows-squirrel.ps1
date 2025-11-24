Param(
  [string]$Config = "Release",
  [string]$BuildDir = "build",
  [string]$OutDir = "dist-squirrel"
)
# Template script: creates a NuSpec + invokes Squirrel (requires Squirrel.Windows installed).
# Install Squirrel.Windows tooling (PowerShell):
#   dotnet tool install --global SquirrelCli
# Then available as 'squirrel'. This approach is experimental for native C++ apps.

$ErrorActionPreference = "Stop"
$exe = Join-Path $BuildDir "Dyscover.exe"
if (!(Test-Path $exe)) { Write-Error "Executable not found: $exe" }
New-Item -Force -ItemType Directory $OutDir | Out-Null
$nuspec = @"
<?xml version="1.0"?>
<package >
  <metadata>
    <id>Dyscover</id>
    <version>4.0.5.0</version>
    <title>Clevy Dyscover 4</title>
    <authors>Alt. Medical B.V.</authors>
    <owners>Alt. Medical B.V.</owners>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>Clevy Dyscover accessibility utility</description>
    <projectUrl>https://clevy.com</projectUrl>
  </metadata>
</package>
"@
$nuspecPath = Join-Path $OutDir "Dyscover.nuspec"
$nuspec | Out-File -Encoding UTF8 $nuspecPath
# Create layout
$layout = Join-Path $OutDir "layout"
New-Item -Force -ItemType Directory $layout | Out-Null
Copy-Item $exe $layout/
Get-ChildItem (Join-Path $BuildDir *.dll) | Copy-Item -Destination $layout/
# Generate package
squirrel pack --packId Dyscover --packVersion 4.0.5.0 --packDirectory $layout --output $OutDir --releaseDir $OutDir
Write-Host "[OK] Squirrel package artifacts in $OutDir (EXPERIMENTAL)"
