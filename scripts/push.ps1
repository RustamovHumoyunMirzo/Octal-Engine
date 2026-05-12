param(
    [string]$Message = "update"
)

Write-Host "=== OctalEngine Git Push ==="

Set-Location $PSScriptRoot

git add .

git commit -m $Message

git push

Write-Host "=== Push Complete ==="