param(
    [Parameter(Mandatory = $true)]
    [string] $CertificateBase64
)

$ErrorActionPreference = 'Stop'
if ($Env:GITHUB_ACTIONS -ne 'true' -or $Env:RUNNER_ENVIRONMENT -ne 'github-hosted') {
    throw 'Test certificate trust is restricted to ephemeral GitHub-hosted Actions runners'
}
$certificate = [System.Security.Cryptography.X509Certificates.X509Certificate2]::new(
    [Convert]::FromBase64String($CertificateBase64)
)
try {
    if ($certificate.HasPrivateKey -or $certificate.Subject -ne $certificate.Issuer) {
        throw 'Expected the public self-signed SignPath test certificate'
    }
    if ($certificate.NotBefore -gt (Get-Date) -or $certificate.NotAfter -lt (Get-Date)) {
        throw 'The test certificate is outside its validity period'
    }
    $eku = $certificate.Extensions | Where-Object { $_.Oid.Value -eq '2.5.29.37' }
    if (-not $eku -or '1.3.6.1.5.5.7.3.3' -notin @($eku.EnhancedKeyUsages | ForEach-Object { $_.Value })) {
        throw 'The test certificate must have the code signing EKU'
    }
    # Only the ephemeral GitHub-hosted test job trusts this public certificate.
    # Release jobs never execute this script and retain normal Windows trust.
    foreach ($storeName in @('Root', 'TrustedPublisher')) {
        $store = [System.Security.Cryptography.X509Certificates.X509Store]::new($storeName, 'CurrentUser')
        try {
            $store.Open('ReadWrite')
            $store.Add($certificate)
        } finally {
            $store.Dispose()
        }
    }
    "SIGNPATH_TEST_CERTIFICATE_THUMBPRINT=$($certificate.Thumbprint)" >> $Env:GITHUB_ENV
    Write-Host "Configured onboarding certificate: $($certificate.Thumbprint)"
} finally {
    $certificate.Dispose()
}
