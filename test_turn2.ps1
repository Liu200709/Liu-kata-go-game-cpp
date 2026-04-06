$ErrorActionPreference = "Stop"

Write-Output "=== Testing Turn System with Correct Position ==="

Write-Output "`n[Test 1] Reset and check initial player"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing
$json = $r.Content | ConvertFrom-Json
Write-Output "Current player after reset: $($json.current_player)"

Write-Output "`n[Test 2] Black makes first move at (3,3)"
$body = '{"x":3,"y":3}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing -TimeoutSec 60
$json = $r.Content | ConvertFrom-Json
Write-Output "Success: $($json.success)"
Write-Output "Current player after response: $($json.current_player)"

Write-Output "`nChecking board positions:"
$foundWhite = $false
for ($i = 0; $i -lt 19; $i++) {
    for ($j = 0; $j -lt 19; $j++) {
        if ($json.board[$i][$j] -eq 2) {
            Write-Output "White stone at board[$i][$j]"
            $foundWhite = $true
        }
    }
}
if (-not $foundWhite) {
    Write-Output "[WARNING] No white stone found!"
}

Write-Output "`nExpected flow:"
Write-Output "1. Black (player 1) moves at (3,3)"
Write-Output "2. AI generates white move"
Write-Output "3. After both moves, current_player should be 1 (black's turn)"

if ($json.current_player -eq 1) {
    Write-Output "`n[PASS] Turn system working correctly!" -ForegroundColor Green
} else {
    Write-Output "`n[FAIL] Wrong current player: $($json.current_player)" -ForegroundColor Red
}

Write-Output "`n=== Test Complete ==="
