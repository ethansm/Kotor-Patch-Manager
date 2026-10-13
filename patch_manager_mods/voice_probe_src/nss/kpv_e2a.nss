// kpv_e2a: E2 container a. kpve2a.wav = RIFF PCM 16-bit mono 22050 Hz.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e2a PlaySound kpve2a");
    PlaySound("kpve2a");
}
