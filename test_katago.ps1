$ErrorActionPreference = "Stop"

Write-Output "=== KataGo Integration Test ==="

Write-Output "`n[Test 1] KataGo Status"
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/katago_status" -UseBasicParsing -TimeoutSec 10
    $status = $r.Content | ConvertFrom-Json
    Write-Output "Initialized: $($status.initialized)"
    Write-Output "Use KataGo: $($status.use_katago)"
    Write-Output "Difficulty: $($status.difficulty)"
    if ($status.initialized -and $status.use_katago) {
        Write-Output "[PASS] KataGo is running" -ForegroundColor Green
    } else {
        Write-Output "[FAIL] KataGo not properly initialized" -ForegroundColor Red
    }
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n[Test 2] Reset Board"
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/reset" -Method POST -UseBasicParsing -TimeoutSec 10
    $result = $r.Content | ConvertFrom-Json
    if ($result.success) {
        Write-Output "[PASS] Board reset" -ForegroundColor Green
    } else {
        Write-Output "[FAIL] Reset failed" -ForegroundColor Red
    }
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n[Test 3] Make Move (KataGo)"
try {
    $body = '{"x":3,"y":3}'
    $r = Invoke-WebRequest -Uri "http://localhost:8000/make_move" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing -TimeoutSec 30
    $result = $r.Content | ConvertFrom-Json
    if ($result.success) {
        Write-Output "[PASS] Move executed" -ForegroundColor Green
        Write-Output "KataGo in response: $($result.katago)"
    } else {
        Write-Output "[FAIL] Move failed: $($result.message)" -ForegroundColor Red
    }
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n[Test 4] Set Difficulty to Expert"
try {
    $body = '{"difficulty":"expert"}'
    $r = Invoke-WebRequest -Uri "http://localhost:8000/set_difficulty" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing -TimeoutSec 30
    $result = $r.Content | ConvertFrom-Json
    if ($result.success) {
        Write-Output "[PASS] Difficulty changed" -ForegroundColor Green
    } else {
        Write-Output "[FAIL] Difficulty change failed" -ForegroundColor Red
    }
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n[Test 5] Verify Difficulty"
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/katago_status" -UseBasicParsing -TimeoutSec 10
    $status = $r.Content | ConvertFrom-Json
    if ($status.difficulty -eq "expert") {
        Write-Output "[PASS] Difficulty verified: expert" -ForegroundColor Green
    } else {
        Write-Output "[FAIL] Difficulty: $($status.difficulty)" -ForegroundColor Red
    }
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n[Test 6] Score Check"
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/score" -Method POST -UseBasicParsing -TimeoutSec 10
    $result = $r.Content | ConvertFrom-Json
    Write-Output "Black: $($result.black_total)"
    Write-Output "White: $($result.white_total)"
    Write-Output "[PASS] Score calculation" -ForegroundColor Green
} catch {
    Write-Output "[ERROR] $($_.Exception.Message)"
}

Write-Output "`n=== Tests Complete ==="
