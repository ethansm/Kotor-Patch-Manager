# T5 voice-line claims (Gemini): reuse K1 .wav in TSL without dialog.tlk

Method: WebSearch + WebFetch, 2026-10-11. ModdingWiki returned Anubis access-denied (unverified). Quotes are WebFetch summaries of each page, not byte-checked.

Q1 folders/archives: PARTIAL. K1 voice = `streamwaves/`, K2 = `StreamVoice/` (PyKotor: "first game ... 'streamwaves' ... second game ... renamed to 'streamvoice'"). Gemini's "K1 in StreamVoice" REFUTED. "Voice ERF" UNVERIFIED: PyKotor models voice as folders only. https://github.com/OpenKotOR/PyKotor/blob/master/Libraries/PyKotor/src/pykotor/extract/installation.py

Q2 format: VERIFIED for K1. Voice files are MP3 inside RIFF/WAVE: RIFF size 50, MP3 starts at byte 58 (riff_size+8). Real PCM REFUTED for K1 voice. K2 UNSURE: 4058 says .wav dead after Steam update; 9013 says raw .wav "seemed to work". https://synchro.codeberg.page/rakata/formats/audio/wav.html

Q3 DLG fields: VERIFIED fields `Sound` and `VO_ResRef` (PyKotor nodes.py). Engine per rakata: `Sound` wins, `VO_ResRef` read only if Sound empty, reply sound never plays. Gemini's "ResRef field" REFUTED: no such field. https://synchro.codeberg.page/rakata/formats/gff/dlg.html

Q4 TLK vs VO_ResRef: UNVERIFIED. TLK `sound_resref` is "voice-over resource, when the entry has one"; no fetched source states which wins. https://synchro.codeberg.page/rakata/formats/text/tlk.html

Q5 lookup: PARTIAL. K1 engine opens voice "by path under streamwaves/, not through the normal resource lookup" (Override not used, inferred). K2: numbered StreamVoice subfolders (203/302/601) per 2558; DLG subfolder named by Voiceover ID per 9013; root-level drop skipped per 4058. Override for .lip is only a tutorial suggestion. https://deadlystream.com/files/file/2558-k2-alternative-voice-overs ; https://deadlystream.com/topic/9013-replacing-alien-voice-with-audio-files/

Gemini (c) "drop K1 .wav into StreamVoice plays": REFUTED. Root-level .wav skipped in game; fix was MP3 re-export (single user, not moderator-confirmed). https://deadlystream.com/topic/4058-problems-with-audio-and-lip-syncing-tsl/

Gemini (b) "type basename into ResRef field": REFUTED as worded. Only VO_ResRef and Sound exist, and reply lines never play.

Forum recipe (9013, DarthParametric, unconfirmed in game): mono MP3 with fake WAV header, VO_ResRef, DLG-specific subfolder.

K2 alien lines (6538, JCarter426, forum claim): VO_ResRef "meaningless"; alien audio chosen from alienvo.2da.

Overall: PARTIAL. K1 container and K2 folder facts hold; Gemini's drop-in path (c) and field name (b) do not.
