Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path (Split-Path $MyInvocation.MyCommand.Path) "..")

$TP = Join-Path $Root "third_party"
$SRC = Join-Path $TP "bgfx_src"

$BGFX = Join-Path $SRC "bgfx"
$BX   = Join-Path $SRC "bx"
$BIMG = Join-Path $SRC "bimg"

$OUT = Join-Path $TP "bgfx"

foreach ($p in @($BGFX, $BX, $BIMG)) {
    if (!(Test-Path $p)) {
        throw "Missing dependency: $p"
    }
}

$IsWindows = $env:OS -eq "Windows_NT"
$IsLinux = (-not $IsWindows) -and (Test-Path "/proc")
$IsMac = (-not $IsWindows) -and (Test-Path "/System/Library/CoreServices")

$Arch = if ([Environment]::Is64BitOperatingSystem) { "x64" } else { "x86" }

Write-Host "Platform: $(if($IsWindows){'Windows'}elseif($IsLinux){'Linux'}else{'macOS'})"
Write-Host "Arch: $Arch"

if ($IsWindows) {
    $Genie = Join-Path $BX "tools/bin/windows/genie.exe"
    $Action = "vs2022"
}
elseif ($IsLinux) {
    $Genie = Join-Path $BX "tools/bin/linux/genie"
    $Action = "gmake"
}
elseif ($IsMac) {
    $Genie = Join-Path $BX "tools/bin/darwin/genie"
    $Action = "xcode14"
}
else {
    throw "Unsupported platform"
}

if (!(Test-Path $Genie)) {
    throw "genie not found: $Genie"
}

$MSBuildPath = $null

if ($IsWindows) {

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if (Test-Path $vswhere) {
        $MSBuildPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe |
            Select-Object -First 1
    }

    if ($MSBuildPath -and !(Test-Path $MSBuildPath)) {
        $MSBuildPath = $null
    }
}

$Config = "Release"

Write-Host "Genie: $Genie"
Write-Host "Action: $Action"
Write-Host "Config: $Config"

foreach ($d in @(
    (Join-Path $BGFX ".build"),
    (Join-Path $BGFX "build"),
    (Join-Path $BGFX "projects")
)) {
    if (Test-Path $d) {
        Remove-Item -Recurse -Force $d
        Write-Host "Cleaned $d"
    }
}

Push-Location $BGFX

try {
    Write-Host "Running genie..."
    & $Genie $Action

    if ($LASTEXITCODE -ne 0) {
        throw "genie failed"
    }
}
finally {
    Pop-Location
}

Write-Host "Building..."

if ($IsWindows -and $MSBuildPath) {

    $Sln = Join-Path $BGFX ".build/projects/vs2022/bgfx.sln"

    if (!(Test-Path $Sln)) {
        throw "Solution not found: $Sln"
    }

    Write-Host "Using MSBuild: $MSBuildPath"

    & $MSBuildPath $Sln `
        /p:Configuration=$Config `
        /p:Platform=$Arch `
        /m

    if ($LASTEXITCODE -ne 0) {
        throw "MSBuild failed"
    }
}
elseif ($IsWindows) {

    Write-Host "MSBuild not found → fallback not available (install VS C++)"
    throw "No build backend available"
}
elseif ($IsLinux) {

    Push-Location (Join-Path $BGFX ".build/projects/gmake")

    & make config=$Config -j

    if ($LASTEXITCODE -ne 0) {
        throw "make failed"
    }

    Pop-Location
}
elseif ($IsMac) {

    Push-Location (Join-Path $BGFX ".build/projects/xcode")

    & xcodebuild -configuration $Config

    if ($LASTEXITCODE -ne 0) {
        throw "xcodebuild failed"
    }

    Pop-Location
}

Write-Host "Packaging SDK..."

$INC = Join-Path $OUT "include"
$LIB = Join-Path $OUT "lib"

New-Item -ItemType Directory -Force -Path $INC | Out-Null
New-Item -ItemType Directory -Force -Path $LIB | Out-Null

foreach ($p in @($BGFX, $BX, $BIMG)) {
    Copy-Item -Recurse -Force "$p/include/*" $INC -ErrorAction SilentlyContinue
}

Get-ChildItem -Recurse $BGFX -Include *.lib,*.a -ErrorAction SilentlyContinue |
    ForEach-Object { Copy-Item $_.FullName $LIB -Force }

Write-Host ""
Write-Host "===================================="
Write-Host ("BGFX BUILD COMPLETE")
Write-Host ("SDK READY")
Write-Host ("PATH: " + $OUT)
Write-Host "===================================="

Write-Host "Copying bgfxConfig.cmake..."

$SRC_CFG = Join-Path $Root "scripts/bgfxConfig.txt"
$DST_DIR = Join-Path $OUT "cmake"
$DST_CFG = Join-Path $DST_DIR "bgfxConfig.cmake"

if (!(Test-Path $SRC_CFG)) {
    throw "Missing config file: $SRC_CFG"
}

New-Item -ItemType Directory -Force -Path $DST_DIR | Out-Null

Copy-Item -Force $SRC_CFG $DST_CFG

Write-Host ("Copied bgfxConfig.cmake: " + $DST_CFG)