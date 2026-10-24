#include "rule_store.h"
#include <cassert>
#include <iostream>
int wmain(int argc,wchar_t** argv){
 assert(argc==2);std::wstring path=std::wstring(argv[1])+L"\\rule-store-test.txt";
 assert(GetFileAttributesW(path.c_str())==INVALID_FILE_ATTRIBUTES);
 salvage::RuleStore first(path);assert(first.Load());assert(first.Add({123,2,7},"Test sword"));
 assert(first.Revision()==1);assert(!first.Add({123,2,7},"Duplicate"));assert(first.Revision()==1);
 salvage::RuleStore second(path);assert(second.Load());assert(second.rules.Contains({123,2,7}));assert(!second.rules.Contains({123,2,8}));
 assert(second.Remove({123,2,7}));salvage::RuleStore third(path);assert(third.Load());assert(third.rules.Size()==0);
 assert(second.Revision()==2);assert(!second.Remove({123,2,7}));assert(second.Revision()==2);
 assert(DeleteFileW(path.c_str()));
 salvage::RuleStore failure(path+L"\\no-parent.txt");assert(!failure.Add({123,2,7},"Test sword"));assert(failure.rules.Size()==0);
 assert(failure.Revision()==0);
 std::cout<<"PASS: disk roundtrip, deletion persistence and failed-save rollback\n";
}
