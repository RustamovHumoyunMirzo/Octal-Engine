$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$ThirdPartyDir = Join-Path $ProjectRoot "third_party"
$AssimpDir = Join-Path $ThirdPartyDir "assimp"

if (-not (Test-Path $ThirdPartyDir)) {
    New-Item -ItemType Directory -Path $ThirdPartyDir | Out-Null
}

if (Test-Path $AssimpDir) {
    Write-Host "Assimp already exists at '$AssimpDir'. Skipping download."
    exit 0
}

Write-Host "Cloning Assimp into '$AssimpDir'..."

git clone --depth 1 https://github.com/assimp/assimp.git $AssimpDir

Write-Host "Assimp downloaded successfully."