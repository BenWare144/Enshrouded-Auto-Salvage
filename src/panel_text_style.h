#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstddef>
namespace panel_text_style {
// UiCharacterStyle.identifier is a BlobString: signed offset from the field,
// then byte count. Native 1177620 hashes it on every style-cache lookup.
// The font reference at16 is a GUID; it is safe to copy verbatim.
struct alignas(16) Style { uint8_t data[88]{}; char identifier[64]{}; };
static_assert(offsetof(Style,identifier)==88);
inline void Initialize(Style& output,const void* source,unsigned role,unsigned rarity,uint64_t color){
 memcpy(output.data,source,88);
 auto length=std::snprintf(output.identifier,sizeof output.identifier,"QuickSalvage55.role%u.rarity%u",role,rarity);
 int32_t offset=88;uint32_t size=uint32_t(length);
 memcpy(output.data,&offset,4);memcpy(output.data+4,&size,4);
 float fontSize=role==0?24.0f:(role==1?20.0f:16.0f);memcpy(output.data+32,&fontSize,4);
 output.data[9]=output.data[12]=output.data[13]=1;output.data[42]=role==2;memcpy(output.data+44,&color,8);
 if(role==2){float tracking=0;output.data[10]=1;memcpy(output.data+36,&tracking,4);}
}
}
