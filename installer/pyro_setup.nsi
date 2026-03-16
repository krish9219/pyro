; Pyro Programming Language - Windows Installer
; P.Y.R.O - Performance You Really Own
; Created by Aravind Pilla | https://aravindlabs.tech

Unicode True

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "WordFunc.nsh"
!include "TextFunc.nsh"

; ============================================
; Installer Configuration
; ============================================
Name "Pyro Programming Language"
OutFile "pyro-1.0.0-windows-setup.exe"
InstallDir "$LOCALAPPDATA\Pyro"
InstallDirRegKey HKCU "Software\Pyro" "InstallDir"
RequestExecutionLevel user
SetCompressor /SOLID lzma
BrandingText "Pyro v1.0.0 - aravindlabs.tech"

; Version Info (helps with SmartScreen reputation)
VIProductVersion "1.0.0.0"
VIFileVersion "1.0.0.0"
VIAddVersionKey "ProductName" "Pyro Programming Language"
VIAddVersionKey "CompanyName" "Aravind Labs"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2024-2026 Aravind Pilla. MIT License."
VIAddVersionKey "FileDescription" "Pyro Programming Language Installer - Performance You Really Own"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "ProductVersion" "1.0.0.0"
VIAddVersionKey "OriginalFilename" "pyro-1.0.0-windows-setup.exe"
VIAddVersionKey "InternalName" "PyroInstaller"

; ============================================
; Modern UI Settings
; ============================================
!define MUI_ICON "pyro.ico"
!define MUI_UNICON "pyro.ico"
!define MUI_ABORTWARNING

; Header image settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "Welcome to Pyro Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will install Pyro Programming Language v1.0.0 on your computer.$\r$\n$\r$\nP.Y.R.O - Performance You Really Own$\r$\n$\r$\nPyro is a compiled language that transpiles to C++.$\r$\nSimpler than Python, 10-100x faster.$\r$\n$\r$\nThis installer includes everything you need:$\r$\n  - Pyro compiler with 76 built-in modules$\r$\n  - MinGW C++ toolchain (for pyro run/build)$\r$\n$\r$\nPublisher: Aravind Pilla (aravindlabs.tech)$\r$\nLicense: MIT (Open Source)$\r$\n$\r$\nClick Next to continue."

; Finish page
!define MUI_FINISHPAGE_TITLE "Pyro Installed Successfully!"
!define MUI_FINISHPAGE_TEXT "Pyro has been installed on your computer.$\r$\n$\r$\nTo get started, open a NEW PowerShell or Command Prompt and type:$\r$\n$\r$\n  pyro --version$\r$\n$\r$\nThen try:$\r$\n  echo print($\"Hello from Pyro!$\") > hello.ro$\r$\n  pyro run hello.ro$\r$\n$\r$\nDocs: https://aravindlabs.tech/pyro-lang/docs.html"
!define MUI_FINISHPAGE_LINK "Visit Pyro Documentation"
!define MUI_FINISHPAGE_LINK_LOCATION "https://aravindlabs.tech/pyro-lang/docs.html"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language
!insertmacro MUI_LANGUAGE "English"

; ============================================
; Variables
; ============================================
Var MINGW_DIR

; ============================================
; Installer Sections
; ============================================

Section "Pyro Compiler (required)" SecPyro
    SectionIn RO ; Required, cannot deselect

    SetOutPath "$INSTDIR\bin"

    ; Install pyro.exe
    File "pyro.exe"

    ; Store install dir in registry
    WriteRegStr HKCU "Software\Pyro" "InstallDir" "$INSTDIR"

    ; Add to user PATH
    ReadRegStr $0 HKCU "Environment" "Path"

    ; Check if already in PATH
    ${If} $0 != ""
        StrCpy $1 "$INSTDIR\bin"
        Push $0
        Push $1
        Call StrContains
        Pop $2
        ${If} $2 == ""
            ; Not in PATH, add it
            WriteRegExpandStr HKCU "Environment" "Path" "$0;$INSTDIR\bin"
        ${EndIf}
    ${Else}
        WriteRegExpandStr HKCU "Environment" "Path" "$INSTDIR\bin"
    ${EndIf}

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Add to Add/Remove Programs
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayName" "Pyro Programming Language 1.0.0"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayIcon" "$INSTDIR\bin\pyro.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "Publisher" "Aravind Labs"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayVersion" "1.0.0"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "URLInfoAbout" "https://aravindlabs.tech/pyro-lang/"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "HelpLink" "https://aravindlabs.tech/pyro-lang/docs.html"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "NoModify" 1
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "NoRepair" 1

    ; Get installed size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "EstimatedSize" "$0"

    ; Create Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\Pyro"
    CreateShortcut "$SMPROGRAMS\Pyro\Pyro REPL.lnk" "$INSTDIR\bin\pyro.exe" "" "$INSTDIR\bin\pyro.exe" 0
    CreateShortcut "$SMPROGRAMS\Pyro\Pyro Docs (Online).lnk" "https://aravindlabs.tech/pyro-lang/docs.html"
    CreateShortcut "$SMPROGRAMS\Pyro\Uninstall Pyro.lnk" "$INSTDIR\uninstall.exe"

    ; Register .ro file extension
    WriteRegStr HKCU "Software\Classes\.ro" "" "PyroSource"
    WriteRegStr HKCU "Software\Classes\PyroSource" "" "Pyro Source File"
    WriteRegStr HKCU "Software\Classes\PyroSource\shell\open\command" "" '"$INSTDIR\bin\pyro.exe" run "%1"'
    WriteRegStr HKCU "Software\Classes\PyroSource\DefaultIcon" "" "$INSTDIR\bin\pyro.exe,0"

    ; Notify shell of environment change
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

    DetailPrint "Pyro compiler installed successfully!"
SectionEnd

Section "MinGW C++ Toolchain (recommended)" SecMinGW
    StrCpy $MINGW_DIR "$INSTDIR\mingw"

    DetailPrint "Downloading MinGW C++ toolchain (~30 MB)..."
    DetailPrint "This may take a few minutes depending on your connection..."

    ; Use PowerShell for HTTPS download (NSISdl cannot handle GitHub HTTPS redirects)
    ; Try w64devkit v2.1.0 first, then v2.0.0, then v1.23.0
    nsExec::ExecToLog 'powershell -ExecutionPolicy Bypass -Command "[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $$ProgressPreference = \"SilentlyContinue\"; try { Invoke-WebRequest -Uri \"https://github.com/skeeto/w64devkit/releases/download/v2.1.0/w64devkit-x64-2.1.0.zip\" -OutFile \"$TEMP\w64devkit.zip\" -UseBasicParsing; Write-Host \"DOWNLOAD_OK\" } catch { try { Invoke-WebRequest -Uri \"https://github.com/skeeto/w64devkit/releases/download/v2.0.0/w64devkit-x64-2.0.0.zip\" -OutFile \"$TEMP\w64devkit.zip\" -UseBasicParsing; Write-Host \"DOWNLOAD_OK\" } catch { try { Invoke-WebRequest -Uri \"https://github.com/skeeto/w64devkit/releases/download/v1.23.0/w64devkit-x64-1.23.0.zip\" -OutFile \"$TEMP\w64devkit.zip\" -UseBasicParsing; Write-Host \"DOWNLOAD_OK\" } catch { Write-Host \"DOWNLOAD_FAILED\" } } }"'
    Pop $0

    ; Check if the zip file exists and has content
    ${IfNot} ${FileExists} "$TEMP\w64devkit.zip"
        DetailPrint "Download failed. You can install a C++ compiler manually later."
        DetailPrint "Run in PowerShell: winget install mingw"
        MessageBox MB_OK|MB_ICONINFORMATION "MinGW download failed (check your internet connection).$\r$\n$\r$\nPyro is installed but 'pyro run' needs a C++ compiler.$\r$\nYou can install one later by running:$\r$\n  winget install mingw$\r$\n$\r$\nOr download w64devkit manually from:$\r$\nhttps://github.com/skeeto/w64devkit/releases"
        Goto mingw_done
    ${EndIf}

    DetailPrint "Extracting MinGW toolchain (this takes a minute)..."

    ; Extract using PowerShell
    nsExec::ExecToLog 'powershell -ExecutionPolicy Bypass -Command "Expand-Archive -Force -Path \"$TEMP\w64devkit.zip\" -DestinationPath \"$INSTDIR\""'
    Pop $0

    ; w64devkit extracts to a w64devkit subfolder - rename to mingw
    ${If} ${FileExists} "$INSTDIR\w64devkit\bin\g++.exe"
        ; Remove old mingw dir if exists
        RMDir /r "$MINGW_DIR"
        Rename "$INSTDIR\w64devkit" "$MINGW_DIR"
    ${EndIf}

    ${If} ${FileExists} "$MINGW_DIR\bin\g++.exe"
        ; Add MinGW to PATH
        ReadRegStr $0 HKCU "Environment" "Path"
        WriteRegExpandStr HKCU "Environment" "Path" "$0;$MINGW_DIR\bin"

        ; Notify shell of environment change
        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

        DetailPrint "MinGW C++ toolchain installed successfully!"
        DetailPrint "g++ is now available for 'pyro run' and 'pyro build'"
    ${Else}
        DetailPrint "MinGW extraction may have failed."
        DetailPrint "You can install a C++ compiler manually: winget install mingw"
        MessageBox MB_OK|MB_ICONINFORMATION "MinGW extraction issue. You can install it manually:$\r$\n  winget install mingw"
    ${EndIf}

    ; Cleanup downloaded zip
    Delete "$TEMP\w64devkit.zip"

    mingw_done:
SectionEnd

; ============================================
; Section Descriptions
; ============================================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyro} "The Pyro compiler, REPL, formatter, and all 76 built-in modules. Required."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMinGW} "MinGW g++ C++ compiler (~200 MB installed). Needed for 'pyro run' and 'pyro build'. Recommended unless you already have g++ or clang++ installed."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; ============================================
; Uninstaller
; ============================================
Section "Uninstall"
    ; Remove files
    Delete "$INSTDIR\bin\pyro.exe"
    RMDir "$INSTDIR\bin"

    ; Remove MinGW if installed
    RMDir /r "$INSTDIR\mingw"

    ; Remove uninstaller
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

    ; Remove Start Menu shortcuts
    Delete "$SMPROGRAMS\Pyro\Pyro REPL.lnk"
    Delete "$SMPROGRAMS\Pyro\Pyro Docs (Online).lnk"
    Delete "$SMPROGRAMS\Pyro\Uninstall Pyro.lnk"
    RMDir "$SMPROGRAMS\Pyro"

    ; Remove registry keys
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro"
    DeleteRegKey HKCU "Software\Pyro"
    DeleteRegKey HKCU "Software\Classes\.ro"
    DeleteRegKey HKCU "Software\Classes\PyroSource"

    ; Remove from PATH
    ReadRegStr $0 HKCU "Environment" "Path"
    ${WordReplace} $0 ";$INSTDIR\bin" "" "+" $0
    ${WordReplace} $0 ";$INSTDIR\mingw\bin" "" "+" $0
    ${WordReplace} $0 "$INSTDIR\bin;" "" "+" $0
    ${WordReplace} $0 "$INSTDIR\mingw\bin;" "" "+" $0
    ${WordReplace} $0 "$INSTDIR\bin" "" "+" $0
    ${WordReplace} $0 "$INSTDIR\mingw\bin" "" "+" $0
    WriteRegExpandStr HKCU "Environment" "Path" "$0"

    ; Notify shell
    SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
SectionEnd

; ============================================
; Helper Functions
; ============================================

Function StrContains
    Exch $1 ; needle
    Exch
    Exch $0 ; haystack
    Push $2
    Push $3
    Push $4
    StrLen $2 $0
    StrLen $3 $1
    ${If} $3 > $2
        StrCpy $1 ""
        Goto done
    ${EndIf}
    IntOp $2 $2 - $3
    StrCpy $4 0
    loop:
        ${If} $4 > $2
            StrCpy $1 ""
            Goto done
        ${EndIf}
        StrCpy $5 $0 $3 $4
        ${If} $5 == $1
            Goto done
        ${EndIf}
        IntOp $4 $4 + 1
        Goto loop
    done:
    Pop $4
    Pop $3
    Pop $2
    Pop $0
    Exch $1
FunctionEnd
