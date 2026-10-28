$ErrorActionPreference = 'Stop'
$dependencyRoot = Join-Path $env:GITHUB_WORKSPACE 'out'
New-Item -ItemType Directory -Path $dependencyRoot -Force | Out-Null
$msiPath = Join-Path $dependencyRoot 'wdk-redist.msi'
Invoke-WebRequest -Uri 'https://go.microsoft.com/fwlink/p/?LinkID=253170' -OutFile $msiPath
if ((Get-FileHash -LiteralPath $msiPath -Algorithm SHA256).Hash -ne '29314207814CE9D5D73695F7E9239539CF37C79E750B9D5EA5A5EF5487A583D6') {
    throw 'Microsoft WDK redistributable checksum mismatch'
}
$signature = Get-AuthenticodeSignature -LiteralPath $msiPath
if ($signature.Status -ne 'Valid' -or $signature.SignerCertificate.Subject -notmatch 'CN=Microsoft Corporation,') {
    throw 'Microsoft WDK redistributable signature verification failed'
}
# Administrative extraction only, on the CI runner; no host driver installation.
$extractPath = Join-Path $dependencyRoot 'wdk-redist'
$process = Start-Process msiexec.exe -ArgumentList @('/a', "`"$msiPath`"", '/qn', "TARGETDIR=`"$extractPath`"") -WindowStyle Hidden -PassThru -Wait
if ($process.ExitCode -ne 0) { throw "WDK administrative extraction failed: $($process.ExitCode)" }
