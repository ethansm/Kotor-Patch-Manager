# W2: raw K1 model/head failures in TSL (recon)
Read 2026-10-11. Sources: 2 DeadlyStream threads, 1 file page (description only, no download). Quotes come from WebFetch summaries, not raw HTML.
Searches: "K1 model TSL crash" and "K1 head TSL invisible" returned Tesla and AGV helmet results. "kotor 1 model kotor 2 black head" returned no black-head report.

Q1 symptoms: VERIFIED. Invisible (name tag shows) in both threads, plus a bone-count load warning. Nothing else reported.
  - Juhani invisible: https://deadlystream.com/topic/9809-porting-juhani-to-tsl/#findComment-87801 (still invisible after MDLEdit update, #findComment-87806)
  - Juhani bone-count warning, cleared by MDLEdit 1.0.104b: #findComment-87804, #findComment-87806 (same thread)
  - Bastila "Invisible Head Syndrome", head absent in game but shown in HT: https://deadlystream.com/topic/11879-porting-heads-from-k1-to-k2/#findComment-98423
  - Not reported: crash on load, distortion, black head, missing animations, wrong head. Only a general claim that the game crashes on a missing or bad model (#findComment-87807, thread 9809, not a test).

Q2 working fix: VERIFIED (UTC swap only). Juhani's UTC was rebuilt from TSL's Bastila UTC; the model was not changed; Juhani appears in game. https://deadlystream.com/topic/9809-porting-juhani-to-tsl/#findComment-87815
  - ASCII round trip + S_Female02/03 supermodel: proposed by Effix (https://deadlystream.com/topic/11879-porting-heads-from-k1-to-k2/#findComment-98418), but Bastila stayed invisible in game. Not a confirmed fix: UNVERIFIED.
  - Head hook edit, texture rename: none reported.

Q3 converted K2 model works in game: VERIFIED, weak (one self-report). Juhani: "get Juhani to appear in the game" and models "ported correctly" (https://deadlystream.com/topic/9809-porting-juhani-to-tsl/#findComment-87815). Counter: converted Bastila head invisible in game (https://deadlystream.com/topic/11879-porting-heads-from-k1-to-k2/#findComment-98423).

Q4 K1 head works with no conversion: UNVERIFIED. No report tests an unconverted K1 head. Nearest: Effix K1-to-K2 Heads v1.2 adds PFHC01 and PMHA05 via TSLPatcher, "Bugs: None known." The page does not say whether the files were converted or tested in game. https://deadlystream.com/files/file/1392-effixians-k1-to-k2-heads/
  - Juhani thread says K1 heads likely need lip adjustment (#findComment-87803, https://deadlystream.com/topic/9809-porting-juhani-to-tsl/). A prediction, not a test.

Caveat: no report tested a fully raw K1 model. Juhani's model was re-saved in MDLEdit K2 mode (#findComment-87801). Bastila's was ASCII round-tripped (#findComment-98418).
