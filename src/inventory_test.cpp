#include "quick_salvage.cpp"
#include <cassert>
#include <iostream>
static std::map<std::pair<uint32_t,uint32_t>,std::vector<uint8_t>> components;
static void* __fastcall Getter(void* out,void*,uint32_t hash,uint32_t entity){auto i=components.find({hash,entity});void* pointer=i==components.end()?nullptr:i->second.data();uintptr_t auxiliary=0;memcpy(out,&pointer,8);memcpy((uint8_t*)out+8,&auxiliary,8);return out;}
static uint8_t* Add(uint32_t hash,uint32_t entity,size_t size){auto& data=components[{hash,entity}];data.resize(size);return data.data();}
static void Put(void* out,size_t offset,uint32_t value){memcpy((uint8_t*)out+offset,&value,4);}
static void Link(uint8_t* setup,unsigned index,uint32_t page,uint8_t category){Put(setup,index*4,page);setup[64+index]=category;setup[83]=1;}
static void Item(uint8_t* view,uint32_t page,uint8_t slot,uint32_t item,uint32_t pide){
 auto& data=components[{0xb19528a9,page}];if(data.empty())data.resize(96);auto stack=data.data()+slot*12;Put(stack,0,item);Put(stack,4,1);Put(stack,8,pide);
 auto meta=Add(0x3a2fac75,pide,20);Put(meta,0,page);meta[4]=slot;Put(meta,8,item);meta[16]=2;meta[17]=12;
 memset(view,0,72);Put(view,0,page);view[4]=slot;Put(view,8,item);Put(view,16,1);Put(view,20,pide);view[44]=2;view[45]=12;
}
int main(){
 logPath=L"native-fixture.log";reader=Getter;ecs=(void*)1;localPlayer=1;
 auto top=Add(0xc24df82a,1,84);top[80]=1;Link(top,0,1,2);Link(top,1,100,4);Link(top,2,200,5);
 auto bag=Add(0xc24df82a,200,84);Link(bag,0,201,4);Link(bag,1,1,2); // Cycle must terminate.
 assert(native_salvage::OwnedPage(100));assert(native_salvage::OwnedPage(200));assert(native_salvage::OwnedPage(201));assert(!native_salvage::OwnedPage(1));assert(!native_salvage::OwnedPage(999));
 uint8_t view[72];Item(view,201,7,456,301);auto c=native_salvage::Candidate(view,true);assert(salvage::Eligible(c));
 // Independent observed game case: upgraded scythe menu level2, ItemState UI
 // level3. Rules must use3 rather than reject it or save a base-level rule.
 view[45]=2;components[{0x3a2fac75,301}][17]=3;c=native_salvage::Candidate(view,true);assert(salvage::Eligible(c)&&c.key.level==3);
 salvage::Rules upgrades;assert(upgrades.Add(c.key,"Upgraded"));assert(upgrades.Contains({456,2,3})&&!upgrades.Contains({456,2,2}));
 view[44]=1;c=native_salvage::Candidate(view,true);assert(!c.metadataValid&&!salvage::Eligible(c));view[44]=2;
 Item(view,201,7,456,301);
 components[{0x3a2fac75,301}][14]=8;c=native_salvage::Candidate(view,true);assert(c.equipped&&!salvage::Eligible(c));components[{0x3a2fac75,301}][14]=0;
 Item(view,999,0,789,302);c=native_salvage::Candidate(view,true);assert(c.metadataValid&&!c.owned&&!salvage::Eligible(c));
 Item(view,201,7,456,301);Put(view,16,2);c=native_salvage::Candidate(view,true);assert(!c.metadataValid&&!salvage::Eligible(c));
 std::cout<<"PASS: nested owned backpack pages, cyclic links, foreign storage, equipped-item and stale-count rejection\n";
}
