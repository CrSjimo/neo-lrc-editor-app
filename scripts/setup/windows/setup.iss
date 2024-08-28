; Pre-defined Environments:
;   SETUP_APP_NAME
;   SETUP_APP_VERSION
;   SETUP_APP_INSTALLED_DIR
;   SETUP_MESSAGE_FILES_DIR
;   SETUP_OUTPUT_DIR
;   SETUP_OUTPUT_FILE_BASE

#define MyAppName GetEnv("SETUP_APP_NAME")
#define MyAppVersion GetEnv("SETUP_APP_VERSION")
#define MyAppPublisher "Crindzebra Sjimo"
#define MyAppURL "https://github.com/CrSjimo/neo-lrc-editor-app"
#define MyAppCopyright "Copyright 2024 Crindzebra Sjimo"

#define MyAppExeName "bin\" + MyAppName + ".exe"

#define MyAppProjectAssocName "LRC Lyric File"
#define MyAppProjectAssocExt ".lrc"
#define MyAppProjectAssocKey StringChange(MyAppProjectAssocName, " ", "") + MyAppProjectAssocExt

#define MyAppInstalledDir GetEnv("SETUP_APP_INSTALLED_DIR")

[Setup]
AppId={{E305B866-EDAD-44A8-8B0C-3FF328D45AE3}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppCopyright={#MyAppCopyright}
DefaultDirName={autopf}\{#MyAppName}
ChangesAssociations=yes
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
LicenseFile={#MyAppInstalledDir}\share\LICENSE
OutputDir={#GetEnv("SETUP_OUTPUT_DIR")}
OutputBaseFilename={#GetEnv("SETUP_OUTPUT_FILE_BASE")}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
VersionInfoDescription={#MyAppName} Installer

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "{#GetEnv('SETUP_MESSAGE_FILES_DIR')}\ChineseSimplified.isl"
Name: "chinesetraditional"; MessagesFile: "{#GetEnv('SETUP_MESSAGE_FILES_DIR')}\ChineseTraditional.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#MyAppInstalledDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Registry]
Root: HKA; Subkey: "Software\Classes\{#MyAppProjectAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppProjectAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppProjectAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppProjectAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppProjectAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},1"
Root: HKA; Subkey: "Software\Classes\{#MyAppProjectAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: "{#MyAppProjectAssocExt}"; ValueData: ""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

