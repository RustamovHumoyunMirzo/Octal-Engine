Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$BaseDir = Join-Path $PSScriptRoot "../third_party"

$Repos = @(
    @{ Name = "bx";    Url = "https://github.com/bkaradzic/bx.git" },
    @{ Name = "bimg";  Url = "https://github.com/bkaradzic/bimg.git" },
    @{ Name = "bgfx";  Url = "https://github.com/bkaradzic/bgfx.git" }
)

function Test-Git {
    try {
        git --version | Out-Null
    } catch {
        throw "Git is not installed or not in PATH."
    }
}

function Clone-Or-Update {
    param (
        [string]$Name,
        [string]$Url
    )

    $Target = Join-Path $BaseDir $Name

    if (Test-Path $Target) {
        Write-Host "Updating $Name..."
        Push-Location $Target
        git pull --rebase
        Pop-Location
    }
    else {
        Write-Host "Cloning $Name..."
        git clone $Url $Target
    }
}

Test-Git

if (!(Test-Path $BaseDir)) {
    New-Item -ItemType Directory -Path $BaseDir | Out-Null
}

foreach ($repo in $Repos) {
    Clone-Or-Update -Name $repo.Name -Url $repo.Url
}

Write-Host "`nDone. Repositories are in: $BaseDir"