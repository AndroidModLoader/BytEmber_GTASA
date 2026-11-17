param([string]$VCVars = 'D:/MicrosoftVSCommunity2022/VC/Auxiliary/Build/vcvars64.bat')

$ErrorActionPreference = 'Stop'
$repoPath = Split-Path $PSScriptRoot -Parent
$outputPath = Join-Path $repoPath 'obj/example-compiler'
New-Item -ItemType Directory -Force $outputPath | Out-Null
$sources = @('registry', 'runtime', 'value', 'compiler', 'bytecode', 'vm', 'native', 'main') |
    ForEach-Object { '"' + "$repoPath/BytEmber/src/$_.cpp" + '"' }
$sources += Get-ChildItem "$repoPath/BytEmber/src/std/*.cpp" |
    ForEach-Object { '"' + $_.FullName + '"' }
$commandPath = Join-Path $outputPath 'compile.cmd'
@"
@echo off
call "$VCVars" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /std:c++14 /O2 /DNOMINMAX /I"$repoPath/BytEmber/include" $($sources -join ' ') /Fe"$outputPath/emberc.exe" /Fo"$outputPath/" /link /INCREMENTAL:NO
"@ | Set-Content -LiteralPath $commandPath
Push-Location $outputPath
try
{
    & $commandPath
    if($LASTEXITCODE) { throw "Example compiler build failed: $LASTEXITCODE" }
    foreach($source in (Get-ChildItem $PSScriptRoot -Recurse -File -Filter '*.es'))
    {
        $bytecode = [IO.Path]::ChangeExtension($source.FullName, '.ebc')
        Write-Output $source.Name
        & "$outputPath/emberc.exe" compile $source.FullName $bytecode arm32,arm64
        if($LASTEXITCODE) { throw "Example compilation failed: $($source.FullName)" }
    }
}
finally { Pop-Location }
