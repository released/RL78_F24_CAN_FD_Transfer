param()
$ErrorActionPreference='Stop'
# Prepare and clean-build only this standalone FD Transfer project.
& (Join-Path $PSScriptRoot '..\RL78_F24_Boot_loader_CAN_FD\build.ps1') -Kind transfer
