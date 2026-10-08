#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cwchar>
int wmain(){
 HMODULE proxy=LoadLibraryW(L".\\dwmapi.dll");if(!proxy){std::printf("FAIL loading proxy: %lu\n",GetLastError());return 1;}
 wchar_t path[MAX_PATH];GetSystemDirectoryW(path,MAX_PATH);wcscat_s(path,L"\\dwmapi.dll");
 HMODULE real=LoadLibraryW(path);if(!real || real==proxy)return 2;
 using Query=HRESULT(WINAPI*)(BOOL*);
 auto p=reinterpret_cast<Query>(GetProcAddress(proxy,"DwmIsCompositionEnabled"));auto r=reinterpret_cast<Query>(GetProcAddress(real,"DwmIsCompositionEnabled"));
 BOOL pv=FALSE,rv=FALSE;if(!p || !r || p(&pv)!=r(&rv) || pv!=rv){std::puts("FAIL proxy forwarding mismatch");return 3;}
 Sleep(200);
 std::puts("PASS: real Windows DwmIsCompositionEnabled result preserved through proxy.");
 return 0;
}
