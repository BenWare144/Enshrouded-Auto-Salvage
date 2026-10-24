#include "rules.h"
#include <cassert>
#include <iostream>
int main(){
 using namespace salvage;Rules rules;Key selected{123,3,25};assert(rules.Add(selected,"Sword"));assert(!rules.Add(selected,"Sword"));assert(rules.Contains(selected));assert(!rules.Contains({123,4,25}));assert(!rules.Contains({123,3,26}));assert(!rules.Contains({124,3,25}));assert(!rules.Add({123,5,25},"Invalid rarity"));assert(!rules.Add({123,3,0},"Unknown level"));assert(!rules.Add({124,3,25},"Injected\nrule"));
 auto saved=rules.Encode();Rules restored;assert(restored.Decode(saved));assert(restored.Contains(selected));assert(restored.List()[0].label=="Sword");assert(restored.Remove(selected));assert(!restored.Contains(selected));assert(restored.Decode(saved));
 for(auto bad:{std::string("QuickSalvageRules 3\n"),std::string("QuickSalvageRules 1\n-1\t3\t25\tSword\n"),saved+"123\t3\t25\tDuplicate\n",saved+"4294967296\t3\t25\tOverflow\n",saved+"42\t4\t0\tUnknown\n"}){assert(!restored.Decode(bad));assert(restored.Size()==1&&restored.Contains(selected));}
 assert(restored.List()[0].created==rules.List()[0].created&&restored.List()[0].created>0);
 Rules legacy;assert(legacy.Decode("QuickSalvageRules 1\n123\t3\t25\tSword\n"));assert(legacy.List()[0].created==0);Rules migrated;assert(migrated.Decode(legacy.Encode()));assert(migrated.Contains(selected));
 Rule older{{1,4,10},"Zulu",10},newer{{2,0,30},"alpha",20};
 assert(RuleBefore(newer,older,RuleSort::Date));assert(RuleBefore(newer,older,RuleSort::Name));assert(RuleBefore(older,newer,RuleSort::Rarity));assert(RuleBefore(newer,older,RuleSort::Level));
 assert(!RuleBefore(newer,newer,RuleSort::Date));assert(legacy.Add({2,0,1},"Newest"));auto ordered=legacy.List();std::sort(ordered.begin(),ordered.end(),[](const Rule&a,const Rule&b){return RuleBefore(a,b,RuleSort::Date);});assert(ordered[0].label=="Newest");
 RuleSort criterion=RuleSort::Date;bool reversed=false;
 for(auto sort:{RuleSort::Date,RuleSort::Name,RuleSort::Rarity,RuleSort::Level}){
  criterion=sort;reversed=false;assert(RuleBefore(older,newer,sort,false)!=RuleBefore(older,newer,sort,true));
  SelectRuleSort(criterion,reversed,sort);assert(reversed);assert(!RuleBefore(older,older,sort,reversed));
  SelectRuleSort(criterion,reversed,sort);assert(!reversed);
 }
 criterion=RuleSort::Date;reversed=true;SelectRuleSort(criterion,reversed,RuleSort::Name);assert(criterion==RuleSort::Name&&!reversed);
 Candidate c{selected,1,100,200,1,2,true,false,true,true};Pending pending;assert(pending.Claim(c));assert(!pending.Claim(c));pending.Observe({c});assert(!pending.Claim(c));pending.Observe({});assert(pending.Claim(c));c.session=2;assert(pending.Claim(c));
 auto moved=c;moved.page=101;moved.slot=4;pending.Observe({moved});assert(!pending.Claim(moved));pending.Cancel(moved);assert(pending.Claim(moved));
 for(int flag=0;flag<9;++flag){auto bad=c;switch(flag){case 0:bad.owned=false;break;case 1:bad.equipped=true;break;case 2:bad.canSalvage=false;break;case 3:bad.metadataValid=false;break;case 4:bad.count=0;break;case 5:bad.slot=8;break;case 6:bad.page=0;break;case 7:bad.session=0;break;case 8:bad.pide=0;break;}assert(!Eligible(bad));assert(!pending.Claim(bad));}
 Gesture g;g.Update(true,true,true);assert(g.Consume());assert(!g.Consume());g.Update(true,true,true);assert(!g.Consume());g.Update(false,true,true);g.Update(true,true,true);assert(g.Consume());g.Update(false,false,true);g.Update(true,false,true);g.Update(true,true,true);assert(!g.Consume());g.Update(false,true,true);g.Update(true,true,false);assert(!g.Consume());
 std::cout<<"PASS: exact type/rarity/level matching, persistence, corrupt-file rejection, native eligibility gates, request deduplication and one action per click\n";
}
