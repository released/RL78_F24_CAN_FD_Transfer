. (Join-Path $PSScriptRoot 'device_hal_paths.ps1')
# Shared F24 CAN-device build contract. Windows PowerShell 5.1 / .NET only.
. (Join-Path $PSScriptRoot 'app_config.ps1')
$script:DeviceBoot = Split-Path -Parent $PSScriptRoot
$script:DeviceConfig = (@('image_metadata_config.h','can_boot_config.h','boot_app_handoff.h') |
    ForEach-Object { [IO.File]::ReadAllText((Get-DeviceProjectSource $script:DeviceBoot $_)) }) -join "`n"
$script:DeviceConfig = [regex]::Replace($script:DeviceConfig, '(?s)/\*.*?\*/|//[^\r\n]*', '')
function Require-Device($Condition, [string]$Message) { if (!$Condition) { throw $Message } }
function Get-DeviceMacro([string]$Name) {
    $definitions=[regex]::Matches($script:DeviceConfig,'(?m)^#define\s+'+[regex]::Escape($Name)+'\s+\((0x[0-9A-Fa-f]+|[0-9]+)[UL]*\)[ \t]*\r?$')
    Require-Device ($definitions.Count -eq 1) ('Missing or duplicate numeric contract '+$Name)
    $m=$definitions[0]
    if($m.Groups[1].Value.StartsWith('0x')){return [Convert]::ToUInt32($m.Groups[1].Value.Substring(2),16)}
    return [uint32]$m.Groups[1].Value
}
function Get-DeviceIdentity([string]$Project) {
    return Read-AppImageConfig $Project (Get-DeviceMacro IMAGE_TARGET_ID)
}
function Get-DeviceMetadata([string]$Project, [uint32]$ImageSize) {
    $info=Get-DeviceIdentity $Project
    $words=New-Object 'Collections.Generic.List[uint32]'
    $words.Add((Get-DeviceMacro IMAGE_MANIFEST_MAGIC))
    $words.Add(((Get-DeviceMacro IMAGE_FORMAT_VERSION) -bor ((Get-DeviceMacro IMAGE_MANIFEST_SIZE) -shl 8) -bor ((Get-DeviceMacro IMAGE_MANIFEST_FLAGS) -shl 16)))
    $words.Add(((Get-DeviceMacro CAN_BOOT_PROTOCOL_ID) -bor ((Get-DeviceMacro CAN_BOOT_PROTOCOL_VERSION) -shl 16) -bor ((Get-DeviceMacro IMAGE_CHECKSUM_CRC32) -shl 24)))
    $words.Add(((Get-DeviceMacro IMAGE_TARGET_ID) -bor ((Get-DeviceMacro CAN_BOOT_CAN_MODE_MASK) -shl 16) -bor ((Get-DeviceMacro CAN_BOOT_CAN_ID_STANDARD) -shl 24)))
    $words.Add($info.Version); $words.Add($info.Build); $words.Add($ImageSize)
    foreach($name in @('APPLICATION_START','APPLICATION_CAPACITY','CODE_FLASH_SIZE','RESERVED_SIZE','ERASE_BLOCK_SIZE')){
        $words.Add((Get-DeviceMacro ('IMAGE_'+$name)))}
    foreach($name in @('NOMINAL_BITRATE_MASK','DATA_BITRATE_MASK','REQUEST_ID','RESPONSE_ID')){
        $words.Add((Get-DeviceMacro ('CAN_BOOT_'+$name)))}
    foreach($i in 0..3){$words.Add((Get-DeviceMacro ('IMAGE_MCU_PART_'+$i)))}
    foreach($i in 0..2){$words.Add((Get-DeviceMacro ('IMAGE_MCU_FAMILY_'+$i)))}
    $words.Add((Get-DeviceMacro IMAGE_MINIMUM_IMAGE_SIZE)); $words.Add(0x31445055)
    foreach($name in @('POLICY_WORD','IRQ_ABI','BOOT_VERSION')){$words.Add((Get-DeviceMacro ('IMAGE_'+$name)))}
    $words.Add(0); $words.Add($info.VersionControl); $words.Add(0); $words.Add(0)
    Require-Device ($words.Count -eq 32) 'Metadata core layout.'
    $data=New-Object byte[] 256
    for($i=0;$i -lt 32;$i++){[BitConverter]::GetBytes($words[$i]).CopyTo($data,$i*4)}
    for($i=128;$i -lt 256;$i++){$data[$i]=255}
    return ,$data
}
function Get-DeviceMetadataAssembly([string]$Project) {
    $data=Get-DeviceMetadata $Project ([uint32]::MaxValue)
    $text='; Generated from app_image_config.h and Boot contract. Do not edit.'+"`n"
    $text+=('.appmeta .CSEG AT 0x{0:X5}' -f ((Get-DeviceMacro IMAGE_APPLICATION_START)+(Get-DeviceMacro IMAGE_MANIFEST_OFFSET)))+"`n"
    for($i=0;$i -lt 256;$i+=4){$text+=('    .DB4 0x{0:X8}' -f [BitConverter]::ToUInt32($data,$i))+"`n"}
    return $text
}
function Get-DeviceProjectFile([string]$Project) {
    $files=@(Get-ChildItem -LiteralPath $Project -Filter '*.mtpj' -File)
    Require-Device ($files.Count -eq 1) 'Exactly one MTPJ is required.'
    return $files[0].FullName
}
function Get-DeviceRelative([string]$Path, [string]$Root) {
    $full=[IO.Path]::GetFullPath($Path); $rootFull=[IO.Path]::GetFullPath($Root)
    Require-Device ([IO.Path]::GetPathRoot($full) -eq [IO.Path]::GetPathRoot($rootFull)) 'Boot and App must use the same volume.'
    if($full.TrimEnd('\') -eq $rootFull.TrimEnd('\')){return '.'}
    $base=New-Object Uri ($rootFull.TrimEnd('\')+'\')
    return [Uri]::UnescapeDataString($base.MakeRelativeUri((New-Object Uri $full)).ToString()).Replace('/','\')
}
function Write-DeviceGenerated([string]$Path, [string]$Text, [switch]$CheckOnly) {
    $value=$Text.Replace("`r`n","`n").Replace("`n","`r`n")
    $old=if(Test-Path -LiteralPath $Path){[IO.File]::ReadAllText($Path)}else{$null}
    if($CheckOnly){Require-Device ($old -ceq $value) ('Stale generated file: '+$Path)}
    elseif($old -cne $value){
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path)) | Out-Null
        [IO.File]::WriteAllText($Path,$value,(New-Object Text.UTF8Encoding($false)))
    }
}
function Get-DeviceSpan($Memory, [long]$Address, [int]$Size) {
    $data=New-Object byte[] $Size
    for($i=0;$i -lt $Size;$i++){
        if($Memory.ContainsKey([long]($Address+$i))){$data[$i]=$Memory[[long]($Address+$i)]}else{$data[$i]=255}
    }
    return ,$data
}
function Get-DeviceCrc32([byte[]]$Data, [int]$Count) {
    [uint64]$crc=4294967295
    for($i=0;$i -lt $Count;$i++){
        $crc=$crc -bxor $Data[$i]
        for($bit=0;$bit -lt 8;$bit++){
            if($crc -band 1){$crc=($crc -shr 1) -bxor 3988292384}else{$crc=$crc -shr 1}
        }
    }
    return [uint32](($crc -bxor 4294967295) -band 4294967295)
}
function Read-DeviceSrec([string]$Path) {
    $memory=New-Object 'Collections.Generic.Dictionary[long,byte]'
    $ended=$false
    foreach($line in [IO.File]::ReadAllLines($Path)){
        if(!$line.Trim()){continue}
        Require-Device (!$ended -and $line -cmatch '^S[012356789][0-9A-Fa-f]+$' -and ($line.Length%2 -eq 0)) 'Bad S-record type/order.'
        $bytes=New-Object byte[] (($line.Length-2)/2);$sum=0
        for($i=0;$i -lt $bytes.Length;$i++){$bytes[$i]=[Convert]::ToByte($line.Substring(2+$i*2,2),16);$sum+=$bytes[$i]}
        Require-Device ($bytes[0] -eq $bytes.Length-1 -and ($sum -band 255) -eq 255) 'Bad S-record checksum/length.'
        $n=@{'0'=2;'1'=2;'2'=3;'3'=4;'5'=2;'6'=3;'7'=4;'8'=3;'9'=2}[[string]$line[1]]
        Require-Device ($bytes.Length -ge $n+2) 'Short S-record.'
        [long]$address=0
        for($i=1;$i -le $n;$i++){$address=($address -shl 8) -bor $bytes[$i]}
        if('123'.Contains([string]$line[1])){
            for($i=$n+1;$i -lt $bytes.Length-1;$i++){
                $at=$address+$i-$n-1
                Require-Device (!$memory.ContainsKey($at) -or $memory[$at] -eq $bytes[$i]) 'Conflicting S-record overlap.'
                $memory[$at]=$bytes[$i]
            }
        }elseif('789'.Contains([string]$line[1])){
            Require-Device ($bytes.Length -eq $n+2) 'Invalid end record.';$ended=$true
        }
    }
    Require-Device ($ended -and $memory.Count -gt 0) 'Missing S-record end/data.'
    return ,$memory
}
function Read-DeviceMap([string]$Path) {
    $text=[IO.File]::ReadAllText($Path).Replace("`r`n","`n")
    $part=[regex]::Match($text,'(?s)\*\*\* Mapping List \*\*\*(.*?)\*\*\* Total Section Size \*\*\*')
    $symbolsPart=[regex]::Match($text,'(?s)\*\*\* Symbol List \*\*\*(.*?)\*\*\* Variable Vector Table List \*\*\*')
    Require-Device ($part.Success -and $symbolsPart.Success) 'Missing linker map lists.'
    $sections=@{};$symbols=@{};$sizes=@{}
    foreach($m in [regex]::Matches($part.Groups[1].Value,'(?m)^(\S+)\s*\n +([0-9a-f]{8}) +([0-9a-f]{8}) +([0-9a-f]+) +\d+')){
        $extent=@();foreach($i in 2..4){$extent+= [Convert]::ToInt64($m.Groups[$i].Value,16)}
        Require-Device ($extent[2] -eq 0 -or $extent[1]-$extent[0]+1 -eq $extent[2]) ('Invalid map extent '+$m.Groups[1].Value)
        $sections[$m.Groups[1].Value]=$extent
    }
    foreach($m in [regex]::Matches($symbolsPart.Groups[1].Value,'(?m)^  (\S+)\s*\n +([0-9a-f]{8}) +([0-9a-f]+) +(?:func|none|data|entry)\b')){
        $symbols[$m.Groups[1].Value]=[Convert]::ToInt64($m.Groups[2].Value,16)
        $sizes[$m.Groups[1].Value]=[Convert]::ToInt64($m.Groups[3].Value,16)
    }
    Require-Device ($sections.Count -and $symbols.Count) 'Missing linker map sections/symbols.'
    return @{Sections=$sections;Symbols=$symbols;Sizes=$sizes;Text=$text}
}
function Get-DeviceIntelRecord([int]$Kind,[int]$Address,[byte[]]$Data) {
    $bytes=@([int]$Data.Length,($Address -shr 8),($Address -band 255),$Kind)+@($Data)
    $sum=0;$text=':'
    foreach($value in $bytes){$text+=('{0:X2}' -f $value);$sum+=$value}
    return $text+('{0:X2}' -f ((-$sum) -band 255))+"`n"
}
function ConvertTo-DeviceIhex($Memory) {
    $addresses=@($Memory.Keys | Sort-Object);$index=0;$upper=-1
    $out=New-Object Text.StringBuilder
    while($index -lt $addresses.Count){
        $at=[long]$addresses[$index]
        if(($at -shr 16) -ne $upper){
            $upper=$at -shr 16
            [void]$out.Append((Get-DeviceIntelRecord 4 0 ([byte[]]@(($upper -shr 8),($upper -band 255)))))
        }
        $data=New-Object 'Collections.Generic.List[byte]'
        while($index -lt $addresses.Count -and $addresses[$index] -eq $at+$data.Count -and $data.Count -lt 16 -and
            ($addresses[$index] -shr 16) -eq $upper){$data.Add($Memory[$addresses[$index]]);$index++}
        [void]$out.Append((Get-DeviceIntelRecord 0 ($at -band 65535) $data.ToArray()))
    }
    [void]$out.Append((Get-DeviceIntelRecord 1 0 ([byte[]]@())))
    return $out.ToString()
}
