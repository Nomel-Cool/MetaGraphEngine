param(
    [string]$RedisHome = $env:QGK_REDIS_HOME,
    [switch]$SkipPreflight
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root "install\bin\QtGraphEngineKit.exe"

if (-not (Test-Path $exe)) {
    Write-Host "[ERROR] $exe not found. Run construct_project.bat first." -ForegroundColor Red
    exit 1
}

if (-not $RedisHome) {
    $RedisHome = "D:\Redis\Redis-x64-5.0.14.1"
}

$redisPort = if ($env:QGK_REDIS_PORT) { [int]$env:QGK_REDIS_PORT } else { 6379 }
$redisListening = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
    Where-Object { $_.LocalPort -eq $redisPort }
if (-not $redisListening) {
    $redisExe = Join-Path $RedisHome "redis-server.exe"
    if (Test-Path $redisExe) {
        Write-Host "[INFO] Starting Redis: $redisExe" -ForegroundColor Cyan
        Start-Process -FilePath $redisExe -ArgumentList "--port", "$redisPort" -WorkingDirectory $RedisHome -WindowStyle Hidden
        Start-Sleep -Seconds 2
    } else {
        Write-Host "[WARN] Redis not listening on $redisPort and redis-server.exe not found at $RedisHome." -ForegroundColor Yellow
    }
}

$mysqlListening = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
    Where-Object { $_.LocalPort -eq 3306 }
if (-not $mysqlListening) {
    Write-Host "[WARN] MySQL is not listening on 3306. Trying to start service MySQL80..." -ForegroundColor Yellow
    try {
        Start-Service MySQL80 -ErrorAction Stop
        Start-Sleep -Seconds 2
    } catch {
        Write-Host "[ERROR] Cannot start MySQL80. Start it as administrator or check the service name." -ForegroundColor Red
    }
}

if ($SkipPreflight) {
    $env:QGK_SKIP_PREFLIGHT = "1"
    Write-Host "[WARN] Startup preflight skipped." -ForegroundColor Yellow
}

Write-Host "[INFO] Starting Graph Engine Kit..." -ForegroundColor Green
$process = Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe) -PassThru
Write-Host "[INFO] PID: $($process.Id)"
Write-Host "[INFO] Log: $(Join-Path (Split-Path $exe) 'logs\app.log')"
