@echo off

rem --- https://github.com/TheLartians/CPM.cmake#cpm_source_cache
rem --- normally ~/.cache/CPM under *nix systems
REM set CPM_SOURCE_CACHE=%Temp%\CPM\grafkit\
REM echo %CPM_SOURCE_CACHE%
REM if not exist %CPM_SOURCE_CACHE% md %CPM_SOURCE_CACHE%
rem --- this does not work.

if not exist msvc md msvc
if exist msvc\CMakeCache.txt del msvc\CMakeCache.txt

FOR /F "tokens=* USEBACKQ" %%F IN (`git describe`) DO ( SET git_describe=%%F )
echo "GIT TOKENS = %git_describe%"

cd msvc
cmake -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%;%QT5_DIR% -DCMAKE_INSTALL_PREFIX=../deploy/%git_describe% -DCMAKE_GENERATOR_PLATFORM=x64 ../
cmake --build ./ --target ExternalDependencies --config Debug
cmake --build ./ --target ExternalDependencies --config MinSizeRel
cd ..
