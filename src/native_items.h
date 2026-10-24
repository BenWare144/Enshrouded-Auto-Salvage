namespace native_items {
using LookupFn=uint8_t*(__fastcall*)(uint32_t,void*);
using LocaFn=uint32_t(__fastcall*)(const void*);
using TextFn=void*(__fastcall*)(void*,uint32_t);
using SalvageFn=bool(__fastcall*)(const void*);
static LookupFn lookup=nullptr;static LocaFn loca=nullptr;static TextFn textValue=nullptr;
static SalvageFn canSalvage=nullptr;
static size_t appField=0,contextField=0,tableField=0;
static void* itemContext=nullptr;static void* itemWorld=nullptr;
static void Capture(){
 __try{if(!native_salvage::appSlot||!*native_salvage::appSlot)return;auto context=Read<void*>(*native_salvage::appSlot,appField);if(context){itemContext=context;itemWorld=ecs;}}
 __except(EXCEPTION_EXECUTE_HANDLER){itemContext=nullptr;itemWorld=nullptr;}
}
static uint8_t* Info(uint32_t item){
 __try{
  if(lookup&&itemContext&&itemWorld==ecs){auto owner=Read<void*>(itemContext);auto client=owner?Read<void*>(owner,contextField):nullptr;auto table=client?Read<void*>(client,tableField):nullptr;auto info=table?lookup(item,table):nullptr;if(info&&Read<uint32_t>(info)==item)return info;}
  if(!lookup||!native_salvage::appSlot||!*native_salvage::appSlot)return nullptr;
  auto ui=Read<void*>(*native_salvage::appSlot,appField);if(!ui)return nullptr;
  auto context=Read<void*>(ui);if(!context)return nullptr;
  auto client=Read<void*>(context,contextField);if(!client)return nullptr;
  auto table=Read<void*>(client,tableField);if(!table)return nullptr;
  auto info=lookup(item,table);return info&&Read<uint32_t>(info)==item?info:nullptr;
 }__except(EXCEPTION_EXECUTE_HANDLER){return nullptr;}
}
static bool Allowed(uint32_t item,uint32_t pide=0){
 // Use the game's exact predicate, including its per-instance fallback when
 // no static item record is available in the current native UI context.
 __try{auto info=Info(item);if(!info)return pide!=0;auto flags=Read<uint32_t>(info,1496);return (flags&(1<<17))&&!(flags&(1<<14));}
 __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static std::string Name(uint32_t item){
 auto info=Info(item);if(info&&loca&&textValue){native_menu::UiText value{};textValue(&value,loca(info+72));if(value.length&&value.length<1024&&Readable(value.text,size_t(value.length)))return {value.text,size_t(value.length)};}
 return "Item "+std::to_string(item);
}
static bool Install(uint8_t* text,size_t size){
 auto info=Unique(text,size,"40 53 48 83 EC 20 48 8B 05 ?? ?? ?? ?? 48 8B D9 48 8B 90 ?? ?? ?? ?? 48 8B 12 48 85 D2 74 04 48 8B 52 38 48 8B 92 ?? ?? ?? ?? 8B 49 08 E8 ?? ?? ?? ?? 48 85 C0 74 1F 8B 80 D8 05 00 00 0F BA E0 0E");
 auto name=Unique(text,size,"48 8D 48 48 E8 ?? ?? ?? ?? 8B D0 48 8D 4C 24 40 E8 ?? ?? ?? ?? 48 8B 05 ?? ?? ?? ?? 48 8D 55 80 0F 10 44 24 40");
 if(!info||!name){Trace("NATIVE_ITEM_METADATA_UNSUPPORTED automatic_rules_disabled=1");return false;}
 appField=Read<uint32_t>(info,19);contextField=info[34];tableField=Read<uint32_t>(info,38);
 canSalvage=(SalvageFn)info;
 lookup=(LookupFn)(info+50+Read<int32_t>(info,46));
 loca=(LocaFn)(name+9+Read<int32_t>(name,5));textValue=(TextFn)(name+21+Read<int32_t>(name,17));
 if(appField>0x1000||contextField>0x1000||tableField>0x1000000)return false;
 Trace("NATIVE_ITEM_METADATA_READY native_names_and_salvage_flags=1");return true;
}
}
