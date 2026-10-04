#include "HotbarConfig.h"
#include "check.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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
  const char* td=getenv("TMPDIR"); std::string path=std::string(td&&*td?td:"/tmp")+"/test_rt_"+std::to_string((long)getpid())+".ini";
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
  KT_CHECK(Parse("item: (3)",&r)); KT_CHECK_EQ(r.name.c_str(), "(3)");   // a bare parenthetical is a name (normalize only strips " (..")
  { // a long bad value must not truncate the slot key out of the warning (msg buffer)
    Config w; w.Parse("[Profile.Long]\nBottom7=item:" + std::string(300, 'x') + "@lowhp:0\n");
    for (auto& x : w.warnings) printf("warn: %s\n", x.c_str());
    KT_CHECK_EQ(w.warnings.size(), 1);
    if (!w.warnings.empty()) { KT_CHECK(w.warnings[0].find("Bottom7") != std::string::npos); KT_CHECK(w.warnings[0].size() <= 200); }
  }
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

  // ================= hardening (doc 20 4.1): parsing robustness =================
  static const Profile kNoProf;
  auto P=[](const Config& c,const char* n)->const Profile&{ const Profile* p=c.find(n); return p?*p:kNoProf; };
  auto W=[](const std::string& t){ Config c; c.Parse(t); return c; };
  { // BOM / CRLF / lone CR
    Config c=W("\xEF\xBB\xBF[General]\nInvoke=0\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,0);
    c=W("[General]\r\nInvoke=0\r\nVerbose=1\r\n[Profile.Default]\r\nBottom1=spell:8 ; hi\r\n");
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,0); KT_CHECK_EQ(c.general.verbose,1);
    KT_CHECK_EQ(P(c,"Default").slots[0][0].id,8); KT_CHECK_EQ(P(c,"Default").notes[0][0].c_str(),"hi");
    c=W("[General]\rInvoke=0\rVerbose=1\r[Profile.Default]\rBottom1=spell:8\r");   // old-Mac line ends: must not be silently ignored
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,0); KT_CHECK_EQ(c.general.verbose,1); KT_CHECK_EQ(P(c,"Default").slots[0][0].id,8);
    c=W("\xEF\xBB\xBF[Profile.Default]\r\nBottom2=feat:3\r\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(P(c,"Default").slots[0][1].id,3);
    // structure oddities
    c=W("[General\nInvoke=0\n"); KT_CHECK_EQ(c.warnings.size(),2); KT_CHECK_EQ(c.general.invoke,1);   // unterminated section, then the key is outside any section
    c=W("Invoke=0\n[General]\nVerbose=1\n"); KT_CHECK_EQ(c.warnings.size(),1); KT_CHECK_EQ(c.general.invoke,1); KT_CHECK_EQ(c.general.verbose,1);
    c=W("[]\nBottom1=spell:1\n"); KT_CHECK_EQ(c.warnings.size(),2); KT_CHECK(c.profiles.empty());
    c=W("[Profile.]\nBottom1=spell:1\n"); KT_CHECK_EQ(c.warnings.size(),2); KT_CHECK(c.profiles.empty());
    c=W("[Profile.X]\nBottom1 spell:1\n"); KT_CHECK_EQ(c.warnings.size(),1);   // no '='
    c=W("# comment\n; other comment\n   # indented\n[General]\n#Invoke=0\n;Verbose=1\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,1); KT_CHECK_EQ(c.general.verbose,0);
    c=W("[General]\nInvoke=0\nInvoke=1\nFoo=a\nFoo=b\n[Profile.D]\nBottom1=spell:1\nBottom1=spell:2\n[profile.d]\nBottom1=spell:3\n");   // last wins, profile names case-insensitive
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,1); KT_CHECK_EQ(c.general.extra["Foo"].c_str(),"b");
    KT_CHECK_EQ(c.profiles.size(),1); KT_CHECK_EQ(P(c,"D").slots[0][0].id,3);
    c=W("[ General ]\n  Invoke  =  0  \nKeyboardDevice\t=\t3\n[ Profile.Z ]\n Bottom3 =  spell:5  \n");
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,0); KT_CHECK_EQ(c.general.keyboardDevice,3); KT_CHECK_EQ(P(c,"Z").slots[0][2].id,5);
    // KeyboardDevice round trip
    c=W("[General]\nKeyboardDevice=2\n"); KT_CHECK_EQ(c.general.keyboardDevice,2);
    std::string o=c.Serialize(); KT_CHECK(o.find("KeyboardDevice=2\n")!=std::string::npos);
    Config d=W(o); KT_CHECK(d.warnings.empty()); KT_CHECK_EQ(d.general.keyboardDevice,2); KT_CHECK(d.Serialize()==o);
    c=W("[General]\nKeyboardDevice=0\n"); KT_CHECK_EQ(c.general.keyboardDevice,0); KT_CHECK(c.Serialize().find("KeyboardDevice=0\n")!=std::string::npos);
    c=W("[General]\nKeyboardDevice=-1\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.keyboardDevice,-1); KT_CHECK(c.Serialize().find("\nKeyboardDevice=")==std::string::npos);
    // documented quirk (NOT fixed): ';' always starts the note, even inside an item name, so such a name is truncated at the ';'.
    c=W("[Profile.Default]\nBottom1=item:Foo; Bar\n");
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(P(c,"Default").slots[0][0].name.c_str(),"Foo"); KT_CHECK_EQ(P(c,"Default").notes[0][0].c_str(),"Bar");
  }
  // ================= strict General ints =================
  { for(const char* bad:{"Invoke=abc","Verbose=x","AutoFill=y","Invoke=","Verbose=1x","AutoFill=0.5","Invoke=99999999999","KeyboardDevice=abc","KeyboardDevice=","KeyboardDevice=-2x","KeyboardDevice=2147483648"}){
      Config c=W(std::string("[General]\n")+bad+"\n");
      if(!KT_CHECK_EQ(c.warnings.size(),1)) printf("  (case %s)\n",bad);
      KT_CHECK_EQ(c.general.invoke,1); KT_CHECK_EQ(c.general.verbose,0); KT_CHECK_EQ(c.general.autoFill,0); KT_CHECK_EQ(c.general.keyboardDevice,-1); }
    Config c=W("[General]\nInvoke=0\nVerbose=1\nAutoFill=1\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,0); KT_CHECK_EQ(c.general.verbose,1); KT_CHECK_EQ(c.general.autoFill,1);
    c=W("[General]\nInvoke=2\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,2);   // any in-range int is kept (callers test != 0)
    c=W("[General]\nInvoke=abc\nInvoke=0\n"); KT_CHECK_EQ(c.warnings.size(),1); KT_CHECK_EQ(c.general.invoke,0);   // a later good line still applies
  }
  // ================= 32-bit range checks (host long is 64-bit; the DLL's is 32-bit) =================
  { ActionRef r;
    KT_CHECK(Parse("spell:2147483647",&r)); KT_CHECK_EQ(r.id,2147483647);
    KT_CHECK(!Parse("spell:2147483648",&r)); KT_CHECK(r.empty());
    KT_CHECK(!Parse("spell:99999999999",&r)); KT_CHECK(r.empty());
    KT_CHECK(!Parse("feat:4294967296",&r)); KT_CHECK(!Parse("feat:99999999999999999999999",&r));
    KT_CHECK(!Parse("spell:-1",&r));
    KT_CHECK(Parse("other:ffffffff",&r)); KT_CHECK_EQ(r.kind,K_Other); KT_CHECK_EQ((unsigned)r.id,0xFFFFFFFFu);
    KT_CHECK_EQ(Format(r).c_str(),"other:ffffffff");
    KT_CHECK(!Parse("other:100000000",&r)); KT_CHECK(r.empty());
    KT_CHECK(!Parse("other:1ffffffff",&r)); KT_CHECK(r.empty());
    KT_CHECK(!Parse("other:ffffffffffffffffff",&r));
    KT_CHECK(!Parse("other:-1",&r)); KT_CHECK(!Parse("other:zz",&r));
    KT_CHECK(Parse("other:80000000",&r)); KT_CHECK_EQ((unsigned)r.id,0x80000000u); KT_CHECK_EQ(Format(r).c_str(),"other:80000000");
    Config c=W("[Profile.D]\nBottom1=spell:99999999999\nBottom2=other:1ffffffff\nBottom3=spell:7\n");
    KT_CHECK_EQ(c.warnings.size(),2); const Profile* p=&P(c,"D");
    KT_CHECK(p->slots[0][0].empty()); KT_CHECK(p->slots[0][1].empty()); KT_CHECK_EQ(p->slots[0][2].id,7);
  }
  // ================= bounds checks =================
  { Config c=W("[Profile.Default]\nBottom1=spell:8\nBottom0=spell:9\n");
    const int rows[]={-1,Rows,Rows+5,0,0,0,0}, slots[]={0,0,0,-1,Slots,Slots+3,-100};
    for(int i=0;i<7;++i){ KT_CHECK(c.slotFor("",rows[i],slots[i]).empty()); KT_CHECK(c.slotFor("Nobody",rows[i],slots[i]).empty());
      KT_CHECK_EQ(SlotKeyName(rows[i],slots[i]).c_str(),""); }
    KT_CHECK_EQ(SlotKeyName(0,0).c_str(),"Bottom1"); KT_CHECK_EQ(SlotKeyName(0,8).c_str(),"Bottom9"); KT_CHECK_EQ(SlotKeyName(0,9).c_str(),"Bottom0");
    ActionRef a; a.kind=K_Spell; a.id=5;
    for(int i=0;i<7;++i){ size_t before=c.profiles.size();
      c.SetSlot("Ghost",rows[i],slots[i],a,"n"); KT_CHECK_EQ(c.profiles.size(),before); KT_CHECK(c.find("Ghost")==nullptr);
      c.SetSlot("Default",rows[i],slots[i],a,"n"); }
    const Profile* p=c.find("Default");
    KT_CHECK_EQ(p->slots[0][0].id,8); KT_CHECK_EQ(p->slots[0][9].id,9); KT_CHECK(p->notes[0][0].empty()); KT_CHECK(p->notes[0][9].empty());
    for(int s=1;s<9;++s) KT_CHECK(p->slots[0][s].empty());
    c.SetSlot("Ghost",0,9,a,"ok"); KT_CHECK(c.find("Ghost")!=nullptr); KT_CHECK_EQ(P(c,"Ghost").slots[0][9].id,5);   // valid edge still works
    Profile pr; pr.slots[0][2]=a; ActionRef q=a; q.id=77;   // moveDuplicates: out-of-range slot/row must not touch anything
    KT_CHECK(moveDuplicates(pr,0,Slots,q).empty()); KT_CHECK(moveDuplicates(pr,0,-1,q).empty()); KT_CHECK(moveDuplicates(pr,-1,0,a).empty()); KT_CHECK(moveDuplicates(pr,Rows,0,a).empty());
    KT_CHECK_EQ(pr.slots[0][2].id,5);
    KT_CHECK_EQ(moveDuplicates(pr,0,Slots,a).size(),1); KT_CHECK(pr.slots[0][2].blank);   // slot==Slots: nothing is "the target", so the one holder is moved
    KT_CHECK_EQ(slotDigit(-1),'?'); KT_CHECK_EQ(slotDigit(10),'?'); KT_CHECK_EQ(slotDigit(9),'0');
  }
  // ================= StackCount semantics =================
  { const int K=0x40000001;
    KT_CHECK_EQ(StackCount(K,"X (0)"),0);          // "0" is not a real stack (engine only prints counts > 1): treated as no count
    KT_CHECK_EQ(StackCount(K,"X (1)"),1); KT_CHECK_EQ(StackCount(K,"X (999999)"),999999);
    KT_CHECK_EQ(StackCount(K,"X (1000000)"),0);     // 7+ digits: not a plausible stack count
    KT_CHECK_EQ(StackCount(K,"X (99999999999999)"),0);
    KT_CHECK_EQ(StackCount(K,"X ( 3)"),0); KT_CHECK_EQ(StackCount(K,"X (3 )"),0); KT_CHECK_EQ(StackCount(K,"X (-3)"),0); KT_CHECK_EQ(StackCount(K,"X (+3)"),0);
    KT_CHECK_EQ(StackCount(K,"X (3) "),0); KT_CHECK_EQ(StackCount(K,"X(3)"),0);
    KT_CHECK_EQ(StackCount(K,"Medpac (Type) (3)"),3); KT_CHECK_EQ(StackCount(K,"Medpac (Type)"),0);
    KT_CHECK_EQ(StackCount(0,"X (3)"),0); KT_CHECK_EQ(StackCount(K,""),0); KT_CHECK_EQ(StackCount(K,")"),0);
  }
  // ================= round trip: everything Serialize writes parses back identically =================
  { Config c; c.general.invoke=0; c.general.verbose=1; c.general.autoFill=1; c.general.keyboardDevice=4; c.general.healThreshold=77; c.general.healFallbackSelf=0;
    c.general.extra["Zed"]="1"; c.general.extra["Alpha"]="two words";
    const char* vals[]={"spell:0","spell:2147483647","feat:12","item:Medpac","item:Adrenal Stimulant@lowhp","spell:8@lowhp:40","feat:3@lowhp::none","item:Repair Kit@lowhp:100:self",
                        "other:1a","other:ffffffff","other:80000000@lowhp","none","item:A@bogus"};
    for(int i=0;i<10;++i){ ActionRef a; KT_CHECK(Parse(vals[i],&a)); c.SetSlot("Default",0,i,a,i%2?"note "+std::to_string(i):"");
      ActionRef b; KT_CHECK(Parse(vals[i+3],&b)); c.SetSlot("Atton",0,i,b,i%3?"":"a; b"); }
    c.SetSlot("Bastila",0,4,ActionRef(),"");   // empty slot: not written
    std::string o=c.Serialize(); Config d=W(o);
    KT_CHECK(d.warnings.empty()); KT_CHECK(d.Serialize()==o);
    KT_CHECK_EQ(d.general.invoke,0); KT_CHECK_EQ(d.general.verbose,1); KT_CHECK_EQ(d.general.autoFill,1); KT_CHECK_EQ(d.general.keyboardDevice,4);
    KT_CHECK_EQ(d.general.healThreshold,77); KT_CHECK_EQ(d.general.healFallbackSelf,0); KT_CHECK(d.general.extra==c.general.extra);
    KT_CHECK_EQ(d.profiles.size(),c.profiles.size());
    for(const char* pn:{"Default","Atton"}){ const Profile* x=c.find(pn); const Profile* y=d.find(pn);
      if(!KT_CHECK(x&&y)) continue;
      KT_CHECK_EQ(y->name.c_str(),pn);
      for(int s=0;s<Slots;++s){ KT_CHECK_EQ(Format(x->slots[0][s]).c_str(),Format(y->slots[0][s]).c_str()); KT_CHECK_EQ(x->slots[0][s].blank,y->slots[0][s].blank);
        KT_CHECK_EQ(x->slots[0][s].kind,y->slots[0][s].kind); KT_CHECK_EQ(x->slots[0][s].target.thresholdPct,y->slots[0][s].target.thresholdPct);
        KT_CHECK_EQ(x->slots[0][s].target.fallback,y->slots[0][s].target.fallback); KT_CHECK_EQ(x->notes[0][s].c_str(),y->notes[0][s].c_str()); } }
    // file round trip through Save/Load
    const char* td=getenv("TMPDIR"); std::string path=std::string(td&&*td?td:"/tmp")+"/test_rt2_"+std::to_string((long)getpid())+".ini";
    KT_CHECK(c.Save(path)); Config e; KT_CHECK(e.Load(path)); KT_CHECK(e.warnings.empty()); KT_CHECK(e.Serialize()==o); remove(path.c_str());
    Config nf; KT_CHECK(!nf.Load("/nonexistent_dir_xyz/none.ini"));
  }
  // ===== mutation-triage additions (doc 20e) =====
  { // layout constants: the INI keys, the editor grid and the file header all derive from these
    KT_CHECK_EQ(Rows,1); KT_CHECK_EQ(Slots,10); KT_CHECK_EQ(SchemaVersion,1);
    KT_CHECK(validSlot(0,0)); KT_CHECK(validSlot(0,9)); KT_CHECK(!validSlot(0,10)); KT_CHECK(!validSlot(0,-1)); KT_CHECK(!validSlot(1,0)); KT_CHECK(!validSlot(-1,0));
    KT_CHECK_EQ(SlotKeyName(0,10).c_str(),""); KT_CHECK_EQ(slotDigit(0),'1'); KT_CHECK_EQ(slotDigit(8),'9');
    Config c; c.profile("Default");
    KT_CHECK(c.Serialize().find("[General]\nSchemaVersion=1\nInvoke=")!=std::string::npos); }
  { // parseInt32 range (through the strict General ints): INT32_MIN..INT32_MAX
    Config c=W("[General]\nInvoke=-2147483648\nVerbose=2147483647\n"); KT_CHECK(c.warnings.empty());
    KT_CHECK_EQ(c.general.invoke,(int)(-2147483647-1)); KT_CHECK_EQ(c.general.verbose,2147483647);
    c=W("[General]\nInvoke=-2147483649\nVerbose=2147483648\nAutoFill=-99999999999\n"); KT_CHECK_EQ(c.warnings.size(),3);
    KT_CHECK_EQ(c.general.invoke,1); KT_CHECK_EQ(c.general.verbose,0); KT_CHECK_EQ(c.general.autoFill,0);
    c=W("[General]\nInvoke=+7\nVerbose=-0\n"); KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(c.general.invoke,7); KT_CHECK_EQ(c.general.verbose,0); }
  { // trim / lower
    for(const char* ws:{" ","\t","\r","\n"}){ std::string w=ws; KT_CHECK_EQ(trim(w+"x"+w).c_str(),"x"); KT_CHECK_EQ(trim("x"+w).c_str(),"x"); KT_CHECK_EQ(trim(w+"x").c_str(),"x"); }
    KT_CHECK_EQ(trim("x \t\r\n").c_str(),"x"); KT_CHECK_EQ(trim(" \t\r\n x").c_str(),"x"); KT_CHECK_EQ(trim(" \t\r\n").c_str(),""); KT_CHECK_EQ(trim("").c_str(),"");
    KT_CHECK_EQ(trim("a b").c_str(),"a b"); KT_CHECK_EQ(trim("\vx").c_str(),"\vx");
    KT_CHECK_EQ(lower("AZ").c_str(),"az"); KT_CHECK_EQ(lower("@A[Zz`").c_str(),"@a[zz`"); }
  { // StackCount edges
    const int K=0x40000001;
    KT_CHECK_EQ(StackCount(K," (1)"),1); KT_CHECK_EQ(StackCount(K,"x555)"),0); KT_CHECK_EQ(StackCount(K,"Medpac (10)"),10); KT_CHECK_EQ(StackCount(K,"A (2)"),2);
    KT_CHECK_EQ(StackCount(K," (1"),0); KT_CHECK_EQ(StackCount(K,"(12)"),0); KT_CHECK_EQ(StackCount(K,"X (12"),0);
    KT_CHECK_EQ(StackCount(0x40000000,"X (3)"),3); KT_CHECK_EQ(StackCount(0x3FFFFFFF,"X (3)"),0); }
  { // formatTarget: shortest form; a threshold of 0 or a fallback of 0 are real values (only < 0 is unset)
    TargetSpec t; t.mode=TM_LowHp;
    KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp");
    t.thresholdPct=0; KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp:0");
    t.fallback=1; KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp:0:self");
    t.thresholdPct=-1; KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp::self");
    t.fallback=0; KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp::none");
    t.thresholdPct=100; KT_CHECK_EQ(formatTarget(t).c_str(),"@lowhp:100:none");
    t.mode=TM_Default; KT_CHECK_EQ(formatTarget(t).c_str(),""); }
  { // parseTargetSuffix / Parse: field and range limits
    TargetSpec ts; KT_CHECK(!parseTargetSuffix("bogus",&ts)); KT_CHECK(!parseTargetSuffix("",&ts)); KT_CHECK(parseTargetSuffix("default",&ts)); KT_CHECK(!parseTargetSuffix("default:1",&ts));
    KT_CHECK(parseTargetSuffix("lowhp",&ts)); KT_CHECK(!parseTargetSuffix("lowhp:1:self:x",&ts));
    ActionRef r;
    KT_CHECK(Parse("spell:1@lowhp:19",&r)); KT_CHECK_EQ(r.target.thresholdPct,19);
    KT_CHECK(Parse("spell:1@lowhp:99",&r)); KT_CHECK_EQ(r.target.thresholdPct,99);
    KT_CHECK(Parse("spell:1@lowhp:0050",&r)); KT_CHECK_EQ(r.target.thresholdPct,50);
    KT_CHECK(!Parse("spell:1@lowhp:4294967346",&r));       // would wrap to 50 without the digit-accumulator guard
    KT_CHECK(!Parse("spell:1@lowhp:1x",&r)); KT_CHECK(!Parse("spell:1@lowhp:9a",&r)); KT_CHECK(!Parse("spell:1@lowhp:101",&r)); KT_CHECK(!Parse("spell:1@lowhp:1000",&r)); KT_CHECK(!Parse("spell:1@lowhp:1001",&r)); KT_CHECK(!Parse("spell:1@lowhp:00000000000000000000000101",&r));
    KT_CHECK(!Parse("item",&r)); KT_CHECK(r.empty()); KT_CHECK(!Parse("item:",&r)); KT_CHECK(r.empty()); KT_CHECK(!Parse("item:   ",&r)); KT_CHECK(!Parse("spell",&r));
    KT_CHECK(!Parse("other:+5",&r)); KT_CHECK(!Parse("other:-0",&r)); KT_CHECK(!Parse("other:-5",&r)); KT_CHECK(r.empty()); }
  { // Same(): identity only
    ActionRef a,b; a.kind=K_Item; a.name="Medpac"; b=a; KT_CHECK(Same(a,b));
    b.name="Repair Kit"; KT_CHECK(!Same(a,b)); b.name="MEDPAC"; KT_CHECK(Same(a,b));
    ActionRef s; s.kind=K_Spell; s.id=8; ActionRef f=s; f.kind=K_Feat; KT_CHECK(!Same(s,f)); f.kind=K_Spell; KT_CHECK(Same(s,f)); f.id=9; KT_CHECK(!Same(s,f)); }
  { // IsRemovedRowKey: exactly Left/Right + one digit
    KT_CHECK(IsRemovedRowKey("Left9")); KT_CHECK(IsRemovedRowKey("Right9")); KT_CHECK(IsRemovedRowKey("Left0")); KT_CHECK(IsRemovedRowKey("RIGHT5"));
    KT_CHECK(!IsRemovedRowKey("Left:")); KT_CHECK(!IsRemovedRowKey("Left/")); KT_CHECK(!IsRemovedRowKey("Left10")); KT_CHECK(!IsRemovedRowKey("Leftx")); KT_CHECK(!IsRemovedRowKey("Lef9")); KT_CHECK(!IsRemovedRowKey("Bottom1")); KT_CHECK(!IsRemovedRowKey("")); }
  { // effective values: a slot override of 0 is a value, not "unset"
    General g; g.healThreshold=70; TargetSpec t; t.thresholdPct=0; KT_CHECK_EQ(effectiveThreshold(t,g),0);
    t.thresholdPct=-1; KT_CHECK_EQ(effectiveThreshold(t,g),70); t.fallback=0; g.healFallbackSelf=1; KT_CHECK(!effectiveFallbackSelf(t,g)); }
  { // Config::readFile: whole file, including a final partial chunk
    const char* td=getenv("TMPDIR"); std::string path=std::string(td&&*td?td:"/tmp")+"/test_rf_"+std::to_string((long)getpid())+".bin";
    for(size_t n:{(size_t)1,(size_t)4095,(size_t)4096,(size_t)4097,(size_t)8193}){
      std::string body(n,'a'); body[n-1]='Z'; FILE* f=fopen(path.c_str(),"wb"); KT_CHECK(f!=nullptr); if(!f) continue; fwrite(body.data(),1,n,f); fclose(f);
      std::string got="stale"; KT_CHECK(Config::readFile(path,&got)); KT_CHECK(got==body); }
    { FILE* f=fopen(path.c_str(),"wb"); if(f) fclose(f); std::string got="stale"; KT_CHECK(Config::readFile(path,&got)); KT_CHECK(got.empty()); }   // empty file: cleared, true
    remove(path.c_str()); std::string g2; KT_CHECK(!Config::readFile(path,&g2)); }
  { // warning line numbers: LF, CRLF, CR, mixed; BOM-only text; first line is line 1
    for(const char* nl:{"\n","\r\n","\r"}){ std::string n=nl;
      Config c=W("[Profile.X]"+n+"bad1"+n+"bad2"+n); KT_CHECK_EQ(c.warnings.size(),2);
      if(c.warnings.size()==2){ KT_CHECK(c.warnings[0].compare(0,8,"line 2: ")==0); KT_CHECK(c.warnings[1].compare(0,8,"line 3: ")==0); } }
    Config c=W("bad\n"); KT_CHECK_EQ(c.warnings.size(),1); if(!c.warnings.empty()) KT_CHECK(c.warnings[0].compare(0,8,"line 1: ")==0);
    c=W("[Profile.X]\r\n\r\nbad\r\n"); KT_CHECK_EQ(c.warnings.size(),1); if(!c.warnings.empty()) KT_CHECK(c.warnings[0].compare(0,8,"line 3: ")==0);   // CRLF is ONE break, a blank line is a line
    c=W("[Profile.X]\n\nbad\n"); KT_CHECK_EQ(c.warnings.size(),1); if(!c.warnings.empty()) KT_CHECK(c.warnings[0].compare(0,8,"line 3: ")==0);          // a blank line after LF is a line
    c=W("[Profile.X]\n\rbad\n"); KT_CHECK_EQ(c.warnings.size(),1); if(!c.warnings.empty()) KT_CHECK(c.warnings[0].compare(0,8,"line 3: ")==0);        // LF then CR are two breaks
    c=W("\xEF\xBB\xBF"); KT_CHECK(c.warnings.empty()); c=W("\xEF\xBB\xBF" "bad\n"); KT_CHECK_EQ(c.warnings.size(),1); if(!c.warnings.empty()) KT_CHECK(c.warnings[0].compare(0,8,"line 1: ")==0);
    c=W(""); KT_CHECK(c.warnings.empty()); KT_CHECK(c.profiles.empty()); }
  { // a note needs no space after ';'; an empty note is empty
    Config c=W("[Profile.Default]\nBottom1=spell:8;note\nBottom2=spell:9;\nBottom3=spell:10;  spaced  \n");
    KT_CHECK(c.warnings.empty()); KT_CHECK_EQ(P(c,"Default").notes[0][0].c_str(),"note"); KT_CHECK_EQ(P(c,"Default").notes[0][1].c_str(),""); KT_CHECK_EQ(P(c,"Default").notes[0][2].c_str(),"spaced"); }
  { // warning text clips long values (key/value/section) but keeps the line number and slot key
    Config c=W("[Profile.Long]\nBottom7=item:"+std::string(300,'x')+"@lowhp:0\n"); KT_CHECK_EQ(c.warnings.size(),1);
    if(!c.warnings.empty()) KT_CHECK_EQ(c.warnings[0].c_str(),("line 2: Bottom7: bad value 'item:"+std::string(95,'x')+"' (slot left empty)").c_str());
    c=W("[Profile.Long]\n"+std::string(300,'k')+"=spell:1\n"); KT_CHECK_EQ(c.warnings.size(),1);
    if(!c.warnings.empty()) KT_CHECK_EQ(c.warnings[0].c_str(),("line 2: unknown key '"+std::string(40,'k')+"' in [Profile.Long]").c_str()); }
  { // Save never leaves a .bak (the hotbar makes its own backups) and replaces the file each time
    const char* td=getenv("TMPDIR"); std::string path=std::string(td&&*td?td:"/tmp")+"/test_save2_"+std::to_string((long)getpid())+".ini";
    remove(path.c_str()); remove((path+".bak").c_str()); remove((path+".tmp").c_str());   // stale files from an aborted earlier run must not matter
    Config c; c.profile("Default"); KT_CHECK(c.Save(path)); c.general.invoke=0; KT_CHECK(c.Save(path));
    FILE* b=fopen((path+".bak").c_str(),"rb"); KT_CHECK(b==nullptr); if(b) fclose(b);
    FILE* t=fopen((path+".tmp").c_str(),"rb"); KT_CHECK(t==nullptr); if(t) fclose(t);
    Config d; KT_CHECK(d.Load(path)); KT_CHECK_EQ(d.general.invoke,0);
    remove(path.c_str()); remove((path+".bak").c_str()); }
  puts(s1.c_str());
  return kt::Report("config_roundtrip_test");
}
