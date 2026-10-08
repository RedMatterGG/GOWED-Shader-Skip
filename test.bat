@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0"
cl /nologo /W4 /WX /std:c++20 /EHsc /MT /O2 test_skip.cpp /Fe:test_skip.exe
if errorlevel 1 exit /b 1
test_skip.exe
exit /b %errorlevel%
