// Native item context-menu extension. Development preview is opt-in; this file
// is not deployed by the restore adapter. No item actions are fabricated.
#include "rule_store.h"
namespace native_menu {
using MenuFn=bool(__fastcall*)(uint8_t*);
using RowFn=bool(__fastcall*)(uint8_t*,uint8_t*);
using GroupFn=void*(__fastcall*)(void*,const void*,float);
using EndFn=void(__fastcall*)(void*);
using LayoutFn=void(__fastcall*)(void*,const void*,bool);
static MenuFn previousMenu=nullptr;static RowFn previousRow=nullptr;
static GroupFn group=nullptr;static EndFn endGroup=nullptr;
static LayoutFn setLayout=nullptr;
static uint8_t* menuEntry=nullptr;static uint8_t* rowEntry=nullptr;
static uint8_t menuSaved[15],rowSaved[19];
static void* inventoryState=nullptr;static uint8_t itemView[72]{};
static unsigned clicked=0;static bool editing=false;static size_t rulePage=0;
static uint64_t menuSerial=0,diagnosedSerial=0;
static std::vector<std::string> labels;
static salvage::RuleStore store(L"");
static constexpr uint64_t marker=0x51534d454e550000ull;
static const char autoLabel[]="Auto Salvage",listLabel[]="...";
static uint64_t Hash(const char* s,size_t n){uint64_t h=0xcbf29ce484222325ull;for(size_t i=0;i<n;++i)h=(h^uint8_t(s[i]))*0x100000001b3ull;return h;}
struct UiText{const char* text;uint64_t length,hash;};
static UiText Text(const char* s){return {s,strlen(s),Hash(s,strlen(s))};}
static void SetText(uint8_t* descriptor,const char* s){auto t=Text(s);memcpy(descriptor+32,&t,sizeof t);}
static void Descriptor(uint8_t* out,const uint8_t* original,unsigned command,const char* label){
 memcpy(out,original,72);out[0]=91;out[1]=1;out[0x19]=0;out[0x1a]=2;out[0x1b]=0;
 memset(out+2,0,23);SetText(out,label);auto id=marker|command;memcpy(out+64,&id,8);
}
static bool Custom(const uint8_t* a){return a[0]==91&&(Read<uint64_t>(a,64)&~0xffffull)==marker;}
static bool __fastcall Row(uint8_t* a,uint8_t* parameters){
 // Arguments are supplied directly by the native renderer. Avoid expensive
 // VirtualQuery calls on every ordinary menu row.
 if(!a||!Custom(a))return previousRow(a,parameters);
 auto command=unsigned(Read<uint64_t>(a,64)&0xffff);
 uint8_t safeParameters[64];memcpy(safeParameters,parameters,64);safeParameters[0xd]=0;
 if(command!=1){bool hit=previousRow(a,safeParameters);if(hit)clicked=command;return hit;}
 // A horizontal group keeps the list button beside Auto Salvage. Native UI
 // context nesting supplies separate mouse targets to the two native buttons.
 alignas(16) uint8_t container[256]{};auto name=Text("quickSalvageRow");group(container,&name,0.0f);
 const float spacing[2]={8.0f,0.0f};setLayout(Read<void*>(container,8),spacing,false);
 uint8_t mainParameters[64];memcpy(mainParameters,safeParameters,64);float width=184;memcpy(mainParameters+4,&width,4);
 bool mainHit=previousRow(a,mainParameters);if(mainHit){clicked=1;Trace("AUTO_SALVAGE_BUTTON_CLICK");}
 uint8_t button[72];Descriptor(button,a,2,listLabel);
 uint8_t buttonParameters[64];memcpy(buttonParameters,safeParameters,64);width=80;memcpy(buttonParameters+4,&width,4);
 // Native standaloneSmall uses the compact settings-button texture family,
 // rather than squeezing the ornate full-size context button into a square.
 uint32_t style=4;memcpy(buttonParameters,&style,4);
 bool listHit=previousRow(button,buttonParameters);if(listHit)clicked=2;
 endGroup(container);return mainHit||listHit;
}
static bool preview=false;
static bool Preview(){return preview;}
static bool Render(uint8_t* model){
 if(!Preview()||!model||!inventoryState)return previousMenu(model);
 auto nativeList=Read<uint8_t*>(model,0x90);if(nativeList!=(uint8_t*)inventoryState+0x248)return previousMenu(model);
 auto count=Read<uint8_t>(nativeList,0x480);if(count>16)return previousMenu(model);
 uint8_t* salvageAction=nullptr;for(unsigned i=0;i<count;++i)if(nativeList[i*72]==66&&nativeList[i*72+1])salvageAction=nativeList+i*72;
 if(!salvageAction)return previousMenu(model);
 if(diagnosedSerial!=menuSerial){diagnosedSerial=menuSerial;char message[220];sprintf_s(message,"AUTO_SALVAGE_MENU_TARGET item=%u page=%u slot=%u selected=%llu captured=%llu salvage_action=%u",Read<uint32_t>(itemView,8),Read<uint32_t>(itemView),itemView[4],Read<uint64_t>(inventoryState,0xb94),Read<uint64_t>(itemView),salvageAction!=nullptr);Trace(message);}
 if(Read<uint64_t>(inventoryState,0xb94)!=Read<uint64_t>(itemView))return previousMenu(model);
 alignas(16) uint8_t list[0x488]{};clicked=0;auto rules=store.rules.List();
 if(!editing){
  if(count>=16)return previousMenu(model);memcpy(list,nativeList,0x481);Descriptor(list+count*72,salvageAction,1,autoLabel);list[0x480]=uint8_t(count+1);
 }else{
  labels.clear();labels.reserve(14);if(rulePage*10>=rules.size())rulePage=0;
  for(size_t i=rulePage*10;i<rules.size()&&i<(rulePage+1)*10;++i){const auto& r=rules[i];labels.push_back("Remove: "+r.label+" [rarity "+std::to_string(r.key.rarity)+", Lv "+std::to_string(r.key.level)+"]");Descriptor(list+list[0x480]*72,salvageAction,100+unsigned(i),labels.back().c_str());++list[0x480];}
  if(rules.empty()){Descriptor(list,salvageAction,0,"No Auto Salvage rules");list[1]=0;list[0x480]=1;}
  if(rulePage){Descriptor(list+list[0x480]*72,salvageAction,3,"Previous page");++list[0x480];}
  if((rulePage+1)*10<rules.size()){Descriptor(list+list[0x480]*72,salvageAction,4,"Next page");++list[0x480];}
  Descriptor(list+list[0x480]*72,salvageAction,5,"Back");++list[0x480];
 }
 memcpy(model+0x90,&(nativeList=list),8);bool result=previousMenu(model);
 auto original=(uint8_t*)inventoryState+0x248;memcpy(model+0x90,&original,8);
 // Preview records gestures only. Rule activation stays disabled until the
 // exact target, menu layout and native salvage transfer have been tested.
 if(clicked==1){
  if(!native_salvage::Enabled()){Trace("AUTO_SALVAGE_MENU_PREVIEW selected=1 rules_unchanged=1");return true;}
  // Validate ownership and current replicated metadata only when committing
  // a rule, not while repeatedly drawing an unchanged menu.
  auto candidate=native_salvage::Candidate(itemView,true,true);if(!salvage::Eligible(candidate)){char message[240];sprintf_s(message,"AUTO_SALVAGE_RULE_BLOCKED item=%u rarity=%u level=%u owned=%u equipped=%u metadata=%u count=%u page=%u pide=%u",candidate.key.item,candidate.key.rarity,candidate.key.level,candidate.owned,candidate.equipped,candidate.metadataValid,candidate.count,candidate.page,candidate.pide);Trace(message);return true;}
  if(store.rules.Contains(candidate.key)){Trace("AUTO_SALVAGE_RULE_ALREADY_EXISTS");return true;}
  bool saved=store.Add(candidate.key,ItemName(candidate.key.item));Trace(saved?"AUTO_SALVAGE_RULE_SAVED exact_type_rarity_level=1":"AUTO_SALVAGE_RULE_SAVE_FAILED rules_unchanged=1");return saved;
 }
 if(clicked==2){if(OpenRulesPanel())return true;editing=true;return false;}
 if(clicked==3&&rulePage){--rulePage;return false;}
 if(clicked==4){++rulePage;return false;}
 if(clicked==5){editing=false;return false;}
 if(clicked>=100&&clicked-100<rules.size()){
  if(!native_salvage::Enabled()){Trace("AUTO_SALVAGE_REMOVE_PREVIEW rules_unchanged=1");return false;}
  bool saved=store.Remove(rules[clicked-100].key);Trace(saved?"AUTO_SALVAGE_RULE_REMOVED":"AUTO_SALVAGE_RULE_REMOVE_FAILED rules_unchanged=1");return false;
 }
 if(result)editing=false;return result;
}
static void Restore();
static unsigned profileMs=0;static uint64_t profileDeadline=0,profileFrames=0;static LONGLONG profileTotal=0,profileMaximum=0,profileFrequency=0;
static bool __fastcall Menu(uint8_t* model){
 if(!profileMs)return Render(model);
 if(!profileDeadline)profileDeadline=GetTickCount64()+profileMs;
 LARGE_INTEGER begin,end;QueryPerformanceCounter(&begin);bool result=Render(model);QueryPerformanceCounter(&end);
 auto elapsed=end.QuadPart-begin.QuadPart;++profileFrames;profileTotal+=elapsed;profileMaximum=(std::max)(profileMaximum,elapsed);
 if(GetTickCount64()>=profileDeadline||elapsed>profileFrequency/125){Restore();preview=false;profileMs=0;char line[220];sprintf_s(line,"MENU_PROFILE frames=%llu average_us=%.2f maximum_us=%.2f menu_hooks_restored=1",profileFrames,profileTotal*1000000.0/profileFrequency/profileFrames,profileMaximum*1000000.0/profileFrequency);Trace(line);}
 return result;
}
static bool Hook(uint8_t* target,size_t n,void* function,uint8_t* saved,void** previous){
 auto trampoline=(uint8_t*)VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!trampoline)return false;
 const uint8_t jump[]={0xff,0x25,0,0,0,0};memcpy(saved,target,n);memcpy(trampoline,target,n);memcpy(trampoline+n,jump,6);auto resume=uintptr_t(target+n);memcpy(trampoline+n+6,&resume,8);
 DWORD old;if(!VirtualProtect(trampoline,4096,PAGE_EXECUTE_READ,&old))return false;FlushInstructionCache(GetCurrentProcess(),trampoline,n+14);*previous=trampoline;
 uint8_t replacement[32];memset(replacement,0x90,n);memcpy(replacement,jump,6);auto destination=uintptr_t(function);memcpy(replacement+6,&destination,8);
 if(!VirtualProtect(target,n,PAGE_EXECUTE_READWRITE,&old))return false;memcpy(target,replacement,n);DWORD ignored;VirtualProtect(target,n,old,&ignored);FlushInstructionCache(GetCurrentProcess(),target,n);return true;
}
static void Unhook(uint8_t* target,size_t n,void* function,uint8_t* saved){if(!target||target[0]!=0xff||target[1]!=0x25||Read<uintptr_t>(target,6)!=uintptr_t(function))return;DWORD old;if(VirtualProtect(target,n,PAGE_EXECUTE_READWRITE,&old)){memcpy(target,saved,n);DWORD ignored;VirtualProtect(target,n,old,&ignored);FlushInstructionCache(GetCurrentProcess(),target,n);}}
static void Restore(){Unhook(menuEntry,15,(void*)&Menu,menuSaved);Unhook(rowEntry,19,(void*)&Row,rowSaved);}
static bool Install(uint8_t* text,size_t size){
 // Keep all experimental menu code inactive in the ordinary trace build.
 preview=GetPrivateProfileIntW(L"QuickSalvage",L"MenuPreview",0,configPath.c_str())!=0;
 if(!Preview())return false;
 profileMs=(std::min)(GetPrivateProfileIntW(L"QuickSalvage",L"MenuProfileMs",0,configPath.c_str()),3000u);if(profileMs){LARGE_INTEGER f;QueryPerformanceFrequency(&f);profileFrequency=f.QuadPart;}
 auto menu=Unique(text,size,"48 8B C4 48 89 58 10 57 48 81 EC B0 06 00 00 66 C7 40 08 00 00 48 8B D9");
 auto row=Unique(text,size,"40 55 53 57 48 8D AC 24 C0 FD FF FF 48 81 EC 40 03 00 00 F6 41 1A 02 48 8B DA");
 auto horizontal=Unique(text,size,"40 53 48 83 EC 30 0F 29 74 24 20 48 8B D9 0F 28 F2 E8 ?? ?? ?? ?? 48 8B 4B 08 48 8D 54 24 40 45 33 C0 F3 0F 11 74 24 44 C7 44 24 40 00 00 00 00");
 auto finish=Unique(text,size,"48 8D 4C 24 40 E8 ?? ?? ?? ?? 4C 8B B4 24 38 03 00 00 0F B6 C3");
 auto horizontalLayout=Unique(text,size,"45 0F B6 D0 C7 81 B8 00 00 00 01 00 00 00 44 0F B6 89 BC 00 00 00 41 0F B6 C1 41 80 C9 01");
 if(!menu||!row||!horizontal||!finish||!horizontalLayout){Trace("NATIVE_MENU_UNSUPPORTED original_behavior_retained=1");return false;}
 group=(GroupFn)horizontal;endGroup=(EndFn)(finish+10+Read<int32_t>(finish,6));
 setLayout=(LayoutFn)horizontalLayout;
 if(!store.Load()){Trace("RULE_STORE_INVALID automatic_rules_disabled=1");return false;}
 if(!Hook(row,19,(void*)&Row,rowSaved,(void**)&previousRow))return false;rowEntry=row;
 if(!Hook(menu,15,(void*)&Menu,menuSaved,(void**)&previousMenu)){Restore();return false;}menuEntry=menu;Trace("NATIVE_MENU_READY exact_rule_controls=1");return true;
}
}
static void ObserveMenu(void* state,uint8_t* view){native_menu::inventoryState=state;memcpy(native_menu::itemView,view,72);native_menu::editing=false;++native_menu::menuSerial;char message[140];sprintf_s(message,"ITEM_ACTIONS_OPEN item=%u rarity=%u level=%u",Read<uint32_t>(view,8),view[44],view[45]);Trace(message);}
