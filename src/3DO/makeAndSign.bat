echo off
cd demo
REM STEP 1 - Compile
dmake

REM STEP 2 - Create Directory Structure
copy LaunchMe ..\takeme /Y
echo 2 of 4: ISO template done.

REM STEP 3 - covert to Opera file system ISO
cd..
operaFS.ahk
echo 3 of 4: ISO file system done

REM STEP 4 - Sign it
cd sign
3doEncrypt.exe genromtags ..\demo.iso
3doEncrypt.exe ..\demo.iso
echo 4 of 4: signed demo.iso
echo Great Success!

pause