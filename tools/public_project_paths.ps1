# Only generator bookkeeping and the iodefine command comment are normalized.
# Compiler inputs, SFR declarations, licenses and tool installation paths stay intact.
function Convert-DeviceIodefineComment([string]$Text) {
    $start=$Text.IndexOf('/* Options    :',[StringComparison]::Ordinal)
    $end=if($start -ge 0){$Text.IndexOf('/* Date       :',$start,[StringComparison]::Ordinal)}else{-1}
    if($start -lt 0 -or $end -le $start){throw 'Unrecognized iodefine generator comment; no automatic edit.'}
    $comment=$Text.Substring($start,$end-$start)
    if($comment -notmatch '\b[A-Za-z]:[\\/]|\\\\'){return $Text}
    $newline=if($Text.Contains("`r`n")){"`r`n"}else{"`n"}
    $portable=@(
        '/* Options    : -df=<CS+ device directory>/RL78/Devicefile/DR7F124FPJ.DVF     */',
        '/*              -o=./iodefine.h -f                                          */',
        '/* Paths      : Build-machine locations are omitted from this comment.     */') -join $newline
    return $Text.Substring(0,$start)+$portable+$newline+$Text.Substring($end)
}

function Set-DevicePublicProjectPaths([xml]$Xml) {
    foreach($node in $Xml.SelectNodes('//*[starts-with(name(),"ProjectGeneratr-GeneratedFile")]')) {
        if($node.InnerText){$node.InnerText=[IO.Path]::GetFileName($node.InnerText)}
    }
    foreach($node in $Xml.SelectNodes('//GenerateFileFolder')){$node.InnerText='.'}
}

function Assert-DevicePublicProjectPaths([xml]$Xml) {
    foreach($node in $Xml.SelectNodes('//*[starts-with(name(),"ProjectGeneratr-GeneratedFile")]')) {
        if($node.InnerText -and $node.InnerText -cne [IO.Path]::GetFileName($node.InnerText)) {
            throw 'Generator file bookkeeping must use a project-local file name.'
        }
    }
    foreach($node in $Xml.SelectNodes('//GenerateFileFolder')) {
        if($node.InnerText -cne '.'){throw 'Generator output folder must be project-relative.'}
    }
}
