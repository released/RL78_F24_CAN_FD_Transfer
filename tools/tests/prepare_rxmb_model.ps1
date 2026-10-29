param([Parameter(Mandatory=$true)][string]$DeviceProject,
      [Parameter(Mandatory=$true)][string]$OutputDirectory)
$ErrorActionPreference='Stop'
# Extract exact vendor implementation/types for native register-model tests.
# Generated includes are disposable test inputs, never firmware source changes.
$driver=Join-Path $DeviceProject 'src\smc_gen\r_rscanfd_rl78\r_rscanfd_rl78.c'
$header=Join-Path $DeviceProject 'src\smc_gen\r_rscanfd_rl78\r_rscanfd_rl78_if.h'
$text=[IO.File]::ReadAllText($driver)
$types=[IO.File]::ReadAllText($header)
$functions=''
foreach($signature in @('e_can_err_t R_CAN_ReadRXMBInHandler(',
                        'R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_glb_rxmb_isr,')) {
    $start=$text.IndexOf($signature)
    if($start -lt 0){throw "Vendor RXMB implementation not found: $signature"}
    $body=$text.Substring($start)
    $end=[regex]::Match($body,'(?m)^\}')
    if(!$end.Success){throw "Vendor function end not found: $signature"}
    $functions+=$body.Substring(0,$end.Index+1)+"`r`n"
}
$tables=''
foreach($name in @('s_dlc_to_length')){
    $match=[regex]::Match($text,'(?s)static const(?: __far)? uint8_t '+$name+'\[.*?\};')
    if(!$match.Success){throw "Vendor table missing: $name"}
    $tables+=$match.Value+"`r`n"
}
$startTypes=$types.IndexOf('/* CAN receive frame information */')
$endTypes=$types.IndexOf('/* CAN transmit frame */',$startTypes)
$startFrame=$types.IndexOf('/* CAN receive frame */',$endTypes)
$endFrame=$types.IndexOf('/* RX rule filter */',$startFrame)
if($startTypes -lt 0 -or $endTypes -lt 0 -or $startFrame -lt 0 -or $endFrame -lt 0){throw 'Vendor RX types not found.'}
$copyright=$text.Substring(0,$text.IndexOf('/**********************************************************************************************************************',10))
$utf8=New-Object Text.UTF8Encoding($false)
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
[IO.File]::WriteAllText((Join-Path $OutputDirectory 'vendor_read_rxmb.inc'),
    $copyright+"`r`n#define CAN_DLC_HIGHER_NUM 8U`r`n"+$tables+$functions,$utf8)
[IO.File]::WriteAllText((Join-Path $OutputDirectory 'vendor_rx_types.inc'),
    $copyright+$types.Substring($startTypes,$endTypes-$startTypes)+$types.Substring($startFrame,$endFrame-$startFrame),$utf8)
Write-Host 'Vendor RXMB ISR/InHandler bodies, DLC table and RX types extracted without changing firmware sources.'
