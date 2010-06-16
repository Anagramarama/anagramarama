;; Anagramarama.nsi - Copyright (C) 2006 Pat Thoyts
;;
;; -------------------------------------------------------------------------
;;
;; This is an NSIS installer script to create a Windows installer for
;; Anagramarama.
;;
;; This script needs to be compiled by the NullSoft installer compiler.
;;
;; -------------------------------------------------------------------------
;;
;;   Copyright (c) 2006 Pat Thoyts
;;
;;   Licensed under the Apache License, Version 2.0 (the "License");
;;   you may not use this file except in compliance with the License.
;;   You may obtain a copy of the License at
;;
;;   	http://www.apache.org/licenses/LICENSE-2.0
;;
;;   Unless required by applicable law or agreed to in writing, software
;;   distributed under the License is distributed on an "AS IS" BASIS,
;;   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;;   See the License for the specific language governing permissions and
;;   limitations under the License.
;;
;; -------------------------------------------------------------------------

!include "MUI.nsh"

!define AG_RELEASE_MAJOR 0
!define AG_RELEASE_MINOR 3
!define AG_RELEASE_PATCH 1

Name "Anagramarama"
OutFile "Anagramarama${AG_RELEASE_MAJOR}${AG_RELEASE_MINOR}.exe"
SetCompressor /solid lzma
CRCCheck on
XPStyle on
SilentInstall normal
ShowInstDetails hide
RequestExecutionLevel admin

InstallDir $PROGRAMFILES\Anagramarama
InstallDirRegKey HKLM "SOFTWARE\Anagramarama" ""
!define UninstallRegistryKey SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Anagramarama

;; -------------------------------------------------------------------------
;; Version resource
;;
ViProductVersion "${AG_RELEASE_MAJOR}.${AG_RELEASE_MINOR}.0.0"
VIAddVersionKey FileVersion ${AG_RELEASE_MAJOR}.${AG_RELEASE_MINOR}.0.${AG_RELEASE_PATCH}"
VIAddVersionKey ProductVersion ${AG_RELEASE_MAJOR}.${AG_RELEASE_MINOR}.0.0"
VIAddVersionKey ProductName "Anagramarama"
VIAddVersionKey CompanyName "Colm Gallagher"
VIAddVersionKey LegalCopyright "Copyright (c) Colm Gallagher"
VIAddVersionKey FileDescription "Anagramarama Installer"

;; -------------------------------------------------------------------------
;; Language strings
;;
LangString DESC_SecMain ${LANG_ENGLISH}  "Install program files and English resources (required)."
LangString DESC_SecMenu ${LANG_ENGLISH}  "Create Start menu items."
LangString DESC_FinishPageText ${LANG_ENGLISH} "Successfully installed Anagramarama."
LangString DESC_SecLangPt ${LANG_ENGLISH} "Include Portugese language support."

;; -------------------------------------------------------------------------
;; Interface Settings

!define MUI_ABORTWARNING

;; -------------------------------------------------------------------------
;; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gpl.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_FINISHPAGE_NOAUTOCLOSE "True"
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TEXT $(DESC_FinishPageText)
!insertmacro MUI_PAGE_FINISH
  
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_UNFINISHPAGE_NOAUTOCLOSE "True"
!insertmacro MUI_UNPAGE_FINISH

;; -------------------------------------------------------------------------
;; Installer Sections
;;
Section "Anagramara" SecMain
    SectionIn RO
    SetOutPath "$INSTDIR"
    File "Release\ag.exe"
    File "Release\SDL.dll"
    File "Release\SDL_mixer.dll"
    File "readme"
    File /r "audio"
    SetOutPath "$INSTDIR\i18n"
    File /r "i18n\en_GB"
    WriteRegStr HKLM "${UninstallRegistryKey}" "DisplayName" "Anagramarama"
    WriteRegStr HKLM "${UninstallRegistryKey}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegDWORD HKLM "${UninstallRegistryKey}" "NoModify" 1
    WriteRegDWORD HKLM "${UninstallRegistryKey}" "NoRepair" 1
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Start Menu" SecMenu
    SetOutPath "$INSTDIR"
    CreateDirectory "$SMPROGRAMS\Anagramarama"
    CreateShortCut "$SMPROGRAMS\Anagramarama\Anagramarama.lnk" "$INSTDIR\ag.exe" "" "$INSTDIR\ag.exe" 0
    CreateShortCut "$SMPROGRAMS\Anagramarama\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
SectionEnd

Section /o "Portugese" SecLangPt
    SetOutPath "$INSTDIR\i18n"
    File /r "i18n\pt_BR"
SectionEnd


;; -------------------------------------------------------------------------
;; Uninstaller Section
;;
Section "Uninstall"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir /r "$INSTDIR"
    DeleteRegKey HKLM "${UninstallRegistryKey}"
    DeleteRegKey HKLM SOFTWARE\Anagramarama
SectionEnd
 
;; -------------------------------------------------------------------------
;; Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain}   $(DESC_SecMain)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMenu}   $(DESC_SecMenu)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLangPt} $(DESC_SecLangPt)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!insertmacro MUI_LANGUAGE "English"
;;!insertmacro MUI_LANGUAGE "PortugueseBR" ;; requires pt_BR translations above

;; -------------------------------------------------------------------------
;; Initialize variables
;;
Function .onInit
FunctionEnd

