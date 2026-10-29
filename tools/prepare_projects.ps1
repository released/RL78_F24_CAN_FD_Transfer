param([ValidateSet('transfer','boot','app','all')][string]$Kind='all',
      [string]$Project='', [switch]$CheckOnly, [switch]$ArtifactsOnly, [switch]$FunctionsOnly)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'device_hal_paths.ps1')

. (Join-Path $PSScriptRoot 'device_common.ps1')
. (Join-Path $PSScriptRoot 'public_project_paths.ps1')

function Get-AppSources([string]$Folder) {
    $paths=@((Join-Path $Folder 'custom_func.c'))+@(Get-ChildItem -LiteralPath (Join-Path $script:DeviceBoot 'hal\device') -Filter '*.c' -File | ForEach-Object FullName)
    [xml]$xml=[IO.File]::ReadAllText((Get-DeviceProjectFile $Folder))
    foreach($node in $xml.SelectNodes('//Instance[Type="File"]')){
        $raw=[string]$node.RelativePath
        if(!$raw){continue}
        $path=if([IO.Path]::IsPathRooted($raw)){[IO.Path]::GetFullPath($raw)}else{[IO.Path]::GetFullPath((Join-Path $Folder $raw))}
        $name=[IO.Path]::GetFileName($path)
        if([IO.Path]::GetExtension($path) -in @('.c','.asm') -and $name -notin @('r_main.c','main.c','custom_func.c',
            'misc.c','can_fd_module.c','device_vectors.asm','cstart.asm','app_metadata.generated.asm','ftable_RL78_F24.asm') -and
            $path -notmatch '[\\/](smc_gen|generated_projection)[\\/]'){$paths+=$path}
    }
    return @($paths | Sort-Object -Unique)
}
function Get-DeviceVectors([string]$ImageKind,[string]$Folder) {
    $start=Get-DeviceMacro IMAGE_APPLICATION_START
    $ram=(Get-DeviceMacro BOOT_APP_SHARED_RAM_ADDRESS) -band 65535
    $owner=Get-DeviceMacro BOOT_APP_RAM_OWNER_APPLICATION
    if($ImageKind -eq 'transfer'){return ".EXTERN _Device_TimerIsr`n_Device_TimerIsr .VECTOR 0x2E`n"}
    if($ImageKind -eq 'app'){
        $mapping=Read-AppIrqConfig $Folder
        $allowed=@{}
        $header=[IO.File]::ReadAllText((Join-Path $Folder 'iodefine.h'))
        foreach($m in [regex]::Matches($header,'#define\s+INT\w+\s+(0x[0-9A-Fa-f]+)')){
            $v=[Convert]::ToInt32($m.Groups[1].Value.Substring(2),16)
            if($v -ge 4 -and $v -le 124){$allowed[$v]=$true}
        }
        foreach($v in $mapping.Keys){
            Require-Device ($allowed.ContainsKey($v)) ('Invalid normal IRQ '+$v)
        }
        $sourceText=(@(Get-AppSources $Folder | ForEach-Object {[IO.File]::ReadAllText($_)}) -join "`n").Replace("`r`n","`n")
        Require-Device ($sourceText -notmatch '#pragma\s+interrupt[^\n]*\bvect\s*=') 'App IRQs use app_irq_handlers.h, not physical vectors.'
        foreach($symbol in $mapping.Values){
            Require-Device ([regex]::Matches($sourceText,'(?m)^\s*#pragma\s+interrupt\s+'+[regex]::Escape($symbol)+'\s*\n').Count -eq 1) ('IRQ requires one normal RETI pragma: '+$symbol)
        }
        $text=".EXTERN _start`n"
        foreach($symbol in @($mapping.Values | Sort-Object -Unique)){$text+=".EXTERN _$symbol`n"}
        $text+=".SECTION .app_vectors, TEXT`n    BR !!_start`n    BR !!device_default_isr`n"
        for($v=4;$v -lt 126;$v+=2){
            $symbol=if($mapping.ContainsKey($v)){'_'+$mapping[$v]}else{'device_default_isr'}
            $text+=('    BR !!{0} ; vector {1:X2}' -f $symbol,$v)+"`n"
        }
        return $text+"    BR !!device_default_brk`n.SECTION .text, TEXT`ndevice_default_isr:`n    RETI`ndevice_default_brk:`n    RETB`n"
    }
    $text=".EXTERN _Device_TimerIsr`n.EXTERN _Device_CanRxIsr`n.PUBLIC _Device_Jump`n.EXTERN _Device_CanStop`n.EXTERN _R_Config_TAU0_1_Stop`n"
    if(Get-DeviceMacro CAN_BOOT_DEBUG_UART){$text+=".EXTERN _R_Config_UART0_Stop`n"}
    $text+=".SECTION .text, TEXT`n"
    for($v=4;$v -lt 126;$v+=2){$text+=('device_irq_{0:X2} .VECTOR 0x{0:X2}' -f $v)+"`n"+('device_irq_{0:X2}:' -f $v)+"`n    CALL !device_dispatch`n"+('    .DB 0x{0:X2}' -f $v)+"`n"}
    $text+=@'
device_dispatch:
    PUSH AX
    PUSH HL
    MOV A,ES
    PUSH AX
    MOV ES,#0
    MOVW AX,[SP+6]
    MOVW HL,AX
    MOV A,ES:[HL]
    MOV X,A
    MOV A,#0
    SHLW AX,1
'@
    $text+="`n"+('    ADDW AX,#0x{0:X4}' -f $start)+"`n"+('    CMP !0x{0:X4},#0x{1:X2}' -f $ram,$owner)+"`n"
    $text+='    BZ $device_irq_target'+"`n"+('    CMPW AX,#0x{0:X4}' -f ($start+0x2E*2))+"`n"
    $text+=@'
    BNZ $device_irq_can
    MOVW AX,#LOWW(_Device_TimerIsr)
    BR $device_irq_target
device_irq_can:
    CMPW AX,#DEVICE_CAN_APP_SLOT
    BNZ $device_irq_default
    MOVW AX,#LOWW(_Device_CanRxIsr)
    BR $device_irq_target
device_irq_default:
    MOVW AX,#LOWW(device_default_isr)
device_irq_target:
    MOVW [SP+6],AX
    POP AX
    MOV ES,A
    POP HL
    POP AX
    RET
device_default_isr:
    RETI
_Device_Jump:
    DI
    CALL !!_Device_CanStop
    CALL !!_R_Config_TAU0_1_Stop
'@
    $text=$text.Replace('DEVICE_CAN_APP_SLOT',('0x{0:X4}' -f ($start+0x4C*2)))
    if(Get-DeviceMacro CAN_BOOT_DEBUG_UART){$text+="`n    CALL !!_R_Config_UART0_Stop"}
    $text+="`n    MOVW AX,#0x4E48`n"+('    MOVW !0x{0:X4},AX' -f ($ram+4))+"`n    MOVW AX,#0x4644`n"
    $text+=('    MOVW !0x{0:X4},AX' -f ($ram+6))+"`n"+('    MOV !0x{0:X4},#0x{1:X2}' -f $ram,$owner)+"`n"+('    BR !!0x{0:X5}' -f $start)+"`n"
    return $text
}
function Prepare-DeviceProject([string]$ImageKind,[string]$Folder) {
    $Folder=(Resolve-Path -LiteralPath $Folder).Path
    $mtpj=Get-DeviceProjectFile $Folder
    $xml=New-Object Xml.XmlDocument; $xml.PreserveWhitespace=$true; $xml.Load($mtpj)
    $root=$xml.DocumentElement; $group=$root.SelectSingleNode('Class')
    $gen=Join-Path $Folder 'src\smc_gen'
    # Validate all user inputs before generating any files.
    if($ImageKind -eq 'app'){$identity=Get-DeviceIdentity $Folder;$metadata=Get-DeviceMetadataAssembly $Folder}
    $vectors=Get-DeviceVectors $ImageKind $Folder
    $iodefine=Join-Path $Folder 'iodefine.h'
    Write-DeviceGenerated $iodefine (Convert-DeviceIodefineComment ([IO.File]::ReadAllText($iodefine))) -CheckOnly:$CheckOnly
    $sources=@($xml.SelectNodes('//Instance[Type="File"]') | ForEach-Object {
        $relative=[string]$_.RelativePath
        if([IO.Path]::GetExtension($relative) -in @('.c','.asm')){Join-Path $Folder $relative}
    })
    $expected=@()
    foreach($path in @($sources | Sort-Object -Unique)){
        Require-Device (Test-Path -LiteralPath $path -PathType Leaf) ('Missing source '+$path)
        $target=$path
        if([IO.Path]::GetFileName($path) -eq 'cstart.asm' -and $path -notmatch '[\\/]generated_projection[\\/]'){
            $text=[IO.File]::ReadAllText($path).Replace("`r`n","`n")
            $text=$text.Replace('$IF (__RENESAS_VERSION__ < 0x01010000)  ; for CC-RL V1.00','$IF (1) ; Explicit stack')
            $text=$text.Replace('.DS     0x200','.DS     0x1000').Replace('__STACK_ADDR_START','_stacktop').Replace('__STACK_ADDR_END','_stackend')
            $text=$text.Replace('.SECTION .const, CONST'+"`n"+'.L_section_const:','; All constants use far ROM; no empty near-mirror anchor.')
            if($ImageKind -eq 'app'){$text=$text.Replace('_start  .VECTOR 0','; Boot owns physical RESET vector.')}
            if($ImageKind -eq 'boot'){
                $declarations='';$copies=''
                foreach($name in @('RFD_DATA_n','RFD_CMN_f','RFD_CF_f','DRAM_f')){
                    $type=if($name -eq 'RFD_DATA_n'){'DATA'}else{'TEXTF'}
                    $declarations+=".SECTION ${name}R, $type`n"
                    $copies+=@"
    MOV ES,#HIGHW(STARTOF($name))
    MOVW HL,#LOWW(STARTOF($name))
    MOVW DE,#LOWW(STARTOF(${name}R))
    MOVW BC,#LOWW(SIZEOF($name))
.L_COPY_${name}:
    MOVW AX,BC
    CMPW AX,#0
    BZ `$.L_DONE_$name
    MOV A,ES:[HL]
    MOV [DE],A
    INCW HL
    INCW DE
    DECW BC
    BR `$.L_COPY_$name
.L_DONE_${name}:

"@
                }
                $text=$text.Replace('.SECTION .dataR, DATA',$declarations+'.SECTION .dataR, DATA')
                $text=$text.Replace('    CALL    !!_bsp_init_hardware',$copies+"`n    CALL    !!_bsp_init_hardware")
            }
            $target=Join-Path $Folder 'generated_projection\cstart.asm'
            Write-DeviceGenerated $target $text -CheckOnly:$CheckOnly
        }
        $expected+=Get-DeviceRelative $target $Folder
    }
    if($ImageKind -eq 'app'){
        Write-DeviceGenerated (Join-Path $Folder 'app_irq_table.inc') ('; Generated fixed normal-vector ABI. Do not edit.'+"`n"+$vectors) -CheckOnly:$CheckOnly
        Write-DeviceGenerated (Join-Path $Folder 'ftable.inc') $metadata -CheckOnly:$CheckOnly
        Write-DeviceGenerated (Join-Path $Folder 'ftable_RL78_F24.asm') ('; Generated wrapper. Do not edit.'+"`n"+'$INCLUDE "app_irq_table.inc"'+"`n"+'$INCLUDE "ftable.inc"'+"`n") -CheckOnly:$CheckOnly
        $expected+='ftable_RL78_F24.asm'
        Write-DeviceGenerated (Join-Path $Folder 'app_identity.h') ("/* App-local identity. Generated, do not edit. */`n"+
            ('#define DEVICE_APP_VERSION ({0}UL)' -f $identity.Version)+"`n"+('#define DEVICE_APP_BUILD ({0}UL)' -f $identity.Build)+"`n") -CheckOnly:$CheckOnly
    }else{
        $vectorFile=if($ImageKind -eq 'boot'){'f24_boot_vectors.asm'}else{'device_vectors.asm'}
        Write-DeviceGenerated (Join-Path $Folder $vectorFile) ('; Generated fixed normal-vector ABI. Do not edit.'+"`n"+$vectors) -CheckOnly:$CheckOnly
        $expected+=$vectorFile
    }
    $pre='powershell.exe -NoProfile -ExecutionPolicy Bypass -File "'+(Get-DeviceRelative (Join-Path $PSScriptRoot 'prepare_projects.ps1') $Folder)+'" -Kind '+$ImageKind+' -Project "." -ArtifactsOnly'+"`n"
    $post='powershell.exe -NoProfile -ExecutionPolicy Bypass -File "'+(Get-DeviceRelative (Join-Path $PSScriptRoot 'package_images.ps1') $Folder)+'" -Kind '+$ImageKind+' -Project "."'+"`n"
    if($CheckOnly){
        Assert-DevicePublicProjectPaths $xml
        $registered=@($group.SelectNodes('Instance[Type="File"]') | ForEach-Object {[string]$_.RelativePath} | Where-Object {[IO.Path]::GetExtension($_) -in @('.c','.asm')})
        Require-Device (@(Compare-Object ($registered | Sort-Object -Unique) ($expected | Sort-Object -Unique)).Count -eq 0) 'Stale MTPJ registrations.'
        Require-Device ($root.SelectSingleNode('.//PreBuildCommands-0').InnerText.Replace("`r`n","`n") -ceq $pre -and
            $root.SelectSingleNode('.//PostBuildCommands-0').InnerText.Replace("`r`n","`n") -ceq $post) 'Stale PowerShell build hooks.'
        return
    }
    if($ArtifactsOnly){return}
    Assert-DeviceLocalBindings $Folder
    return

}

if($FunctionsOnly){return}
if($Kind -eq 'all'){
    Require-Device (!$Project -and !$ArtifactsOnly) '-Project/ArtifactsOnly requires one explicit Kind.'
    foreach($item in @('boot','app')){
        $folder=switch($item){boot {'RL78_F24_Boot_loader_CAN_FD'} app {'RL78_F24_Boot_loader_CAN_FD_app'}}
        Prepare-DeviceProject $item (Join-Path (Split-Path -Parent $script:DeviceBoot) $folder)
    }
}else{
    if(!$Project){$Project=$script:DeviceBoot}
    Prepare-DeviceProject $Kind $Project
}
