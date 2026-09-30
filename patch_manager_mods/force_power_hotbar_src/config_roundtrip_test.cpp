#include "HotbarConfig.h"
#include <assert.h>
using namespace hb;
int main(){
  const char* sample =
   "[General]\nSchemaVersion=1\nInvoke=0 ; safe\nVerbose=1\nFoo=bar\n"
   "[Profile.Default]\nBottom1=spell:8 ; speed burst\nBottom0=spell:135\nLeft1=item:Medpac (3)\nRight3=other:1a\nBottom5=feat:12\nBottom6=garbage\nLeft2=\n"
   "[Profile.Atton]\nBottom1=spell:22\nBogus9=spell:1\n";
  Config a; a.Parse(sample);
  for(auto&w:a.warnings) printf("warn: %s\n",w.c_str());
  assert(a.warnings.size()==2);
  std::string s1=a.Serialize(); Config b; b.Parse(s1); std::string s2=b.Serialize();
  assert(s1==s2 && b.warnings.empty());
  assert(Format(a.find("default")->slots[1][0])=="item:Medpac");
  assert(a.slotFor("Atton",0,0).id==22 && a.slotFor("Atton",0,9).id==135 && a.slotFor("Nobody",0,0).id==8);
  assert(a.slotFor("atton",1,0).kind==K_Item);
  assert(a.general.extra["Foo"]=="bar");
  const char* path="test_rt.ini"; assert(a.Save(path)); Config c; assert(c.Load(path)); assert(c.Serialize()==s1);
  ActionRef r; assert(!Parse("spell:x",&r) && r.empty()); assert(Parse("",&r)&&r.empty()); assert(Parse("OTHER:FF",&r)&&r.id==255);
  int row,slot; assert(SlotFromKeyName("Right0",&row,&slot)&&row==2&&slot==9); assert(!SlotFromKeyName("Right",&row,&slot));
  { Config n; n.Parse("[Profile.Default]\nBottom1=spell:8\nBottom2=NONE ; cleared\n[Profile.Bob]\nBottom1=none\n");
    assert(n.warnings.empty());
    assert(n.slotFor("Bob",0,0).empty() && n.slotFor("Bob",0,0).blank);   // none blocks the Default fallback
    assert(!n.slotFor("Nobody",0,0).blank && n.slotFor("Nobody",0,0).id==8);
    assert(n.slotFor("Nobody",0,1).blank && n.slotFor("Nobody",0,1).empty());
    n.SetSlot("Default",0,2,ActionRef(),"x"); n.SetSlot("Default",0,3,[]{ActionRef r; r.kind=K_Item; r.name="Medpac"; return r;}(),"Medpac");
    std::string o=n.Serialize(); Config m; m.Parse(o); assert(m.warnings.empty() && m.Serialize()==o);
    assert(Format(m.find("default")->slots[0][3])=="item:Medpac" && m.slotFor("",0,1).blank);
    assert(o.find("Bottom2=none")!=std::string::npos && o.find("Bottom3=")==std::string::npos); }
  puts(s1.c_str()); puts("config round-trip OK");
}
