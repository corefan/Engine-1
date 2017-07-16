if not exist "build" mkdir build
cd build
if not exist "vs-clang" mkdir vs-clang
cd vs-clang
cmake ..\.. -G "Visual Studio 14 2015 Win64" -T "LLVM-vs2014"
pause
