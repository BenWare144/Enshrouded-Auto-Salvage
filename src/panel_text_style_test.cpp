#include "panel_text_style.h"
#include <cassert>
#include <set>
#include <string>
int main(){
 uint8_t original[88];memset(original,0xa5,sizeof original);
 // Poison the old relative identifier, reproducing the copied-theme hazard.
 int32_t invalid=0x70000000;memcpy(original,&invalid,4);
 panel_text_style::Style styles[5][3];std::set<std::string> names;
 for(unsigned rarity=0;rarity<5;++rarity)for(unsigned role=0;role<3;++role){
  auto& value=styles[rarity][role];uint64_t color=0x3f800000ffffffffull-rarity;
  panel_text_style::Initialize(value,original,role,rarity,color);
  // Independently reproduce the executable's BlobString read,1177658..1177694.
  int32_t offset;uint32_t count;memcpy(&offset,value.data,4);memcpy(&count,value.data+4,4);
  assert(offset==88&&count>0&&count<64);auto string=value.data+offset;
  assert(string==(const uint8_t*)value.identifier&&string[count]==0);
  assert(names.emplace((const char*)string,count).second);
  assert(memcmp(value.data+16,original+16,16)==0); // preserve actual font GUID
  uint64_t actual;memcpy(&actual,value.data+44,8);assert(actual==color);
  if(role==2){float tracking;memcpy(&tracking,value.data+36,4);assert(tracking==0&&value.data[10]==1);}
 }
 assert(names.size()==15);puts("UI style relative-identifier/cache identity tests passed (15 styles).");
}
