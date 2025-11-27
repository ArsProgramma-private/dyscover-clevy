; Inno Setup script template for Dyscover
; Usage: iscc package-windows-inno.iss /DSourceDir=dist-windows-Release /DOutputDir=installer
#define AppName "Clevy Dyscover 4"
#define AppVersion "4.0.5.0"
#define Publisher "Alt. Medical B.V."
#define URL "https://clevy.com"
#define ExeName "Dyscover.exe"
[Setup]
AppId={{12C19B6B-BB03-4255-AD65-0A46D994886B}}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
DefaultDirName={pf}\Clevy Dyscover 4
DefaultGroupName=Clevy Dyscover 4
OutputBaseFilename=Dyscover-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
DisableDirPage=no
DisableProgramGroupPage=yes
LicenseFile=..\res\License.rtf
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
[Files]
Source: "{#SourceDir}\\{#ExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\\audio\\*"; DestDir: "{app}\\audio"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "{#SourceDir}\\tts\\*"; DestDir: "{app}\\tts"; Flags: recursesubdirs createallsubdirs ignoreversion
[Icons]
Name: "{group}\\Clevy Dyscover 4"; Filename: "{app}\\{#ExeName}"; WorkingDir: "{app}"
[Run]
Filename: "{app}\\{#ExeName}"; Description: "Launch Dyscover"; Flags: nowait postinstall skipifsilent
