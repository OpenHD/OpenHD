param(
    [Parameter(Mandatory = $true)]
    [string] $Path,
    [ValidateSet('test', 'release')]
    [string] $SigningMode = 'release',
    [string] $ExpectedSignerThumbprint
)

$ErrorActionPreference = 'Stop'
if ($SigningMode -eq 'test' -and [string]::IsNullOrWhiteSpace($ExpectedSignerThumbprint)) {
    throw 'Test verification requires the configured test certificate thumbprint'
}
$signature = Get-AuthenticodeSignature -LiteralPath $Path
if ($signature.Status -ne 'Valid' -or -not $signature.SignerCertificate -or -not $signature.TimeStamperCertificate) {
    throw "Expected a valid, timestamped Authenticode signature on $Path; status: $($signature.Status)"
}
if ($SigningMode -eq 'test' -and $signature.SignerCertificate.Thumbprint -ne $ExpectedSignerThumbprint) {
    throw "Unexpected test signing certificate on $Path"
}
Write-Host "Verified timestamped signature: $Path"
