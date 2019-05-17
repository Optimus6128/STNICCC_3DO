echo off
REM STEP 4 - Sign it
cd tools\sign
3doEncrypt.exe genromtags ..\..\demo.iso
echo 4 of 4: signed demo.iso
echo Great Success!

pause