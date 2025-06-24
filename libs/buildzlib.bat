call ..\ci_scripts\find_cmake.bat
pushd zlib
setlocal

set Platform=win32
rem set Platform=x64
rem set Platform=arm64
set Toolset=v143
set INSTALL_DIR=../zlib_vs_%Toolset%_%Platform%
set BUILD_DIR=build_vs_%Toolset%_%Platform%

"%CMAKE_COMMAND%" ^
  -B %BUILD_DIR% -S . ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
  -DZLIB_BUILD_EXAMPLES=OFF ^
  -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% ^
  -A %Platform%
pushd %BUILD_DIR%
"%CMAKE_COMMAND%" --build . --config Debug -j
"%CMAKE_COMMAND%" --build . --config Release -j
"%CMAKE_COMMAND%" --install . --config Debug
"%CMAKE_COMMAND%" --install . --config Release
popd

"%CMAKE_COMMAND%" -E copy_if_different %BUILD_DIR%/Debug/zlibd.pdb %INSTALL_DIR%/lib
"%CMAKE_COMMAND%" -E copy_if_different %BUILD_DIR%/Debug/zlibstaticd.pdb %INSTALL_DIR%/lib

endlocal
popd
