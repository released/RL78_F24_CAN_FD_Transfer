# Golden device post-package report. Windows PowerShell 5.1 / .NET only.
# Local identical copies accompany independent Boot repos; no Tool dependency.
function ConvertTo-AppReportHtml([object]$Value) {
    return [Net.WebUtility]::HtmlEncode([string]$Value)
}
function Get-AppReportCrc32([byte[]]$Data, [int]$Count) {
    [uint32]$crc=[uint32]::MaxValue
    for($i=0;$i -lt $Count;$i++) {
        $crc=$crc -bxor $Data[$i]
        for($bit=0;$bit -lt 8;$bit++) {
            if($crc -band 1) { $crc=($crc -shr 1) -bxor [uint32]3988292384 }
            else { $crc=$crc -shr 1 }
        }
    }
    return [uint32]($crc -bxor [uint32]::MaxValue)
}
function Get-AppReportVersion([uint32]$Value) {
    return '{0}.{1}.{2}' -f (($Value -shr 16) -band 255),(($Value -shr 8) -band 255),($Value -band 255)
}
function Write-AppBuildReport([string]$Project, [string]$ImagePath) {
    # Called only after the packager's header/map/BR/RETI validation succeeds.
    # Verify the delivered bytes again; never construct values from predictions.
    $data=[IO.File]::ReadAllBytes($ImagePath)
    if($data.Length -lt 516 -or ($data.Length -band 3)) { throw 'Report: invalid packaged image size.' }
    $m=256
    $u32={param([int]$Offset) [BitConverter]::ToUInt32($data,$m+$Offset)}
    $start=& $u32 28
    $identity=Read-AppImageConfig $Project ([BitConverter]::ToUInt16($data,$m+12))
    $mapping=Read-AppIrqConfig $Project
    if((& $u32 0) -ne 0x004E4143 -or $data[$m+4] -ne 6 -or $data[$m+5] -ne 128 -or
        (& $u32 24) -ne $data.Length -or (& $u32 16) -ne $identity.Version -or
        (& $u32 20) -ne $identity.Build -or (& $u32 116) -ne $identity.VersionControl) { throw 'Report: BIN differs from App header/metadata.' }
    for($i=112;$i -lt 256;$i++) {
        if($i -ge 116 -and $i -lt 120) { continue }
        $expected=if($i -lt 128){0}else{255}
        if($data[$m+$i] -ne $expected) { throw 'Report: invalid reserved metadata byte.' }
    }
    $stored=[BitConverter]::ToUInt32($data,$data.Length-4)
    if((Get-AppReportCrc32 $data ($data.Length-4)) -ne $stored) { throw 'Report: BIN CRC32 mismatch.' }
    $sha=[Security.Cryptography.SHA256]::Create()
    try { $hash=([BitConverter]::ToString($sha.ComputeHash($data))).Replace('-','') }
    finally { $sha.Dispose() }
    $registers=@{}
    $header=[IO.File]::ReadAllText((Join-Path $Project 'iodefine.h'))
    foreach($match in [regex]::Matches($header,'(?m)^#define\s+(INT\w+)\s+(0x[0-9A-Fa-f]+)\s*$')) {
        $vector=[Convert]::ToInt32($match.Groups[2].Value.Substring(2),16)
        if(!$registers.ContainsKey($vector)) { $registers[$vector]=@() }
        $registers[$vector]+=$match.Groups[1].Value
    }
    # Offset, width, field, authoring source. Raw bytes remain visible below.
    $fields=@(
        @(0,4,'Magic','Boot contract'), @(4,1,'Image format','Boot contract'),
        @(5,1,'Core length','Boot contract'), @(6,2,'Manifest flags','Boot contract'),
        @(8,2,'Protocol ID','Boot contract'), @(10,1,'Protocol version','Boot contract'),
        @(11,1,'Checksum type','Boot contract'), @(12,2,'Target ID','App header / Boot contract'),
        @(14,1,'CAN mode','Boot contract'), @(15,1,'CAN ID format','Boot contract'),
        @(16,4,'App version','app_image_config.h: APP_IMAGE_VERSION'),
        @(20,4,'App build','app_image_config.h: APP_IMAGE_BUILD'),
        @(24,4,'Actual image bytes','Linker map / post-build'),
        @(28,4,'App start','Boot contract'), @(32,4,'App capacity','Boot contract'),
        @(36,4,'Code Flash bytes','Boot contract'), @(40,4,'Boot reserve bytes','Boot contract'),
        @(44,4,'Erase block bytes','Boot contract'), @(48,4,'CAN nominal rate mask','Boot contract'),
        @(52,4,'CAN data rate mask','Boot contract'), @(56,4,'Request CAN ID','Boot contract'),
        @(60,4,'Response CAN ID','Boot contract'), @(64,16,'MCU part ASCII','Boot contract'),
        @(80,12,'MCU family ASCII','Boot contract'), @(92,4,'Minimum image bytes','Boot contract'),
        @(96,4,'Policy magic','Boot contract'), @(100,1,'Policy version','Boot contract'),
        @(101,1,'Policy length','Boot contract'), @(102,1,'Image role','App header / Boot contract'),
        @(103,1,'Allowed update transports','App header / Boot contract'),
        @(104,4,'IRQ ABI','Boot contract'), @(108,4,'Minimum Boot version','Boot contract'),
        @(112,4,'Device UART baud (CAN: zero)','Boot contract'),
        @(116,4,'Version control:0=any,1=same/newer','app_image_config.h: APP_IMAGE_VERSION_CONTROL'),
        @(120,8,'Policy reserved zero','Boot contract'), @(128,128,'Metadata reserved FF','Boot contract'))
    $metadataRows=New-Object Text.StringBuilder
    $byteRows=New-Object Text.StringBuilder
    foreach($field in $fields) {
        $off=[int]$field[0];$width=[int]$field[1]
        if($off -eq 16 -or $off -eq 108) { $value=Get-AppReportVersion (& $u32 $off) }
        elseif($off -eq 64 -or $off -eq 80) { $value=[Text.Encoding]::ASCII.GetString($data,$m+$off,$width).TrimEnd([char]0) }
        elseif($off -eq 24 -or $off -eq 32 -or $off -eq 36 -or $off -eq 40 -or $off -eq 44 -or $off -eq 92 -or $off -eq 20) { $value=[string](& $u32 $off) }
        elseif($width -eq 4) { $value='0x{0:X8}' -f (& $u32 $off) }
        elseif($width -eq 2) { $value='0x{0:X4}' -f [BitConverter]::ToUInt16($data,$m+$off) }
        elseif($width -eq 1) { $value='0x{0:X2}' -f $data[$m+$off] }
        else { $value=if($off -eq 128){'128 bytes FF'}else{'8 bytes 00'} }
        [void]$metadataRows.Append(('<tr><td>0x{0:X2}..0x{1:X2}</td><td>{2}</td><td>{3}</td><td>{4}</td></tr>' -f
            $off,($off+$width-1),(ConvertTo-AppReportHtml $field[2]),(ConvertTo-AppReportHtml $value),(ConvertTo-AppReportHtml $field[3])))
        for($i=$off;$i -lt $off+$width;$i++) {
            [void]$byteRows.Append(('<tr><td>0x{0:X2}</td><td>0x{1:X3}</td><td>0x{2:X5}</td><td>{3:X2}</td><td>{4} [{5}]</td></tr>' -f
                $i,($m+$i),($start+$m+$i),$data[$m+$i],(ConvertTo-AppReportHtml $field[2]),($i-$off)))
        }
    }
    $irqRows=New-Object Text.StringBuilder
    for($vector=0;$vector -le 126;$vector+=2) {
        $slot=$vector*2
        $raw=([BitConverter]::ToString($data,$slot,4)).Replace('-',' ')
        $target=[uint32]$data[$slot+1] -bor ([uint32]$data[$slot+2] -shl 8) -bor ([uint32]$data[$slot+3] -shl 16)
        $kind=if($vector -eq 0){'RESET'}elseif($vector -eq 2){'RESERVED'}elseif($vector -eq 126){'BRK (not forwarded)'}elseif($mapping.ContainsKey($vector)){'MAPPED'}else{'DEFAULT / keep masked'}
        $name=if($mapping.ContainsKey($vector)){$mapping[$vector]}elseif($vector -eq 0){'_start'}else{'Generated entry'}
        $sources=if($registers.ContainsKey($vector)){$registers[$vector] -join ' / '}else{'See hardware manual'}
        $destination=if($data[$slot] -eq 236){'0x{0:X5}' -f $target}else{'No BR entry'}
        [void]$irqRows.Append(('<tr class="{0}"><td>0x{1:X2}</td><td>{2}</td><td>0x{3:X5}</td><td>{4}</td><td>{5}</td><td>{6}</td><td>{7}</td></tr>' -f
            $(if($kind -eq 'MAPPED'){'mapped'}else{'plain'}),$vector,(ConvertTo-AppReportHtml $sources),($start+$slot),$raw,$destination,(ConvertTo-AppReportHtml $name),$kind))
    }
    $title=ConvertTo-AppReportHtml ([IO.Path]::GetFileName($Project))
    $filename=ConvertTo-AppReportHtml ([IO.Path]::GetFileName($ImagePath))
    $version=Get-AppReportVersion $identity.Version
    $date=[DateTime]::UtcNow.ToString("yyyy-MM-dd HH:mm:ss 'UTC'")
    $part=ConvertTo-AppReportHtml ([Text.Encoding]::ASCII.GetString($data,$m+64,16).TrimEnd([char]0))
    $crcText='0x{0:X8}' -f $stored
    $layout='Boot 0x00000..0x{0:X5}; App vectors 0x{1:X5}..0x{2:X5}; metadata 0x{3:X5}..0x{4:X5}; code from 0x{5:X5}; CRC 0x{6:X5}..0x{7:X5}' -f
        ($start-1),$start,($start+255),($start+256),($start+511),($start+512),($start+$data.Length-4),($start+$data.Length-1)
    $html=@"
<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>$title - App build summary</title>
<style>
:root{color-scheme:light}body{font:16px/1.5 system-ui,sans-serif;color:#19344d;background:#f4f8fc;margin:0 auto;padding:24px;max-width:1400px}
h1,h2{color:#134c7b}a{color:#075f9c}.cards{display:flex;gap:12px;flex-wrap:wrap}.card{background:white;border:1px solid #b1c5d8;border-top:5px solid #257ab7;border-radius:7px;padding:14px;flex:1;min-width:210px}
.ok{border-color:#198359;background:#e6faef}.note{background:#fff5d6;border-left:5px solid #b47800;padding:12px}
table{width:100%;border-collapse:collapse;background:white;font-size:14px}th,td{text-align:left;border:1px solid #bdcddd;padding:6px 9px;vertical-align:top}th{background:#deedf8}tr:nth-child(even){background:#f4f8fc}.mapped{color:#075e39;font-weight:600}
.scroll{overflow-x:auto}code{overflow-wrap:anywhere}summary{cursor:pointer;font-weight:600;padding:12px 0}@media print{body{padding:0;font-size:11px}th{display:table-cell}tr{break-inside:avoid}h2{break-after:avoid}details{display:block}.scroll{overflow:visible}}
</style></head><body>
<h1>$title</h1><p>App build summary &middot; $date &middot; <a href="#metadata">Metadata</a> &middot; <a href="#irq">Vector table</a> &middot; <a href="#bytes">All metadata bytes</a></p>
<div class="cards"><section class="card"><h2>Packaged image</h2><strong>$part &middot; $version</strong><p>Build $($identity.Build)<br>$filename<br>$($data.Length) bytes</p></section>
<section class="card ok"><h2>CRC32 MATCH</h2><strong>$crcText</strong><p>Recomputed over BIN[0..size-5].<br>Stored in final four bytes (LE32).</p></section>
<section class="card"><h2>IRQ configuration</h2><strong>$($mapping.Count) mapped normal vectors</strong><p>64 fixed slots, 4 bytes each.<br>Full decoded table below.</p></section></div>
<p><strong>BIN SHA-256</strong><br><code>$hash</code></p><p>$layout</p>
<p class="note">This is a summary of this successfully packaged artifact, not live MCU data or a hardware test. After a failed build, an earlier report/BIN may remain: check the build result, date and SHA-256. CRC is integrity, not authentication.</p>
<p>Editable inputs: <code>app_image_config.h</code> and <code>app_irq_handlers.h</code>. Generated INC/identity/table files are build outputs.
<a href="https://released.github.io/slide_boot_loader_for_RL78/#device-metadata-vector-workshop">Field, IRQ and build guide</a>.</p>
<h2 id="metadata">Metadata field summary</h2><p>Offsets are relative to metadata at BIN+0x100. Multi-byte values use little-endian order.</p>
<div class="scroll"><table><thead><tr><th>Metadata offsets</th><th>Field</th><th>Decoded value</th><th>Source</th></tr></thead><tbody>$metadataRows</tbody></table></div>
<h2 id="irq">App vector table</h2><p>Physical vector byte address &rarr; App slot = App start + vector &times; 2. Interrupt names come from this project's iodefine.h; aliases do not select hardware modes. The packager validates linked BR entries against the map. Peripheral initialization and physical IRQ behavior need separate hardware checks.</p>
<div class="scroll"><table><thead><tr><th>Physical vector</th><th>iodefine.h names</th><th>App slot</th><th>BIN bytes</th><th>BR destination</th><th>Configured C handler</th><th>Entry role</th></tr></thead><tbody>$irqRows</tbody></table></div>
<h2 id="bytes">Metadata byte addresses</h2><details><summary>Expand all 256 metadata bytes</summary>
<div class="scroll"><table><thead><tr><th>Metadata offset</th><th>BIN offset</th><th>Flash address</th><th>Hex byte</th><th>Field [byte index]</th></tr></thead><tbody>$byteRows</tbody></table></div></details>
<p>This compile/package report is separate from the Tool's per-update operation reports (result, timings, retries and diagnostics).</p>
</body></html>
"@
    $output=Join-Path ([IO.Path]::GetDirectoryName($ImagePath)) 'app_build_report.html'
    [IO.File]::WriteAllText($output,$html,(New-Object Text.UTF8Encoding($false)))
    Write-Host ('Generated App build summary: '+$output)
}
