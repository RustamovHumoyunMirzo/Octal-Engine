$ErrorActionPreference = "Stop"

$root = Resolve-Path "$PSScriptRoot/.."
$thirdParty = Join-Path $root "third_party"
New-Item -ItemType Directory -Force -Path $thirdParty | Out-Null

$version = "2.32.8"
$url = "https://github.com/libsdl-org/SDL/releases/download/release-$version/SDL2-$version.tar.gz"
$archive = Join-Path $env:TEMP "sdl2.tar.gz"
$srcDir = Join-Path $thirdParty "SDL2-src"

Write-Host "Downloading SDL2 $version..."
Invoke-WebRequest $url -OutFile $archive

if (Test-Path $srcDir) {
    Remove-Item -Recurse -Force $srcDir
}

Write-Host "Extracting with CMake..."
cmake -E tar xzf $archive

$extracted = Get-ChildItem | Where-Object { $_.Name -like "SDL2-*" -and $_.PSIsContainer } | Select-Object -First 1
Move-Item $extracted.FullName $srcDir

Write-Host "SDL2 source ready at $srcDir"