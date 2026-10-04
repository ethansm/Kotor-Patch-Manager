# Opus gate B13a part r (REJECT/NOCAND rows, lead addition)

- These rows have no shippable gate pick. For each, search the Steam binary for the true counterpart of the GOG body
  (whole-binary fp compare: size, retn, n_insn, callee set mapped via known names, disp/imm sets; neighbour runs).
- Ship (`verified`/`likely`) only a counterpart whose Steam body you checked against the GOG fp yourself (quote an asm line).
- If the GOG label contradicts the body, use `label_suspect` and name the Mac symbol.
- Otherwise `rejected` (gate pick wrong, none found) or `unresolved`. sample_errors does not apply to this part (write 0).

| # | gate | Class::Name | GOG addr | gate pick | gog size/retn |
|---|---|---|---|---|---|
| 1 | NOCAND | CSWCCreature::SwitchMode | 0x00443170 | - | 70/8 |
