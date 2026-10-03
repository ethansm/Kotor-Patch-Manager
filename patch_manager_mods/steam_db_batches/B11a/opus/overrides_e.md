# B11a Opus gate part e (8 AUTO rows)

Method: GOG fp vs Steam fp (size/retn/n_insn/imm/disp/str, callees mapped), whole-binary best match, Steam asm, Mac x86 symbol disp sets.
All 8 Steam picks are the unique top whole-binary match (next best <= 3.5 of 4-6) and the field offsets agree with the Mac symbol of the same name.

| GOG | Name | Steam | Evidence | Verdict |
|---|---|---|---|---|
| 0x0040c7d0 | CClientExoApp::GetClientName | 0x0073f310 | `0073f326 ADD ECX,0x140`, `0073f336 CALL 0x007335f0`, RET 4; Mac imm 0x140 | verified |
| 0x0040c790 | CClientExoApp::GetCaptureEvents | 0x0073f2d0 | `0073f2df MOV EDX,[ECX+0x8c]`; Mac disp 0x8c; Internal::SetCaptureEvents writes +0x8c | verified |
| 0x0040dc30 | CClientExoApp::SetFutureLeader | 0x00740770 | `00740788 MOV [EDX+0x338],EAX`, RET 4; Mac disp 0x338 | verified |
| 0x004316e0 | CClientExoAppInternal::DestroyQuestJournal | 0x007850c0 | `007850cc CMP [EAX+0x38],0`, `007850ea CALL 0x00785130`, free, `00785117 MOV [ECX+0x38],0`; Mac disp 0x38 | verified |
| 0x0044d5b0 | CClientExoAppInternal::PlayMoviesInExoArrayList | 0x007a0fb0 | sz309 RET 0x18; SetInputActive/HideMouse/ShowMouse/SetGlobalUnpauseableState + CExoMoviePlayer 0x93b590/5c0/5e0; disp 0x130/0x1f8/0x40 = GOG = Mac | verified |
| 0x0040c870 | CClientExoApp::GetWorldTimer | 0x0073f3b0 | `0073f3bf MOV EDX,[ECX+0x24]`; Mac disp 0x24; called by Internal::HandleMessage 0x781840 (as GOG) | verified |
| 0x00431ef0 | CClientExoAppInternal::SetCaptureEvents | 0x007858d0 | `00785902 MOV [EAX+0x8c],1`; strings "HD0:%s", "Start: %d\n"; 14 callees 1:1 | verified |
| 0x0040c930 | CClientExoApp::GetPlayerCreatureId | 0x0073f470 | `0073f47f MOV EDX,[ECX+0x20]`; Mac disp 0x20 (twin GetWorldTimer +0x24) | verified |

Note: DestroyQuestJournal's callee 0x785130 is GOG 0x431750 "DestroyQuestJournal_2" (label_suspect in part a-d; it's the journal dtor, called on Internal+0x38). That doesn't affect this row.

## AUTO meaning checks

| GOG | route | body = counterpart | name fits body | result |
|---|---|---|---|---|
| 0x0040c7d0 | sig | yes | yes (Mac +0x140) | pass |
| 0x0040c790 | sig | yes | yes (Mac +0x8c) | pass |
| 0x0040dc30 | sig | yes | yes (Mac +0x338) | pass |
| 0x004316e0 | sig | yes | yes (Mac +0x38) | pass |
| 0x0044d5b0 | sig | yes | yes (Mac disp set) | pass |
| 0x0040c870 | c2 | yes | yes (Mac +0x24) | pass |
| 0x00431ef0 | c2 | yes | yes (+0x8c flag, capture file strings) | pass |
| 0x0040c930 | c2 | yes | yes (Mac +0x20) | pass |

sample_errors: 0 (part e has no sample rows)
