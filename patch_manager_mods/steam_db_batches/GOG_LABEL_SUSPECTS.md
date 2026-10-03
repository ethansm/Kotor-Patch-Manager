# GOG DB label suspects (for the user; the GOG DB is never edited without approval)

Rule (user, 2026-10-03): the PR changes only the Steam DB. GOG label bugs are recorded here and proposed only when
confirmed. Evidence comes from the GOG binary on the OpenKotOR server (`fp.tsv`: callees, displacements, immediates of
the function at the GOG address) plus the Steam counterpart's decompile.

How Steam rows are handled meanwhile: a row whose Steam counterpart is certain but whose GOG name contradicts the body
gets final verdict `label_suspect` and is not shipped.

| # | GOG addr | GOG DB / server name | what the GOG body does (GOG fp) | Steam counterpart | name it fits | status |
|---|---|---|---|---|---|---|
| 1 | 0x4E0C60 | `CGuiInGame::GetMiniMapVisible` | 49 B, no callees; disp `+0x3C,+0x40,+0x84`: if `[this+0x3C]!=[this+0x40]` calls `vt+0x84` on the panel (dismisses it) | 0x7D0560 | `RemoveGUIPopUps` | confirmed swap (pair with #2) |
| 2 | 0x4E0CA0 | `CGuiInGame::RemoveGUIPopUps` | 120 B; calls `CClientExoApp::GetClientOptions` 0x40CCF0 and `GetMiniGame` 0x40D580; reads `[this+0x98]+0x59B4` bit; imm 3, 0x10; modifies nothing | 0x7D05A0 | `GetMiniMapVisible` | confirmed swap (pair with #1) |
| 3 | 0x4DBDE0 | `CGuiInGame::IsAnimationPlayingInDialog` | 55 B; if `[+0x64]` sets `[+0xFB4]=1` and calls `vt+0x84` on `[+0x3C]`; returns nothing | 0x7CB6E0 | not a predicate; real name unknown | suspect |
| 4 | 0x4DF1F0 | `CGuiInGame::GetCanOrientInDialog` | 90 B, RET 4; disp `+0x9C`, imm `0x440` | 0x7CEAF0 (Batch 1 chose 0x7CF710 by meaning: the `+0xF24`/`+0xF28` list pair) | to check | suspect |

Impact on GOG today: KPM GameAPI's `CGuiInGame::GetMiniMapVisible` wrapper (used by AdditionalConsoleCommands' init)
calls the popup-dismissing function on GOG.

Found by: Steam DB calibration chunk 1 (2026-10-03), reviewer `label_suspect` cases checked by the lead on the GOG fp.
