// TEST OVERRIDE for k_hen_heartbt01 (vanilla henchman heartbeat). Doc 41 V-C trigger route 3: fires by itself every ~6 s
// per companion, no console needed. Replaces the vanilla script for EVERY save while staged in Override.
// v2: no credits, no overhead text, no counter (v1's SetLocalNumber guard did not hold). Only a message and the tone.
// CLEANUP: rm Override/k_hen_heartbt01.ncs (it does NOT start with kpv).

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: hb tone");
    PlaySound("kpve1a");
}
