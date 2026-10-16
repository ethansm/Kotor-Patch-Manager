#!/usr/bin/env python3
"""holster_pose_solve.py - solve holster.ini place_rot (Z*Y*X euler degrees, the DLL's qbase order) from a target pose.

Bone frame (pelvis_g / torsoUpr_g, live-checked 10-16): x = character's right, y = forward, z = up.
You give the direction the weapon's LONG axis should point (tilt in the back plane, lean away from the back) and which
model axis must face backward (flat against the back). Weapon model axes: sword blade = model +z, gun barrel = model +y.

  python3 scripts/holster_pose_solve.py blade --tilt 30 --back 8            # sword: hilt over right shoulder, tip down-left
  python3 scripts/holster_pose_solve.py barrel --tilt 30 --back 5           # rifle: barrel down-left, side flat on the back
  python3 scripts/holster_pose_solve.py barrel --tilt 0 --back 3 --side z+  # heavy: barrel down, grip (model -z) points left
  --mirror  mirrors tilt for the left shoulder (criss-cross); negate place_off x yourself.

Pure stdlib (no numpy). Prints place_rot=rx,ry,rz.
"""
import argparse
import math


def _norm(v):
    n = math.sqrt(sum(c * c for c in v))
    return [c / n for c in v]


def _cross(a, b):
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]]


def _dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def solve(axis, tilt, back, side, mirror=False):
    t, b = math.radians(-tilt if mirror else tilt), math.radians(back)
    d = [-math.sin(t) * math.cos(b), -math.sin(b), -math.cos(t) * math.cos(b)]   # long axis: down, toward the left hip, out from the back
    if side in ("y", "-y"):   # the named model axis faces +y/-y (flat side toward/away from the back)
        n = [0.0, 1.0 if side == "y" else -1.0, 0.0]
    else:                     # 'z+'/'z-': model z points to the character's right (+) or left (-)... grip (-z) the other way
        n = [1.0 if side == "z+" else -1.0, 0.0, 0.0]
    n = _norm([n[i] - d[i] * _dot(n, d) for i in range(3)])
    if axis == "blade":       # model z = d, model x = n, model y = z cross x
        x, z = n, d
        y = _cross(z, x)
    elif side in ("z+", "z-"):   # gun, model y = d, model z = n, model x = y cross z
        y, z = d, n
        x = _cross(y, z)
    else:                     # gun, model y = d, model x = n, model z = x cross y
        y, x = d, n
        z = _cross(x, y)
    R = [[x[i], y[i], z[i]] for i in range(3)]   # columns = images of the model axes
    ry = math.degrees(-math.asin(max(-1.0, min(1.0, R[2][0]))))
    rx = math.degrees(math.atan2(R[2][1], R[2][2]))
    rz = math.degrees(math.atan2(R[1][0], R[0][0]))
    return rx, ry, rz


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("axis", choices=["blade", "barrel"])
    ap.add_argument("--tilt", type=float, default=30.0, help="degrees from vertical toward the left hip (0 = straight down)")
    ap.add_argument("--back", type=float, default=8.0, help="degrees the tip leans away from the back")
    ap.add_argument("--side", default="y", choices=["y", "-y", "z+", "z-"], help="model axis facing the back (y) or grip side (z+/z-)")
    ap.add_argument("--mirror", action="store_true")
    a = ap.parse_args()
    print("place_rot=%.0f,%.0f,%.0f" % solve(a.axis, a.tilt, a.back, a.side, a.mirror))


if __name__ == "__main__":
    main()
