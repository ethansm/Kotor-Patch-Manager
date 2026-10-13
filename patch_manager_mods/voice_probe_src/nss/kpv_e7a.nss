// kpv_e7a: E7 concurrency probe. 3 PlaySound calls in one frame.

void main()
{
    SendMessageToPC(GetFirstPC(), "kpv: e7a 3x PlaySound kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
    PlaySound("kpve1a");
}
