import tomllib,itertools,os
exec(open('/tmp/claude-1000/overlap.py').read().split("print('hooks loaded")[0])
# add DllMain-patched code sites (hardcoded in .cpp, not in hooks.toml); addr,len
dll=[('ActivationFix','k2-activation-fix',0x40D69E,6,'DllMain CALL [PostMessageA]'),
('EquipIconFix','k2-equip-icon-fix',0x8ABA8C,5,'rel32 call retarget'),
('EquipIconFix','k2-equip-icon-fix',0x8ADA08,5,'rel32'),('EquipIconFix','k2-equip-icon-fix',0x8ADA81,5,'rel32'),('EquipIconFix','k2-equip-icon-fix',0x8AE793,5,'rel32'),
('EquipIconFix','k2-equip-icon-fix',0x8ADFA2,5,'rel32'),('EquipIconFix','k2-equip-icon-fix',0x8AE02D,5,'rel32'),('EquipIconFix','k2-equip-icon-fix',0x8AE19A,5,'rel32'),('EquipIconFix','k2-equip-icon-fix',0x8AE1CD,5,'rel32'),
('TextureMemoryRelease','k2-texture-memory-release',0x4261CF,7,'DllMain E9 cave'),
('PerfLab','perf-lab',0x4261CF,7,'tx=1 experiment (off by default)'),('PerfLab','perf-lab',0x4265F3,2,'g1skip=1 experiment (off by default)'),
('RobustnessFixes','k2-robustness-fixes',0x726527,10,'KtA'),('RobustnessFixes','k2-robustness-fixes',0x726C9C,5,'KtB'),('RobustnessFixes','k2-robustness-fixes',0x668FE0,2,'Vmid'),('RobustnessFixes','k2-robustness-fixes',0x669012,5,'VmidB'),('RobustnessFixes','k2-robustness-fixes',0x7054CE,12,'Sndri'),('RobustnessFixes','k2-robustness-fixes',0x704406,15,'NetA'),('RobustnessFixes','k2-robustness-fixes',0x7040EF,5,'NetB'),('RobustnessFixes','k2-robustness-fixes',0x40F972,6,'Guiid'),('RobustnessFixes','k2-robustness-fixes',0x58C61A,8,'AbB')]
allr=[(r[0],r[1],r[2],r[3],r[4],r[5]) for r in rng if r[1] in liveids or r[0] in OURS]
for d,pid,a,n,why in dll: allr.append((d,pid,a,a+n,'dllmain',why))
print('sites:',len(allr))
for a,b in itertools.combinations(allr,2):
    if a[0]==b[0]: continue
    if a[2]<b[3] and b[2]<a[3]: print('OVERLAP',a[0],hex(a[2]),hex(a[3]),a[4],a[5],'<->',b[0],hex(b[2]),hex(b[3]),b[4],b[5],'liveboth=',a[1] in liveids and b[1] in liveids)
    elif max(a[2]-b[3],b[2]-a[3])<=8: print('near<=8',a[0],hex(a[2]),a[5],'|',b[0],hex(b[2]),b[5], 'gap',max(a[2]-b[3],b[2]-a[3]))
