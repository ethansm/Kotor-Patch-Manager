// kpv_c1: runscript control. Proves the console route runs scripts at all, independent of audio.
// Visible effects: +1234 credits (check the inventory/credits readout) and the PC's overhead line.

void main()
{
    GiveGoldToCreature(GetFirstPC(), 1234);
    AssignCommand(GetFirstPC(), SpeakString("kpv c1 ran"));
    SendMessageToPC(GetFirstPC(), "kpv: c1 ran");
}
