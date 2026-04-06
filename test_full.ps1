$ErrorActionPreference = "Stop"
$results = @()

function Test-API {
    param($Name, $Url, $Method="GET", $Body=$null)
    try {
        $params = @{Uri=$Url; UseBasicParsing=$true}
        if ($Method -eq "POST") { $params.Method = "POST" }
        if ($Body) { $params.Body = $Body; $params.ContentType = "application/json" }
        $r = Invoke-WebRequest @params
        return @{Success=$true; Status=$r.StatusCode; Content=$r.Content}
    } catch {
        return @{Success=$false; Error=$_.Exception.Message}
    }
}

Write-Output "=== Project_seven API Test ==="

Write-Output "`n[Test 1] GET /katago_status"
$r = Test-API -Name "katago_status" -Url "http://localhost:8000/katago_status"
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n[Test 2] POST /reset"
$r = Test-API -Name "reset" -Url "http://localhost:8000/reset" -Method "POST"
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n[Test 3] POST /make_move at (3,3)"
$body = '{"x":3,"y":3}'
$r = Test-API -Name "make_move" -Url "http://localhost:8000/make_move" -Method "POST" -Body $body
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n[Test 4] POST /check_game_over"
$r = Test-API -Name "check_game_over" -Url "http://localhost:8000/check_game_over" -Method "POST"
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n[Test 5] POST /score"
$r = Test-API -Name "score" -Url "http://localhost:8000/score" -Method "POST"
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n[Test 6] POST /set_difficulty (expert)"
$body = '{"difficulty":"expert"}'
$r = Test-API -Name "set_difficulty" -Url "http://localhost:8000/set_difficulty" -Method "POST" -Body $body
Write-Output "Status: $($r.Status), Content: $($r.Content)"

Write-Output "`n=== Tests Complete ==="
