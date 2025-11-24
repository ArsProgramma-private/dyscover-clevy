<#!
.SYNOPSIS
  Build Dyscover (Clevy) on Windows with configurable options.
.DESCRIPTION
  Wraps CMake generation + build + optional test + optional packaging.
.PARAMETER Config [Debug|Release]
.PARAMETER Arch   [x64]
.PARAMETER Language (e.g. nl, en)
.PARAMETER Licensing [demo|full]
.PARAMETER Tests [On|Off]
.PARAMETER Integration [On|Off]
.PARAMETER Package [On|Off] produces a minimal deploy folder.
.EXAMPLE
  ./build-windows.ps1 -Config Release -Language nl -Licensing demo -Tests On -Integration Off -Package On
.NOTES
  Requires: VS 2022, CMake >=3.15, wxWidgets 3.2, optional PortAudio, librstts placed under lib/rstts/platforms/x86_64-pc-windows-msvc/.
#>
param(
  [ValidateSet('Debug','Release')] [string]$Config='Debug',
  [ValidateSet('x64')] [string]$Arch='x64',
  [string]$Language='nl',
  [ValidateSet('demo','full')] [string]$Licensing='demo',
  [ValidateSet('On','Off')] [string]$Tests='On',
  [ValidateSet('On','Off')] [string]$Integration='Off',
  [ValidateSet('On','Off')] [string]$Package='Off'
)

$ErrorActionPreference='Stop'
$buildDir="build-windows-$Config"; if(!(Test-Path $buildDir)){ New-Item -ItemType Directory -Path $buildDir | Out-Null }

Write-Host "[Build] Generating CMake project ($Config, $Arch)" -ForegroundColor Cyan
$cmakeArgs=@(
  '-S','.',
  '-B', $buildDir,
  '-G','Visual Studio 17 2022',
  '-A', $Arch,
  "-DLANGUAGE=$Language",
  "-DLICENSING=$Licensing",
  "-DBUILD_TESTS=$Tests",
  "-DBUILD_INTEGRATION_TESTS=$Integration"
)
& cmake @cmakeArgs

Write-Host "[Build] Building target Dyscover" -ForegroundColor Cyan
& cmake --build $buildDir --config $Config --target Dyscover

if($Tests -eq 'On'){
  Write-Host "[Test] Running unit tests" -ForegroundColor Yellow
  & ctest --test-dir $buildDir -C $Config --output-on-failure
}

if($Package -eq 'On'){
  $outDir="dist-windows-$Config"; if(Test-Path $outDir){ Remove-Item -Recurse -Force $outDir }
  New-Item -ItemType Directory -Path $outDir | Out-Null
  Copy-Item "$buildDir/$Config/Dyscover.exe" $outDir
  # Copy required DLLs if using dynamic wxWidgets or librstts; user adjusts as needed.
  Write-Host "[Package] Created $outDir (manual DLL dependency copy may be required)" -ForegroundColor Green
}

Write-Host "[Done] Build finished." -ForegroundColor Green
