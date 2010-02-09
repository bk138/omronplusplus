!define VERSION "1.0.0-pre"

Name "Omron++ ${VERSION}"

OutFile "Omron++_${VERSION}-setup.exe"

InstallDir $PROGRAMFILES\Omron++

Page license
LicenseData COPYING.TXT

Page directory

Page instfiles

Section ""

  SetOutPath $INSTDIR

  File src\Omron++.exe
  File NEWS.TXT	
  File README.TXT

  writeUninstaller $INSTDIR\Omron++-uninstall.exe

  # now the shortcuts
  CreateDirectory "$SMPROGRAMS\Omron++"
  createShortCut  "$SMPROGRAMS\Omron++\Omron++.lnk" "$INSTDIR\Omron++.exe"
  createShortCut  "$SMPROGRAMS\Omron++\Readme.lnk" "$INSTDIR\README.TXT"
  createShortCut  "$SMPROGRAMS\Omron++\News.lnk" "$INSTDIR\NEWS.TXT"
  createShortCut  "$SMPROGRAMS\Omron++\Uninstall Omron++.lnk" "$INSTDIR\Omron++-uninstall.exe"

SectionEnd 

section "Uninstall"
 
  # Always delete uninstaller first
  delete $INSTDIR\Omron++-uninstall.exe

  # now delete installed files
  delete $INSTDIR\Omron++.exe
  delete $INSTDIR\NEWS.TXT
  delete $INSTDIR\README.TXT
  RMDir  $INSTDIR
 
  # delete shortcuts
  delete "$SMPROGRAMS\Omron++\Omron++.lnk"
  delete "$SMPROGRAMS\Omron++\Readme.lnk"
  delete "$SMPROGRAMS\Omron++\News.lnk"
  delete "$SMPROGRAMS\Omron++\Uninstall Omron++.lnk"
  RMDir  "$SMPROGRAMS\Omron++"
  
sectionEnd

Function un.onInit
    MessageBox MB_YESNO "This will uninstall Omron++. Continue?" IDYES NoAbort
      Abort ; causes uninstaller to quit.
    NoAbort:
  FunctionEnd
