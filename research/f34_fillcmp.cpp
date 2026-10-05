#include <cstdio>
// shield drawGauge: int(double(h)*cur/mx+0.5)  vs  gui2d::vfillClip with frac=float(cur)/float(mx)
static int g(int h,int c,int m){ return int(double(h)*c/m+0.5); }
static int v(int h,float f){ if(!(f>0.0f))f=0.0f; if(f>1.0f)f=1.0f; int fh=int(float(h)*f+0.5f); if(fh>h)fh=h; if(fh<0)fh=0; return fh; }
int main(){ long long n=0,d=0; int eh=0,ec=0,em=0;
 for(int h=4;h<=256;++h) for(int m=1;m<=2000;++m) for(int c=0;c<=m;++c){ ++n; if(g(h,c,m)!=v(h,float(c)/float(m))){ if(!d){eh=h;ec=c;em=m;} ++d; } }
 printf("cases=%lld diffs=%lld first h=%d cur=%d max=%d\n",n,d,eh,ec,em); }
