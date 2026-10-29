param([string]$ToolProject='', [switch]$CModels)
$ErrorActionPreference='Stop'
& (Join-Path $PSScriptRoot 'tools\test_device.ps1')
if(!$CModels -and !$ToolProject){return}
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(-not $installation){throw 'MSVC x64 tools required for 32-bit-long host tests.'}
$bootstrap=Join-Path $installation 'Common7\Tools\VsDevCmd.bat'
$environmentLines=& cmd /d /c ('call "'+$bootstrap+'" -no_logo -arch=amd64 >nul && set')
if($LASTEXITCODE -ne 0){throw 'MSVC environment setup failed.'}
foreach($line in $environmentLines){if($line -match '^([^=]+)=(.*)$'){[Environment]::SetEnvironmentVariable($Matches[1],$Matches[2],'Process')}}
$output=Join-Path $PSScriptRoot 'tmp\tests'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$arguments=@('/nologo','/std:c++17','/W4','/WX','/EHsc',('/I'+$PSScriptRoot),('/Fe'+(Join-Path $output 'device_contract_tests.exe')))
$sources=@((Join-Path $PSScriptRoot 'tools\tests\device_contract_tests.cpp'),
    (Join-Path $PSScriptRoot 'hal\image\image_metadata.c'),(Join-Path $PSScriptRoot 'hal\image\image_metadata_profile.c'))
if($ToolProject){
    $core=Join-Path (Resolve-Path -LiteralPath $ToolProject).Path 'src\core'
    $arguments+=@('/DF24_TEST_TOOL',('/I'+$core))
    $sources+=@(Join-Path $core 'firmware_update_profile.cpp')
}
Push-Location -LiteralPath $output
try {
    $probe=Join-Path $output 'fd_pair_config_probe.c'
    [IO.File]::WriteAllText($probe,@'
#include "f24_device_config.h"
#if CAN_CFG_NRXMB != 1 || CAN_CFG_RMPLS != 7 || CAN_CFG_RMIE_ALL != 2 || CAN_CFG_RMIE_VALUE != 1
#error Boot_App_require_64_byte_RXMB0_with_RMIEC_bit0
#endif
#if CAN_CFG_INTRCANGRVC_USE != 0 || CAN_CFG_INTRCANGRFR_USE != 0 || CAN_CFG_INTRCAN0TRM_USE != 0
#error Boot_App_own_the_RX_ISR_and_poll_TX
#endif
#if CAN_CFG_RF0_RFDC != 0 || CAN_CFG_RF1_RFDC != 0 || CAN_CFG_CLOE != 0 || CAN_CFG_FDOE != 1
#error Boot_App_require_FD_RXMB_without_FIFO
#endif
#if CAN_CFG_NBRP != 3 || CAN_CFG_NMNL_TSEG1 != 15 || CAN_CFG_NMNL_TSEG2 != 4 || CAN_CFG_DBRP != 1 || CAN_CFG_DATA_TSEG1 != 7 || CAN_CFG_DATA_TSEG2 != 2
#error FD_500K_2M_clock_profile_changed
#endif
'@,(New-Object Text.UTF8Encoding($false)))
    foreach($profile in @('BOOT','APP')) {
        $settings=if($profile -eq 'APP'){Join-Path $PSScriptRoot 'src\smc_gen\r_config'}else{Join-Path $PSScriptRoot 'src\smc_gen\r_config'}
        & cl /nologo /EP ("/DF24_DEVICE_"+$profile) ('/I'+(Join-Path $PSScriptRoot 'hal\device')) ('/I'+$settings) $probe | Out-Null
        if($LASTEXITCODE){throw 'Actual FD Boot/App configuration check failed.'}
        & cl /nologo /TC /Za /W4 /WX ("/DF24_DEVICE_"+$profile) (('/I')+(Join-Path $PSScriptRoot 'tools\tests\transfer_mock')) (('/I')+(Join-Path $PSScriptRoot 'hal\device')) (('/I')+$PSScriptRoot) (Join-Path $PSScriptRoot 'tools\tests\fd_pair_can_tests.c') /Fe:fd_can_tests.exe
        if($LASTEXITCODE -ne 0){throw 'FD HAL test build failed.'}
        & (Join-Path $output 'fd_can_tests.exe')
        if($LASTEXITCODE -ne 0){throw 'FD HAL test failed.'}
    }
    & cl @arguments @sources
    if($LASTEXITCODE -ne 0){throw 'C/C++ RAM helper test build failed.'}
    & (Join-Path $output 'device_contract_tests.exe') (Join-Path $PSScriptRoot 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD_app_update.bin')
    if($LASTEXITCODE -ne 0){throw 'C/C++ RAM helper tests failed.'}
} finally { Pop-Location }
