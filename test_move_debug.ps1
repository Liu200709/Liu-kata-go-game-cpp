$ErrorActionPreference = "Stop"

Write-Output "=== Testing Make Move API ==="

Write-Output "`n[Test] Reset Board"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing -TimeoutSec 30
Write-Output "Status: $($r.StatusCode)"

Write-Output "`n[Test] Make Move at (3,3) - with longer timeout"
$body = '{"x":3,"y":3}'
$sw = [Diagnostics.Stopwatch]::StartNew()
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -TimeoutSec 60 -UseBasicParsing
    $sw.Stop()
    Write-Output "Time taken: $($sw.ElapsedMilliseconds) ms"
    Write-Output "Status: $($r.StatusCode)"
    $json = $r.Content | ConvertFrom-Json
    Write-Output "Success: $($json.success)"
    Write-Output "Message: $($json.message)"
    Write-Output "KataGo: $($json.katago)"
} catch {
    $sw.Stop()
    Write-Output "Time before error: $($sw.ElapsedMilliseconds) ms"
    Write-Output "Error: $($_.Exception.Message)"
}

Write-Output "`n=== Test Complete ==="
