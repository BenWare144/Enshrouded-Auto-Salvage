#include "quick_salvage.cpp"
#include <cassert>
#include <iostream>
static __declspec(noinline) void* __fastcall Draw(void* out,void*,void*,void*){return out;}
int main(){
 alignas(16) uint8_t out[32]{};native_shortcut::previous=&Draw;
 using Fn=void*(__fastcall*)(void*,void*,void*,void*);volatile Fn draw=&native_shortcut::Slot;
 LARGE_INTEGER frequency,start,finish;QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&start);
 // Exercise the actual installed wrapper's ordinary draw path, with null
 // other arguments. Any accidental eligibility/memory/logging work will fail.
 constexpr unsigned iterations=10000000;
 for(unsigned i=0;i<iterations;++i)assert(draw(out,nullptr,nullptr,nullptr)==out);
 QueryPerformanceCounter(&finish);double ns=double(finish.QuadPart-start.QuadPart)*1e9/double(frequency.QuadPart)/iterations;
 std::cout<<"PASS: ordinary draw passthrough, "<<ns<<" ns/call in native mock (not a game FPS measurement)\n";
}
