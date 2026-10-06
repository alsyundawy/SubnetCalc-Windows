!ifndef ARCH
  !define ARCH "x64"
!endif

Unicode True
RequestExecutionLevel admin

!define PRODUCT_NAME "SubnetCalc"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Harry Dertin Sutisna Alsyundawy"
!define PRODUCT_WEB_SITE "https://github.com/alsyundawy/SubnetCalc-Windows"

!if "${ARCH}" == "x64"
  !define ARCH_NAME "x64"
  !define EXE_NAME "SubnetCalc-x64.exe"
  !define OUTFILE_NAME "../dist/SubnetCalc-Windows-x64-setup.exe"
  !define DEFAULT_INST_DIR "$PROGRAMFILES64\SubnetCalc"
!else
  !define ARCH_NAME "x86"
  !define EXE_NAME "SubnetCalc-x86.exe"
  !define OUTFILE_NAME "../dist/SubnetCalc-Windows-x86-setup.exe"
  !define DEFAULT_INST_DIR "$PROGRAMFILES\SubnetCalc"
!endif

Name "${PRODUCT_NAME} (${ARCH_NAME})"
OutFile "${OUTFILE_NAME}"
InstallDir "${DEFAULT_INST_DIR}"
InstallDirRegKey HKLM "Software\SubnetCalc" "Install_Dir"

; Supported OS GUIDs: Win7, Win8, Win8.1, Win10/11
ManifestSupportedOS {35138b9a-5d96-4fbd-8e2d-a2440225f93a}
ManifestSupportedOS {4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}
ManifestSupportedOS {1f676c76-80e1-4239-95bb-83d0f6d0da78}
ManifestSupportedOS {8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "../dist/${EXE_NAME}"
  File "../LICENSE"
  File "../dist/WIN7.txt"

  ; Create Start Menu Shortcuts
  CreateDirectory "$SMPROGRAMS\SubnetCalc"
  CreateShortcut "$SMPROGRAMS\SubnetCalc\SubnetCalc.lnk" "$INSTDIR\${EXE_NAME}"
  CreateShortcut "$SMPROGRAMS\SubnetCalc\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  ; Write Uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write Registry for Add/Remove Programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "DisplayName" "${PRODUCT_NAME} (${ARCH_NAME})"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "DisplayIcon" '"$INSTDIR\${EXE_NAME}"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\${EXE_NAME}"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\WIN7.txt"
  Delete "$INSTDIR\uninstall.exe"

  Delete "$SMPROGRAMS\SubnetCalc\SubnetCalc.lnk"
  Delete "$SMPROGRAMS\SubnetCalc\Uninstall.lnk"
  RMDir "$SMPROGRAMS\SubnetCalc"

  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SubnetCalc_${ARCH_NAME}"
  DeleteRegKey HKLM "Software\SubnetCalc"
SectionEnd
