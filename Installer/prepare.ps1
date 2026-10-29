#Requires -Version 5.1
<#
.SYNOPSIS
	インストーラのビルドに必要なファイルを準備します。

.DESCRIPTION
	Prepare
		インストールするファイルを収集して payload.zip にまとめ、
		バージョン情報の instinfo.h を生成します。
		Installer.vcxproj のビルド前のイベントから呼び出されます。
	Publish
		ビルドしたインストーラを wwwc<バージョン>.exe として出力します。
		Installer.vcxproj のビルド後のイベントから呼び出されます。

	通常は build.ps1 から Visual Studio のプロジェクト経由で実行されます。
#>
param(
	# 動作
	[ValidateSet("Prepare", "Publish")]
	[string]$Mode = "Prepare",
	# Prepare: 生成先のフォルダ / Publish: 出力先のフォルダ
	[string]$OutDir = "",
	# ビルド構成
	[string]$Configuration = "Release",
	# インストールするファイルの収集元 (省略時は <リポジトリ>\<ビルド構成>)
	[string]$SourceDir = "",
	# バージョン (省略時は WWWC\WWWC.rc の FILEVERSION を使用)
	[string]$Version = "",
	# Publish: ビルドしたインストーラのパス
	[string]$TargetPath = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$InstallerDir = $PSScriptRoot
$RootDir = Split-Path -Parent $InstallerDir

# バージョンの取得
function Get-WwwcVersion([string]$specified) {
	if (-not [string]::IsNullOrEmpty($specified)) {
		$m = [regex]::Match($specified, '^(\d+)\.(\d+)\.(\d+)$')
		if (-not $m.Success) {
			throw "バージョンの指定が正しくありません (例: 1.2.0): $specified"
		}
	} else {
		$rcPath = Join-Path $RootDir "WWWC\WWWC.rc"
		if (-not (Test-Path $rcPath)) {
			throw "WWWC.rc が見つかりません: $rcPath"
		}
		$rcText = Get-Content -Path $rcPath -Raw -Encoding UTF8
		$m = [regex]::Match($rcText, 'FILEVERSION\s+(\d+)\s*,\s*(\d+)\s*,\s*(\d+)')
		if (-not $m.Success) {
			throw "WWWC.rc から FILEVERSION を取得できませんでした。"
		}
	}
	$major = [int]$m.Groups[1].Value
	$minor = [int]$m.Groups[2].Value
	$revision = [int]$m.Groups[3].Value
	return [pscustomobject]@{
		Major    = $major
		Minor    = $minor
		Revision = $revision
		Text     = "$major.$minor.$revision"
		FileName = "wwwc$major$minor$revision.exe"
	}
}

# インストールするファイル (収集元のパス = インストール後のファイル名)
function Get-TargetFiles([string]$binDir) {
	return [ordered]@{
		(Join-Path $binDir "WWWC.exe")                  = "WWWC.exe"
		(Join-Path $binDir "wwwc.dll")                  = "wwwc.dll"
		(Join-Path $RootDir "WWWC\wwwc.html")           = "wwwc.html"
		(Join-Path $RootDir "WWWC\wwwc_en.html")        = "wwwc_en.html"
		(Join-Path $RootDir "WWWC\readme.txt")          = "readme.txt"
		(Join-Path $RootDir "WWWC\readme_en.txt")       = "readme_en.txt"
		(Join-Path $RootDir "WWWC\ini_sample.txt")      = "ini_sample.txt"
		(Join-Path $RootDir "WWWC\filter_sample.txt")   = "filter_sample.txt"
		# 設定とアイテムを %LOCALAPPDATA%\WWWC に保存させる (インストール先には書き込まない)
		(Join-Path $InstallerDir "wwwc_app.ini")        = "wwwc_app.ini"
	}
}

# マニュアルの画像 (manual フォルダごと)
function Get-ManualFiles() {
	$files = [ordered]@{}
	Get-ChildItem -Path (Join-Path $RootDir "WWWC\manual") -Filter "*.png" | Sort-Object Name | ForEach-Object {
		$files[$_.FullName] = "manual\" + $_.Name
	}
	return $files
}

$ver = Get-WwwcVersion $Version

# 相対パスは呼び出し元の作業フォルダに左右されないようにする
if (-not [string]::IsNullOrEmpty($OutDir) -and -not [System.IO.Path]::IsPathRooted($OutDir)) {
	$OutDir = Join-Path $InstallerDir $OutDir
}

if ($Mode -eq "Publish") {
	if ([string]::IsNullOrEmpty($TargetPath) -or -not (Test-Path $TargetPath)) {
		throw "ビルドしたインストーラが見つかりません: $TargetPath"
	}
	if ([string]::IsNullOrEmpty($OutDir)) {
		$OutDir = Join-Path $InstallerDir "out"
	}
	New-Item -Path $OutDir -ItemType Directory -Force | Out-Null
	$outPath = Join-Path $OutDir $ver.FileName
	Copy-Item -Path $TargetPath -Destination $outPath -Force
	Write-Host ("インストーラを作成しました: {0} ({1:N0} bytes)" -f $outPath, (Get-Item $outPath).Length)
	exit 0
}

if ([string]::IsNullOrEmpty($OutDir)) {
	throw "-OutDir を指定してください。"
}
if ([string]::IsNullOrEmpty($SourceDir)) {
	$SourceDir = Join-Path $RootDir $Configuration
}
New-Item -Path $OutDir -ItemType Directory -Force | Out-Null

# ------------------------------------------------------------------
# バージョン情報のヘッダを生成
# ------------------------------------------------------------------
$instInfo = @"
/*
 * WWWC Installer
 *
 * instinfo.h
 *
 * prepare.ps1 が生成するファイルです。直接編集しないでください。
 */

#ifndef _INC_WWWC_INSTINFO_H
#define _INC_WWWC_INSTINFO_H

#define INST_VER_MAJOR					$($ver.Major)
#define INST_VER_MINOR					$($ver.Minor)
#define INST_VER_REVISION				$($ver.Revision)
#define INST_VERSION_STR				"$($ver.Text)"
#define INST_FILE_VERSION_STR			"$($ver.Text).0"
#define INST_FILE_NAME					"$($ver.FileName)"

#endif
/* End of source */
"@ -replace "`r`n", "`n" -replace "`n", "`r`n"

# 内容が変わらない場合は書き換えない (不要な再コンパイルを避ける)
$instInfoPath = Join-Path $OutDir "instinfo.h"
$utf8Bom = New-Object System.Text.UTF8Encoding $true
if (-not (Test-Path $instInfoPath) -or
	[System.IO.File]::ReadAllText($instInfoPath) -ne $instInfo) {
	[System.IO.File]::WriteAllText($instInfoPath, $instInfo, $utf8Bom)
	Write-Host "instinfo.h を生成しました (Ver $($ver.Text))"
}

# ------------------------------------------------------------------
# インストールするファイルを ZIP にまとめる
# ------------------------------------------------------------------
$targetFiles = Get-TargetFiles $SourceDir
$targetFiles += Get-ManualFiles
foreach ($from in $targetFiles.Keys) {
	if (-not (Test-Path $from)) {
		throw "インストールするファイルが見つかりません: $from"
	}
}

# 収集元がすべて ZIP より古い場合は作り直さない
$zipPath = Join-Path $OutDir "payload.zip"
$latest = ($targetFiles.Keys | ForEach-Object { (Get-Item $_).LastWriteTimeUtc } |
	Sort-Object -Descending | Select-Object -First 1)
if ((Test-Path $zipPath) -and (Get-Item $zipPath).LastWriteTimeUtc -gt $latest) {
	exit 0
}

$stageDir = Join-Path $OutDir "files"
if (Test-Path $stageDir) {
	Remove-Item -Path $stageDir -Recurse -Force
}
New-Item -Path $stageDir -ItemType Directory -Force | Out-Null
foreach ($from in $targetFiles.Keys) {
	$to = Join-Path $stageDir $targetFiles[$from]
	New-Item -Path (Split-Path -Parent $to) -ItemType Directory -Force | Out-Null
	Copy-Item -Path $from -Destination $to -Force
}
if (Test-Path $zipPath) {
	Remove-Item -Path $zipPath -Force
}
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory(
	$stageDir, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $false)
Write-Host ("payload.zip を作成しました ({0} ファイル / {1:N0} bytes)" -f `
	$targetFiles.Count, (Get-Item $zipPath).Length)
