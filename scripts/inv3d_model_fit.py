"""Hierarchical model-space bounding boxes for KOTOR item MDLs (Inventory 3D Viewport v2).

Why this exists: the MDL header bbox is a constant placeholder in every item model, and pykotor's per-mesh
bb_min/bb_max and vertex_positions are EMPTY/zero for several classes (blasters). So this module walks the binary
MDL itself, reads mesh vertex positions straight from the MDX, composes the node transforms (position + quaternion,
parent -> child), and returns a real extent / centre / longest axis.

Binary layout used (K2, offsets relative to the data start = file offset 12), cross-checked against pykotor's
io_mdl.py (_TrimeshHeader K2_SIZE = 340):
  geometry header 80 B: root node offset at +0x28.  model header adds names-array offset at +184, count at +188.
  node header 80 B: u16 type@0 u16 id@2 u16 nameIdx@4 | pos f3@16 | orient f4@28 (file order w,x,y,z) |
      children array @44 (ptr,count,alloc) | ctrl-key array @56 | ctrl-data array @68.
  trimesh (type & 0x20): header follows the 80 B node header. mdx stride @252, vertex offset (in vertex) @260,
      vertex count u16 @304, render flag u8 @313, mdx data offset u32 @332.

Usage:  inv3d_model_fit.py <game dir> [out.txt]     -> survey of every baseitems class variation
        (import: load_model, model_bounds, survey_installation)
"""
import struct, sys, math

NT_LIGHT, NT_EMITTER, NT_CAMERA, NT_REF, NT_MESH, NT_SKIN, NT_ANIM, NT_DANGLY, NT_AABB, NT_SABER = (
    0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x800)


def load_model(inst, name):
    """(mdl_bytes, mdx_bytes) via an Installation, or None if the MDL doesn't resolve."""
    from pykotor.resource.type import ResourceType
    try:
        m = inst.resource(name, ResourceType.MDL)
        if m is None:
            return None
        x = inst.resource(name, ResourceType.MDX)
    except Exception:
        return None
    return m.data, (x.data if x is not None else b"")


# --- quaternion / vector helpers (quaternions are (w, x, y, z) like the engine) ---
def qmul(a, b):
    aw, ax, ay, az = a
    bw, bx, by, bz = b
    return (aw * bw - ax * bx - ay * by - az * bz,
            aw * bx + ax * bw + ay * bz - az * by,
            aw * by - ax * bz + ay * bw + az * bx,
            aw * bz + ax * by - ay * bx + az * bw)


def qrot(q, v):
    """Rotate vector v by unit quaternion q (w,x,y,z)."""
    w, x, y, z = q
    tx, ty, tz = 2 * (y * v[2] - z * v[1]), 2 * (z * v[0] - x * v[2]), 2 * (x * v[1] - y * v[0])
    return (v[0] + w * tx + (y * tz - z * ty), v[1] + w * ty + (z * tx - x * tz), v[2] + w * tz + (x * ty - y * tx))


def qnorm(q):
    n = math.sqrt(sum(c * c for c in q)) or 1.0
    return tuple(c / n for c in q)


class Node:
    __slots__ = ("name", "flags", "pos", "quat", "children", "verts", "render", "off", "ref")

    def __init__(self):
        self.children, self.verts, self.render, self.ref = [], [], 1, None


def parse_model(mdl, mdx):
    """-> (root Node, model name). Positions/quats are the node-header (rest pose) values."""
    d = mdl[12:]  # all offsets relative to here
    u32 = lambda o: struct.unpack_from("<I", d, o)[0]
    u16 = lambda o: struct.unpack_from("<H", d, o)[0]
    f32 = lambda o, n=1: struct.unpack_from("<%df" % n, d, o)
    model_name = d[8:40].split(b"\0")[0].decode("latin1")
    root_off = u32(0x28)
    names_off, names_cnt = u32(184), u32(188)
    names = []
    for i in range(names_cnt):
        p = u32(names_off + 4 * i)
        names.append(d[p:d.index(b"\0", p)].decode("latin1"))
    seen = set()

    def rd(off):
        if off in seen:
            raise ValueError("node cycle")
        seen.add(off)
        n = Node()
        n.off = off
        n.flags = u16(off)
        ni = u16(off + 4)
        n.name = names[ni] if ni < len(names) else "?%d" % ni
        n.pos = f32(off + 16, 3)
        w, x, y, z = f32(off + 28, 4)
        n.quat = qnorm((w, x, y, z))
        if n.flags & NT_MESH:
            m = off + 80
            stride, vcount = u32(m + 252), u16(m + 304)
            voff = struct.unpack_from("<i", d, m + 260)[0]
            mdx_off = u32(m + 332)
            n.render = d[m + 313]
            if n.flags & NT_SABER:  # saber blade meshes keep their vertices in the MDL, not the MDX
                vo = u32(m + 336)
                for i in range(vcount if vo and vo + 12 * vcount <= len(d) else 0):
                    n.verts.append(struct.unpack_from("<3f", d, vo + 12 * i))
            elif vcount and stride and voff >= 0 and mdx:
                for i in range(vcount):
                    p = mdx_off + i * stride + voff
                    if p + 12 > len(mdx):
                        break
                    n.verts.append(struct.unpack_from("<3f", mdx, p))
        if n.flags & NT_REF:  # reference node: another MDL (e.g. a saber hilt) is attached here at runtime
            n.ref = d[off + 80:off + 112].split(b"\0")[0].decode("latin1").lower()
        cptr, ccnt = u32(off + 44), u32(off + 48)
        for i in range(ccnt):
            n.children.append(rd(u32(cptr + 4 * i)))
        return n

    return rd(root_off), model_name


def model_bounds(mdl, mdx, skip_names=(), resolver=None):
    """Union bbox of every rendering mesh vertex, transformed to model space.

    resolver(name) -> (mdl, mdx) | None is used to expand reference nodes (saber hilts) into the bbox.
    Returns dict(min, max, ext, centre, longest ('x'|'y'|'z'), nverts, nmesh, node-type counts, refs)."""
    root0, name = parse_model(mdl, mdx)
    lo, hi = [1e30] * 3, [-1e30] * 3
    info = dict(nverts=0, nmesh=0, emitter=0, light=0, dangly=0, skin=0, saber=0, nodes=0, refs="")

    def walk(root, n, q, p, depth):
        info["nodes"] += 1
        top = n is root  # root pose is the model origin
        q2 = q if top else qnorm(qmul(q, n.quat))
        p2 = p if top else tuple(a + b for a, b in zip(p, qrot(q, n.pos)))
        for key, flag in (("emitter", NT_EMITTER), ("light", NT_LIGHT), ("dangly", NT_DANGLY), ("skin", NT_SKIN), ("saber", NT_SABER)):
            if n.flags & flag:
                info[key] += 1
        if n.flags & NT_MESH and n.render and n.verts and n.name.lower() not in skip_names:
            info["nmesh"] += 1
            info["nverts"] += len(n.verts)
            for v in n.verts:
                r = qrot(q2, v)
                for k in range(3):
                    t = r[k] + p2[k]
                    lo[k], hi[k] = min(lo[k], t), max(hi[k], t)
        if n.ref and resolver and depth < 3:
            sub = resolver(n.ref)
            if sub:
                info["refs"] += n.ref + ";"
                sroot, _ = parse_model(*sub)
                walk(sroot, sroot, q2, p2, depth + 1)
        for c in n.children:
            walk(root, c, q2, p2, depth)

    walk(root0, root0, (1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 0.0), 0)
    if info["nverts"] == 0:
        return dict(model=name, empty=True, **info)
    ext = [hi[k] - lo[k] for k in range(3)]
    return dict(model=name, empty=False, min=lo, max=hi, ext=ext,
                centre=[(lo[k] + hi[k]) / 2 for k in range(3)], longest="xyz"[ext.index(max(ext))], **info)


def survey_installation(game, log=print):
    """Bounds for every existing <itemclass>_<NNN> of every baseitems row. Returns {(row, var): bounds}."""
    import logging
    logging.disable(logging.CRITICAL)
    from pykotor.extract.installation import Installation
    from pykotor.resource.type import ResourceType
    from pykotor.resource.formats.twoda import read_2da
    inst = Installation(game)
    bi = read_2da(inst.resource("baseitems", ResourceType.TwoDA).data)
    out, cache = {}, {}
    for row in range(bi.get_height()):
        r = bi.get_row(row)
        cls = r.get_string("itemclass").lower()
        if not cls or cls == "****":
            continue
        for var in range(1, 100):
            nm = "%s_%03d" % (cls, var)
            if nm not in cache:
                mm = load_model(inst, nm)
                try:
                    cache[nm] = model_bounds(*mm, resolver=lambda r: load_model(inst, r)) if mm else None
                except Exception as e:  # keep going, record the failure
                    cache[nm] = dict(model=nm, empty=True, error=str(e))
            if cache[nm] is not None:
                out[(row, var)] = dict(cache[nm], cls=cls, label=r.get_string("label"))
    return out


if __name__ == "__main__":
    game = sys.argv[1]
    dest = sys.argv[2] if len(sys.argv) > 2 else None
    res = survey_installation(game)
    lines = ["# R17 hierarchical bbox survey: per (baseitems row, variation): extent xyz, centre xyz, longest axis, nverts, node-type counts"]
    outliers = []
    for (row, var), b in sorted(res.items()):
        if b.get("empty"):
            lines.append("%3d %-26s %s_%03d EMPTY %s" % (row, b["label"], b["cls"], var, b.get("error", "")))
            outliers.append((row, var, "empty/no vertices", b))
            continue
        e, c = b["ext"], b["centre"]
        lines.append("%3d %-26s %s_%03d ext=[%.3f %.3f %.3f] c=[%.3f %.3f %.3f] long=%s v=%d m=%d em=%d li=%d dg=%d sk=%d sb=%d" % (
            row, b["label"], b["cls"], var, e[0], e[1], e[2], c[0], c[1], c[2], b["longest"], b["nverts"], b["nmesh"],
            b["emitter"], b["light"], b["dangly"], b["skin"], b["saber"]) + (" refs=" + b["refs"] if b["refs"] else ""))
        why = []
        if max(e) > 1.8:
            why.append("extent>1.8")
        if max(e) < 0.12:
            why.append("extent<0.12")
        if b["emitter"] or b["light"] or b["dangly"]:
            why.append("emitter/light/dangly")
        if max(abs(x) for x in c) > 0.5 * max(max(e), 0.05) + 0.3:
            why.append("centre far from origin")
        if why:
            outliers.append((row, var, ",".join(why), b))
    lines.append("")
    lines.append("# OUTLIERS (%d of %d)" % (len(outliers), len(res)))
    for row, var, why, b in outliers:
        lines.append("  %3d %s_%03d %s" % (row, b["cls"], var, why))
    text = "\n".join(lines)
    if dest:
        open(dest, "w").write(text + "\n")
    print(text[-3000:] if dest else text)
