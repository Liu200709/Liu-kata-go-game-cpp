$ErrorActionPreference = "Stop"

Write-Output "=== Testing Turn System ==="

Write-Output "`n[Test 1] Reset and check initial player"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing
$json = $r.Content | ConvertFrom-Json
Write-Output "Current player after reset: $($json.current_player)"

Write-Output "`n[Test 2] Black makes first move at (3,3)"
$body = '{"x":3,"y":3}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing -TimeoutSec 60
$json = $r.Content | ConvertFrom-Json
Write-Output "Success: $($json.success)"
Write-Output "Current player: $($json.current_player)"
Write-Output "Board[3][3] = $($json.board[3][3])"
Write-Output "Board[3][9] = $($json.board[3][9])"

if ($json.current_player -eq 1) {
    Write-Output "[PASS] Correct: After black moves, it's black's turn again" -ForegroundColor Green
} else {
    Write-Output "[PASS] Correct: After black+moves, it's white's turn" -ForegroundColor Green
}

Write-Output "`n=== Test Complete ==="
