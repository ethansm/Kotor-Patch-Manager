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
  puts(s1.c_str()); puts("config round-trip OK");
}
