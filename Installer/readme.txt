WWWC インストーラ
--

■ 概要
WWWC のインストーラと、インストーラを作成するツールです。
CLCL のインストーラを元にしています。

インストーラは C のみで記述しており、Windows の API 以外のライブラリは使用して
いません。ZIP の展開処理 (deflate) もインストーラ内に実装しているため、実行時に
必要なランタイムやライブラリはありません。

■ ファイル
  Installer.vcxproj インストーラのプロジェクト (WWWC.sln に含まれる)
  build.ps1         インストーラを作成する PowerShell スクリプト
  prepare.ps1       ファイルの収集と ZIP の作成 (プロジェクトから呼ばれる)
  Installer.c       インストーラ本体
  Installer.h       インストーラの定義
  Installer.rc      リソース (ダイアログ / 各言語の文字列 / ZIP)
  resource.h        リソース ID
  unzip.c           ZIP の展開 (deflate と CRC32 の実装)
  unzip.h           ZIP の展開の定義
  wwwc_app.ini      インストール先に配置する設定 (データの保存先の指定)
  res/manifest.xml  マニフェスト (管理者権限で実行)

■ インストーラの作成
Visual Studio 2017 以降 (C++ デスクトップ開発) と Windows SDK が必要です。
PowerShell で以下を実行すると、WWWC 本体と wwwc.dll のビルドからインストーラの
作成までを一括で行います。

  PS> cd Installer
  PS> .\build.ps1

作成したインストーラは Installer\out に出力されます。
出力するファイル名は wwwc<バージョン>.exe です。Ver 1.2.0 なら wwwc120.exe に
なります。バージョンは WWWC\WWWC.rc の FILEVERSION から取得します。
インストーラに含まれるのは Release 構成でビルドした WWWC.exe と wwwc.dll です。

ビルド済みのファイルからインストーラだけを作成する場合は -SkipBuild を指定しま
す。

  PS> .\build.ps1 -SkipBuild

主なオプション
  -Configuration <構成>   ビルド構成 (既定: Release)
  -Platform <プラットフォーム>
                          ビルドするプラットフォーム (既定: x86)
  -Version <バージョン>   バージョン (既定: WWWC\WWWC.rc の FILEVERSION)
  -SourceDir <フォルダ>   WWWC.exe と wwwc.dll の収集元のフォルダ
                          (既定: <リポジトリ>\<ビルド構成>)
  -OutDir <フォルダ>      出力先のフォルダ (既定: Installer\out)
  -SkipBuild              WWWC 本体と wwwc.dll のビルドを行わない

■ Visual Studio でのビルド
Installer.vcxproj は WWWC.sln に含まれています。ソリューションをビルドすると、
WWWC 本体と wwwc.dll のビルドの後にインストーラが作成されます。Installer
プロジェクトだけをビルドすることもできます。その場合は WWWC 本体と wwwc.dll を
ビルド済みにしておいてください。

ビルド前のイベントで prepare.ps1 がファイルの収集と ZIP の作成、バージョン情報の
instinfo.h の生成を行い、ビルド後のイベントで prepare.ps1 が Installer\out へ
wwwc<バージョン>.exe として出力します。build.ps1 はこのプロジェクトを msbuild で
ビルドしているだけなので、どちらの方法でも結果は同じです。

Installer\out に出力するのは Release 構成のときだけです。Debug 構成でビルドした
インストーラは Installer\Debug\WWWCInst.exe になります。Debug 構成では Debug 構成
の WWWC.exe などを収集するため、配布するインストーラと混ざらないようにしています。

プロジェクトの設定は msbuild のプロパティで変更できます。build.ps1 のオプション
はこれらに対応しています。フォルダは末尾に \ を付けないでください。
  WwwcVersion    バージョン
  WwwcSourceDir  WWWC.exe と wwwc.dll の収集元
  WwwcOutDir     インストーラの出力先

インストーラのバージョン情報はビルド時に生成する instinfo.h で決まります。この
ファイルはリポジトリには含めていません。

■ インストールするファイル
prepare.ps1 が以下のファイルを収集して ZIP にまとめ、インストーラのリソース
(RCDATA) として登録します。インストーラは実行時にこの ZIP を展開して配置します。

  WWWC.exe           Release フォルダから収集
  wwwc.dll           Release フォルダから収集
  wwwc.html          WWWC フォルダから収集
  wwwc_en.html
  readme.txt
  readme_en.txt
  ini_sample.txt
  filter_sample.txt
  wwwc_app.ini       Installer フォルダから収集
  manual\*.png       WWWC\manual フォルダから収集 (マニュアルの画像)

収集するファイルを変更する場合は prepare.ps1 の Get-TargetFiles を編集してくださ
い。

■ データの保存先
WWWC は実行ファイルと同じフォルダに設定とアイテムの情報 (ユーザ名のフォルダ、
general.ini、wwwc.dll の wwwc.ini と filter.txt、アーカイブ) を保存します。
インストール先の Program Files には一般ユーザーが書き込めないため、インストーラは
以下の内容の wwwc_app.ini をインストール先に配置します。

  [GENERAL]
  portable=0

WWWC と wwwc.dll は実行ファイルと同じフォルダの wwwc_app.ini で portable=0 が
指定されている場合、データを %LOCALAPPDATA%\WWWC に保存します。wwwc_app.ini が
無い場合 (ZIP で配布したファイルを任意のフォルダに置いて使う場合) は従来どおり
実行ファイルと同じフォルダに保存します。

■ インストーラの動作
管理者権限で実行します (マニフェストで requireAdministrator を指定)。

・インストール先を選択できます。
  既定は %ProgramFiles%\WWWC です。既にインストールされている場合は、登録されて
  いるインストール先が既定になります。
・スタートアップ、スタートメニュー、デスクトップへのショートカットの作成を選択
  できます。ショートカットはすべてのユーザ用の場所に作成します。
  新規にインストールする場合、スタートアップは登録しない状態が既定です。
・コントロールパネルの「プログラムと機能」(アプリと機能) に登録します。
・既にインストールされている場合は更新します。
  登録済みのアプリの一覧のキーをそのまま使用するため、二重に登録されません。
・WWWC が起動中でファイルを上書きできない場合は、WWWC の終了を促します。
・インストール後に WWWC を起動するかどうかを確認します。起動する場合は管理者権
  限を引き継がないように、エクスプローラ経由で起動します。

■ アンインストール
コントロールパネルから実行できます。
インストーラに引数を指定して実行することもできます。

  wwwc120.exe /uninstall

インストール先に配置される uninstall.exe も同じ動作です。

  "C:\Program Files (x86)\WWWC\uninstall.exe" /uninstall

インストールしたファイルとインストール時に作成したフォルダの一覧はインストール先
の uninstall.dat に記録しており、アンインストールではこの一覧のファイルとショート
カット、空になったフォルダを削除します。一覧に書かれていても、インストール先の配
下にあるファイルと、スタートアップ / スタートメニュー / デスクトップの直下にある
WWWC*.lnk 以外は削除しません。
設定とアイテムのデータ (%LOCALAPPDATA%\WWWC、アーカイブを含む) を削除するかどう
かは実行時に確認します。削除されるのはログオン中のユーザー (デスクトップを実行し
ているユーザー) の %LOCALAPPDATA%\WWWC です。他のユーザーのデータは残ります。

■ アプリの一覧への登録
以下のいずれかのキーの配下に、表示名が「WWWC」で始まるキーがあるかを検索し、
見つかった場合はそのキーをそのまま使用します。

  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall
    (32bit ビューと 64bit ビューの両方)
  HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall

見つからない場合は以下に新規に作成します。

  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall\WWWC
    (32bit のため、64bit 環境では WOW6432Node の配下になります)

登録する値
  DisplayName, DisplayVersion, DisplayIcon, Publisher, URLInfoAbout,
  InstallLocation, UninstallString, InstallDate, NoModify, NoRepair,
  EstimatedSize, VersionMajor, VersionMinor

値を設定する前にキーの値をすべて削除します。以前の登録の値が残らないようにする
ためです。

■ 多言語対応
WWWC 本体と同じく日本語と英語のリソースを持っています。
Windows の UI 言語が日本語の場合は日本語、それ以外の場合は英語で表示します。
ダイアログのレイアウトは言語に依存しないようにしており、表示する文字列はすべて
実行時に文字列リソースから設定しています。
翻訳を修正する場合は Installer.rc の STRINGTABLE を編集してください。
