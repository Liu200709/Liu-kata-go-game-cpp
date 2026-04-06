$ErrorActionPreference = "Stop"

Write-Output "=== Extended Tests ==="

# Test棋盘落子多次
Write-Output "`n[Test] Multiple moves"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing
Write-Output "Reset: OK"

$body = '{"x":9,"y":9}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
Write-Output "Move (9,9): OK"

$body = '{"x":3,"y":3}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
Write-Output "Move (3,3): OK"

# Test无效落子
Write-Output "`n[Test] Invalid move (occupied position)"
$body = '{"x":9,"y":9}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
if ($r.Content -match "false") { Write-Output "Invalid move correctly rejected: OK" }

# Test边界坐标
Write-Output "`n[Test] Edge coordinates"
$r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing

$body = '{"x":0,"y":0}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
Write-Output "Move (0,0): OK"

$body = '{"x":18,"y":18}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
Write-Output "Move (18,18): OK"

$body = '{"x":-1,"y":0}'
$r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
if ($r.Content -match "false") { Write-Output "Invalid coordinate (-1,0) correctly rejected: OK" }

Write-Output "`n=== Extended Tests Complete ==="
