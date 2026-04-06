$ErrorActionPreference = "Stop"

Write-Output "=== Frontend Test ==="

Write-Output "`n[Test] GET / (index.html)"
try {
    $r = Invoke-WebRequest -Uri "http://localhost:8000/" -UseBasicParsing
    Write-Output "Status: $($r.StatusCode)"
    Write-Output "Content-Type: $($r.Headers['Content-Type'])"
    Write-Output "Content Length: $($r.Content.Length)"
    if ($r.Content -match "KataGo") {
        Write-Output "SUCCESS: Frontend contains KataGo reference"
    }
} catch {
    Write-Output "ERROR: $($_.Exception.Message)"
}

Write-Output "`n=== Frontend Test Complete ==="
