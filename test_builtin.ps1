$ErrorActionPreference = "Stop"

Write-Output "=== Built-in AI Test ==="

Write-Output "`n[Test 1] Reset"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing -TimeoutSec 10
Write-Output "Reset: $($r.StatusCode)"

Write-Output "`n[Test 2] Make Move (built-in AI)"
$body = '{"x":3,"y":3}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -TimeoutSec 30
$json = $r.Content | ConvertFrom-Json
Write-Output "Success: $($json.success)"
Write-Output "KataGo: $($json.katago)"

Write-Output "`n[Test 3] Score"
$r = Invoke-WebRequest -Uri "http://localhost:8000/score" -Method POST -UseBasicParsing -TimeoutSec 10
$json = $r.Content | ConvertFrom-Json
Write-Output "Black: $($json.black_total)"
Write-Output "White: $($json.white_total)"

Write-Output "`n=== Tests Complete ==="
