// kpv_e7b: E7 concurrency probe. 8 PlaySound calls in one frame.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e7b 8x PlaySound kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
}
