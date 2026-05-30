$ErrorActionPreference = "Stop"

$rootDir = Split-Path -Parent $PSScriptRoot
$tinygltfDir = Join-Path $rootDir "third_party\tinygltf"
$headerPath = Join-Path $tinygltfDir "tiny_gltf.h"

Write-Host "Checking tinygltf installation..."

if (-not (Test-Path $tinygltfDir)) {
    throw "tinygltf directory not found: $tinygltfDir"
}

if (-not (Test-Path $headerPath)) {
    throw "tiny_gltf.h not found: $headerPath"
}

Write-Host "tinygltf is header-only. No build step required."
Write-Host "Found: $headerPath"