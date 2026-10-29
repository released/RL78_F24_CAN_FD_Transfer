param([string[]]$Project=@())
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '..\public_project_paths.ps1')
function Assert-Test($Value,[string]$Message){if(!$Value){throw $Message}}
$header="/* License retained. */`r`n/* Options    : -df=C:\SDK\Device */`r`n/*              -o=D:\private\project\iodefine.h */`r`n/*              -f */`r`n/* Date       : retained */`r`n#define REG (*(volatile unsigned char *)0xFFF00)`r`n"
$normalized=Convert-DeviceIodefineComment $header
Assert-Test ($normalized -notmatch '\b[A-Za-z]:[\\/]') 'Drive path remains in generator comment.'
Assert-Test ($normalized.StartsWith('/* License retained. */')) 'License changed.'
Assert-Test (($header -replace '(?s)/\*.*?\*/','') -ceq ($normalized -replace '(?s)/\*.*?\*/','')) 'C declarations or line count changed.'
Assert-Test ((Convert-DeviceIodefineComment $normalized) -ceq $normalized) 'Comment normalization is not idempotent.'
$stopped=$false
try{[void](Convert-DeviceIodefineComment '#define REG 1')}catch{$stopped=$true}
Assert-Test $stopped 'Unknown header layout must not be guessed.'
[xml]$xml='<Project><ProjectGeneratr-GeneratedFile0>D:\private\project\main.c</ProjectGeneratr-GeneratedFile0><GenerateFileFolder>D:\private\project</GenerateFileFolder><COption>C:\SDK\include</COption><RelativePath>..\common\device_can.c</RelativePath></Project>'
$stopped=$false
try{Assert-DevicePublicProjectPaths $xml}catch{$stopped=$true}
Assert-Test $stopped 'Check-only accepts absolute generator bookkeeping.'
Set-DevicePublicProjectPaths $xml
Assert-DevicePublicProjectPaths $xml
Assert-Test ($xml.Project.'ProjectGeneratr-GeneratedFile0' -ceq 'main.c') 'Wrong generator file.'
Assert-Test ($xml.Project.GenerateFileFolder -ceq '.') 'Wrong generator folder.'
Assert-Test ($xml.Project.COption -ceq 'C:\SDK\include' -and $xml.Project.RelativePath -ceq '..\common\device_can.c') 'Compiler inputs or SDK path changed.'
$snapshot=$xml.OuterXml
Set-DevicePublicProjectPaths $xml
Assert-Test ($xml.OuterXml -ceq $snapshot) 'Project normalization is not idempotent.'
foreach($folder in $Project) {
    $mtpj=@(Get-ChildItem -LiteralPath $folder -Filter '*.mtpj' -File)
    Assert-Test ($mtpj.Count -eq 1) 'Expected one selected project.'
    [xml]$actual=[IO.File]::ReadAllText($mtpj[0].FullName)
    Assert-DevicePublicProjectPaths $actual
    $source=[IO.File]::ReadAllText((Join-Path $folder 'iodefine.h'))
    Assert-Test ((Convert-DeviceIodefineComment $source) -ceq $source) 'Selected header still contains generator drive paths.'
}
Write-Host 'PASS: generator path/comment normalization, semantic preservation, idempotence and selected-project checks.'
