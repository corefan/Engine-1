if not exist "build" mkdir build
cd build
if not exist "vs2015" mkdir vs2015
cd vs2015
cmake ..\.. -G "Visual Studio 14 2015 Win64"
pause
