#include "quick_salvage.cpp"
#include <cassert>
#include <iostream>
#include <crtdbg.h>
static uint8_t setup[84]{},contents[96]{},metadata[20]{};
static unsigned requests=0;static bool nativeAllows=true;static uint64_t requestedSlot=0;
static void Put(void* p,size_t offset,uint32_t value){memcpy((uint8_t*)p+offset,&value,4);}
static void* __fastcall Getter(void* out,void*,uint32_t hash,uint32_t id){
 void* value=nullptr;if(hash==0xc24df82a&&id==1)value=setup;
 if(hash==0xb19528a9&&id==100)value=contents;if(hash==0x3a2fac75&&id==301)value=metadata;
 uintptr_t auxiliary=0;memcpy(out,&value,8);memcpy((uint8_t*)out+8,&auxiliary,8);return out;
}
static bool __fastcall Allows(const void* view){return nativeAllows&&Read<uint32_t>(view,8)==456&&Read<uint32_t>(view,20)==301;}
static bool __fastcall Queue(void* root,uint64_t slot){assert(root==(void*)11);++requests;requestedSlot=slot;return true;}
static salvage::Candidate Item(){salvage::Candidate c;c.session=1;c.page=100;c.slot=2;c.pide=301;c.count=1;c.key={456,2,12};return c;}
static void Attempt(bool succeeds){native_runtime::queuedItem=Item();native_runtime::queuedState=1;auto before=requests;native_runtime::CommitFromUi();assert(native_runtime::queuedState==(succeeds?3u:4u));assert(requests==before+unsigned(succeeds));native_runtime::CommitFromUi();assert(requests==before+unsigned(succeeds));}
int main(){
 _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
 wchar_t path[32768]{};assert(GetFullPathNameW(L"automatic-test.ini",32768,path,nullptr));configPath=path;
 assert(GetFullPathNameW(L"automatic-test.log",32768,path,nullptr));logPath=path;
 assert(WritePrivateProfileStringW(L"QuickSalvage",L"Enabled",L"1",configPath.c_str()));
 ecs=(void*)1;localPlayer=1;reader=Getter;native_items::canSalvage=Allows;native_salvage::queue=Queue;
 void* root=(void*)11;native_salvage::appSlot=&root;
 Put(setup,0,100);setup[64]=4;setup[80]=1;setup[83]=1;
 Put(contents,24,456);Put(contents,28,1);Put(contents,32,301);
 Put(metadata,0,100);metadata[4]=2;Put(metadata,8,456);metadata[16]=2;metadata[17]=12;
 native_menu::store.rules.Add({456,2,12},"Fixture");Attempt(true);assert(requestedSlot==(100ull|(2ull<<32)));
 metadata[14]=8;Attempt(false);metadata[14]=0;
 Put(contents,28,2);Attempt(false);Put(contents,28,1);
 native_menu::store.rules.Remove({456,2,12});Attempt(false);native_menu::store.rules.Add({456,2,12},"Fixture");
 metadata[16]=3;Attempt(false);metadata[16]=2;metadata[17]=13;Attempt(false);metadata[17]=12;
 root=nullptr;Attempt(false);root=(void*)11;ecs=(void*)2;Attempt(false);ecs=(void*)1;
 nativeAllows=false;Attempt(false);nativeAllows=true;
 WritePrivateProfileStringW(L"QuickSalvage",L"Enabled",L"0",configPath.c_str());Attempt(false);
 DeleteFileW(configPath.c_str());DeleteFileW(logPath.c_str());
 std::cout<<"PASS: native UI handoff, one request, equipped/stale/changed rule/rarity/level/world/root/native-denial/disabled cancellation (mock only)\n";
}
