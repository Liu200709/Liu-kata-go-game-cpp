$sourceFiles = @(
    "c:\Users\Legolas\Documents\trae_projects\Project_six\katago.exe",
    "c:\Users\Legolas\Documents\trae_projects\Project_six\default_model.bin.gz",
    "c:\Users\Legolas\Documents\trae_projects\Project_six\default_gtp.cfg"
)

$destinationZip = "c:\Users\Legolas\Documents\trae_projects\Project_seven\KataGo_Files.zip"

Write-Output "=== KataGo Files Packaging ==="
Write-Output "Source files:"

foreach ($file in $sourceFiles) {
    if (Test-Path $file) {
        $size = (Get-Item $file).Length
        Write-Output "  [OK] $file ($size bytes)"
    } else {
        Write-Output "  [MISSING] $file"
    }
}

if (Test-Path $destinationZip) {
    Remove-Item $destinationZip -Force
    Write-Output "`nRemoved existing zip file"
}

Compress-Archive -Path $sourceFiles -DestinationPath $destinationZip -CompressionLevel Optimal

if (Test-Path $destinationZip) {
    $zipSize = (Get-Item $destinationZip).Length
    Write-Output "`n=== Packaged Successfully ==="
    Write-Output "Output: $destinationZip"
    Write-Output "Size: $zipSize bytes"

    Write-Output "`n=== Zip Contents ==="
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [System.IO.Compression.ZipFile]::OpenRead($destinationZip)
    foreach ($entry in $zip.Entries) {
        Write-Output "  $($entry.Name) - $($entry.Length) bytes"
    }
    $zip.Dispose()
} else {
    Write-Output "`n[ERROR] Failed to create zip file"
}
