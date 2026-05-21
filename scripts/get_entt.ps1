$ErrorActionPreference = "Stop"

$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path

$ROOT_DIR = Resolve-Path (Join-Path $SCRIPT_DIR "..")

$THIRD_PARTY_DIR = Join-Path $ROOT_DIR "third_party"

$ENTT_DIR = Join-Path $THIRD_PARTY_DIR "entt"

$ENTT_VERSION = "v3.16.0"

$URL = "https://github.com/skypjack/entt/archive/refs/tags/$ENTT_VERSION.zip"

$TEMP_ZIP = Join-Path $env:TEMP "entt.zip"
$TEMP_DIR = Join-Path $env:TEMP "entt_extract"

Write-Host "Downloading EnTT $ENTT_VERSION..." -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path $THIRD_PARTY_DIR | Out-Null
Remove-Item -Recurse -Force $TEMP_DIR -ErrorAction SilentlyContinue

Invoke-WebRequest -Uri $URL -OutFile $TEMP_ZIP

Write-Host "Extracting..." -ForegroundColor Cyan
Expand-Archive -Path $TEMP_ZIP -DestinationPath $TEMP_DIR -Force

$EXTRACTED_FOLDER = Get-ChildItem $TEMP_DIR | Where-Object { $_.PSIsContainer } | Select-Object -First 1

$SOURCE_PATH = Join-Path $EXTRACTED_FOLDER.FullName "single_include\entt"

Remove-Item -Recurse -Force $ENTT_DIR -ErrorAction SilentlyContinue

Copy-Item -Recurse -Force $SOURCE_PATH $ENTT_DIR

Remove-Item -Recurse -Force $TEMP_ZIP, $TEMP_DIR

Write-Host "EnTT installed successfully at: $ENTT_DIR" -ForegroundColor Green