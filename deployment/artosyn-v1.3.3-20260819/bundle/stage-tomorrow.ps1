param(
    [string]$GroundIp = "192.168.1.171"
)

$ErrorActionPreference = "Stop"
$Bundle = Split-Path -Parent $MyInvocation.MyCommand.Path
$Adb = "C:\Users\Raphael\AppData\Local\Android\Sdk\platform-tools\adb.exe"
$Plink = "C:\Program Files\PuTTY\plink.exe"
$Pscp = "C:\Program Files\PuTTY\pscp.exe"

function Invoke-Checked([scriptblock]$Command) {
    & $Command
    if ($LASTEXITCODE -ne 0) { throw "Command failed with exit code $LASTEXITCODE" }
}

$devices = & $Adb devices | Select-String "\sdevice$"
if ($devices.Count -ne 1) {
    throw "Expected exactly one online ADB device; found $($devices.Count)."
}
$serial = (($devices[0].Line -split "\s+")[0])

& $Adb -s $serial shell "mkdir -p /ohd/storage/l4-v1.3.3/air-arm64"
if ($LASTEXITCODE -ne 0) { throw "Cannot create persistent air staging directory." }
& $Adb -s $serial push "$Bundle\air-arm64\." /ohd/storage/l4-v1.3.3/air-arm64/
if ($LASTEXITCODE -ne 0) { throw "Air staging failed." }
& $Adb -s $serial shell "chmod 755 /ohd/storage/l4-v1.3.3/air-arm64/*.sh"
if ($LASTEXITCODE -ne 0) { throw "Air script permission setup failed." }

& $Plink -batch -ssh -pw openhd "openhd@$GroundIp" "mkdir -p /home/openhd/l4-v1.3.3/ground-armhf"
if ($LASTEXITCODE -ne 0) { throw "Cannot create persistent ground staging directory." }
& $Pscp -batch -pw openhd -r "$Bundle\ground-armhf\*" "openhd@${GroundIp}:/home/openhd/l4-v1.3.3/ground-armhf/"
if ($LASTEXITCODE -ne 0) { throw "Ground staging failed." }
& $Plink -batch -ssh -pw openhd "openhd@$GroundIp" "chmod 755 /home/openhd/l4-v1.3.3/ground-armhf/*.sh"
if ($LASTEXITCODE -ne 0) { throw "Ground script permission setup failed." }

Write-Host "Staged persistently:"
Write-Host "  air:   /ohd/storage/l4-v1.3.3/air-arm64"
Write-Host "  ground: /home/openhd/l4-v1.3.3/ground-armhf"
