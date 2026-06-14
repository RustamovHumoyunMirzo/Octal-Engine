param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

$ScriptDir   = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

$AssimpDir = Join-Path $ProjectRoot "third_party\assimp"
$BuildDir  = Join-Path $AssimpDir "build"

if (-not (Test-Path $AssimpDir)) {
    Write-Error "Assimp source directory not found: $AssimpDir"
    Write-Error "Run get_assimp.ps1 first."
    exit 1
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "CMake was not found in PATH."
    exit 1
}

Write-Host "Configuring Assimp ($Configuration)..."

cmake `
    -S $AssimpDir `
    -B $BuildDir `
    -DASSIMP_BUILD_TESTS=OFF `
    -DASSIMP_INSTALL=OFF `
    -DBUILD_SHARED_LIBS=OFF `
    -DCMAKE_BUILD_TYPE=$Configuration

Write-Host "Building Assimp..."

cmake `
    --build $BuildDir `
    --config $Configuration

Write-Host ""
Write-Host "Assimp built successfully."
Write-Host "Build directory: $BuildDir"