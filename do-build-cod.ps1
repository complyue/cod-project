#requires -Version 5.1

[CmdletBinding()]
param(
    [string]$LLVMBranch = "release/18.x",
    [string]$TargetsToBuild = $(if ($env:COD_TARGETS_TO_BUILD) { $env:COD_TARGETS_TO_BUILD } else { "Native" }),
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$BuildType = $(if ($env:COD_BUILD_TYPE) { $env:COD_BUILD_TYPE } else { "Release" }),
    [int]$Jobs = [Math]::Max(1, [Environment]::ProcessorCount - 2),
    [switch]$NoPull,
    [switch]$EnableLLDB,
    [switch]$VerifyStage3SelfHost,
    [switch]$KeepBootstrap
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

function Write-Step {
    param([Parameter(Mandatory)] [string]$Message)
    Write-Host ""
    Write-Host "==> $Message"
}

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

function Invoke-Tool {
    param(
        [Parameter(Mandatory)] [string]$FilePath,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = (Get-Location).Path
    )

    Write-Host "[$WorkingDirectory]> $FilePath $($Arguments -join ' ')"
    Push-Location -LiteralPath $WorkingDirectory
    try {
        & $FilePath @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code ${LASTEXITCODE}: $FilePath $($Arguments -join ' ')"
        }
    } finally {
        Pop-Location
    }
}

function Invoke-ToolExitCode {
    param(
        [Parameter(Mandatory)] [string]$FilePath,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = (Get-Location).Path
    )

    Push-Location -LiteralPath $WorkingDirectory
    try {
        & $FilePath @Arguments *> $null
        return $LASTEXITCODE
    } finally {
        Pop-Location
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

    if (-not (Get-Command -Name cl.exe -CommandType Application -ErrorAction SilentlyContinue)) {
        throw "Visual Studio environment was imported, but cl.exe is still not available."
    }
}

function Assert-PlainDirectoryIfPresent {
    param([Parameter(Mandatory)] [string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }

    $item = Get-Item -LiteralPath $Path -Force
    if (($item.Attributes -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
        throw "'$Path' must be a real directory, not a symlink or junction."
    }
}

function Ensure-Directory {
    param([Parameter(Mandatory)] [string]$Path)
    New-Item -ItemType Directory -Path $Path -Force | Out-Null
}

function Assert-InSourceTree {
    param([Parameter(Mandatory)] [string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }

    $source = (Resolve-Path -LiteralPath $script:SourceDir).Path.TrimEnd('\')
    $resolved = (Resolve-Path -LiteralPath $Path).Path.TrimEnd('\')
    if ($resolved -eq $source -or
        -not $resolved.StartsWith($source + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to modify path outside the source tree: $resolved"
    }
}

function Remove-GeneratedTree {
    param([Parameter(Mandatory)] [string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }

    Assert-InSourceTree $Path
    Remove-Item -LiteralPath $Path -Recurse -Force
}

function Get-CMakeCacheValue {
    param(
        [Parameter(Mandatory)] [string]$CachePath,
        [Parameter(Mandatory)] [string]$Name
    )

    if (-not (Test-Path -LiteralPath $CachePath -PathType Leaf)) {
        return $null
    }

    $escaped = [Regex]::Escape($Name)
    foreach ($line in Get-Content -LiteralPath $CachePath) {
        if ($line -match "^$escaped(?::[^=]*)?=(.*)$") {
            return $Matches[1]
        }
    }

    return $null
}

function Normalize-PathText {
    param([AllowNull()] [string]$Path)

    if (-not $Path) {
        return ""
    }

    try {
        return (Resolve-Path -LiteralPath $Path).Path.TrimEnd('\').ToLowerInvariant()
    } catch {
        return $Path.Replace('/', '\').TrimEnd('\').ToLowerInvariant()
    }
}

function New-StringFromCodePoints {
    param([Parameter(Mandatory)] [int[]]$CodePoints)

    $builder = [System.Text.StringBuilder]::new()
    foreach ($codePoint in $CodePoints) {
        [void]$builder.Append([char]$codePoint)
    }
    return $builder.ToString()
}

function Write-Utf8NoBomFile {
    param(
        [Parameter(Mandatory)] [string]$Path,
        [Parameter(Mandatory)] [string]$Content
    )

    $encoding = [System.Text.UTF8Encoding]::new($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Repair-MsvcDepsPrefix {
    param([Parameter(Mandatory)] [string]$BuildDir)

    $cmakeFilesDir = Join-Path $BuildDir "CMakeFiles"
    if (-not (Test-Path -LiteralPath $cmakeFilesDir -PathType Container)) {
        return
    }

    $mojibakePrefix = New-StringFromCodePoints @(
        0x5A09, 0x3126, 0x5270, 0x003A, 0x0020,
        0x9356, 0x546D, 0x60C8, 0x93C2, 0x56E6, 0x6B22,
        0x003A, 0x0020, 0x0020
    )
    $correctPrefix = New-StringFromCodePoints @(
        0x6CE8, 0x610F, 0x003A, 0x0020,
        0x5305, 0x542B, 0x6587, 0x4EF6,
        0x003A, 0x0020, 0x0020
    )

    $files = @()
    $rulesPath = Join-Path $cmakeFilesDir "rules.ninja"
    if (Test-Path -LiteralPath $rulesPath -PathType Leaf) {
        $files += Get-Item -LiteralPath $rulesPath
    }
    $files += Get-ChildItem -LiteralPath $cmakeFilesDir -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -in @("CMakeCCompiler.cmake", "CMakeCXXCompiler.cmake") }

    $changed = 0
    foreach ($file in $files) {
        $content = [System.IO.File]::ReadAllText($file.FullName)
        $updated = $content.Replace($mojibakePrefix, $correctPrefix)

        if ($file.Name -eq "rules.ninja") {
            $updated = [Regex]::Replace(
                $updated,
                "(?m)^(msvc_deps_prefix\s*=\s*).*$",
                { param($match) $match.Groups[1].Value + $correctPrefix }
            )
        } elseif ($file.Name -eq "CMakeCCompiler.cmake") {
            $updated = [Regex]::Replace(
                $updated,
                'set\(CMAKE_C_CL_SHOWINCLUDES_PREFIX ".*"\)',
                "set(CMAKE_C_CL_SHOWINCLUDES_PREFIX `"$correctPrefix`")"
            )
        } elseif ($file.Name -eq "CMakeCXXCompiler.cmake") {
            $updated = [Regex]::Replace(
                $updated,
                'set\(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX ".*"\)',
                "set(CMAKE_CXX_CL_SHOWINCLUDES_PREFIX `"$correctPrefix`")"
            )
        }

        if ($updated -ne $content) {
            Write-Utf8NoBomFile $file.FullName $updated
            $changed += 1
        }
    }

    if ($changed -gt 0) {
        Write-Host "Repaired MSVC showIncludes prefix in $changed generated file(s)."
    }
}

function Repair-ClangClResourceRules {
    param([Parameter(Mandatory)] [string]$BuildDir)

    $rulesPath = Join-Path $BuildDir "CMakeFiles\rules.ninja"
    if (-not (Test-Path -LiteralPath $rulesPath -PathType Leaf)) {
        return
    }

    $content = [System.IO.File]::ReadAllText($rulesPath)
    $updated = $content.Replace(
        '$FLAGS -clang:-MD -clang:-MF -clang:$DEP_FILE /fo',
        '$FLAGS /fo'
    )
    $updated = $updated.Replace(
        ' -E -- $in ++',
        ' /EP -- $in ++'
    )

    if ($updated -ne $content) {
        Write-Utf8NoBomFile $rulesPath $updated
        Write-Host "Repaired clang-cl resource compiler dep flags in generated Ninja rules."
    }
}

function Assert-CMakeCompiler {
    param(
        [Parameter(Mandatory)] [string]$BuildDir,
        [Parameter(Mandatory)] [string]$ExpectedCompiler
    )

    $cache = Join-Path $BuildDir "CMakeCache.txt"
    if (-not (Test-Path -LiteralPath $cache -PathType Leaf)) {
        return
    }

    $actual = Get-CMakeCacheValue $cache "CMAKE_CXX_COMPILER"
    if (-not $actual) {
        return
    }

    if ((Normalize-PathText $actual) -ne (Normalize-PathText $ExpectedCompiler)) {
        throw @"
Existing CMake build tree '$BuildDir' was configured with:
  $actual
but this bootstrap expects:
  $ExpectedCompiler

Use a clean checkout/build directory for this development environment, or remove the stale build tree manually.
"@
    }
}

function Test-Executable {
    param([Parameter(Mandatory)] [string]$Path)
    return (Test-Path -LiteralPath $Path -PathType Leaf)
}

function Apply-LlvmPatches {
    param(
        [Parameter(Mandatory)] [string]$Git,
        [Parameter(Mandatory)] [string]$LlvmProjectDir,
        [Parameter(Mandatory)] [string]$PatchRoot
    )

    if (-not (Test-Path -LiteralPath $PatchRoot -PathType Container)) {
        Write-Host "No LLVM patch directory found at $PatchRoot"
        return
    }

    $patches = Get-ChildItem -LiteralPath $PatchRoot -Filter "*.patch" |
        Sort-Object -Property Name
    foreach ($patch in $patches) {
        $applyCode = Invoke-ToolExitCode $Git @("-C", $LlvmProjectDir, "apply", "--check", $patch.FullName)
        if ($applyCode -eq 0) {
            Invoke-Tool $Git @("-C", $LlvmProjectDir, "apply", $patch.FullName)
            continue
        }

        $reverseCode = Invoke-ToolExitCode $Git @("-C", $LlvmProjectDir, "apply", "--reverse", "--check", $patch.FullName)
        if ($reverseCode -eq 0) {
            Write-Host "Patch already applied: $($patch.Name)"
            continue
        }

        throw "LLVM patch cannot be applied cleanly: $($patch.FullName)"
    }
}

function Invoke-CMakeConfigure {
    param(
        [Parameter(Mandatory)] [string]$CMake,
        [Parameter(Mandatory)] [string[]]$Arguments
    )

    Invoke-Tool $CMake $Arguments $script:SourceDir
}

function Invoke-CMakeBuild {
    param(
        [Parameter(Mandatory)] [string]$CMake,
        [Parameter(Mandatory)] [string]$BuildDir,
        [string[]]$Targets = @()
    )

    $arguments = @("--build", $BuildDir)
    foreach ($target in $Targets) {
        $arguments += @("--target", $target)
    }
    $arguments += @("--", "-j$Jobs")
    Invoke-Tool $CMake $arguments $script:SourceDir
}

$script:SourceDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location -LiteralPath $script:SourceDir

$BuildDir = Join-Path $script:SourceDir "build"
$BootstrapDir = Join-Path $BuildDir ".bootstrap"
$Stage1Dir = Join-Path $BootstrapDir "stage1"
$Stage2Dir = Join-Path $BootstrapDir "stage2"
$Stage3SelfHostDir = Join-Path $BootstrapDir "stage3-selfhost"
$Stage2InstallDir = Join-Path $BuildDir "cod"
$FinalInstallDir = Join-Path $script:SourceDir "built"
$LlvmProjectDir = Join-Path $script:SourceDir "llvm-project"
$LlvmSourceDir = Join-Path $LlvmProjectDir "llvm"
$PatchRoot = Join-Path $script:SourceDir (Join-Path "patches\llvm" $LLVMBranch)

Write-Step "Checking build layout"
Assert-PlainDirectoryIfPresent $BuildDir
Assert-PlainDirectoryIfPresent $FinalInstallDir

Write-Step "Discovering host tools"
$CMake = Resolve-Tool "cmake.exe" @(
    "$env:ProgramFiles\CMake\bin\cmake.exe",
    "${env:ProgramFiles(x86)}\CMake\bin\cmake.exe"
)
$Ninja = Resolve-Tool "ninja.exe" @(
    "$env:LOCALAPPDATA\Microsoft\WinGet\Links\ninja.exe",
    "$env:ProgramFiles\Ninja\ninja.exe"
)
$Git = Resolve-Tool "git.exe" @(
    "$env:ProgramFiles\Git\cmd\git.exe",
    "${env:ProgramFiles(x86)}\Git\cmd\git.exe"
)
Add-PathDirectory $CMake
Add-PathDirectory $Ninja
Add-PathDirectory $Git
Import-VsDevEnvironment

$Cl = Resolve-Tool "cl.exe"
$Link = Resolve-Tool "link.exe"
Write-Host "CMake: $CMake"
Write-Host "Ninja: $Ninja"
Write-Host "Git:   $Git"
Write-Host "MSVC:  $Cl"
Write-Host "Link:  $Link"
Write-Host "Jobs:  $Jobs"

Write-Step "Preparing llvm-project"
if (Test-Path -LiteralPath (Join-Path $LlvmProjectDir ".git") -PathType Container) {
    $revParseCode = Invoke-ToolExitCode $Git @("-C", $LlvmProjectDir, "rev-parse", "--is-inside-work-tree")
    if ($revParseCode -ne 0 -or -not (Test-Path -LiteralPath (Join-Path $LlvmSourceDir "CMakeLists.txt") -PathType Leaf)) {
        Write-Host "Removing incomplete llvm-project checkout at $LlvmProjectDir"
        Remove-GeneratedTree $LlvmProjectDir
    } elseif (-not $NoPull) {
        Invoke-Tool $Git @("-c", "http.version=HTTP/1.1", "-C", $LlvmProjectDir, "pull", "--ff-only")
    }
}

if (-not (Test-Path -LiteralPath $LlvmProjectDir)) {
    Invoke-Tool $Git @("-c", "http.version=HTTP/1.1", "clone", "--progress", "--depth", "1", "--single-branch", "-b", $LLVMBranch, "https://github.com/llvm/llvm-project.git", $LlvmProjectDir)
} elseif (-not (Test-Path -LiteralPath (Join-Path $LlvmProjectDir ".git") -PathType Container)) {
    throw "'$LlvmProjectDir' exists but is not a git checkout."
}
Apply-LlvmPatches $Git $LlvmProjectDir $PatchRoot

$CommonCMakeArgs = @(
    "-G", "Ninja",
    "-S", $LlvmSourceDir,
    "-DLLVM_ENABLE_RUNTIMES=",
    "-DLLVM_ENABLE_EH=ON",
    "-DLLVM_ENABLE_RTTI=ON",
    "-DLLVM_ENABLE_FFI=OFF",
    "-DLLVM_INCLUDE_DOCS=OFF",
    "-DLLVM_INCLUDE_TESTS=OFF",
    "-DLLVM_INSTALL_UTILS=OFF",
    "-DLLVM_OPTIMIZED_TABLEGEN=ON",
    "-DLLVM_ENABLE_IDE=ON",
    "-DLLVM_LINK_LLVM_DYLIB=OFF",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    "-DCMAKE_C_FLAGS=/utf-8 /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH",
    "-DCMAKE_CXX_FLAGS=/utf-8 /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH",
    "-DCMAKE_MT=mt.exe",
    "-DCMAKE_RC_COMPILER=rc.exe"
)

$Stage1ClangCl = Join-Path $Stage1Dir "bin\clang-cl.exe"
$Stage1LldLink = Join-Path $Stage1Dir "bin\lld-link.exe"
$Stage2Cod = Join-Path $Stage2InstallDir "bin\cod.exe"
$Stage2ClangCl = Join-Path $Stage2InstallDir "bin\clang-cl.exe"
$Stage2LldLink = Join-Path $Stage2InstallDir "bin\lld-link.exe"
$Stage2Ready = (Test-Executable $Stage2Cod) -and
    (Test-Executable $Stage2ClangCl) -and
    (Test-Executable $Stage2LldLink)

Write-Step "Stage 1: build Clang/LLD with MSVC"
if ($Stage2Ready) {
    Write-Host "Stage 2 install already has $Stage2Cod, $Stage2ClangCl, and $Stage2LldLink"
    Write-Host "Skipping transient stage 1."
} elseif (-not ((Test-Executable $Stage1ClangCl) -and (Test-Executable $Stage1LldLink))) {
    Ensure-Directory $Stage1Dir
    $stage1Args = @(
        "-B", $Stage1Dir,
        "-DCMAKE_INSTALL_PREFIX=$(Join-Path $BootstrapDir 'stage1-install')",
        "-DCMAKE_C_COMPILER=$Cl",
        "-DCMAKE_CXX_COMPILER=$Cl",
        "-DCMAKE_LINKER=$Link",
        "-DLLVM_ENABLE_PROJECTS=clang;lld",
        "-DLLVM_TARGETS_TO_BUILD=Native",
        "-DCMAKE_BUILD_TYPE=Release"
    ) + $CommonCMakeArgs
    Invoke-CMakeConfigure $CMake $stage1Args
    Repair-MsvcDepsPrefix $Stage1Dir
    Repair-ClangClResourceRules $Stage1Dir
    Invoke-CMakeBuild $CMake $Stage1Dir @("clang", "lld")
} else {
    Write-Host "Stage 1 already has $Stage1ClangCl and $Stage1LldLink"
}

Write-Step "Stage 2: build/install bootstrap CoD toolchain with stage 1 Clang"
if (-not $Stage2Ready) {
    Ensure-Directory $Stage2Dir
    $stage2Args = @(
        "-B", $Stage2Dir,
        "-DCMAKE_INSTALL_PREFIX=$Stage2InstallDir",
        "-DCMAKE_SYSTEM_PREFIX_PATH=$Stage1Dir",
        "-DCMAKE_C_COMPILER=$Stage1ClangCl",
        "-DCMAKE_CXX_COMPILER=$Stage1ClangCl",
        "-DCMAKE_LINKER=$Stage1LldLink",
        "-DLLVM_USE_LINKER=lld",
        "-DLLVM_ENABLE_PROJECTS=clang;lld",
        "-DLLVM_EXTERNAL_PROJECTS=cod",
        "-DLLVM_EXTERNAL_COD_SOURCE_DIR=$script:SourceDir",
        "-DLLVM_TARGETS_TO_BUILD=Native",
        "-DCMAKE_BUILD_TYPE=Release"
    ) + $CommonCMakeArgs
    Invoke-CMakeConfigure $CMake $stage2Args
    Repair-MsvcDepsPrefix $Stage2Dir
    Repair-ClangClResourceRules $Stage2Dir
    Invoke-CMakeBuild $CMake $Stage2Dir
    Invoke-CMakeBuild $CMake $Stage2Dir @("install")
    $Stage2Ready = $true
} else {
    Write-Host "Stage 2 install already has $Stage2Cod, $Stage2ClangCl, and $Stage2LldLink"
}

Write-Step "Stage 3: self-build final development tree with stage 2 CoD toolchain"
Assert-CMakeCompiler $BuildDir $Stage2ClangCl
Ensure-Directory $BuildDir
$finalProjects = if ($EnableLLDB) { "clang;lld;lldb" } else { "clang;lld" }
$stage3Args = @(
    "-B", $BuildDir,
    "-DCMAKE_INSTALL_PREFIX=$FinalInstallDir",
    "-DCMAKE_SYSTEM_PREFIX_PATH=$Stage2InstallDir",
    "-DCMAKE_C_COMPILER=$Stage2ClangCl",
    "-DCMAKE_CXX_COMPILER=$Stage2ClangCl",
    "-DCMAKE_LINKER=$Stage2LldLink",
    "-DLLVM_USE_LINKER=lld",
    "-DLLVM_EXTERNAL_PROJECTS=cod",
    "-DLLVM_EXTERNAL_COD_SOURCE_DIR=$script:SourceDir",
    "-DLLVM_TARGETS_TO_BUILD=$TargetsToBuild",
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DLLVM_ENABLE_PROJECTS=$finalProjects"
) + $CommonCMakeArgs
Invoke-CMakeConfigure $CMake $stage3Args
Repair-MsvcDepsPrefix $BuildDir
Repair-ClangClResourceRules $BuildDir
Invoke-CMakeBuild $CMake $BuildDir
Invoke-CMakeBuild $CMake $BuildDir @("install")

if ($VerifyStage3SelfHost) {
    Write-Step "Stage 4: verify stage 3 toolchain can self-build"
    $Stage3ClangCl = Join-Path $FinalInstallDir "bin\clang-cl.exe"
    $Stage3LldLink = Join-Path $FinalInstallDir "bin\lld-link.exe"
    if (-not ((Test-Executable $Stage3ClangCl) -and (Test-Executable $Stage3LldLink))) {
        throw "Stage 3 installed toolchain is incomplete. Expected $Stage3ClangCl and $Stage3LldLink."
    }

    Assert-CMakeCompiler $Stage3SelfHostDir $Stage3ClangCl
    Ensure-Directory $Stage3SelfHostDir
    $stage4Args = @(
        "-B", $Stage3SelfHostDir,
        "-DCMAKE_INSTALL_PREFIX=$(Join-Path $BootstrapDir 'stage3-selfhost-install')",
        "-DCMAKE_SYSTEM_PREFIX_PATH=$FinalInstallDir",
        "-DCMAKE_C_COMPILER=$Stage3ClangCl",
        "-DCMAKE_CXX_COMPILER=$Stage3ClangCl",
        "-DCMAKE_LINKER=$Stage3LldLink",
        "-DLLVM_USE_LINKER=lld",
        "-DLLVM_EXTERNAL_PROJECTS=cod",
        "-DLLVM_EXTERNAL_COD_SOURCE_DIR=$script:SourceDir",
        "-DLLVM_TARGETS_TO_BUILD=$TargetsToBuild",
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DLLVM_ENABLE_PROJECTS=$finalProjects"
    ) + $CommonCMakeArgs
    Invoke-CMakeConfigure $CMake $stage4Args
    Repair-MsvcDepsPrefix $Stage3SelfHostDir
    Repair-ClangClResourceRules $Stage3SelfHostDir
    Invoke-CMakeBuild $CMake $Stage3SelfHostDir
}

if (-not $KeepBootstrap) {
    Write-Step "Cleaning transient bootstrap build trees"
    if (Test-Path -LiteralPath $Stage1Dir) {
        Remove-GeneratedTree $Stage1Dir
    }
    if (Test-Path -LiteralPath $Stage2Dir) {
        Remove-GeneratedTree $Stage2Dir
    }
    if (Test-Path -LiteralPath $Stage3SelfHostDir) {
        Remove-GeneratedTree $Stage3SelfHostDir
    }
}

Write-Step "Bootstrap complete"
Write-Host "Development build: $BuildDir"
Write-Host "Installed toolchain: $FinalInstallDir"
Write-Host "Bootstrap compiler kept for compile_commands.json: $Stage2InstallDir"
