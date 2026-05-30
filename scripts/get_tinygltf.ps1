$ErrorActionPreference = "Stop"

$version = "v3.0.0"
$repoUrl = "https://github.com/syoyo/tinygltf"
$zipUrl = "$repoUrl/archive/refs/tags/$version.zip"

$rootDir = Split-Path -Parent $PSScriptRoot
$targetDir = Join-Path $rootDir "third_party\tinygltf"

$zipPath = Join-Path $env:TEMP "tinygltf_$version.zip"
$extractPath = Join-Path $env:TEMP "tinygltf_extract"

Write-Host "Downloading tinygltf $version..."

New-Item -ItemType Directory -Force -Path (Split-Path $targetDir) | Out-Null

if (Test-Path $targetDir) {
    Write-Host "Removing existing tinygltf..."
    Remove-Item -Recurse -Force $targetDir
}

if (Test-Path $zipPath) { Remove-Item -Force $zipPath }
if (Test-Path $extractPath) { Remove-Item -Recurse -Force $extractPath }

New-Item -ItemType Directory -Force -Path $extractPath | Out-Null

Invoke-WebRequest -Uri $zipUrl -OutFile $zipPath

Expand-Archive -Path $zipPath -DestinationPath $extractPath -Force

$innerFolder = Get-ChildItem $extractPath | Where-Object { $_.PSIsContainer } | Select-Object -First 1

if (-not $innerFolder) {
    throw "Failed to locate extracted tinygltf folder."
}

Move-Item -Path (Join-Path $innerFolder.FullName "*") -Destination $targetDir -Force

Write-Host "tinygltf installed to $targetDir"

Remove-Item -Force $zipPath
Remove-Item -Recurse -Force $extractPath