param([switch]$SkipExamples)

$NDKPath = (Get-Content $PSScriptRoot/ndkpath.txt).Trim()
Write-Output "NDK located at: $NDKPath"

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core") -or $IsWindows) {
    $buildScript += ".cmd"
}

Write-Output "[BUILD] Starting NDK..."
& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk NDK_DEBUG=0 -j12
$result = $LASTEXITCODE
Write-Output "[BUILD] Done!"

if (-not $result -and -not $SkipExamples) {
    Write-Output "[BUILD] Compiling examples..."
    & $PSScriptRoot/examples/build.ps1
    $result = $LASTEXITCODE
    Write-Output "[BUILD] Examples done!"
}

Exit $result
