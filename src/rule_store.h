#pragma once
#include "rules.h"
#include <windows.h>
#include <cstdio>
namespace salvage {
// UTF-8 data, same-directory atomic replacement. Keep the previous good file
// until the complete new file has reached disk.
class RuleStore {
 std::wstring path;
 uint64_t revision=0;
 public:
 Rules rules;
 uint64_t Revision()const{return revision;}
 explicit RuleStore(std::wstring filename):path(std::move(filename)){}
 bool Load(){
  FILE* file=nullptr;
  if(_wfopen_s(&file,path.c_str(),L"rb")||!file)
   return GetFileAttributesW(path.c_str())==INVALID_FILE_ATTRIBUTES&&GetLastError()==ERROR_FILE_NOT_FOUND;
  std::string data;char block[4096];size_t count;
  while((count=fread(block,1,sizeof block,file))!=0){data.append(block,count);if(data.size()>1024*1024){fclose(file);return false;}}
  bool good=!ferror(file);fclose(file);if(!good||!rules.Decode(data))return false;++revision;return true;
 }
 bool Save()const{
  const auto temporary=path+L".tmp";const auto data=rules.Encode();
  HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(file==INVALID_HANDLE_VALUE)return false;
  DWORD written=0;bool good=WriteFile(file,data.data(),DWORD(data.size()),&written,nullptr)&&written==data.size()&&FlushFileBuffers(file);
  CloseHandle(file);
  if(good)good=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
  if(!good)DeleteFileW(temporary.c_str());return good;
 }
 bool Add(const Key& key,const std::string& name){auto before=rules;if(!rules.Add(key,name))return false;if(Save()){++revision;return true;}rules=std::move(before);return false;}
 bool Remove(const Key& key){auto before=rules;if(!rules.Remove(key))return false;if(Save()){++revision;return true;}rules=std::move(before);return false;}
};
}
