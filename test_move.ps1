$ErrorActionPreference = "Stop"

Write-Output "=== KataGo Make Move Test ==="

Write-Output "`n[Test] Reset and Make Move"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing -TimeoutSec 30
Write-Output "Reset: $($r.StatusCode)"

Write-Output "`nNow making a move (this may take 20-30 seconds)..."
$body = '{"x":3,"y":3}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -TimeoutSec 60
$json = $r.Content | ConvertFrom-Json
Write-Output "Move Status: $($json.success)"
Write-Output "KataGo used: $($json.katago)"

if ($json.success -and $json.katago) {
    Write-Output "`n[PASS] KataGo move successful!" -ForegroundColor Green
} else {
    Write-Output "`n[INFO] Move result: $($json.message)"
}

Write-Output "`n=== Test Complete ==="
