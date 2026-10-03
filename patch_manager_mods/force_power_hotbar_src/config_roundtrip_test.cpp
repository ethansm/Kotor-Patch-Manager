#include "HotbarConfig.h"
#include "check.h"
#include <stdlib.h>
#include <stdio.h>
using namespace hb;
int main(){
  const char* sample =
   "[General]\nSchemaVersion=1\nInvoke=0 ; safe\nVerbose=1\nFoo=bar\n"
   "[Profile.Default]\nBottom1=spell:8 ; speed burst\nBottom0=spell:135\nLeft1=item:Medpac (3)\nRight3=other:1a\nBottom5=feat:12\nBottom6=garbage\nLeft2=\n"
   "[Profile.Atton]\nBottom1=spell:22\nBogus9=spell:1\n";
  Config a; a.Parse(sample);
  for(auto&w:a.warnings) printf("warn: %s\n",w.c_str());
  KT_CHECK_EQ(a.warnings.size(),2);
  std::string s1=a.Serialize(); Config b; b.Parse(s1); std::string s2=b.Serialize();
  KT_CHECK(s1==s2);
  KT_CHECK(b.warnings.empty());
  KT_CHECK(s1.find("Left")==std::string::npos && s1.find("Right")==std::string::npos);   // removed Ctrl/Alt rows: dropped silently
  KT_CHECK_EQ(a.slotFor("Atton",0,0).id,22);
  KT_CHECK_EQ(a.slotFor("Atton",0,9).id,135);
  KT_CHECK_EQ(a.slotFor("Nobody",0,0).id,8);
  KT_CHECK_EQ(a.general.extra["Foo"].c_str(),"bar");
  const char* td=getenv("TMPDIR"); std::string path=std::string(td&&*td?td:"/tmp")+"/test_rt.ini";
  KT_CHECK(a.Save(path));
  Config c;
  KT_CHECK(c.Load(path));
  KT_CHECK(c.Serialize()==s1);
  KT_CHECK(c.general.extra.count("Foo"));
  { Config o; o.Parse("[General]\nCreatureTagOffset=0x10\n");
    KT_CHECK(o.warnings.empty());
    KT_CHECK_EQ(o.general.extra["CreatureTagOffset"].c_str(),"0x10");
    KT_CHECK(o.Serialize().find("CreatureTagOffset=0x10")!=std::string::npos); }
  { Config t; t.profile("Default"); std::string ts = t.Serialize();   // release template defaults
    KT_CHECK_EQ(t.general.invoke,1);
    KT_CHECK_EQ(t.general.verbose,0);
    KT_CHECK(ts.find("Invoke=1\n")!=std::string::npos);
    KT_CHECK(ts.find("Verbose=0\n")!=std::string::npos); }
  KT_CHECK_EQ(StackCount(0x40000272,"Medpac (7)"),7);
  KT_CHECK_EQ(StackCount(0x40000273,"Repair Kit (19)"),19);
  KT_CHECK_EQ(StackCount(0x40000272,"Medpac"),0);
  KT_CHECK_EQ(StackCount(0x20000008,"Burst (3)"),0);   // stack counts
  KT_CHECK_EQ(StackCount(0x40000001,"Odd (x)"),0);
  KT_CHECK_EQ(StackCount(0x40000001,"()"),0);
  KT_CHECK_EQ(StackCount(0x40000001,"A ()"),0);
  remove(path.c_str());
  ActionRef r;
  KT_CHECK(!Parse("spell:x",&r));
  KT_CHECK(r.empty());
  KT_CHECK(Parse("",&r));
  KT_CHECK(r.empty());
  KT_CHECK(Parse("OTHER:FF",&r));
  KT_CHECK_EQ(r.id,255);
  int row=-1,slot=-1;
  KT_CHECK(SlotFromKeyName("Bottom0",&row,&slot));
  KT_CHECK_EQ(row,0);
  KT_CHECK_EQ(slot,9);
  KT_CHECK(!SlotFromKeyName("Right0",&row,&slot));
  KT_CHECK(IsRemovedRowKey("Right0") && IsRemovedRowKey("left1") && !IsRemovedRowKey("Left"));
  { Config n; n.Parse("[Profile.Default]\nBottom1=spell:8\nBottom2=NONE ; cleared\n[Profile.Bob]\nBottom1=none\n");
    KT_CHECK(n.warnings.empty());
    KT_CHECK(n.slotFor("Bob",0,0).empty());   // none blocks the Default fallback
    KT_CHECK(n.slotFor("Bob",0,0).blank);
    KT_CHECK(!n.slotFor("Nobody",0,0).blank);
    KT_CHECK_EQ(n.slotFor("Nobody",0,0).id,8);
    KT_CHECK(n.slotFor("Nobody",0,1).blank);
    KT_CHECK(n.slotFor("Nobody",0,1).empty());
    n.SetSlot("Default",0,2,ActionRef(),"x"); n.SetSlot("Default",0,3,[]{ActionRef r; r.kind=K_Item; r.name="Medpac"; return r;}(),"Medpac");
    std::string o=n.Serialize(); Config m; m.Parse(o);
    KT_CHECK(m.warnings.empty());
    KT_CHECK(m.Serialize()==o);
    const Profile* dp=m.find("default");
    if(KT_CHECK(dp!=nullptr)) KT_CHECK_EQ(Format(dp->slots[0][3]).c_str(),"item:Medpac");
    KT_CHECK(m.slotFor("",0,1).blank);
    KT_CHECK(o.find("Bottom2=none")!=std::string::npos);
    KT_CHECK(o.find("Bottom3=")==std::string::npos); }
  puts(s1.c_str());
  return kt::Report("config_roundtrip_test");
}
