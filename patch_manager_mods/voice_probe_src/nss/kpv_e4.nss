// kpv_e4: E4 BarkString route. oNpc = nearest living creature to the PC.
// nStrRef 36878 = vanilla dialog.tlk row "Let's move together again." with SoundResRef p_bastila_rprty
// (picked 2026-10-13 from the 1,636 rows that carry a SoundResRef; text is real, not a placeholder).
// Pass = audio plays and overhead text matches the TLK row.

void main()
{
    object oPC = GetFirstPC();
    object oNpc;
    int nStrRef;

    nStrRef = 36878;

    oNpc = GetNearestCreature(CREATURE_TYPE_IS_ALIVE, TRUE, GetFirstPC());
    if (!GetIsObjectValid(oNpc))
    {
        SendMessageToPC(oPC, "kpv: e4 no creature found");
        return;
    }

    AssignCommand(oNpc, ActionSpeakStringByStrRef(nStrRef));
    SendMessageToPC(oPC, "kpv: e4 ActionSpeakStringByStrRef " + IntToString(nStrRef));
}
