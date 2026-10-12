# W1 voice priority: TSL DLG Sound vs VO_ResRef, TLK sound

Method: WebSearch + WebFetch, 2026-10-11. Claims are from WebFetch summaries of page text; code quotes are short. PyKotor dlg.py returned 404 at raw master, raw main, and github blob, so it is UNVERIFIED.

Q1 Does the engine play Sound over VO_ResRef when both are set? (prior: "Sound beats VO")
- UNVERIFIED. No in-game test found. Rakata says Sound first, VO only "when Sound is empty", provenance "derived, not attested". https://synchro.codeberg.page/rakata/formats/gff/dlg.html
- Search snippets said "fails to execute"; the page itself says "empty". Use the page wording.
- Conflict: xoreos `playSounds` plays voice and sound on separate channels, no priority, no fallback. https://raw.githubusercontent.com/xoreos/xoreos/master/src/engines/kotorbase/gui/dialog.cpp
- Forum thread: audio inconsistent by facing angle; nobody confirmed precedence. https://deadlystream.com/topic/8463-voice-oversound-on-a-placeable-help/

Q2 Are Sound and VO_ResRef on both entry and reply nodes?
- VERIFIED. xoreos `DLGFile::readEntry` is shared by NPC entries and PC replies and reads `Sound` and `VO_ResRef`. https://raw.githubusercontent.com/xoreos/xoreos/master/src/aurora/dlgfile.cpp

Q3 Does DLG have a plain 'ResRef' field on a node? (prior: "no ResRef field")
- VERIFIED narrowly: no field named `ResRef` in xoreos readEntry. Reads: Script, Script2, Speaker, Text, Sound, VO_ResRef, Animation, Quest, QuestEntry, ActionParam*, RepliesList, EntriesList.
- REFUTED as stated ("no ResRef-typed field"): rakata types `Script` (EntryList/ReplyList) and `Active` as CResRef. https://synchro.codeberg.page/rakata/formats/gff/dlg.html
- xoreos reads Script/Sound/VO_ResRef with getString, not getResRef. Schema typing is rakata's, not from the binary.

Q4 Does a TLK sound resref play in dialog, and which wins over DLG VO?
- UNVERIFIED. xoreos dialog reads only `line.voice` and `line.sound`; no TLK lookup in the dialog path. That is an xoreos fact, not an engine fact. https://raw.githubusercontent.com/xoreos/xoreos/master/src/engines/kotorbase/gui/dialog.cpp
- Rakata tlk page: sound_resref is "Voice-over resource, when the entry has one"; no playback rule. https://synchro.codeberg.page/rakata/formats/text/tlk.html

Q5 Does the precedence matter in shipped TSL? (context only)
- UNVERIFIED (single user). Sound "almost always empty", VO_ResRef "always" set in TSL DLGs. https://deadlystream.com/topic/6538-tsl-dlg-vo-clarification/

Q6 Alien lines ignore VO_ResRef and use alienvo.2da? (context only)
- UNVERIFIED (forum claim, 2018). Same thread. https://deadlystream.com/topic/6538-tsl-dlg-vo-clarification/

Not reached: PyKotor dlg.py (404 on all three URL forms).
Next: rea-recon on swkotor2.exe, dialog-audio playback and CSWSDialog::LoadDialog (0x005a2ae0, cited by rakata's derived audit note).
