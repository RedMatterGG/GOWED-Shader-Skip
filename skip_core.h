#pragma once
#include <windows.h>
#include <cstdint>
namespace profile {
constexpr size_t imageSize=322289664;
constexpr size_t constructorRva=0x105ef30;
constexpr size_t checkRva=0x981992e;
constexpr size_t wrapperRva=0xecf27c8;
constexpr size_t wrapperVtableRva=0xadf54d8;
constexpr size_t objectVtableRva=0xb8b0668;
constexpr unsigned char constructorBytes[]={0x48,0x83,0xec,0x38,0x4c,0x8d,0x0d,0x15,0x6c,0xdb,0x0a,0xc7,0x44,0x24,0x40,0x01,0x00,0x00,0x00,0x4c,0x8d,0x44,0x24,0x40,0xc7,0x44,0x24,0x20,0x00,0x00,0x00,0x00,0x48,0x8d,0x15,0x09,0x6d,0xdb,0x0a,0x48,0x8d,0x0d,0x6a,0x38,0xc9,0x0d,0xe8,0x85,0x21,0x0d,0x00};
constexpr unsigned char checkBytes[]={0x48,0x8b,0x05,0xa3,0x8e,0x4d,0x05,0x83,0x38,0x00,0x0f,0x8e,0xa8,0x00,0x00,0x00};
constexpr char exeSha256[]="8947c46f97467ba64f4eb8bad7b600c660bb109f2890c431b444238ba6af8e72";
}
inline bool SpanAccessible(const void* pointer,size_t size,bool write=false){
 const uintptr_t start=reinterpret_cast<uintptr_t>(pointer);
 if (!start || size==0 || start+size<start) return false;
 MEMORY_BASIC_INFORMATION info{};
 if (!VirtualQuery(pointer,&info,sizeof(info)) || info.State!=MEM_COMMIT || (info.Protect&(PAGE_GUARD|PAGE_NOACCESS))) return false;
 const DWORD mode=info.Protect&0xff;
 const bool readable=mode==PAGE_READONLY || mode==PAGE_READWRITE || mode==PAGE_WRITECOPY || mode==PAGE_EXECUTE_READ || mode==PAGE_EXECUTE_READWRITE || mode==PAGE_EXECUTE_WRITECOPY;
 const bool writable=mode==PAGE_READWRITE || mode==PAGE_WRITECOPY || mode==PAGE_EXECUTE_READWRITE || mode==PAGE_EXECUTE_WRITECOPY;
 return readable && (!write||writable) && start+size<=reinterpret_cast<uintptr_t>(info.BaseAddress)+info.RegionSize;
}
inline bool TryApplyWelcomeSkip(unsigned char* image,size_t size){
 if (!image || size!=profile::imageSize) return false;
 if (!SpanAccessible(image+profile::constructorRva,sizeof(profile::constructorBytes)) || !SpanAccessible(image+profile::checkRva,sizeof(profile::checkBytes))) return false;
 if (memcmp(image+profile::constructorRva,profile::constructorBytes,sizeof(profile::constructorBytes)) || memcmp(image+profile::checkRva,profile::checkBytes,sizeof(profile::checkBytes))) return false;
 const auto base=reinterpret_cast<uintptr_t>(image);
 const auto* wrapper=reinterpret_cast<const uintptr_t*>(image+profile::wrapperRva);
 if (!SpanAccessible(wrapper,3*sizeof(uintptr_t)) || wrapper[0]!=base+profile::wrapperVtableRva) return false;
 const uintptr_t object=wrapper[1], data=wrapper[2];
 if (!object || data!=object+0x50 || (data&3) || !SpanAccessible(reinterpret_cast<const void*>(object),0x58,true)) return false;
 if (*reinterpret_cast<const uintptr_t*>(object)!=base+profile::objectVtableRva) return false;
 auto* values=reinterpret_cast<volatile LONG*>(data);
 if ((values[0]!=0 && values[0]!=1) || (values[1]!=0 && values[1]!=1)) return false;
 InterlockedExchange(values,0);
 InterlockedExchange(values+1,0);
 return values[0]==0 && values[1]==0;
}

