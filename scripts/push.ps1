param(
    [string]$Message = "update"
)

Set-Location $PSScriptRoot/..

git add .

git diff --cached --quiet

if ($LASTEXITCODE -eq 0)
{
    Write-Host "No changes to commit."
    exit
}

git commit -m $Message
git push