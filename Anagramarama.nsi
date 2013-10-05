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

!include "MUI2.nsh"
!include "LangFile.nsh"

!define AG_RELEASE_MAJOR 0
!define AG_RELEASE_MINOR 5
!define AG_RELEASE_PATCH 0

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
VIAddVersionKey LegalCopyright "Copyright (c) Colm Gallagher et al."
VIAddVersionKey FileDescription "Anagramarama Installer"

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

;; Specify languages supported
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
;;!insertmacro MUI_LANGUAGE "PortugueseBR"

!insertmacro MUI_RESERVEFILE_LANGDLL

;; -------------------------------------------------------------------------
;; Language strings
;;
;; NSIS stupidly doesn't actually support unicode or utf-8 input files so we
;; will have trouble with characters than cannot be represented in cp1252 (like
;; russian). It looks like you might include a separate language file using
;; !insertmacro LANGFILE_INCLUDE "ru.nsh" but something was failing when I
;; tried this. In the meantime just plug them straight in here...
;;
;; English ------
LangString DESC_SecMain ${LANG_ENGLISH}  "Install program files and English resources (required)."
LangString DESC_SecMenu ${LANG_ENGLISH}  "Create Start menu items."
LangString DESC_FinishPageText ${LANG_ENGLISH} "Successfully installed Anagramarama."
LangString DESC_SecLangFr ${LANG_ENGLISH} "Include French language support."
LangString DESC_SecLangPtBr ${LANG_ENGLISH} "Include Brazilian language support."
;; French -------
LangString DESC_SecMain ${LANG_FRENCH}  "Les fichiers programme d'installation et de ressources en Anglais (obligatoire)."
LangString DESC_SecMenu ${LANG_FRENCH}  "Créer des éléments du menu Démarrer."
LangString DESC_FinishPageText ${LANG_FRENCH} "Installé avec succès Anagramarama."
LangString DESC_SecLangFr ${LANG_FRENCH} "Inclure le soutien de langue Française."
LangString DESC_SecLangPtBr ${LANG_FRENCH} "Inclure le soutien de langue Brésilienne."


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

Section /o "French" SecLangFr
    SetOutPath "$INSTDIR\i18n"
    File /r "i18n\fr"
SectionEnd

Section /o "Brazilian" SecLangPtBr
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
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLangFr} $(DESC_SecLangFr)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLangPtBr} $(DESC_SecLangPtBr)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;; -------------------------------------------------------------------------
;; Initialize variables
;;
Function .onInit
  ;; Language selection.
  ;;!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

