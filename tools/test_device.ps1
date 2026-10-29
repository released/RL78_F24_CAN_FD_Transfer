$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'prepare_projects.ps1') -FunctionsOnly
. (Join-Path $PSScriptRoot 'package_images.ps1') -Kind app -Project '.' -FunctionsOnly
$testBoot=$script:DeviceBoot
$testApp=Join-Path $testBoot '..\RL78_F24_Boot_loader_CAN_FD_app'
$bootMemory=Read-DeviceSrec (Join-Path $testBoot 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD.mot')
$bootMap=Read-DeviceMap (Join-Path $testBoot 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD.map')
$appMemory=Read-DeviceSrec (Join-Path $testApp 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD_app.mot')
$appMap=Read-DeviceMap (Join-Path $testApp 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD_app.map')
function Test-Rejected([scriptblock]$Action,[string]$Pattern) {
    $rejected=$false
    try{& $Action | Out-Null}catch{
        if($_.Exception.Message -notmatch $Pattern){throw}
        $rejected=$true
    }
    Require-Device $rejected ('Expected rejection: '+$Pattern)
}
function Test-Case([string]$Name,[scriptblock]$Action) {& $Action;Write-Host ('PASS: '+$Name)}
function Copy-TestMap($Map) {
    return @{Sections=$Map.Sections.Clone();Symbols=$Map.Symbols.Clone();Sizes=$Map.Sizes.Clone();Text=$Map.Text}
}

Test-Case 'production artifacts / project hooks' {
    $report=Test-DeviceBoot $bootMemory $bootMap
    Require-Device ($report.boot_spare -ge 512) 'Boot spare.'
    $image=Get-PackagedDeviceApp $testApp $appMemory $appMap
    $actual=[IO.File]::ReadAllBytes((Join-Path $testApp 'DefaultBuild\RL78_F24_Boot_loader_CAN_FD_app_update.bin'))
    Require-Device ([Convert]::ToBase64String($image) -ceq [Convert]::ToBase64String($actual)) 'Packaged image mismatch.'
    foreach($item in @('boot','app')){& (Join-Path $PSScriptRoot 'prepare_projects.ps1') -Kind $item -CheckOnly}
}
Test-Case 'Boot 8KiB / 512-byte spare guard' {
    $bad=Copy-TestMap $bootMap;$s=$bad.Sections['DRAM_f'];$bad.Sections['DRAM_f']=@($s[0],0x1E00,(0x1E01-$s[0]))
    Test-Rejected {Test-DeviceBoot $bootMemory $bad} 'spare'
}
Test-Case 'RAM Flash and handoff guards' {
    $bad=Copy-TestMap $bootMap;$bad.Sections['.bss']=@(0xFF710,0xFF713,4)
    Test-Rejected {Test-DeviceCommon $bad.Sections} 'Handoff'
    $bad=Copy-TestMap $bootMap;$bad.Symbols['_R_RFD_WriteCodeFlashReq']=0x1000
    Test-Rejected {Test-DeviceBoot $bootMemory $bad} 'outside RAM'
    $bad=Copy-TestMap $bootMap
    $bad.Text+="`n    00001234 0000(000fb222:DRAM_fR)`n"
    Test-Rejected {Test-DeviceBoot $bootMemory $bad} 'references Code Flash'
}
Test-Case 'compiled vectors / metadata / partition corruption' {
    $bad=New-Object 'Collections.Generic.Dictionary[long,byte]'
    foreach($key in $bootMemory.Keys){$bad[$key]=$bootMemory[$key]}
    $dispatchAt=$bootMap.Symbols['device_dispatch']
    $dispatchCount=$bootMap.Symbols['_Device_Jump']-$dispatchAt
    $patched=$false
    for($off=0;$off -lt $dispatchCount-2;$off++){
        if($bad[$dispatchAt+$off] -eq 0x44 -and $bad[$dispatchAt+$off+1] -eq 0x98 -and $bad[$dispatchAt+$off+2] -eq 0x20){
            $bad[$dispatchAt+$off+1]=0x99;$patched=$true;break
        }
    }
    Require-Device $patched 'CAN dispatch compare not found.'
    Test-Rejected {Test-DeviceBoot $bad $bootMap} 'CAN RX dispatch'
    foreach($at in @(0x2000,0x2001,0x2098,0x2099,0x2100,0x2180)){
        $bad=New-Object 'Collections.Generic.Dictionary[long,byte]'
        foreach($key in $appMemory.Keys){$bad[$key]=$appMemory[$key]}
        $bad[[long]$at]=$bad[[long]$at] -bxor 1
        Test-Rejected {Get-PackagedDeviceApp $testApp $bad $appMap} 'metadata|IRQ'
    }
    foreach($first in @(0x1FFC,0x3FBFF,0xFF710)){
        $bad=Copy-TestMap $appMap;$bad.Sections['.bad']=@($first,($first+3),4)
        Test-Rejected {Get-PackagedDeviceApp $testApp $appMemory $bad} 'partition|Handoff'
    }
    foreach($pair in @(@($bootMemory,$bootMap,'boot'),@($appMemory,$appMap,'app'))){
        $bad=New-Object 'Collections.Generic.Dictionary[long,byte]'
        foreach($key in $pair[0].Keys){$bad[$key]=$pair[0][$key]}
        $last=$pair[1].Symbols['_Device_CanRxIsr']+$pair[1].Sizes['_Device_CanRxIsr']-1
        $bad[$last]=$bad[$last] -bxor 1
        if($pair[2] -eq 'boot'){Test-Rejected {Test-DeviceBoot $bad $pair[1]} 'CAN RX handler must RETI'}
        else{Test-Rejected {Get-PackagedDeviceApp $testApp $bad $pair[1]} 'CAN RX handler must RETI'}
    }
}
Test-Case 'owner AA/55 and complete physical vectors' {
    Require-Device ((Get-DeviceMacro BOOT_APP_RAM_OWNER_BOOT) -eq 0xAA -and
        (Get-DeviceMacro BOOT_APP_RAM_OWNER_APPLICATION) -eq 0x55) 'Device owner contract must be AA/55.'
    $dispatcher=Get-DeviceSpan $bootMemory $bootMap.Symbols['device_dispatch'] 32
    $hex=[BitConverter]::ToString($dispatcher)
    Require-Device ($hex.Contains('40-10-F7-55')) 'Compiled dispatcher owner is stale.'
    foreach($pair in @(@($bootMemory,$bootMap),@($appMemory,$appMap))){
        foreach($handler in @('_Device_TimerIsr','_Device_CanRxIsr')) {
            $address=$pair[1].Symbols[$handler];$size=$pair[1].Sizes[$handler]
            Require-Device ($size -gt 2 -and [BitConverter]::ToUInt16((Get-DeviceSpan $pair[0] ($address+$size-2) 2),0) -eq 0xFC61) 'ISR must end in RETI.'
        }
    }
    for($v=4;$v -lt 126;$v+=2){Require-Device $bootMap.Symbols.ContainsKey(('device_irq_{0:X2}' -f $v)) 'Missing vector.'}
}
Test-Case 'CRC32 golden vector' {
    $data=[Text.Encoding]::ASCII.GetBytes('123456789')
    Require-Device ((Get-DeviceCrc32 $data 9) -eq [Convert]::ToUInt32('CBF43926',16)) 'CRC32 golden vector.'
}

$scratch=Join-Path $testBoot ('tmp\tests\powershell_'+[guid]::NewGuid().ToString('N'))
[IO.Directory]::CreateDirectory($scratch) | Out-Null
try{
    foreach($name in @('app_image_config.h','app_irq_handlers.h','iodefine.h','app_main.c')){
        [IO.File]::Copy((Join-Path $testApp $name),(Join-Path $scratch $name))
    }
    $xml=New-Object Xml.XmlDocument;$xml.Load((Get-DeviceProjectFile $testApp))
    foreach($node in $xml.SelectNodes('//Instance[Type="File"]/RelativePath')){
        if($node.InnerText){$node.InnerText=[IO.Path]::GetFullPath((Join-Path $testApp $node.InnerText))}
    }
    $xml.Save((Join-Path $scratch 'sample.mtpj'))
    Test-Case 'App-local IRQ generation / invalid mappings / Boot isolation' {
        $before=(Get-FileHash -LiteralPath (Join-Path $testBoot 'f24_boot_vectors.asm')).Hash
        $path=Join-Path $scratch 'app_irq_handlers.h'
        $prefix="#ifndef APP_IRQ_HANDLERS_H`n#define APP_IRQ_HANDLERS_H`n#define APP_IRQ_HANDLERS(X) "
        $suffix="`n#endif`n"
        [IO.File]::WriteAllText($path,($prefix+'X(0x0A, Device_TimerIsr)'+$suffix))
        $table=Get-DeviceVectors 'app' $scratch
        Require-Device ($table.Contains('BR !!_Device_TimerIsr ; vector 0A')) 'App-local mapping missing.'
        foreach($invalid in @('X(0x7E, Device_TimerIsr)','X(0x05, Device_TimerIsr)','X(0x2E, not_registered)')){
            [IO.File]::WriteAllText($path,($prefix+$invalid+$suffix))
            Test-Rejected {Get-DeviceVectors 'app' $scratch} 'IRQ'
        }
        [IO.File]::Copy((Join-Path $testApp 'app_irq_handlers.h'),$path,$true)
        Require-Device ($before -ceq (Get-FileHash -LiteralPath (Join-Path $testBoot 'f24_boot_vectors.asm')).Hash) 'App generation changed Boot.'
    }
    Test-Case 'C-header schema / stale compiled metadata' {
        $path=Join-Path $scratch 'app_image_config.h'
        $valid=[IO.File]::ReadAllText($path)
        foreach($pair in @(@('VERSION','0xFFFFFFFFUL'),@('BUILD','"20261016"'),@('BUILD','1.5'),@('BUILD','true'),
            @('BUILD','-1'),@('BUILD','4294967295UL'),@('ROLE','2U'),@('UPDATE_TRANSPORTS','2U'),@('TARGET_ID','0x1234U'))){
            $invalid=[regex]::Replace($valid,'(?m)(#define\s+APP_IMAGE_'+$pair[0]+'\s+)\([^\r\n)]*\)',('${1}'+$pair[1]))
            [IO.File]::WriteAllText($path,$invalid)
            Test-Rejected {Get-DeviceMetadata $scratch 516} 'version|target|build|identity|literal|contract'
        }
        [IO.File]::WriteAllText($path,$valid)
        $info=Get-DeviceIdentity $scratch
        $changed='0x{0:X8}UL' -f ($info.Version -bxor 1)
        $invalid=[regex]::Replace($valid,'(?m)(#define\s+APP_IMAGE_VERSION\s+)\([^\r\n)]*\)',('${1}'+$changed))
        [IO.File]::WriteAllText($path,$invalid)
        Test-Rejected {Get-PackagedDeviceApp $scratch $appMemory $appMap} 'Compiled metadata differs'
    }
    Test-Case 'S-record checksum rejection' {
        $path=Join-Path $scratch 'bad.mot'
        [IO.File]::WriteAllText($path,"S1062000000000D8`nS9030000FC`n")
        Test-Rejected {Read-DeviceSrec $path} 'checksum'
    }
}finally{
    $resolved=[IO.Path]::GetFullPath($scratch)
    $allowed=[IO.Path]::GetFullPath((Join-Path $testBoot 'tmp\tests')).TrimEnd('\')+'\'
    Require-Device ($resolved.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)) 'Unsafe temporary test path.'
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
Write-Host 'PASS: 9 PowerShell artifact/contract checks. Hardware NOT RUN.'
