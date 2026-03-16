; Pyro Programming Language - Windows Installer
; P.Y.R.O - Performance You Really Own
; Created by Aravind Pilla

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

; Version Info
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "Pyro Programming Language"
VIAddVersionKey "CompanyName" "Aravind Pilla"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2024 Aravind Pilla"
VIAddVersionKey "FileDescription" "Pyro Programming Language Installer"
VIAddVersionKey "FileVersion" "1.0.0"
VIAddVersionKey "ProductVersion" "1.0.0"

; ============================================
; Modern UI Settings
; ============================================
!define MUI_ICON "pyro.ico"
!define MUI_UNICON "pyro.ico"
!define MUI_ABORTWARNING

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "Welcome to Pyro Setup"
!define MUI_WELCOMEPAGE_TEXT "This wizard will install Pyro Programming Language on your computer.$\r$\n$\r$\nP.Y.R.O - Performance You Really Own$\r$\n$\r$\nPyro is a compiled language that transpiles to C++. Simpler than Python, 10-100x faster.$\r$\n$\r$\nThis installer includes everything you need:$\r$\n  - Pyro compiler (pyro.exe)$\r$\n  - MinGW C++ toolchain (for pyro run/build)$\r$\n$\r$\nClick Next to continue."

; Finish page
!define MUI_FINISHPAGE_TITLE "Pyro Installed Successfully!"
!define MUI_FINISHPAGE_TEXT "Pyro has been installed on your computer.$\r$\n$\r$\nTo get started, open a NEW PowerShell or Command Prompt and type:$\r$\n$\r$\n  pyro --version$\r$\n$\r$\nThen try:$\r$\n  echo print($\"Hello from Pyro!$\") > hello.ro$\r$\n  pyro run hello.ro$\r$\n$\r$\nDocs: https://aravindlabs.tech/pyro-lang/docs.html"

; Pages
!insertmacro MUI_PAGE_WELCOME
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
    ; Read current user PATH
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
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayName" "Pyro Programming Language"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayIcon" "$INSTDIR\bin\pyro.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "Publisher" "Aravind Pilla"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "DisplayVersion" "1.0.0"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pyro" "URLInfoAbout" "https://aravindlabs.tech/pyro-lang/"
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
    ; Download w64devkit - a portable, self-contained MinGW
    ; This allows pyro run/build to compile C++ code

    StrCpy $MINGW_DIR "$INSTDIR\mingw"

    DetailPrint "Downloading MinGW C++ toolchain..."
    DetailPrint "This may take a few minutes depending on your connection..."

    ; Download w64devkit (portable MinGW, ~30MB download, ~200MB extracted)
    NSISdl::download "https://github.com/skeeto/w64devkit/releases/download/v2.1.0/w64devkit-x64-2.1.0.zip" "$TEMP\w64devkit.zip"
    Pop $0
    ${If} $0 != "success"
        ; Try alternate version
        NSISdl::download "https://github.com/skeeto/w64devkit/releases/download/v2.0.0/w64devkit-x64-2.0.0.zip" "$TEMP\w64devkit.zip"
        Pop $0
        ${If} $0 != "success"
            DetailPrint "Download failed. You can install MinGW manually later."
            DetailPrint "Run: winget install mingw"
            MessageBox MB_OK|MB_ICONINFORMATION "MinGW download failed. Pyro is installed but 'pyro run' needs a C++ compiler.$\r$\n$\r$\nYou can install one later by running:$\r$\n  winget install mingw"
            Goto mingw_done
        ${EndIf}
    ${EndIf}

    DetailPrint "Extracting MinGW toolchain..."

    ; Extract using PowerShell (available on all modern Windows)
    nsExec::ExecToLog 'powershell -Command "Expand-Archive -Force -Path $\"$TEMP\w64devkit.zip$\" -DestinationPath $\"$INSTDIR$\""'

    ; w64devkit extracts to a w64devkit subfolder
    ; Rename to mingw for cleaner paths
    ${If} ${FileExists} "$INSTDIR\w64devkit\bin\g++.exe"
        Rename "$INSTDIR\w64devkit" "$MINGW_DIR"
    ${EndIf}

    ${If} ${FileExists} "$MINGW_DIR\bin\g++.exe"
        ; Add MinGW to PATH
        ReadRegStr $0 HKCU "Environment" "Path"
        WriteRegExpandStr HKCU "Environment" "Path" "$0;$MINGW_DIR\bin"

        ; Notify shell of environment change
        SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000

        DetailPrint "MinGW C++ toolchain installed!"
        DetailPrint "g++ is now available for pyro run/build"
    ${Else}
        DetailPrint "MinGW extraction may have failed."
        DetailPrint "You can install MinGW manually: winget install mingw"
        MessageBox MB_OK|MB_ICONINFORMATION "MinGW extraction issue. You can install it manually:$\r$\n  winget install mingw"
    ${EndIf}

    ; Cleanup
    Delete "$TEMP\w64devkit.zip"

    mingw_done:
SectionEnd

; ============================================
; Section Descriptions
; ============================================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPyro} "The Pyro compiler, REPL, formatter, and all 76 built-in modules. Required."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMinGW} "MinGW g++ C++ compiler. Needed for 'pyro run' and 'pyro build' commands. Recommended unless you already have a C++ compiler installed."
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
    ; Simple removal - remove our entries from PATH
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

; StrContains - check if string contains substring
; Usage:
;   Push "haystack"
;   Push "needle"
;   Call StrContains
;   Pop $0 ; $0 = "needle" if found, "" if not
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
