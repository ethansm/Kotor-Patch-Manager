#!/usr/bin/env python3
"""Fit the holster-editor preview camera from CALIB click lines (HW-UX2 U0b). Stdlib only.

Usage: fit_camera.py <log-lines.txt>   (lines from `kplog_parse.py --bare holster-editor_log.txt | rg CALIB`)

Model per round: pinhole camera at position C (world), orientation = heading (yaw about world Z, 0 = looking along +Y), pitch (up +) and roll; world +Z is up. fovy and ext come from the log line. Residual = pixel distance between the click and the
projected node. Six unknowns (C, yaw, pitch, roll) -> needs >= 3 clicks (6 equations); the script prints the residual
per click so a bad fit is visible. Nelder-Mead with random restarts.
"""
import math, random, re, sys

NODE = re.compile(r'(head|belt|rhand|lhand|rfoot|lfoot)=(?:[\w]+)?\(([-\d.]+),([-\d.]+),([-\d.]+)\)')
HEAD = re.compile(r'CALIB round=(\d+) step=(\d+) target=(.+?) click=\(([-\d.]+),([-\d.]+)\) yaw=([-\d.]+) zoom=([-\d.]+) panZ=([-\d.]+) '
                  r'fovy=([-\d.]+) ext=\(([-\d.]+),([-\d.]+),([-\d.]+),([-\d.]+)\)')
TARGET_NODE = {'head centre': 'head', 'belt': 'belt', 'right hand': 'rhand', 'left hand': 'lhand', 'right foot': 'rfoot', 'left foot': 'lfoot'}


def rot(yaw, pitch, roll):
    """Columns = camera right, up, back (world frame). yaw=0,pitch=0 looks along world +Y with +Z up; pitch>0 looks up."""
    cy, sy, cp, sp = math.cos(yaw), math.sin(yaw), math.cos(pitch), math.sin(pitch)
    f = (-sy * cp, cy * cp, sp)
    r = (cy, sy, 0.0)
    u = (r[1] * f[2] - r[2] * f[1], r[2] * f[0] - r[0] * f[2], r[0] * f[1] - r[1] * f[0])
    cr, sr = math.cos(roll), math.sin(roll)
    r2 = tuple(cr * r[i] + sr * u[i] for i in range(3))
    u2 = tuple(-sr * r[i] + cr * u[i] for i in range(3))
    return [[r2[i], u2[i], -f[i]] for i in range(3)]


def project(p, C, R, fovy, ext):
    d = [p[i] - C[i] for i in range(3)]
    # camera axes are the columns of R: right=R[:,0], up=R[:,1], back=R[:,2] (camera looks down -back)
    xc = sum(d[i] * R[i][0] for i in range(3)); yc = sum(d[i] * R[i][1] for i in range(3)); depth = -sum(d[i] * R[i][2] for i in range(3))
    if depth <= 0.05: return None
    t = math.tan(math.radians(fovy) / 2); asp = ext[2] / ext[3]
    return (ext[0] + (xc / (depth * t * asp) + 1) / 2 * ext[2], ext[1] + (1 - yc / (depth * t)) / 2 * ext[3])


def cost(x, pts, fovy, ext):
    C, R = x[:3], rot(x[3], x[4], x[5]); e = 0.0
    for p, c in pts:
        s = project(p, C, R, fovy, ext)
        e += 1e6 if s is None else (s[0] - c[0]) ** 2 + (s[1] - c[1]) ** 2
    return e


def nelder(f, x0, step, it=1500):
    n = len(x0); S = [list(x0)] + [[x0[j] + (step[j] if j == i else 0) for j in range(n)] for i in range(n)]
    F = [f(s) for s in S]
    for _ in range(it):
        o = sorted(range(n + 1), key=lambda i: F[i]); S = [S[i] for i in o]; F = [F[i] for i in o]
        c = [sum(S[i][j] for i in range(n)) / n for j in range(n)]
        xr = [c[j] + (c[j] - S[-1][j]) for j in range(n)]; fr = f(xr)
        if fr < F[0]:
            xe = [c[j] + 2 * (c[j] - S[-1][j]) for j in range(n)]; fe = f(xe)
            S[-1], F[-1] = (xe, fe) if fe < fr else (xr, fr)
        elif fr < F[-2]: S[-1], F[-1] = xr, fr
        else:
            xc = [c[j] + 0.5 * (S[-1][j] - c[j]) for j in range(n)]; fc = f(xc)
            if fc < F[-1]: S[-1], F[-1] = xc, fc
            else:
                for i in range(1, n + 1):
                    S[i] = [S[0][j] + 0.5 * (S[i][j] - S[0][j]) for j in range(n)]; F[i] = f(S[i])
    i = min(range(n + 1), key=lambda i: F[i]); return S[i], F[i]


def main():
    rounds = {}
    for line in open(sys.argv[1], encoding='utf-8', errors='replace'):
        h = HEAD.search(line)
        if not h: continue
        nodes = {m.group(1): tuple(float(m.group(k)) for k in (2, 3, 4)) for m in NODE.finditer(line)}
        key = (h.group(1), h.group(6), h.group(7), h.group(8))   # round, yaw, zoom, panZ
        tn = TARGET_NODE.get(h.group(3).strip())
        if tn in nodes:
            rounds.setdefault(key, {'fovy': float(h.group(9)), 'ext': tuple(float(h.group(i)) for i in range(10, 14)), 'pts': []})['pts'].append(
                (nodes[tn], (float(h.group(4)), float(h.group(5)))))
    if not rounds: print('no usable CALIB lines'); return 1
    random.seed(1)
    for key, r in rounds.items():
        pts, fovy, ext = r['pts'], r['fovy'], r['ext']
        print('round=%s yaw=%s zoom=%s panZ=%s clicks=%d' % (key + (len(pts),)))
        if len(pts) < 3: print('  need >= 3 clicks'); continue
        cx = sum(p[0][0] for p in pts) / len(pts); cy = sum(p[0][1] for p in pts) / len(pts); cz = sum(p[0][2] for p in pts) / len(pts)
        best = None
        for _ in range(60):
            x0 = [cx + random.uniform(-6, 6), cy + random.uniform(-6, 6), cz + random.uniform(-2, 3), random.uniform(-math.pi, math.pi),
                  random.uniform(-0.8, 0.8), random.uniform(-0.3, 0.3)]
            x, fv = nelder(lambda v: cost(v, pts, fovy, ext), x0, [1, 1, 0.5, 0.5, 0.2, 0.1])
            if best is None or fv < best[1]: best = (x, fv)
        x, fv = best; R = rot(x[3], x[4], x[5])
        print('  C=(%.3f,%.3f,%.3f) yaw=%.1f pitch=%.1f roll=%.1f deg  rms=%.1f px' % (x[0], x[1], x[2], math.degrees(x[3]), math.degrees(x[4]),
              math.degrees(x[5]), math.sqrt(fv / len(pts))))
        print('  forward(world)=(%.3f,%.3f,%.3f) up=(%.3f,%.3f,%.3f)' % (-R[0][2], -R[1][2], -R[2][2], R[0][1], R[1][1], R[2][1]))
        for p, c in pts:
            s = project(p, x[:3], R, fovy, ext)
            print('  click=(%.0f,%.0f) fit=%s' % (c[0], c[1], 'behind' if s is None else '(%.0f,%.0f)' % s))
    return 0


if __name__ == '__main__':
    sys.exit(main())
