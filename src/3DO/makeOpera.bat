echo off
cd src
REM STEP 1 - Compile
dmake

REM STEP 2 - Create Directory Structure
copy LaunchMe ..\CD /Y
echo 2 of 4: ISO template done.

REM STEP 3 - covert to Opera file system ISO
cd..
tools\opera\3doiso.exe -in CD -out demo.iso
echo 3 of 4: ISO file system done

pause