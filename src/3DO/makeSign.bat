echo off
REM STEP 4 - Sign it
cd sign
3doEncrypt.exe genromtags ..\demo.iso
3doEncrypt.exe ..\demo.iso
echo 4 of 4: signed demo.iso
echo Great Success!

pause