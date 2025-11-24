#!/usr/bin/env pwsh
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
.PARAMETER Generator [MSVC|MinGW|Ninja] choose build backend (no IDE required for MinGW/Ninja).
.PARAMETER CopyDeps [On|Off] attempt to copy common runtime DLLs into dist folder.
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
  [ValidateSet('On','Off')] [string]$Package='Off',
  [ValidateSet('MSVC','MinGW','Ninja')] [string]$Generator='MSVC',
  [ValidateSet('On','Off')] [string]$CopyDeps='Off'
)

$ErrorActionPreference='Stop'
$buildDir="build-windows-$Config"; if(!(Test-Path $buildDir)){ New-Item -ItemType Directory -Path $buildDir | Out-Null }

Write-Host "[Build] Generating CMake project ($Config, $Arch)" -ForegroundColor Cyan
# Select CMake generator logic
switch ($Generator) {
  'MSVC'  { $cmakeArgs=@('-S','.', '-B', $buildDir, '-G','Visual Studio 17 2022', '-A', $Arch) }
  'MinGW' { $cmakeArgs=@('-S','.', '-B', $buildDir, '-G','MinGW Makefiles') }
  'Ninja' { $cmakeArgs=@('-S','.', '-B', $buildDir, '-G','Ninja') }
}

$cmakeArgs += @(
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
  if ($Generator -eq 'MSVC') {
    & ctest --test-dir $buildDir -C $Config --output-on-failure
  } else {
    & ctest --test-dir $buildDir --output-on-failure
  }
}

if($Package -eq 'On'){
  $outDir="dist-windows-$Config"; if(Test-Path $outDir){ Remove-Item -Recurse -Force $outDir }
  New-Item -ItemType Directory -Path $outDir | Out-Null
  Copy-Item "$buildDir/$Config/Dyscover.exe" $outDir
  Write-Host "[Package] Created $outDir" -ForegroundColor Green
  if($CopyDeps -eq 'On') {
    Write-Host "[Deps] Attempting DLL copy (wx*, librstts*, libintl*, libiconv*, portaudio*)" -ForegroundColor Yellow
    $dllPatterns = @('wx*.dll','librstts*.dll','libintl*.dll','libiconv*.dll','portaudio*.dll')
    $searchRoots = @(
      (Join-Path $PWD 'lib/rstts/platforms/x86_64-pc-windows-msvc'),
      (Join-Path $Env:WXWIN 'lib'),
      (Join-Path $Env:WXWIN 'lib\vc_x64_dll'),
      (Join-Path $Env:ProgramFiles 'gettext/bin'),
      (Join-Path $Env:ProgramFiles 'PortAudio'),
      (Join-Path $Env:ProgramFiles 'portaudio'),
      (Join-Path $Env:ProgramFiles 'portaudio\bin')
    ) | Where-Object { $_ -and (Test-Path $_) }
    foreach($root in $searchRoots) {
      foreach($pat in $dllPatterns) {
        Get-ChildItem -Path $root -Filter $pat -File -ErrorAction SilentlyContinue | ForEach-Object {
          Copy-Item $_.FullName $outDir -Force
        }
      }
    }
    Write-Host "[Deps] DLL copy pass complete (verify contents)." -ForegroundColor Green
  } else {
    Write-Host "[Deps] Skipped DLL copy (use -CopyDeps On to enable)." -ForegroundColor DarkGray
  }
}

Write-Host "[Done] Build finished." -ForegroundColor Green
