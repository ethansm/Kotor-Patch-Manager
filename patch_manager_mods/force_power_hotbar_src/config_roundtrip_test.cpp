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

  // --- per-slot target mode (@lowhp) ---
  { ActionRef r;
    KT_CHECK(Parse("item:Medpac@lowhp",&r));
    KT_CHECK_EQ(r.kind,K_Item); KT_CHECK_EQ(r.name.c_str(),"Medpac"); KT_CHECK_EQ(r.target.mode,TM_LowHp);
    KT_CHECK_EQ(r.target.thresholdPct,-1); KT_CHECK_EQ(r.target.fallback,-1);
    KT_CHECK(Parse("item:Medpac@lowhp:40:none",&r));
    KT_CHECK_EQ(r.name.c_str(),"Medpac"); KT_CHECK_EQ(r.target.mode,TM_LowHp); KT_CHECK_EQ(r.target.thresholdPct,40); KT_CHECK_EQ(r.target.fallback,0);
    KT_CHECK(Parse("item:Medpac@LowHP:40:SELF",&r)); KT_CHECK_EQ(r.target.fallback,1);
    KT_CHECK(Parse("item:Medpac@lowhp::none",&r)); KT_CHECK_EQ(r.target.thresholdPct,-1); KT_CHECK_EQ(r.target.fallback,0);
    KT_CHECK(Parse("item:Medpac@lowhp:100",&r)); KT_CHECK_EQ(r.target.thresholdPct,100); KT_CHECK_EQ(r.target.fallback,-1);
    KT_CHECK(Parse("item:Medpac@lowhp:1",&r)); KT_CHECK_EQ(r.target.thresholdPct,1);
    KT_CHECK(Parse("spell:8@lowhp",&r)); KT_CHECK_EQ(r.kind,K_Spell); KT_CHECK_EQ(r.id,8); KT_CHECK_EQ(r.target.mode,TM_LowHp);
    KT_CHECK(Parse("feat:3@lowhp:25",&r)); KT_CHECK_EQ(r.kind,K_Feat); KT_CHECK_EQ(r.target.thresholdPct,25);
    KT_CHECK(Parse("item:Medpac@lowhp:40:none",&r)); KT_CHECK(Parse("item:Medpac@default",&r));
    KT_CHECK_EQ(r.name.c_str(),"Medpac"); KT_CHECK_EQ(r.target.mode,TM_Default);
    KT_CHECK(Parse("item:A@bogus",&r)); KT_CHECK_EQ(r.name.c_str(),"A@bogus"); KT_CHECK_EQ(r.target.mode,TM_Default);
    KT_CHECK(Parse("item:A@bogus@lowhp",&r)); KT_CHECK_EQ(r.name.c_str(),"A@bogus"); KT_CHECK_EQ(r.target.mode,TM_LowHp);
    KT_CHECK(!Parse("item:X@lowhp:0",&r)); KT_CHECK(r.empty());
    KT_CHECK(!Parse("item:X@lowhp:101",&r));
    KT_CHECK(!Parse("item:X@lowhp:4x",&r));
    KT_CHECK(!Parse("item:X@lowhp:-5",&r));
    KT_CHECK(!Parse("item:X@lowhp:40:maybe",&r));
    KT_CHECK(!Parse("item:X@lowhp:40:none:extra",&r));
    KT_CHECK(!Parse("none@lowhp",&r));
    KT_CHECK(!Parse("@lowhp",&r));
    KT_CHECK(!Parse("item:@lowhp",&r));
    KT_CHECK(!Parse("bogus:1@lowhp",&r));
    // Format: shortest form that round-trips
    const char* forms[]={"item:Medpac","item:Medpac@lowhp","item:Medpac@lowhp:40","item:Medpac@lowhp::none","item:Medpac@lowhp::self","item:Medpac@lowhp:40:none","item:Medpac@lowhp:40:self",
                         "spell:8","spell:8@lowhp","spell:8@lowhp:40","spell:8@lowhp::none","spell:8@lowhp:40:none","feat:3@lowhp","other:1a"};
    for(const char* f:forms){ ActionRef q; KT_CHECK(Parse(f,&q)); KT_CHECK_EQ(Format(q).c_str(),f); }
    { ActionRef q; Parse("item:Medpac@default",&q); KT_CHECK_EQ(Format(q).c_str(),"item:Medpac");
      q=ActionRef(); q.blank=true; KT_CHECK_EQ(Format(q).c_str(),"none"); }
    // Same() is identity only
    { ActionRef x,y; Parse("item:Medpac",&x); Parse("item:medpac@lowhp:40:none",&y); KT_CHECK(Same(x,y));
      Parse("spell:8",&x); Parse("spell:8@lowhp",&y); KT_CHECK(Same(x,y)); Parse("spell:9@lowhp",&y); KT_CHECK(!Same(x,y)); }
    // effective values: slot override, else [General]
    { General g; TargetSpec t;
      KT_CHECK_EQ(g.healThreshold,50); KT_CHECK_EQ(g.healFallbackSelf,1);
      KT_CHECK_EQ(effectiveThreshold(t,g),50); KT_CHECK(effectiveFallbackSelf(t,g));
      g.healThreshold=70; g.healFallbackSelf=0;
      KT_CHECK_EQ(effectiveThreshold(t,g),70); KT_CHECK(!effectiveFallbackSelf(t,g));
      t.thresholdPct=30; t.fallback=1;
      KT_CHECK_EQ(effectiveThreshold(t,g),30); KT_CHECK(effectiveFallbackSelf(t,g));
      t.fallback=0; g.healFallbackSelf=1; KT_CHECK(!effectiveFallbackSelf(t,g)); } }
  { Config g; g.Parse("[General]\nHealThreshold=35\nHealFallback=NONE\n");
    KT_CHECK(g.warnings.empty()); KT_CHECK_EQ(g.general.healThreshold,35); KT_CHECK_EQ(g.general.healFallbackSelf,0);
    std::string o=g.Serialize(); KT_CHECK(o.find("HealThreshold=35\n")!=std::string::npos); KT_CHECK(o.find("HealFallback=none\n")!=std::string::npos);
    KT_CHECK(g.general.extra.empty());
    Config g2; g2.Parse("[General]\nHealFallback=Self\nHealThreshold=100\n"); KT_CHECK(g2.warnings.empty());
    KT_CHECK_EQ(g2.general.healThreshold,100); KT_CHECK_EQ(g2.general.healFallbackSelf,1);
    Config g3; g3.Parse("[General]\nHealThreshold=1\n"); KT_CHECK(g3.warnings.empty()); KT_CHECK_EQ(g3.general.healThreshold,1);
    for(const char* bad:{"HealThreshold=0","HealThreshold=101","HealThreshold=abc","HealThreshold=","HealThreshold=40x","HealFallback=maybe","HealFallback="}){
      Config w; w.Parse(std::string("[General]\n")+bad+"\n");
      KT_CHECK_EQ(w.warnings.size(),1); KT_CHECK_EQ(w.general.healThreshold,50); KT_CHECK_EQ(w.general.healFallbackSelf,1); }
    Config d; d.Parse("[General]\nInvoke=1\n"); std::string od=d.Serialize();
    KT_CHECK(od.find("HealThreshold=50\n")!=std::string::npos); KT_CHECK(od.find("HealFallback=self\n")!=std::string::npos);
    KT_CHECK(od.find("HealThreshold")>od.find("AutoFill=")); }
  { const char* ini="[General]\nHealThreshold=60\nHealFallback=none\n[Profile.Default]\nBottom1=item:Medpac@lowhp\nBottom2=item:Medpac@lowhp:40:none ; heal\nBottom3=spell:8@lowhp::self\nBottom4=item:Adrenal@default\nBottom5=item:A@bogus\n"
                    "[Profile.Atton]\nBottom1=spell:22@lowhp:10\n";
    Config a; a.Parse(ini); KT_CHECK(a.warnings.empty());
    ActionRef s=a.slotFor("Nobody",0,1); KT_CHECK_EQ(s.target.mode,TM_LowHp); KT_CHECK_EQ(effectiveThreshold(s.target,a.general),40); KT_CHECK(!effectiveFallbackSelf(s.target,a.general));
    s=a.slotFor("Nobody",0,0); KT_CHECK_EQ(effectiveThreshold(s.target,a.general),60); KT_CHECK(!effectiveFallbackSelf(s.target,a.general));
    s=a.slotFor("Nobody",0,2); KT_CHECK(effectiveFallbackSelf(s.target,a.general));
    KT_CHECK_EQ(a.slotFor("Atton",0,0).target.thresholdPct,10);
    std::string o1=a.Serialize(); Config b; b.Parse(o1); KT_CHECK(b.warnings.empty()); KT_CHECK(b.Serialize()==o1);
    KT_CHECK(o1.find("Bottom1=item:Medpac@lowhp\n")!=std::string::npos);
    KT_CHECK(o1.find("Bottom2=item:Medpac@lowhp:40:none   ; heal")!=std::string::npos);
    KT_CHECK(o1.find("Bottom3=spell:8@lowhp::self\n")!=std::string::npos);
    KT_CHECK(o1.find("Bottom4=item:Adrenal\n")!=std::string::npos);
    KT_CHECK(o1.find("Bottom5=item:A@bogus\n")!=std::string::npos);
    KT_CHECK(o1.find("[Profile.Atton]\nBottom1=spell:22@lowhp:10\n")!=std::string::npos); }
  puts(s1.c_str());
  return kt::Report("config_roundtrip_test");
}
