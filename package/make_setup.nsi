;--------------------------------
; Modern UI
!include "MUI2.nsh"
!include "x64.nsh"
!include "nsProcess.nsh"
!include "proj_version.nsh"

Unicode true
RequestExecutionLevel admin

;--------------------------------
; App Info
!define PRODUCT_NAME "GammaRay"
!define APPNAME "GammaRay"
!define COMPANY "GammaRay"
!define INSTALL_DIR "C:\Program Files\GammaRay\${APPNAME}"

!define BUILD_PATH "app"

OutFile "${PRODUCT_NAME}_${PRODUCT_VERSION}_Setup.exe"

InstallDir "${INSTALL_DIR}"

Name "${PRODUCT_NAME}"

;--------------------------------
!define MUI_ICON "image\logo.ico"
!define MUI_UNICON "image\uninstall.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "image\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "image\welcome.bmp"

!define MUI_ABORTWARNING

;--------------------------------
; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "SimpChinese"

;--------------------------------
; Sections
Section "安装主程序" SecMain

    SetOutPath "$INSTDIR"

    ; 1. 解压 app.7z
    File "app\app.7z"
    Nsis7z::ExtractWithCallback "$INSTDIR\app.7z" $R9
    Delete "$INSTDIR\app.7z"
	
	; 2. 

    ; 3. 创建快捷方式
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${APPNAME}.exe"
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${APPNAME}.exe"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\卸载.lnk" "$INSTDIR\Uninstall.exe"

    ; 4. 注册表信息（控制面板卸载显示）
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}" "Publisher" "${COMPANY}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}" "DisplayVersion" "${PRODUCT_VERSION}"

    ; 设置程序为管理员运行
    WriteRegStr HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$INSTDIR\${APPNAME}.exe" "RUNASADMIN"

    ; 创建卸载程序
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
; Uninstaller
Section "Uninstall"
    ; 删除文件
    RMDir /r "$INSTDIR"

    ; 删除快捷方式
    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\*.lnk"
    RMDir "$SMPROGRAMS\${PRODUCT_NAME}"

    ; 删除注册表
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY} ${APPNAME}"
    DeleteRegKey HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers\$INSTDIR\${APPNAME}.exe"

SectionEnd

;--------------------------------
Function .onInit
    ; 检查旧版本是否运行
    ${nsProcess::FindProcess} "${APPNAME}.exe" $R0
    ${If} $R0 == 0
        MessageBox MB_OK "检测到程序正在运行，将自动关闭。"
        Call StopAndDeleteService
		Call KillProcesses
    ${EndIf}
FunctionEnd

Function un.onInit
    Call un.StopAndDeleteService
    Call un.KillProcesses
FunctionEnd

Function LaunchLink
    ExecShell "" "$INSTDIR\${APPNAME}.exe"
FunctionEnd

Function StopAndDeleteService
    nsExec::ExecToLog 'sc stop "GammaRayService"'
    nsExec::ExecToLog 'sc delete "GammaRayService"'
FunctionEnd

Function un.StopAndDeleteService
    nsExec::ExecToLog 'sc stop "GammaRayService"'
    nsExec::ExecToLog 'sc delete "GammaRayService"'
FunctionEnd


Function KillProcesses
    nsExec::ExecToLog 'taskkill /F /T /IM GammaRay.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayClientInner.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayGuard.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayRender.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayService.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayServiceManager.exe'
FunctionEnd

Function un.KillProcesses
    nsExec::ExecToLog 'taskkill /F /T /IM GammaRay.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayClientInner.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayGuard.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayRender.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayService.exe'
	nsExec::ExecToLog 'taskkill /F /T /IM GammaRayServiceManager.exe'
FunctionEnd