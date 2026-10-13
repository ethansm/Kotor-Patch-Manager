// kpv_e8: E8 naming and case probe.
// Plays "KpvE8Mixed" now, "kpve8_sixteenchr" (16 chars) at +3 s,
// "kpve8_seventeench" (17 chars) at +6 s.
// Helper KpvE8Play logs the name and plays it.

void KpvE8Play(string sName)
{
    SendMessageToPC(GetFirstPC(), "kpv: e8 PlaySound " + sName);
    PlaySound(sName);
}

void main()
{
    KpvE8Play("KpvE8Mixed");
    DelayCommand(3.0, KpvE8Play("kpve8_sixteenchr"));
    DelayCommand(6.0, KpvE8Play("kpve8_seventeench"));
}
