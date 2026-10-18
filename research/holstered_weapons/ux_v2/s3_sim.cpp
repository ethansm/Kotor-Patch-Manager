#include "HolsterPreview.h"
#include <cstdio>
using namespace hs;
static float angBetween(const Q& a,const Q& b){ float d=std::fabs(a.w*b.w+a.x*b.x+a.y*b.y+a.z*b.z); if(d>1)d=1; return 2*std::acos(d)*57.29578f;}
int main(){
  for (int dir=1; dir>=-1; dir-=2) for (int axis=0; axis<3; ++axis){
  EditOverlay o; TuneCfg c; Q q0=qbase(30,84,10);
  o.active=true; o.scope=ScopeWeapon; o.placeQ=q0; o.cur.rot[0]=30;o.cur.rot[1]=84;o.cur.rot[2]=10;
  float maxErr=0,maxJump=0; float prev[3]={30,84,10}; int at=-1;
  for(int i=0;i<90;i++){
    overlayNudgeLocal(o,axis,(float)dir);
    EditView v; overlayGetEdit(o,v);
    Q rend=qbase(v.r.rot[0],v.r.rot[1],v.r.rot[2]);
    float err=angBetween(rend,o.placeQ);
    float j=0; for(int k=0;k<3;k++) j=std::fmax(j,std::fabs(wrap180(v.r.rot[k]-prev[k])));
    if(err>maxErr)maxErr=err; if(j>maxJump){maxJump=j;at=i;}
    for(int k=0;k<3;k++)prev[k]=v.r.rot[k];
    if(axis==0&&dir==1&&(i>=44&&i<=52)) printf(" i=%d rot=%.2f %.2f %.2f err=%.2f\n",i,v.r.rot[0],v.r.rot[1],v.r.rot[2],err);
  }
  printf("dir=%d axis=%d maxRenderErr=%.2f maxJump=%.2f at step %d\n",dir,axis,maxErr,maxJump,at);
  }
}
