// kpv_e1: E1 loose-Override route. Plays the 2 s tone kpve1a.wav.
// Pass = audible tone and the "kpv: e1" line in the log.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e1 PlaySound kpve1a");
    PlaySound("kpve1a");
}
