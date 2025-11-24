Param(
  [string]$Binary = "build\Dyscover.exe"
)
$ErrorActionPreference = 'Stop'
if (!(Test-Path $Binary)) { Write-Error "Binary not found: $Binary" }
Write-Host "[INFO] Verifying hardening for $Binary"
# dumpbin /headers shows characteristics; look for 'Dynamic base' (ASLR) and 'NX compatible'
$hdr = & dumpbin /headers $Binary 2>$null | Out-String
if (!$hdr) { Write-Error 'dumpbin failed. Use Developer PowerShell.' }
$aslr = ($hdr -match 'Dynamic base')
$nx = ($hdr -match 'NX compatible')
$cert = & signtool verify /pa $Binary 2>$null; $signed = $LASTEXITCODE -eq 0
Write-Host ('ASLR (Dynamic base): ' + ($aslr ? 'yes' : 'no'))
Write-Host ('DEP (NX compatible): ' + ($nx ? 'yes' : 'no'))
Write-Host ('Authenticode signature: ' + ($signed ? 'valid' : 'missing'))
