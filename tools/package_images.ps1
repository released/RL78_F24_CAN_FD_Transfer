param([Parameter(Mandatory=$true)][ValidateSet('transfer','boot','app')][string]$Kind,
      [Parameter(Mandatory=$true)][string]$Project, [switch]$FunctionsOnly)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'device_common.ps1')
. (Join-Path $PSScriptRoot 'app_build_report.ps1')

function Test-DeviceCommon($Sections) {
    Require-Device ($Sections.ContainsKey('.stack_bss') -and $Sections['.stack_bss'][2] -eq 4096) 'Explicit 4096-byte stack required.'
    Require-Device (!$Sections.ContainsKey('.const') -or $Sections['.const'][2] -eq 0) 'No near-ROM mirror constants.'
    $handoff=Get-DeviceMacro BOOT_APP_SHARED_RAM_ADDRESS
    foreach($name in $Sections.Keys){
        $s=$Sections[$name]
        if($s[2] -and $s[0] -ge 0xF0000){
            Require-Device ($s[0] -ge 0xF9F00 -and $s[1] -lt 0xFFF00) ('RAM out of device range '+$name)
            Require-Device ($s[1] -lt $handoff -or $s[0] -ge $handoff+16) ('Handoff RAM overlap '+$name)
        }
    }
}
function Test-DeviceBoot($Memory,$Map) {
    $sections=$Map.Sections;$symbols=$Map.Symbols
    $start=Get-DeviceMacro IMAGE_APPLICATION_START
    Require-Device ($start -eq 8192 -and (Get-DeviceMacro IMAGE_RESERVED_SIZE) -eq 8192) 'Device contract requires 8KiB Boot.'
    $ends=@($sections.Keys | Where-Object {$sections[$_][2] -and $sections[$_][0] -lt 0xF0000 -and $_ -ne '.monitor2'} | ForEach-Object {$sections[$_][1]})
    $end=($ends | Measure-Object -Maximum).Maximum
    Require-Device ($end -lt $start-512) 'Boot exceeds 8KiB / minimum 512-byte spare guard.'
    foreach($at in $Memory.Keys){Require-Device ($at -lt $start -or ($at -ge 0x3FF00 -and $at -le 0x3FFFF)) 'Boot/App Flash overlap.'}
    Test-DeviceCommon $sections
    foreach($name in @('RFD_DATA_n','RFD_CMN_f','RFD_CF_f','DRAM_f')){
        Require-Device ($sections.ContainsKey($name) -and $sections.ContainsKey($name+'R') -and
            $sections[$name][2] -gt 0 -and $sections[$name][2] -eq $sections[$name+'R'][2]) ('RFD copy pair '+$name)
    }
    $names=@($symbols.Keys | Where-Object {$_.StartsWith('_R_RFD_') -or $_.Split('@')[0] -in @('_fail_reset','_wait_end','_execute')})
    Require-Device ($names.Count -ge 14) 'Missing RAM Flash call-chain symbols.'
    foreach($name in $names){Require-Device ($symbols[$name] -ge 0xF9F00 -and $symbols[$name] -lt
        (Get-DeviceMacro BOOT_APP_SHARED_RAM_ADDRESS)) ('Flash callee outside RAM '+$name)}
    for($v=4;$v -lt 126;$v+=2){
        $name='device_irq_{0:X2}' -f $v
        Require-Device $symbols.ContainsKey($name) ('Missing physical IRQ '+$name)
        $at=$symbols[$name];$entry=Get-DeviceSpan $Memory $v 2;$code=Get-DeviceSpan $Memory $at 4
        Require-Device ([BitConverter]::ToUInt16($entry,0) -eq $at) 'Wrong physical IRQ.'
        Require-Device ($code[0] -eq 253 -and $code[3] -eq $v -and [BitConverter]::ToUInt16($code,1) -eq $symbols['device_dispatch']) 'Wrong CALL/literal IRQ stub.'
    }
    $brk=[BitConverter]::ToUInt16((Get-DeviceSpan $Memory 126 2),0)
    Require-Device ($brk -in @(0,65535)) 'BRK must not be forwarded.'
    Require-Device ($symbols.ContainsKey('_Device_CanRxIsr') -and
        !$symbols.ContainsKey('_r_can_glb_rxmb_isr@1')) 'Boot owns the RXMB IRQ through its complete dispatcher.'
    $isr=$symbols['_Device_CanRxIsr'];$isrSize=$Map.Sizes['_Device_CanRxIsr']
    Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory ($isr+$isrSize-2) 2),0) -eq 0xFC61) 'CAN RX handler must RETI.'
    $dispatchSize=$symbols['_Device_Jump']-$symbols['device_dispatch']
    $dispatch=[BitConverter]::ToString((Get-DeviceSpan $Memory $symbols['device_dispatch'] $dispatchSize))
    $slot=$start+0x4C*2
    $pattern='44-{0:X2}-{1:X2}-DF-05-30-{2:X2}-{3:X2}' -f ($slot -band 255),($slot -shr 8),($isr -band 255),($isr -shr 8)
    Require-Device ($dispatch.Contains($pattern)) 'Missing compiled Boot CAN RX dispatch route.'
    Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory 0 2),0) -eq $symbols['_start']) 'Wrong RESET vector.'
    $cross=[regex]::Match($Map.Text,'(?s)\*\*\* Cross Reference List \*\*\*(.*)')
    Require-Device $cross.Success 'Missing RFD RAM cross-reference list.'
    $target=$null;$count=0
    foreach($line in $cross.Groups[1].Value.Split("`n")){
        $m=[regex]::Match($line,'^ +([0-9a-f]{8}) (.*)')
        if($m.Success){$target=[Convert]::ToInt64($m.Groups[1].Value,16)}
        foreach($ref in [regex]::Matches($line,'\(([0-9a-f]{8}):([^)]*)\)')){
            if($ref.Groups[2].Value -in @('DRAM_fR','RFD_CMN_fR','RFD_CF_fR')){
                Require-Device ($null -ne $target -and $target -ge 0xF0000) 'RAM Flash routine references Code Flash.';$count++
            }
        }
    }
    Require-Device ($count -ge 15) 'Missing RFD RAM cross-reference evidence.'
    return @{boot_end=('0x{0:X4}' -f [long]$end);boot_spare=($start-$end-1);ram_flash_functions=$names.Count;ram_flash_references=$count}
}
function Get-PackagedDeviceApp([string]$Folder,$Memory,$Map) {
    $sections=$Map.Sections;$symbols=$Map.Symbols
    Test-DeviceCommon $sections
    $start=Get-DeviceMacro IMAGE_APPLICATION_START;$limit=$start+(Get-DeviceMacro IMAGE_APPLICATION_CAPACITY)
    Require-Device ($sections.ContainsKey('.app_vectors') -and ($sections['.app_vectors'] -join ',') -eq (@($start,($start+255),256) -join ',')) 'Wrong 256-byte IRQ table.'
    $metadataNames=@($sections.Keys | Where-Object {$_.TrimStart('.').StartsWith('appmeta_AT',[StringComparison]::OrdinalIgnoreCase)})
    Require-Device ($metadataNames.Count -eq 1 -and ($sections[$metadataNames[0]] -join ',') -eq
        (@(($start+256),($start+511),256) -join ',')) 'Missing linked generated metadata region.'
    for($v=0;$v -lt 128;$v+=2){Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory $v 2),0) -in @(0,65535)) 'App owns a physical vector.'}
    $fixed=@('.vect','.option_byte','.security_id','.flash_security_id','.monitor1','.monitor2')
    $appSections=@{};$ranges=@();$fixedRanges=@()
    foreach($name in $sections.Keys){
        $s=$sections[$name]
        if(!$s[2]){continue}
        if($name -in $fixed){$fixedRanges+=,@($s[0],$s[1]);continue}
        if($s[0] -ge 0xF0000){continue}
        Require-Device ($s[0] -ge $start -and $s[0] -le $s[1] -and $s[1] -lt $limit) ('App outside protected partition '+$name)
        Require-Device ($name -eq '.app_vectors' -or $name -in $metadataNames -or $s[0] -ge $start+512) ('Code occupies reserved metadata '+$name)
        $appSections[$name]=$s;$ranges+=,@($s[0],$s[1])
    }
    $end=(@($appSections.Values | ForEach-Object {$_[1]}) | Measure-Object -Maximum).Maximum+1
    [int]$crcAt=([long]$end+3) -band -4;[int]$size=$crcAt+4-$start
    Require-Device ($size -ge (Get-DeviceMacro IMAGE_MINIMUM_IMAGE_SIZE) -and $size -le $limit-$start) 'App exceeds capacity.'
    foreach($at in $Memory.Keys){
        $found=$false
        foreach($range in ($ranges+$fixedRanges)){if($at -ge $range[0] -and $at -le $range[1]){$found=$true;break}}
        Require-Device $found 'Unmapped App ROM byte.'
    }
    $image=Get-DeviceSpan $Memory $start $size
    $expected=Get-DeviceMetadata $Folder ([uint32]::MaxValue)
    for($i=0;$i -lt 256;$i++){Require-Device ($image[256+$i] -eq $expected[$i]) 'Compiled metadata differs from App header/Boot contract. Clean rebuild.'}
    $handlers=Read-AppIrqConfig $Folder
    $mapping=@{}
    foreach($v in $handlers.Keys){$mapping[$v]='_'+[string]$handlers[$v]}
    for($v=0;$v -lt 128;$v+=2){
        $off=$v*2
        $name=if($v -eq 0){'_start'}elseif($v -eq 126){'device_default_brk'}elseif($mapping.ContainsKey($v)){$mapping[$v]}else{'device_default_isr'}
        $address=[long]$image[$off+1] -bor ([long]$image[$off+2] -shl 8) -bor ([long]$image[$off+3] -shl 16)
        Require-Device ($symbols.ContainsKey($name) -and $image[$off] -eq 236 -and $address -eq $symbols[$name] -and
            $address -ge $start+512 -and $address -lt $crcAt) ('Wrong full App IRQ entry {0:X2}' -f $v)
    }
    Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory $symbols['device_default_isr'] 2),0) -eq 0xFC61) 'Default ISR must RETI.'
    if($symbols.ContainsKey('_Device_CanInit')) {
        Require-Device ($mapping.ContainsKey(0x4C) -and $mapping[0x4C] -ceq '_Device_CanRxIsr') 'Active CAN RX requires App IRQ mapping at0x4C.'
        $handler=$symbols['_Device_CanRxIsr'];$handlerSize=$Map.Sizes['_Device_CanRxIsr']
        Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory ($handler+$handlerSize-2) 2),0) -eq 0xFC61) 'App CAN RX handler must RETI.'
    }
    Require-Device ([BitConverter]::ToUInt16((Get-DeviceSpan $Memory $symbols['device_default_brk'] 2),0) -eq 0xEC61) 'Default BRK must RETB.'
    (Get-DeviceMetadata $Folder $size).CopyTo($image,256)
    [BitConverter]::GetBytes((Get-DeviceCrc32 $image ($size-4))).CopyTo($image,$size-4)
    return ,$image
}

if($FunctionsOnly){return}
$Project=(Resolve-Path -LiteralPath $Project).Path
$stem=[IO.Path]::GetFileNameWithoutExtension((Get-DeviceProjectFile $Project));$build=Join-Path $Project 'DefaultBuild'
$memory=Read-DeviceSrec (Join-Path $build ($stem+'.mot'))
$map=Read-DeviceMap (Join-Path $build ($stem+'.map'))
$report=@{kind=$Kind;hardware='NOT RUN'}
$output=New-Object 'Collections.Generic.Dictionary[long,byte]'
if($Kind -eq 'boot'){
    $audit=Test-DeviceBoot $memory $map
    foreach($key in $audit.Keys){$report[$key]=$audit[$key]}
    foreach($at in $memory.Keys){if($at -lt (Get-DeviceMacro IMAGE_APPLICATION_START)){$output[$at]=$memory[$at]}}
    $hex=Join-Path $build 'boot0000_1FFF.hex'
}elseif($Kind -eq 'app'){
    $image=Get-PackagedDeviceApp $Project $memory $map
    for($i=0;$i -lt $image.Length;$i++){$output[[long]((Get-DeviceMacro IMAGE_APPLICATION_START)+$i)]=$image[$i]}
    $report.image_bytes=$image.Length;$report.crc32='0x{0:X8}' -f (Get-DeviceCrc32 $image ($image.Length-4))
    $hex=Join-Path $build 'flash_app_variable_CRC.hex'
}else{
    Test-DeviceCommon $map.Sections
    foreach($at in $memory.Keys){
        Require-Device ($at -lt 0x3FC00 -or ($at -ge 0x3FF00 -and $at -lt 0x40000)) 'Transfer out of Flash range.'
        if($at -lt 0x3FC00){$output[$at]=$memory[$at]}
    }
    $hex=Join-Path $build ($stem+'.hex')
}
$combined=$null
if($Kind -eq 'app'){
    $region='flash{0:X4}_{1:X5}.hex' -f (Get-DeviceMacro IMAGE_APPLICATION_START),
        ((Get-DeviceMacro IMAGE_APPLICATION_START)+(Get-DeviceMacro IMAGE_APPLICATION_CAPACITY)-1)
    $bootMot=Join-Path $script:DeviceBoot 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD.mot'
    $bootMap=Join-Path $script:DeviceBoot 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD.map'
    if((Test-Path -LiteralPath $bootMot) -and (Test-Path -LiteralPath $bootMap)){
        $boot=Read-DeviceSrec $bootMot
        Test-DeviceBoot $boot (Read-DeviceMap $bootMap) | Out-Null
        $combined=New-Object 'Collections.Generic.Dictionary[long,byte]'
        $start=Get-DeviceMacro IMAGE_APPLICATION_START
        foreach($at in $boot.Keys){if($at -lt $start){$combined[$at]=$boot[$at]}}
        foreach($at in $output.Keys){$combined[$at]=$output[$at]}
        $combinedText=ConvertTo-DeviceIhex $combined
        $combinedBytes=Get-DeviceSpan $combined 0 ($start+(Get-DeviceMacro IMAGE_APPLICATION_CAPACITY))
    }
}
# All validation and output construction finish before touching delivery files.
$hexText=ConvertTo-DeviceIhex $output
if($Kind -eq 'app'){
    [IO.File]::WriteAllBytes((Join-Path $build ($stem+'_update.bin')),$image)
    [IO.File]::WriteAllText((Join-Path $build $region),$hexText,[Text.Encoding]::ASCII)
    if($combined){
        [IO.File]::WriteAllText((Join-Path $Project 'boot_app.hex'),$combinedText,[Text.Encoding]::ASCII)
        [IO.File]::WriteAllBytes((Join-Path $Project 'boot_app.bin'),$combinedBytes)
    }
}
[IO.File]::WriteAllText($hex,$hexText,[Text.Encoding]::ASCII)
if($Kind -eq 'app') { Write-AppBuildReport -Project $Project -ImagePath (Join-Path $build ($stem+'_update.bin')) }
Write-Host ('PASS: '+($report | ConvertTo-Json -Compress))

# APP_TIMING_BINS_BEGIN
if($Kind -eq 'app') {
    $LASTEXITCODE=0
    & (Join-Path $Project 'generate_test_bins.ps1') -ProjectRoot $Project -Automatic
    if($LASTEXITCODE -ne 0){throw 'App timing BIN generation failed.'}
}
# APP_TIMING_BINS_END
