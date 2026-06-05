; Self-contained PixelStudio installer. Build: installer\build-installer.ps1
; Installs only files listed in build-installer.ps1 (staged folder).

#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif
#ifndef MyAppVersionInfo
  #define MyAppVersionInfo "0.1.0.0"
#endif
#ifndef StageDir
  #define StageDir "staging"
#endif

#define MyAppName "PixelStudio"
#define MyAppPublisher "DeeTech"
#define MyAppExeName "appPixelStudio.exe"
; Qt AppDataLocation + default Documents (see AppPaths.cpp)
#define MyAppUserDataRoaming "{userappdata}\DeeTech\PixelStudio"
#define MyAppUserDataLocal "{localappdata}\DeeTech\PixelStudio"
#define MyAppUserDocs "{userdocs}\PixelStudio"
#define MyAppId "{{8F2A1C4E-9B3D-4F6A-A1E2-3C5D7E9B0F12}"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
SetupIconFile=..\resources\ico\PixelStudio.ico
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableDirPage=no
DisableProgramGroupPage=yes
OutputDir=output
OutputBaseFilename={#MyAppName}-Setup-{#MyAppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
CloseApplications=yes
CloseApplicationsFilter={#MyAppExeName}
RestartApplications=no
VersionInfoVersion={#MyAppVersionInfo}
VersionInfoCompany={#MyAppPublisher}
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersionInfo}
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; staging = windeployqt output from build-installer.ps1 (minus build artifacts)
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[InstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
function TrimStr(const S: String): String;
begin
  Result := Trim(S);
end;

function ReadIniKeyValue(const FileName, Key: String): String;
var
  Lines: TArrayOfString;
  I: Integer;
  Line, Prefix: String;
begin
  Result := '';
  if not FileExists(FileName) then
    Exit;
  Prefix := Key + '=';
  if LoadStringsFromFile(FileName, Lines) then
    for I := 0 to GetArrayLength(Lines) - 1 do
    begin
      Line := TrimStr(Lines[I]);
      if Pos(Prefix, Line) = 1 then
      begin
        Result := TrimStr(Copy(Line, Length(Prefix) + 1, MaxInt));
        if (Length(Result) >= 2) and (Result[1] = '"') and (Result[Length(Result)] = '"') then
          Result := Copy(Result, 2, Length(Result) - 2);
        Exit;
      end;
    end;
end;

procedure DeleteDirIfExists(const DirName: String);
begin
  if DirExists(DirName) then
    DelTree(DirName, True, True, True);
end;

procedure DeleteCustomPathsFromIni(const IniPath: String);
var
  Custom: String;
begin
  if not FileExists(IniPath) then
    Exit;
  Custom := ReadIniKeyValue(IniPath, 'documentsRoot');
  if Custom <> '' then
    DeleteDirIfExists(Custom);
  Custom := ReadIniKeyValue(IniPath, 'projectsRoot');
  if Custom <> '' then
    DeleteDirIfExists(Custom);
  Custom := ReadIniKeyValue(IniPath, 'exportsRoot');
  if Custom <> '' then
    DeleteDirIfExists(Custom);
end;

procedure CloseRunningApp;
var
  ErrorCode: Integer;
begin
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /T /IM {#MyAppExeName}',
    '', SW_HIDE, ewWaitUntilTerminated, ErrorCode);
  Sleep(300);
end;

procedure RemovePixelStudioUserData;
var
  Roaming, Local, IniPath: String;
begin
  Roaming := ExpandConstant('{#MyAppUserDataRoaming}');
  Local := ExpandConstant('{#MyAppUserDataLocal}');

  IniPath := Roaming + '\app.ini';
  DeleteCustomPathsFromIni(IniPath);
  IniPath := Local + '\app.ini';
  DeleteCustomPathsFromIni(IniPath);

  DeleteDirIfExists(Roaming);
  DeleteDirIfExists(Local);
  DeleteDirIfExists(ExpandConstant('{#MyAppUserDocs}'));
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    CloseRunningApp;
  if CurUninstallStep = usPostUninstall then
    RemovePixelStudioUserData;
end;
