// kpv_e2b: E2 container b. kpve2b = 58-byte fake header + MP3 32k (data size 0).

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e2b PlaySound kpve2b");
    PlaySound("kpve2b");
}
