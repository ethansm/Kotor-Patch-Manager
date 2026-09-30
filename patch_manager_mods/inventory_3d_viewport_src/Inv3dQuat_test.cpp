#include "Inv3dQuat.h"
#include <stdio.h>
#include <string.h>
static int fails = 0;
static void chk(const char* n, const float* g, float a, float b, float c) {
    bool ok = fabsf(g[0]-a)<1e-5f && fabsf(g[1]-b)<1e-5f && fabsf(g[2]-c)<1e-5f;
    printf("%s %s got (%.4f %.4f %.4f) want (%g %g %g)\n", ok?"PASS":"FAIL", n, g[0],g[1],g[2],a,b,c); if(!ok) ++fails;
}
int main() {
    const float X[3]={1,0,0},Y[3]={0,1,0},Z[3]={0,0,1}; float o[3];
    qrotate(qaxis(Y,90), Z, o); chk("Y90 * (0,0,1)", o, 1,0,0);
    qrotate(qaxis(Z,90), X, o); chk("Z90 * (1,0,0)", o, 0,1,0);
    qrotate(qaxis(X,90), Y, o); chk("X90 * (0,1,0)", o, 0,0,1);
    // composition: qmul(a,b) applies b first: (Y90 * X90) on (0,1,0): X90 -> (0,0,1), Y90 -> (1,0,0)
    qrotate(qmul(qaxis(Y,90), qaxis(X,90)), Y, o); chk("(Y90*X90)(0,1,0) applies X first", o, 1,0,0);
    qrotate(qmul(qaxis(X,90), qaxis(Y,90)), Y, o); chk("(X90*Y90)(0,1,0) applies Y first", o, 0,0,1);
    // camera-frame yaw: cam up = Z, right = Y: positive yaw about up moves the object's +right point away (toward fwd=-X)
    const float R[3]={0,1,0}; qrotate(qaxis(Z,90), R, o); chk("yaw90 about up moves right pt to -X (fwd)", o, -1,0,0);
    // base: qZ(90)*qY(0)*qX(90) applied to (0,1,0): X90 -> (0,0,1), Z90 leaves it
    qrotate(qbase(90,0,90), Y, o); chk("qbase(90,0,90)(0,1,0)", o, 0,0,1);
    Q q = qnormalize(qmul(qaxis(Z,33), qaxis(X,17))); printf("norm %.6f\n", q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
    float f[3]={-1,0,0}, u[3]={0,0,1}, r[3]; vcross(f,u,r); chk("fwd x up = right", r, 0,1,0);
    printf(fails? "FAILED\n":"ALL PASS\n"); return fails;
}
