# Proposal (from companion-selfheal, doc 27 §7) for the queue-hud / hud-host owner

Status: proposal, 2026-10-05. Nothing in queue-hud or hud-host was edited. Rows are doc 24 CS10-CS12.

## 1. Shared strip: `ReserveSlot`
Add this to `K2HudApi` (append-only; struct_size gates it):
```c
/* Reserve a slot in the companion card's right-to-left strip. order 0 = right-most (next to the vit).
   Returns 1 and writes *out (screen rect, height = the slot side), 0 if the card is unknown or the strip is full. */
int32_t (KPATCH_CALL *ReserveSlot)(const char* owner, int32_t card, int32_t order, int32_t width, K2Rect* out);
```
- companion-selfheal asks for order 0, width = `shui::buttonRect` side + ring overhang.
- queue-hud asks for 1..n (queue icons, Cancel last, so it is left-most).
- hud-host computes the strip from the live card/vit rects once per layout and replaces the `selfHealReserve(vit)` constant (QueueHudLogic.h:178).
- Until it lands, the constant stays. A selfheal contract test pins that `selfHealReserve` matches `shui::buttonRect` on vit (3671,1285,47,96).

## 2. Hook collision on 0x0040F5A0
- hud-host 0.3.0-dev (9e18444) hooks 0x0040F5A0 for `K2HUD_BUILD`.
- companion-selfheal 0.3.0+ hooks the same address, and that build is live.
- Ask: before hud-host 0.3.0 installs, either
  - (a) keep selfheal's hook and delay the hud-host BUILD install, or
  - (b) ship hud-host 0.3.0 together with a selfheal build that uses `Register(K2HUD_BUILD, ...)` and drops its own hook.
- Selfheal prefers (b). We will build it once hud-host 0.3.0 is final.

## 3. Queue UX kit (user direction 10-05)
- Dark-green hotbar panels:
  - border (0,44,26), hover (208,204,170);
  - fill (0,0,0,175), inset 6/128;
  - radius 22/128;
  - accent (10,96,80).
  - Source: `scripts/gen_hotbar_textures.py`.
- A Cancel button modelled on vanilla "Cancel Combat".
- A slim per-companion variant, with icon side = the selfheal button side.
- Selfheal's CS9 `csh_disc` (fill (0,0,0,175), rim (10,96,80)) already uses these colours.
