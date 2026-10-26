param(
    [string]$MysqlExe = "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe",
    [string]$User = "root",
    [string]$HostName = "127.0.0.1",
    [switch]$WithSeed
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot

if (-not (Test-Path $MysqlExe)) {
    $found = Get-Command mysql -ErrorAction SilentlyContinue
    if ($found) { $MysqlExe = $found.Source }
}
if (-not (Test-Path $MysqlExe)) {
    Write-Host "[ERROR] mysql.exe not found. Pass -MysqlExe <path>." -ForegroundColor Red
    exit 1
}

$secure = Read-Host "MySQL password for $User" -AsSecureString
$plain = [Runtime.InteropServices.Marshal]::PtrToStringAuto(
    [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure))
$env:MYSQL_PWD = $plain

$schema = Join-Path $root "sqls\schema.sql"
$seed = Join-Path $root "sqls\seed.sql"
try {
    Write-Host "[INFO] Applying $schema" -ForegroundColor Cyan
    Get-Content $schema -Raw | & $MysqlExe -h $HostName -u $User --default-character-set=utf8mb4
    if ($LASTEXITCODE -ne 0) { throw "schema failed" }

    if ($WithSeed) {
        Write-Host "[WARN] Applying seed data (overwrites demo model rows)." -ForegroundColor Yellow
        Get-Content $seed -Raw | & $MysqlExe -h $HostName -u $User --default-character-set=utf8mb4
        if ($LASTEXITCODE -ne 0) { throw "seed failed" }
    }
    Write-Host "[OK] Database initialization complete." -ForegroundColor Green
} finally {
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
}
