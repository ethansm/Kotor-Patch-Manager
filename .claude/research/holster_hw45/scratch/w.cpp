#include "HolsterQuat.h"
#include "HolsterReach.h"
#include <cstdio>
#include <cstdlib>
using namespace hs;
int main(int argc,char**argv){ float wt=(float)atof(argv[1]); const float ax[3]={0.8f,0,0.6f}; Q hint{1,0,0,0},prev{1,0,0,0}; bool have=false; float mx=0,at=0;
 for(float d=100; d<=260; d+=0.1f){ Q cur=reachConj(qaxis(ax,d)); Q dq=reachWristDelta(Q{1,0,0,0},Q{1,0,0,0},0.f,cur,180.f,wt,have?&hint:nullptr); hint=dq;
  if(have){ float s=reachQAngleDeg(qmul(prev,reachConj(dq))); if(s>mx){mx=s;at=d;} } prev=dq; have=true; }
 std::printf("weight %.2f: max step of dq fed to split = %.2f deg at turn %.1f\n",wt,mx,at); }
