// Resolve a native UI-frame context independently of Auto Loot. Chain the
// current frame call target so existing inventory mods retain their hooks.
namespace native_runtime {
using Loop=void(__fastcall*)(void*);using Player=uint32_t(__fastcall*)(void*);using World=void*(__fastcall*)(void*);
static Loop previous=nullptr;static Player playerId=nullptr;static World unwrap=nullptr;
static size_t appUi=0,uiClient=0,clientWorld=0,clientPlayer=0;
static uint8_t* callSite=nullptr;static int32_t oldDisplacement=0,newDisplacement=0;
static void Pump(void* app);
static void Restore();
static uint64_t profileDeadline=0,profileCount=0;static LONGLONG profileTotal=0,profileMaximum=0,profileFrequency=0;
static LONGLONG stages[7]{},stageStart=0;
static void Stage(unsigned i){if(!profileDeadline)return;LARGE_INTEGER t;QueryPerformanceCounter(&t);stages[i]+=t.QuadPart-stageStart;stageStart=t.QuadPart;}
static void __fastcall Frame(void* app){
 previous(app);LARGE_INTEGER begin{},end{};if(profileDeadline){QueryPerformanceCounter(&begin);stageStart=begin.QuadPart;}
 __try{Pump(app);}__except(EXCEPTION_EXECUTE_HANDLER){ecs=nullptr;localPlayer=0;Trace("AUTO_SALVAGE_CONTEXT_EXCEPTION no_request=1");}
 if(!profileDeadline)return;QueryPerformanceCounter(&end);auto elapsed=end.QuadPart-begin.QuadPart;
 ++profileCount;profileTotal+=elapsed;profileMaximum=(std::max)(profileMaximum,elapsed);
 if(GetTickCount64()>=profileDeadline||elapsed>profileFrequency/125){
  Restore();profileDeadline=0;char line[240];sprintf_s(line,"CONTEXT_PROFILE frames=%llu average_us=%.2f maximum_us=%.2f hook_restored=1",profileCount,1000000.0*profileTotal/profileFrequency/profileCount,1000000.0*profileMaximum/profileFrequency);Trace(line);
  sprintf_s(line,"CONTEXT_STAGES_US app=%.1f ui=%.1f client=%.1f wrapper=%.1f world=%.1f player=%.1f rules=%.1f",stages[0]*1000000.0/profileFrequency,stages[1]*1000000.0/profileFrequency,stages[2]*1000000.0/profileFrequency,stages[3]*1000000.0/profileFrequency,stages[4]*1000000.0/profileFrequency,stages[5]*1000000.0/profileFrequency,stages[6]*1000000.0/profileFrequency);Trace(line);
 }
}
static void* Relative(uint8_t* call){return call+5+Read<int32_t>(call,1);}
static bool Executable(const void* address){MEMORY_BASIC_INFORMATION m{};return VirtualQuery(address,&m,sizeof m)&&m.State==MEM_COMMIT&&!(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))&&(m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY));}
static uint8_t* Near(void* target){SYSTEM_INFO info{};GetSystemInfo(&info);auto gran=uintptr_t(info.dwAllocationGranularity);auto center=uintptr_t(target)&~(gran-1);for(uintptr_t offset=gran;offset<0x70000000;offset+=gran){if(center>=offset){auto p=(uint8_t*)VirtualAlloc((void*)(center-offset),4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(p)return p;}auto p=(uint8_t*)VirtualAlloc((void*)(center+offset),4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(p)return p;}return nullptr;}
static bool Install(uint8_t* text,size_t size){
 auto app=Unique(text,size,"48 8B 8F ?? ?? ?? ?? 48 85 C9 74 08 48 8B D6 E8 ?? ?? ?? ?? 49 83 C6 08");
 auto ui=Unique(text,size,"48 8B 83 ?? ?? ?? ?? C6 83 ?? ?? ?? ?? 01 C6 83 ?? ?? ?? ?? 00 48 85 C0 74 13 48 8B 88 ?? ?? ?? ?? 48 85 C9");
 auto world=Unique(text,size,"48 8B 8F ?? ?? ?? ?? E8 ?? ?? ?? ?? 44 0F B7 0D ?? ?? ?? ?? 41 B8 28 00 00 00 8B 15 ?? ?? ?? ?? 48 8B C8 E8 ?? ?? ?? ?? 4C 8B F0");
 auto player=Unique(text,size,"48 8D 8F ?? ?? ?? ?? E8 ?? ?? ?? ?? 41 89 46 08 48 8B 4E 10 49 89 4E 10");
 auto frame=Unique(text,size,"48 8B CE E8 ?? ?? ?? ?? 48 8B 86 ?? ?? ?? ?? 48 85 C0 74 54 48 8B 80 ?? ?? ?? ??");
 auto component=Unique(text,size,"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8D BA ?? ?? ?? ?? 44 89 44 24 40 48 8B F1 48 8D 54 24 40 48 8B 0F 41 8B D9 48 81 C1 88 00 00 00 E8");
 if(!app||!ui||!world||!player||!frame||!component){Trace("NATIVE_RUNTIME_UNSUPPORTED no_frame_hook=1");return false;}
 appUi=Read<uint32_t>(app,3);uiClient=Read<uint32_t>(ui,3);clientWorld=Read<uint32_t>(world,3);clientPlayer=Read<uint32_t>(player,3);
 playerId=(Player)Relative(player+7);unwrap=(World)Relative(world+7);
 if(appUi>0x100000||uiClient>0x10000||clientWorld>0x1000000||clientPlayer>0x1000000||!Executable(playerId)||!Executable(unwrap)||memcmp((void*)unwrap,"\x48\x8b\x41\x08\xc3",5))return false;
 reader=(GetFn)component;auto call=frame+3;previous=(Loop)Relative(call);if(!Executable(previous))return false;
 auto stub=Near(call);if(!stub)return false;const uint8_t jump[]={0xff,0x25,0,0,0,0};memcpy(stub,jump,6);auto function=uintptr_t(&Frame);memcpy(stub+6,&function,8);auto delta=stub-(call+5);if(delta<INT32_MIN||delta>INT32_MAX)return false;
 DWORD old;if(!VirtualProtect(stub,4096,PAGE_EXECUTE_READ,&old))return false;FlushInstructionCache(GetCurrentProcess(),stub,14);
 if(!VirtualProtect(call,5,PAGE_EXECUTE_READWRITE,&old))return false;oldDisplacement=Read<int32_t>(call,1);newDisplacement=int32_t(delta);memcpy(call+1,&newDisplacement,4);DWORD ignored;VirtualProtect(call,5,old,&ignored);FlushInstructionCache(GetCurrentProcess(),call,5);callSite=call;
 auto duration=GetPrivateProfileIntW(L"QuickSalvage",L"ContextProfileMs",0,configPath.c_str());if(duration){LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);profileFrequency=frequency.QuadPart;profileDeadline=GetTickCount64()+(std::min)(duration,3000u);}
 Trace("NATIVE_RUNTIME_READY independent_context=1 existing_frame_target_chained=1");return true;
}
static void Restore(){if(!callSite||Read<int32_t>(callSite,1)!=newDisplacement)return;DWORD old;if(VirtualProtect(callSite,5,PAGE_EXECUTE_READWRITE,&old)){memcpy(callSite+1,&oldDisplacement,4);DWORD ignored;VirtualProtect(callSite,5,old,&ignored);FlushInstructionCache(GetCurrentProcess(),callSite,5);}}
}
