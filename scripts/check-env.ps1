$ErrorActionPreference = "Continue"
$root = Split-Path -Parent $PSScriptRoot

function Show-Command($name) {
    $command = Get-Command $name -ErrorAction SilentlyContinue
    if ($command) {
        Write-Host ("[OK]   {0} -> {1}" -f $name, $command.Source) -ForegroundColor Green
        return $command
    }
    Write-Host ("[FAIL] {0} not found in PATH" -f $name) -ForegroundColor Red
    return $null
}

Write-Host "=== Commands ===" -ForegroundColor Cyan
$cmake = Show-Command cmake
$qmake = Show-Command qmake6
$redis = Show-Command redis-server
$redisCli = Show-Command redis-cli
$mysql = Show-Command mysql

Write-Host "=== Qt ===" -ForegroundColor Cyan
if ($qmake) {
    $prefix = & qmake6 -query QT_INSTALL_PREFIX
    Write-Host ("[OK]   Qt prefix: {0}" -f $prefix) -ForegroundColor Green
}

Write-Host "=== Services / ports ===" -ForegroundColor Cyan
Get-Service MySQL80 -ErrorAction SilentlyContinue | Select-Object Status, Name, DisplayName | Format-Table -AutoSize
$ports = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
    Where-Object { $_.LocalPort -in 3306, 6379 } |
    Select-Object LocalAddress, LocalPort, OwningProcess
$ports | Format-Table -AutoSize
if (-not ($ports | Where-Object LocalPort -eq 6379)) {
    Write-Host "[FAIL] Redis is not listening on 6379. Start redis-server.exe." -ForegroundColor Red
}
if (-not ($ports | Where-Object LocalPort -eq 3306)) {
    Write-Host "[FAIL] MySQL is not listening on 3306. Start the MySQL80 service." -ForegroundColor Red
}

Write-Host "=== Database ===" -ForegroundColor Cyan
$mysqlExe = "C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe"
if (-not (Test-Path $mysqlExe)) {
    $mysqlExe = (Get-Command mysql -ErrorAction SilentlyContinue).Source
}
if ($mysqlExe) {
    $env:MYSQL_PWD = $env:QGK_DB_PASSWORD
    if (-not $env:MYSQL_PWD) { $env:MYSQL_PWD = "123456" }
    $user = if ($env:QGK_DB_USER) { $env:QGK_DB_USER } else { "root" }
    $database = if ($env:QGK_DB_NAME) { $env:QGK_DB_NAME } else { "meta_graph_model" }
    & $mysqlExe -h 127.0.0.1 -u $user --default-character-set=utf8mb4 -e "SHOW TABLES FROM $database;"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] Cannot query $database. Run sqls/schema.sql and sqls/seed.sql." -ForegroundColor Red
    }
    Remove-Item Env:MYSQL_PWD -ErrorAction SilentlyContinue
} else {
    Write-Host "[WARN] mysql.exe not found; skip database check." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "If all checks are OK, run: powershell -ExecutionPolicy Bypass -File scripts/run.ps1" -ForegroundColor Cyan
