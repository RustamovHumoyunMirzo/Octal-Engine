$ErrorActionPreference = "Stop"

$root = Resolve-Path "$PSScriptRoot/.."
$thirdParty = Join-Path $root "third_party"

$srcDir = Join-Path $thirdParty "SDL2-src"
$buildDir = Join-Path $srcDir "build"
$finalDir = Join-Path $thirdParty "SDL2"

if (!(Test-Path $srcDir)) {
    Write-Error "SDL2 source not found. Run get_sdl2.ps1 first."
    exit 1
}

if (Test-Path $finalDir) {
    Remove-Item -Recurse -Force $finalDir
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

Push-Location $buildDir

Write-Host "Configuring..."
cmake .. `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="$finalDir" `
    -DSDL_SHARED=ON `
    -DSDL_STATIC=OFF

Write-Host "Building..."
cmake --build . --config Release --parallel

Write-Host "Installing..."
cmake --install . --config Release

Pop-Location

Write-Host "Cleaning source..."
Remove-Item -Recurse -Force $srcDir

Write-Host "Done. SDL2 ready at $finalDir"