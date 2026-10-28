# B17 Opus gate part b (rows 22-42)

All 21 GOG/Steam pairs have identical fp rows (size, retn, n_insn, disp, imm, callee count, str count), except row 27, where the pipeline candidate was wrong and the reviewer correction 0x0091eb8b is the identical twin. Callee identity was checked by matching Steam callee size to the GOG-named callee size.

- 22 FrameBufferModificationsATI: label_suspect. The body is a VBO and multitexture vertex-array setup (0x8892 GL_ARRAY_BUFFER, 0x84c0 GL_TEXTURE0, 0x1406 GL_FLOAT; callees AurMultiTextureAvailable and AurVertexBufferObjectARBAvailable). The Mac FrameBufferModifications* bodies use blend constants 0x302/0x303 and double constants, which this body lacks. The flag shows a mac_rotation. The right name is not known.
- 23 GetResource: label_suspect. The only callee is DelIndex (size 75 on both exes), so the body removes an array entry. Counterpart is right.
- 24 ExplosionEmitter::Update: verified. Anchor: PUSH 0x74 followed by operator new, plus 16 hits on +0x1c8.
- 25 ImageRead16bit: verified. Anchor: SUB ESP,0x114.
- 26 PlayOutOfOrderAnimation: verified. Anchors: 0x34 alloc, 0x209 store, List::Insert callee.
- 27 rand: verified correction to 0x0091eb8b. Anchor: the LCG constants 0x343fd/0x269ec3. Steam fp names it _rand. The original 0x008103c0 is unrelated (328 bytes).
- 28/34/35 CClientOptions SetShadows / SetSoftShadows / SetVSync: verified. Each has its own field (+0x2c / +0x30 / +0x48) and an Aur* callee whose size matches.
- 29 SetBlendingMode: verified. The .c calls glBlendFunc, glDepthMask and glAlphaFunc.
- 30 WriteDWORD: verified. Callees ExtendWriteBuffer and WriteUnsigned match by size.
- 31 SetSizeBoundingRect: likely. It is a 2-store setter stub.
- 32 QueryTimer: verified. Anchors: QPC and 1000000.
- 33 RestoreDesktopDisplayMode: verified. It calls ChangeDisplaySettingsA(NULL,0).
- 36 ~CSWSPlayerLUOInventory: verified. It makes 4 Clear* calls and calls ~CExoLinkedListInternal.
- 37 LoadBodyBag: verified. Anchors: the BodyBagId/PositionX/Y/Z strings and ReadField callees.
- 38 BinaryInsertModel: verified. Anchors: self-recursion and the List::Insert callee.
- 39 Quicksave: verified. Anchors: the 'QUICKSAVE' string and SendSaveGameRequest.
- 40 ResizePool: verified. Anchor: GL_BUFFER_SIZE 0x8764.
- 41 OnTexQualLeft: verified. Anchors: decrement of +0x2920, PlayGuiSound, SetTexQualText.
- 42 CSWGui3DSceneView::Draw: likely. It is a 37-byte forward to vtable slot 3 of this+0x60.

Sampled reviewer `match` rows 28-42: all 15 have the correct counterpart and verdict.

sample_errors: 0
