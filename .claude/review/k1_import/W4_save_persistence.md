# W4: TSL save vs template appearance (recon)

Q1 save stores full creature data, not template ref: VERIFIED (K1 GOG decomp). https://synchro.codeberg.page/rakata/internals/save_deep_dive.html "full self-contained snapshot", "TemplateResRef ... absent (not read)". Save creature has no TemplateResRef: https://zff.dev/Synchro/rakata/issues/37 (game unnamed).
Q1 Appearance_Type inside saved record: UNVERIFIED. rakata git.html lists Creature List[].Appearance_Type as "NOT EXAMINED". Issue #37 names BodyVariation/TextureVar as not re-serialized, Appearance_Type not named.
Q2 engine re-applies template to saved creatures on reload: REFUTED (K1). "UseTemplates=0 ... no blueprint load"; omitted fields take engine default, not blueprint (save_deep_dive.html). Benched companions use template loader pointed at save's own AVAILNPC%d resource, not .utc. TSL: UNVERIFIED.
Q3 community: template edit absent in existing save: VERIFIED (3 found, 2 TSL).
  a) TSL https://deadlystream.com/topic/9679-moddahnis-unique-look (Leilukin, 12 Nov 2022): "must load a save before visiting Nar Shaddaa's Entertainment Promenade for the first time."
  b) TSL https://deadlystream.com/topic/5716-questions-about-editing-utc-files-in-tsl/ (DarthParametric, 2 Jan 2021, Atton-specific): "you'd need to start a fresh game, since his UTC is pulled into the GIT right at the start."
  c) K1 https://deadlystream.com/topic/4431-k1-changing-appearance/ (DarthParametric, 22 May 2016): "The appearance.2da reference is permanently stored in your save."
  Reddit/PCGamingWiki: no hits in search. Nexus kotor2 mod 46: HTTP 403.
Q4 fresh area visit reads .utc: VERIFIED (K1 decomp + TSL source + TSL community).
  git.html UseTemplates=1: "engine loads the blueprint and overlays those few fields": https://synchro.codeberg.page/rakata/formats/gff/git.html
  xoreos kotor2 Creature::load reads TemplateResRef, loads UTC, then loadProperties(blueprint) then instance (instance Appearance_Type overrides if present, inferred): https://xoreos.org/doxygen/d5/d02/kotor2_2creature_8cpp_source.html lines 105/118/147
  No TSL engine decomp found for the area-loader call site.
Gemini claim "existing saves keep vanilla NPC data, need manual KSE edits": PARTIAL.
  Supported: saved snapshot beats template edits (Q1-Q3).
  Not supported: "vanilla" (snapshot holds whatever state was saved, mod edits included). "Need KSE": community says fresh game or pre-visit save; manual GIT edit is only a test workaround (topic 5716).
  Boundary: community timing is per first visit, not per save. Engine per-area mechanics: UNVERIFIED.

Sources
- https://synchro.codeberg.page/rakata/internals/save_deep_dive.html (K1 only, no TSL mention)
- https://synchro.codeberg.page/rakata/formats/gff/git.html (K1 GOG, CSWSArea::LoadGIT 0x0050dd80)
- https://zff.dev/Synchro/rakata/issues/37
- https://xoreos.org/doxygen/d5/d02/kotor2_2creature_8cpp_source.html
- https://deadlystream.com/topic/9679-moddahnis-unique-look
- https://deadlystream.com/topic/5716-questions-about-editing-utc-files-in-tsl/
- https://deadlystream.com/topic/4431-k1-changing-appearance/
