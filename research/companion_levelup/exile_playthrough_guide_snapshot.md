# KOTOR II Playthrough & Narrative Optimization Guide

A comprehensive, community-vetted reference guide for a **Light Side Female "Jack-of-all-Trades" (Jedi Sentinel → Jedi Watchman)** playthrough of *Star Wars: Knights of the Old Republic II: The Sith Lords*.

This guide is specifically tailored to the **r/kotor Neocities Full Community Mod Build** (including **TSLRCM**, **PartySwap**, and **Extended Enclave**), correcting common AI hallucinations and integrating mechanics from **StrategyWiki**, **r/kotor community compendiums (Snigaroo)**, and **DeadlyStream**.

---

## 1. Executive Fact-Check: Auditing Gemini / LLM Advice

Large language models frequently conflate *KotOR 1* and *KotOR 2* rulesets or hallucinate quest mechanics. Before starting, review these critical engine verifications:

| Topic / Claim | LLM Claim | Verified Game Engine Reality | Build Impact |
| :--- | :--- | :--- | :--- |
| **Sentinel Class Skills** | *"Sentinels effortlessly craft weapon mods"* & suggested *"Consular with Class Skill: Repair"* | **Sentinel does NOT have Repair as a class skill.** Consular has Repair naturally. Sentinel and Watchman both treat Repair as cross-class. | **Critical:** Sentinel **must take `Class Skill: Repair` at Level 1** (or 3). Without it, reaching Repair 20 costs double skill points and caps at half max rank. |
| **Female Exile & Feats** | Female Exile misses out on *Battle Precognition* because she recruits Disciple instead of Handmaiden. | **False with PartySwap installed.** Handmaiden joins the ship at Telos Polar Academy. You can spar with her to unlock **Battle Precognition** (+WIS modifier added directly to Defense in robes/unarmored). | **Massive buff:** High Wisdom directly boosts Defense while wearing Jedi robes. |
| **Dantooine Saber Part** | *"Head to the Crystal Cave... Collect the final lightsaber fixture here."* | **The Crystal Cave does NOT contain a lightsaber fixture.** It contains only focusing crystals and your personal Name Crystal. | Lightsaber parts come dynamically from quest rewards (Telos, Lootra on Nar Shaddaa, Jorran in Enclave sublevel, and defeating Visas Marr). |
| **Palace Museum & Thorium** | Use a Thorium Charge to blow open the Onderon Royal Palace Museum for the Ossus Keeper Robe. | **Thorium Charges cannot be used on Onderon.** The museum cases/vaults are opened via terminal math puzzles. Thorium Charges are *only* used on the Dxun jungle cache and the Korriban Sith Academy sealed door. | Do not hoard or seek Thorium Charges for Onderon. Solve the terminal puzzle instead. |
| **HK Pacifist Package** | Calls it *"HK-47's combat upgrade"*. | The Pacifist Package is a **humorous protocol package** that forces HK-47 to apologize for violence and adopt a pacifist personality—the exact opposite of a combat upgrade. | Purely comic relief; not a combat boost. |
| **Peragus Loot Fluff** | *"Sonic Mining Laser: High-accuracy early sidearm with infinite ammo."* | It is simply a standard **Mining Laser**. In KotOR, *all* blasters have infinite ammo (the engine has no ammo mechanic). | Minor LLM flavor fluff. |
| **HK Factory Requirement** | *"HK-47 must personally destroy 3 squads... If he is on the ship during these fights, the quest will not unlock."* | HK-47 does **not** need to be physically in your party during combat. The hard gate is **talking to HK-47 on the Ebon Hawk first** about the sonic sensor to officially activate the quest. Encounters defeated before that talk don't count. | Talk to HK-47 immediately upon rebuilding him; you do not need to drag him into every fight. |

---

## 2. Character Creation & Build Blueprint

### Starting Class & Archetype
* **Starting Class**: **Jedi Sentinel** (levels 1–15)
  * High skill points base (3 + INT modifier = 5 skill points/level with 14 INT).
  * Native immunities to Paralysis, Stuns, and Fear (`Force Immunity` feat line).
* **Prestige Class**: **Jedi Watchman** (levels 15+)
  * Continues high skill progression (5 points/level).
  * Adds `Force Camouflage` and scaling `Sneak Attack` damage dice (devastating against stunned or Force-insanity targets).

### Starting Attributes (32-Point Buy)
* **STR: 10** — Keep at 10. Accuracy scales off DEX via Finesse; lightsaber upgrades supply massive flat damage.
* **DEX: 14** — Early accuracy and defense.
* **CON: 14** — **Key benchmark.** Invest your attribute points at levels 4, 8, 12, and 16 here to reach **18 CON**. An 18 natural CON is required to equip the most powerful top-tier D-Package implants.
* **INT: 14** — Provides 5 skill points per level as a Sentinel, allowing you to hit all personal narrative and repair skill checks effortlessly.
* **WIS: 14** — Boosts Force DC, Force Point pool, and adds directly to **Defense** once you get *Battle Precognition* from Handmaiden.
* **CHA: 12** — Boosts Persuade, adds to Force DC, and reduces the opposing-alignment FP cost penalty.

### Feat Progression Priorities
1. **Level 1**: **`Class Skill: Repair`** *(Mandatory!)* Turns Repair into a native class skill.
2. **Level 3+**:
   * **Two-Weapon Fighting** line (Basic → Improved → Master) if dual-wielding or using a Double-bladed lightsaber.
   * **Weapon Finesse** (Finesse: Melee Weapons or Lightsabers) so melee attack rolls use DEX modifier.
   * **Toughness** line for bonus vitality and flat damage reduction.

### Solo Skill Targets (Ebon Hawk & Dialogue Checks)
Companions cannot assist with ship conversations, droid repairs, or player dialogue checks. Your Exile must hit these benchmarks personally (base ranks + gear + Master Valor):

* **Persuade**: **Keep Maxed** (Player character exclusive; essential for major companion revelations and peaceful quest resolutions).
* **Repair**: **Target ~20**
  * **Rank 20**: Grants 100% item breakdown efficiency at workbenches.
  * **Ranks 8, 11, 14, 17**: Unlocks all four HK-47 memory core repairs (+CON, +WIS, +Defense, +FP).
  * Unlocks T3-M4’s encrypted memory holograms regarding Revan and the Ebon Hawk.
* **Computer Use**: **Target 15–17**
  * Completes T3-M4’s secondary upgrade checks (+INT, +Defense).
  * Slices all planetary terminal consoles for lore logs with minimal spike cost.
* **Awareness**: **Target 15**
  * Unlocks perceptive dialogue options across the galaxy (detecting lies, noticing Kreia's subtle manipulations, and unmasking Jedi Master motives).
* **Treat Injury / Security / Demolitions**: Keep at 1–2 invested ranks; gear and Force buffs cover the rest.

---

## 3. Planet Routing Strategy: Why "Nar Shaddaa First" Wins

While partial "planet hopping" (e.g. dipping into Dantooine) is often proposed, the established community gold standard is:

```
Peragus II  ──►  Telos (Citadel & Surface)  ──►  Nar Shaddaa (Full Clear)
                                                       │
Dxun / Onderon (Part 1)  ◄──  Dantooine (Full Clear)  ◄┘
        │
        ▼
   Korriban  ──►  Dxun / Onderon (Part 2)  ──►  Point of No Return (Rebuilt Enclave)
```

### Why Nar Shaddaa First is the Community Standard:
1. **Fastest Lightsaber**:
   * Part 1: Guaranteed from Telos (Chodo Habat / Grenn).
   * Part 2: Rewarded by Lootra in the Nar Shaddaa Docks (resolve his wife's situation).
   * Part 3: Defeating Visas Marr when she attacks the Ebon Hawk (triggers naturally upon reaching 25% alignment shift, which typically occurs on or right before landing on Nar Shaddaa).
   * Result: You construct your lightsaber within ~1 hour of arriving.
2. **The Companion Recruitment Factory**:
   * Recruits **Mira**, recruits **G0-T0**, and supplies the final parts to assemble **HK-47**.
   * Converts **Atton** and **Bao-Dur** into Jedi before ever leaving the planet.
3. **Smooth TSLRCM Bounty Hunter Gauntlet**:
   * Restored content forces Atton, Mira, and your party to fight alone without the Exile. If Atton and Bao-Dur are already trained as lightsaber-wielding Jedi, these solo segments are trivial instead of punishing.

### Why Korriban Must Be Saved for Late:
* **Alignment Gate**: The secret tomb in the Shyrack Cave requires you to be strongly aligned (deep Light or deep Dark) to open its door. Visiting early often results in being locked out.
* **Boss Scaling**: The phantom visions inside the tomb (Darth Malak, your former Republic squad) scale aggressively with character level and require mid-to-late-game gear.

---

## 4. Companion Deployment & Influence Rules

### The "Kreia Interception" Rule
Kreia's dialogue script takes conversational priority in the field. If you perform a moral deed with Kreia in your active party, she often intercepts the dialogue to lecture you, denying the influence gain to your second companion.
* **Rule**: When exploring hubs with high moral opportunities (Refugee Sector on Nar Shaddaa, Khoonda on Dantooine), **leave Kreia on the ship**. Bring the companion who needs influence (Bao-Dur or Atton) to absorb the approval triggers.

### Managing the PartySwap Mod (Female Exile)
With the **PartySwap** mod installed:
* Both **Handmaiden** and **Disciple** are recruited.
* You receive an armband/item to switch which companion is displayed in the active party selection screen.
* **Handmaiden (Cargo Hold)**: Spar with her at levels 10, 14, and 18. Winning these bouts unlocks **Battle Precognition** (+WIS modifier to Defense) and opens her dialogue path to become a Jedi Guardian.
* **Disciple (Briefing Room)**: Exhaust his dialogue regarding Republic history, the Jedi Council, and Revan to gain rapid influence and train him as a Jedi Consular.

### Companion Conversion Quick Reference
* **Atton (Jedi Sentinel)**: Walk through the Refugee Landing Pad on Nar Shaddaa with him until two Twi'leks pull you aside to reveal his past. Return to the Ebon Hawk, confront him with empathy, forgive him, and train him.
* **Bao-Dur (Jedi Guardian)**: Nar Shaddaa has three quick Light Side triggers: heal the sick refugee, defend the extorted refugee near the docks, and repair the airspeeder. Maxing his influence lets you train him immediately.
* **Mira (Jedi Sentinel)**: Gain influence through ship dialogue, show empathy regarding her reluctance to kill, and escort her back to the central pit overlooking the Nar Shaddaa docks.
* **Handmaiden (Jedi Guardian)**: Complete all three sparring matches in the cargo hold, put on regular clothes when requested, and discuss the Force.

---

## 5. Critical Missables & Lockouts Checklist

### 1. Nar Shaddaa Exchange Invitation (Point of Lockdown)
Once your notoriety with the Exchange peaks, a cutscene triggers and you receive an invitation from Visquis to meet at the Jekk'Jekk Tarr.
* **Warning**: Entering this sequence triggers a continuous gauntlet that prevents returning to the Ebon Hawk or regular shops for several hours.
* **Checklist**: Complete all Refugee Sector side quests, free the sick refugee, and repair the airspeeder **before** entering the Jekk'Jekk Tarr.

### 2. TSLRCM HK-47 Manufacturing Plant Trigger
* **Assemble HK-47** on the Ebon Hawk as early as possible.
* **Exhaust his dialogue immediately**: Question him about the HK-50 droids and show him the Sonic Imprint Sensor retrieved from Peragus until you obtain the quest entry to track their production signal.
* **Defeat 3 HK-50 squads**: Only squads defeated *after* initiating this journal quest count toward the 3-squad requirement.

### 3. The Absolute Galaxy Point of No Return
* Once all four main planets (Dantooine, Nar Shaddaa, Onderon/Dxun, Korriban) are resolved, you are instructed to return to the **Rebuilt Jedi Enclave on Dantooine**.
* **CRITICAL LOCKOUT**: The moment you step through the entrance of the Rebuilt Jedi Enclave, **free travel across the galaxy is permanently disabled**.
* The game enters an unbroken endgame sequence: Dantooine Enclave → Citadel Station under siege → Malachor V (including the restored HK Factory and Trayus Academy).
* **Pre-Enclave Checklist**:
  * Pass all personal Repair and Computer Use checks on T3-M4 and HK-47.
  * Complete all companion training and ship conversations.
  * Finish all planetary side quests and spend remaining credits at endgame vendors (Geeda, Dendis Dobo).

---

## 6. Authoritative References & Further Reading

* **[StrategyWiki — KotOR II: The Sith Lords](https://strategywiki.org/wiki/Star_Wars_Knights_of_the_Old_Republic_II:_The_Sith_Lords)**: The definitive resource for exact DC formulas, container loot tables, influence matrices, and terminal puzzle solutions.
* **[r/kotor Community Guides (Snigaroo's Compendium)](https://www.reddit.com/r/kotor/)**: The standard community reference for restored content quirks, companion influence routing, and mod compatibility.
* **[DeadlyStream TSLRCM Official Documentation](https://deadlystream.com/)**: Restored content changelogs, quest flowcharts, and technical patch notes.
