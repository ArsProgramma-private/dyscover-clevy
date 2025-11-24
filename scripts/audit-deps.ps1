Param(
  [string]$Target = "Dyscover.exe",
  [string]$BuildDir = "build"
)
$ErrorActionPreference = "Stop"
$bin = Join-Path $BuildDir $Target
if (!(Test-Path $bin)) { Write-Error "Binary not found: $bin" }
Write-Host "[INFO] Auditing (dumpbin /DEPENDENTS): $bin"
$dump = & dumpbin /DEPENDENTS $bin 2>$null
if (!$?) { Write-Error "dumpbin failed. Ensure VS Developer Tools environment." }
$lines = $dump -split "`n"
$deps = @()
foreach ($l in $lines) { if ($l -match "\.dll") { $deps += $l.Trim() } }
foreach ($d in $deps) {
  $foundPath = $null
  foreach ($p in $Env:PATH -split ";") {
    $candidate = Join-Path $p $d
    if (Test-Path $candidate) { $foundPath = $candidate; break }
  }
  if ($foundPath) {
    $hash = (Get-FileHash $foundPath -Algorithm SHA256).Hash
    Write-Host "[OK] $d SHA256=$hash"
  } else {
    Write-Host "[MISSING] $d"
  }
}
