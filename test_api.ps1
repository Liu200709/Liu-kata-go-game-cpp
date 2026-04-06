# Test all API endpoints
Write-Output "=== Testing KataGo Go Server API ==="

# Test 1: KataGo status
Write-Output "`n[Test 1] KataGo Status:"
$response = Invoke-WebRequest -Uri http://localhost:8000/katago_status -UseBasicParsing
Write-Output $response.Content

# Test 2: Reset board
Write-Output "`n[Test 2] Reset Board:"
$response = Invoke-WebRequest -Uri http://localhost:8000/reset -Method POST -UseBasicParsing
Write-Output $response.Content

# Test 3: Make a move at (9,9) - center
Write-Output "`n[Test 3] Make Move at (9,9):"
$body = '{"x":9,"y":9}'
$response = Invoke-WebRequest -Uri http://localhost:8000/make_move -Method POST -Body $body -ContentType 'application/json' -UseBasicParsing
Write-Output $response.Content

# Test 4: Check game over
Write-Output "`n[Test 4] Check Game Over:"
$response = Invoke-WebRequest -Uri http://localhost:8000/check_game_over -Method POST -UseBasicParsing
Write-Output $response.Content

# Test 5: Check score
Write-Output "`n[Test 5] Check Score:"
$response = Invoke-WebRequest -Uri http://localhost:8000/score -Method POST -UseBasicParsing
Write-Output $response.Content

Write-Output "`n=== All Tests Completed ==="
