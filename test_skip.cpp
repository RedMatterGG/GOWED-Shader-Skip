#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstring>
#include "skip_core.h"
int main() {
 auto* image = static_cast<unsigned char*>(VirtualAlloc(nullptr, profile::imageSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE));
 if (!image) return 2;
 std::memcpy(image+profile::constructorRva, profile::constructorBytes, sizeof(profile::constructorBytes));
 std::memcpy(image+profile::checkRva, profile::checkBytes, sizeof(profile::checkBytes));
 auto* object=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
 auto* wrapper=reinterpret_cast<uintptr_t*>(image+profile::wrapperRva);
 wrapper[0]=reinterpret_cast<uintptr_t>(image+profile::wrapperVtableRva);
 wrapper[1]=reinterpret_cast<uintptr_t>(object);
 wrapper[2]=reinterpret_cast<uintptr_t>(object+0x50);
 *reinterpret_cast<uintptr_t*>(object)=reinterpret_cast<uintptr_t>(image+profile::objectVtableRva);
 auto* values=reinterpret_cast<volatile LONG*>(object+0x50);
 values[0]=1;values[1]=1;
 if (!TryApplyWelcomeSkip(image,profile::imageSize) || values[0]!=0 || values[1]!=0) {
  std::puts("FAIL: initialized welcome variable was not changed from [1,1] to [0,0]"); return 1;
 }
 std::puts("PASS: initialized welcome variable changed from [1,1] to [0,0]");
 VirtualFree(object,0,MEM_RELEASE);VirtualFree(image,0,MEM_RELEASE);return 0;
}
