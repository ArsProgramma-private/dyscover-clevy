Param(
  [string]$Binary = "build\Dyscover.exe",
  [string]$CertFile = "cert.pfx",
  [string]$TimestampUrl = "http://timestamp.digicert.com",
  [string]$Description = "Clevy Dyscover",
  [string]$Password = ""
)
# Usage: pwsh ./scripts/sign-windows.ps1 -Binary build\Dyscover.exe -CertFile mycert.pfx -Password 'secret'
$ErrorActionPreference = 'Stop'
if (!(Test-Path $Binary)) { Write-Error "Binary not found: $Binary" }
if (!(Test-Path $CertFile)) { Write-Error "Certificate file not found: $CertFile" }
$pwdArg = @()
if ($Password -ne "") { $pwdArg = @('/p', $Password) }
& signtool sign /fd sha256 /tr $TimestampUrl /td sha256 /d $Description /v /f $CertFile @pwdArg $Binary
if ($LASTEXITCODE -ne 0) { Write-Error 'Signing failed' } else { Write-Host '[OK] Signed with timestamp' }
