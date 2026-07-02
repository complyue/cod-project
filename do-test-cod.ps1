#requires -Version 5.1

[CmdletBinding()]
param(
    [ValidateSet("build", "built")]
    [string]$Toolchain = $(if ($env:COD_TEST_TOOLCHAIN) { $env:COD_TEST_TOOLCHAIN } else { "build" }),
    [string[]]$Components = @("shilos", "yaml-cmp", "yaml-pretty", "yaml-ux", "yaml-exc-trace", "codp", "cod")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

function Resolve-Tool {
    param(
        [Parameter(Mandatory)] [string]$Name,
        [string[]]$Candidates = @()
    )

    $command = Get-Command -Name $Name -CommandType Application -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "Required tool '$Name' was not found. Install it or add it to PATH."
}

function Add-PathDirectory {
    param([Parameter(Mandatory)] [string]$Path)

    $directory = if (Test-Path -LiteralPath $Path -PathType Leaf) {
        Split-Path -Parent $Path
    } else {
        $Path
    }

    if (-not (Test-Path -LiteralPath $directory -PathType Container)) {
        return
    }

    $resolved = (Resolve-Path -LiteralPath $directory).Path
    $parts = $env:Path -split ';' | Where-Object { $_ }
    if ($parts -notcontains $resolved) {
        $env:Path = "$resolved;$env:Path"
    }
}

function Import-VsDevEnvironment {
    if ($env:VSCMD_VER -and (Get-Command -Name cl.exe -CommandType Application -ErrorAction SilentlyContinue)) {
        Write-Host "Using existing Visual Studio developer environment $env:VSCMD_VER"
        return
    }

    $vswhere = Resolve-Tool "vswhere.exe" @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe",
        "$env:ProgramFiles\Microsoft Visual Studio\Installer\vswhere.exe"
    )

    $installPath = & $vswhere -latest -products "*" `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $installPath) {
        $installPath = & $vswhere -latest -products "*" -property installationPath
    }

    $vsDevCmdCandidates = @()
    if ($installPath) {
        $vsDevCmdCandidates += (Join-Path $installPath "Common7\Tools\VsDevCmd.bat")
    }
    $vsDevCmdCandidates += @(
        "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
    )

    $vsDevCmd = $vsDevCmdCandidates |
        Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Leaf) } |
        Select-Object -First 1
    if (-not $vsDevCmd) {
        throw "Visual Studio Build Tools with MSVC x64 tools were not found."
    }

    Write-Host "Importing Visual Studio x64 developer environment from $vsDevCmd"
    $setOutput = & cmd.exe /s /c "`"$vsDevCmd`" -arch=x64 -host_arch=x64 >nul && set"
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to import the Visual Studio developer environment."
    }

    foreach ($line in $setOutput) {
        if ($line -match '^([^=]+)=(.*)$') {
            [Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process")
        }
    }
}

function Convert-ToBashPath {
    param([Parameter(Mandatory)] [string]$Path)
    return $Path.Replace('\', '/')
}

function Quote-Bash {
    param([Parameter(Mandatory)] [string]$Value)
    return "'" + $Value.Replace("'", "'\''") + "'"
}

$SourceDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -LiteralPath $SourceDir
$Components = @(
    $Components |
        ForEach-Object { $_ -split "," } |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_ }
)

$CMake = Resolve-Tool "cmake.exe" @(
    "$env:ProgramFiles\CMake\bin\cmake.exe",
    "${env:ProgramFiles(x86)}\CMake\bin\cmake.exe"
)
$Ninja = Resolve-Tool "ninja.exe" @(
    "$env:LOCALAPPDATA\Microsoft\WinGet\Links\ninja.exe",
    "$env:ProgramFiles\Ninja\ninja.exe"
)
$Bash = Resolve-Tool "bash.exe" @(
    "$env:ProgramFiles\Git\bin\bash.exe",
    "$env:ProgramFiles\Git\usr\bin\bash.exe",
    "C:\msys64\usr\bin\bash.exe"
)

Add-PathDirectory $CMake
Add-PathDirectory $Ninja
Add-PathDirectory (Join-Path $SourceDir (Join-Path $Toolchain "bin"))
Add-PathDirectory $Bash
Import-VsDevEnvironment

$env:COD_TEST_TOOLCHAIN = $Toolchain

Write-Host "CMake:     $CMake"
Write-Host "Ninja:     $Ninja"
Write-Host "Bash:      $Bash"
Write-Host "Toolchain: $Toolchain"

$bashSourceDir = Convert-ToBashPath $SourceDir
foreach ($component in $Components) {
    $script = Join-Path $SourceDir (Join-Path "tests\$component" "run-all.sh")
    if (-not (Test-Path -LiteralPath $script -PathType Leaf)) {
        throw "Unknown or unsupported test component '$component': $script"
    }

    Write-Host ""
    Write-Host "==> Running $component tests"
    $command = "cd $(Quote-Bash $bashSourceDir) && export COD_TEST_TOOLCHAIN=$(Quote-Bash $Toolchain) && ./tests/$component/run-all.sh"
    & $Bash "-lc" $command
    if ($LASTEXITCODE -ne 0) {
        throw "Tests failed for component '$component' with exit code $LASTEXITCODE."
    }
}

Write-Host ""
Write-Host "==> All requested tests passed"
