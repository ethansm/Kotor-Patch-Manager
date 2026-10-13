// kpv_e2c: E2 container c. kpve2c = bare .mp3 file.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e2c PlaySound kpve2c");
    PlaySound("kpve2c");
}
