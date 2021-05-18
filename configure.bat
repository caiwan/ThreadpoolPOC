@echo off

if not exist msvc md msvc
if exist msvc\CMakeCache.txt del msvc\CMakeCache.txt

cd msvc
cmake -G "Visual Studio 16 2019" -A x64 ../ 
cd ..
