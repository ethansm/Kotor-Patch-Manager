// kpv_e2d: E2 container d. kpve2d = 470-byte MP3-frame prefix + RIFF PCM.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e2d PlaySound kpve2d");
    PlaySound("kpve2d");
}
