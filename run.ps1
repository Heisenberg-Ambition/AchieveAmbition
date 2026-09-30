
# Create log directory
New-Item -ItemType Directory -Force "log" | Out-Null

# Timestamp
$ts = Get-Date -Format "yyyyMMdd_HHmmss"

# Log file
$logFile = "log/MCAP_CPP_$ts.log"

# Execute
.\bin\MCAP_CPP.exe `
    -v F:\Southlake\Mcap_Cpp\input\20260317100615\20260317100615.h265 `
    -c F:\Southlake\Mcap_Cpp\calibration\calibration_result_2M_Fitting_wuling_zhenzhiche.json `
    -o F:\Southlake\Mcap_Cpp\output 2>&1 |
    Tee-Object -FilePath $logFile

# Exit code
$exitCode = $LASTEXITCODE

Write-Host ""
Write-Host "Process Exit Code: $exitCode"
Write-Host ""

exit $exitCode


#  F:\Southlake\AdasLogDumps_PV\EastLake\CLIP-20260118095904 F:\Southlake\AdasLogDumps_PV\output -b 2>&1 |
# Tee-Object "log/AdasLogDump_$ts.log"

# realVec:

# .\bin\AdasLogDump.exe F:\Southlake\AdasLogDumps_PV\EastLake\realVec F:\Southlake\AdasLogDumps_PV\output -b 2>&1 |
# Tee-Object "log/AdasLogDump_$ts.log"
