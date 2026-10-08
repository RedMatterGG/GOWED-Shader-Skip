@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
cd /d "%~dp0"
cl /nologo /W4 /WX /std:c++20 /EHsc /MT /O2 test_skip.cpp /Fe:test_skip.exe
if errorlevel 1 exit /b 1
test_skip.exe
if errorlevel 1 exit /b 1
ml64 /nologo /c /Fo dwmapi_asm.obj dwmapi.asm
if errorlevel 1 exit /b 1
cl /nologo /W4 /WX /std:c++20 /EHsc /MT /O2 /LD shader_skip.cpp dwmapi_asm.obj /link /DEF:dwmapi.def /OUT:dwmapi.dll /INCREMENTAL:NO
if errorlevel 1 exit /b 1
cl /nologo /W4 /WX /std:c++20 /EHsc /MT /O2 test_proxy.cpp /Fe:test_proxy.exe
if errorlevel 1 exit /b 1
test_proxy.exe
exit /b %errorlevel%
