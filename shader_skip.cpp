#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "skip_core.h"
#include "proxy_exports.h"
#pragma comment(lib,"bcrypt.lib")
extern "C" { uintptr_t mProcs[exportCount]{}; }
static INIT_ONCE forwardOnce=INIT_ONCE_STATIC_INIT;
static HMODULE thisModule;
static wchar_t logPath[MAX_PATH];
static ULONGLONG began;
static void Log(const char* message){
 HANDLE file=CreateFileW(logPath,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return;
 char line[512];const int count=snprintf(line,sizeof(line),"[%llu ms] %s\r\n",GetTickCount64()-began,message);
 if(count>0 && count<static_cast<int>(sizeof(line))){DWORD done;WriteFile(file,line,static_cast<DWORD>(count),&done,nullptr);}CloseHandle(file);
}
static BOOL CALLBACK InitializeForwarders(PINIT_ONCE,void*,void**){
 wchar_t path[MAX_PATH];const UINT count=GetSystemDirectoryW(path,MAX_PATH);
 if(!count || count>MAX_PATH-12)return FALSE;
 wcscat_s(path,L"\\dwmapi.dll");
 HMODULE real=LoadLibraryExW(path,nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
 if(!real)return FALSE;
 for(size_t i=0;i<exportCount;++i){
  FARPROC function=GetProcAddress(real,exportNames[i]?exportNames[i]:MAKEINTRESOURCEA(exportOrdinals[i]));
  if(!function)return FALSE;
  mProcs[i]=reinterpret_cast<uintptr_t>(function);
 }
 return TRUE;
}
extern "C" void EnsureForwarders(){
 if(!InitOnceExecuteOnce(&forwardOnce,InitializeForwarders,nullptr,nullptr))RaiseFailFastException(nullptr,nullptr,0);
}
static bool SupportedExecutable(){
 wchar_t path[MAX_PATH];if(!GetModuleFileNameW(nullptr,path,MAX_PATH))return false;
 const wchar_t* name=wcsrchr(path,L'\\');if(!name || _wcsicmp(name+1,L"GoWEDay-Steam.exe"))return false;
 HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
 if(file==INVALID_HANDLE_VALUE)return false;
 BCRYPT_ALG_HANDLE alg=nullptr;BCRYPT_HASH_HANDLE hash=nullptr;unsigned char digest[32],buffer[65536];
 bool ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
 if(ok)ok=BCryptCreateHash(alg,&hash,nullptr,0,nullptr,0,0)>=0;
 DWORD count=0;
 while(ok){if(!ReadFile(file,buffer,sizeof(buffer),&count,nullptr)){ok=false;break;}if(!count)break;ok=BCryptHashData(hash,buffer,count,0)>=0;}
 if(ok)ok=BCryptFinishHash(hash,digest,sizeof(digest),0)>=0;
 if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);CloseHandle(file);
 if(!ok)return false;
 char hex[65];for(size_t i=0;i<32;++i)snprintf(hex+2*i,3,"%02x",digest[i]);
 return strcmp(hex,profile::exeSha256)==0;
}
static DWORD WINAPI Run(void*){
 HMODULE pinned=nullptr;
 GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Run),&pinned);
 began=GetTickCount64();GetModuleFileNameW(thisModule,logPath,MAX_PATH);wchar_t* slash=wcsrchr(logPath,L'\\');if(!slash)return 0;wcscpy_s(slash+1,MAX_PATH-static_cast<size_t>(slash+1-logPath),L"ShaderSkip.log");
 EnsureForwarders();
 Log("Dedicated Gears startup shader-wait bypass loaded; no UE4SS runtime.");
 wchar_t disabled[MAX_PATH];wcscpy_s(disabled,logPath);wchar_t* end=wcsrchr(disabled,L'\\');wcscpy_s(end+1,MAX_PATH-static_cast<size_t>(end+1-disabled),L"ShaderSkip.disabled");
 if(GetFileAttributesW(disabled)!=INVALID_FILE_ATTRIBUTES){Log("Disabled by ShaderSkip.disabled; forwarding only.");return 0;}
 if(!SupportedExecutable()){Log("Unsupported executable name/hash; refusing all game-memory writes.");return 0;}
 Log("Executable SHA256 verified. Waiting for native variable initialization.");
 auto* image=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
 const ULONGLONG deadline=GetTickCount64()+60000;
 while(GetTickCount64()<deadline){
  if(TryApplyWelcomeSkip(image,profile::imageSize)){Log("SUCCESS: TC.ShaderCompileBlock.Welcome values set to [0,0]; no code patch or PSO-cache disabling.");return 0;}
  Sleep(1);
 }
 Log("Timed out: signature/layout/initialization guard not satisfied; no game-memory writes.");return 0;
}
BOOL WINAPI DllMain(HMODULE module,DWORD reason,void*){
 if(reason==DLL_PROCESS_ATTACH){thisModule=module;DisableThreadLibraryCalls(module);HANDLE thread=CreateThread(nullptr,0,Run,nullptr,0,nullptr);if(thread)CloseHandle(thread);}
 return TRUE;
}
