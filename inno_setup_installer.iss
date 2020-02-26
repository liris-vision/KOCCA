; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "KOCCA - Kinect & Optitrack Calibration & Capture Application"
#define MyAppVersion "1.0"
#define MyAppPublisher "LIRIS - Laboratoire d'InfoRmatique en Image et Syst�mes d'information"
#define MyAppURL "https://liris.cnrs.fr/"
#define MyAppExeName "KOCCA.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{26333C98-6BD5-45D7-AF2A-C67CF6775C36}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\KOCCA
DisableProgramGroupPage=yes
OutputDir=.\build\Installer
OutputBaseFilename=KOCCA_{#MyAppVersion}_setup
SetupIconFile=.\kocca.ico
Compression=lzma
SolidCompression=yes
ChangesAssociations=yes

[Registry]

; Associate "KOCCA Sequence Archive" extension (.ksa) with our app
Root: HKCR; Subkey: ".ksa"; ValueType: string; ValueName: ""; ValueData: "KOCCASequenceArchive"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "KOCCASequenceArchive"; ValueType: string; ValueName: ""; ValueData: "KOCCA Sequence Archive"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "KOCCASequenceArchive\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCR; Subkey: "KOCCASequenceArchive\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

; Associate "Kinect Calibration File" extension (.kcf) with our app
Root: HKCR; Subkey: ".kcf"; ValueType: string; ValueName: ""; ValueData: "KinectCalibrationFile"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "KinectCalibrationFile"; ValueType: string; ValueName: ""; ValueData: "Kinect Calibration File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "KinectCalibrationFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCR; Subkey: "KinectCalibrationFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: ".\build\Release\KOCCA.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\kocca.ui"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\kocca.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\doc\KOCCA_user_manual.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\img\*"; DestDir: "{app}\img"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\share\*"; DestDir: "{app}\share"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ".\build\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Dirs]
Name: "{localappdata}\KOCCA"

[Icons]
Name: "{commonprograms}\{#MyAppName}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commonprograms}\{#MyAppName}\KOCCA User Manual"; Filename: "{app}\KOCCA_user_manual.pdf"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

