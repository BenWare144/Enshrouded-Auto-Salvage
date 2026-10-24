#include "quick_salvage.cpp"
#include <cassert>
#include <iostream>
static unsigned calls=0;
static void* __fastcall Getter(void* out,void*,uint32_t,uint32_t){
 ++calls;assert(uintptr_t(out)%16==0);
 // Independently observed native stores at9a7355/9a7358 and movups at9a737b
 // write two qwords, even though consumers usually need only the first one.
 uintptr_t actual=0x12345678,auxiliary=0xabcdef0123456789ull;
 memcpy(out,&actual,8);memcpy((uint8_t*)out+8,&auxiliary,8);return out;
}
int main(){
 reader=Getter;ecs=(void*)1;localPlayer=1;
 assert(native_salvage::Component(1,1)==(void*)0x12345678);assert(calls==1);
 assert(native_salvage::Component(1,0)==nullptr);assert(calls==1);
 reader=nullptr;assert(native_salvage::Component(1,1)==nullptr);
 std::cout<<"PASS: aligned aggregate-result storage, native pointer extraction and null-input gates\n";
}
