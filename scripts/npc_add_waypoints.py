#!/usr/bin/env python3
"""Pure waypoint helper for npc_add routes (204TEL ward). No file I/O, no pykotor.

build_route(creature_tag, points) returns:
  utw_files:  {template_resref: UTW bytes}  one GFF 'UTW ' per waypoint (Tag + TemplateResRef only)
  wp_structs: [dict, ...]                   one GIT WaypointList struct per waypoint, keys in WAYPOINT_FIELDS order

Names: creature tag `npcadd_<slug>`; waypoint Tag `WP_npcadd_<slug>_NN`; waypoint TemplateResRef `npcwp_<slug7>_NN`
(<= 16 chars). NN runs 01..NN contiguously. Orientation is the unit vector toward the next point (the last point
faces the previous one). LocString fields are stringrefs (-1 = none, no substrings). Positions and orientations
are stored as float32 values, so they equal what the .mod stores.
"""
import hashlib
import math
import re
import struct

ROUTES = {
    "R1": ((-65.70, 20.05), (-70.45, 26.30), (-66.20, 32.05)),
    "R2": ((-73.45, 20.30), (-65.70, 21.05), (-70.20, 27.55)),
    "R3": ((-68.70, 19.30), (-75.45, 22.55), (-73.70, 30.30)),
}

# GIT WaypointList struct (struct_id 5), field order per W1d_git_fields.md:13.
WAYPOINT_FIELDS = (
    ("Appearance", "UInt8"),
    ("LinkedTo", "String"),
    ("TemplateResRef", "ResRef"),
    ("Tag", "String"),
    ("LocalizedName", "LocString"),
    ("Description", "LocString"),
    ("HasMapNote", "UInt8"),
    ("MapNote", "LocString"),
    ("MapNoteEnabled", "UInt8"),
    ("XPosition", "Single"),
    ("YPosition", "Single"),
    ("ZPosition", "Single"),
    ("XOrientation", "Single"),
    ("YOrientation", "Single"),
)

APPEARANCE = 1          # all 48 existing 204TEL waypoints use 1
NO_STRREF = -1          # LocString with no stringref and no substrings
MAX_RESREF = 16
MIN_POINTS = 3          # P6 A5: waypoints >= 3
MAX_POINTS = 99         # two-digit numbering

_CREATURE_RE = re.compile(r"npcadd_([a-z0-9_]+)")
_HEADER_SIZE = 56
_GFF_STRING, _GFF_RESREF = 10, 11


def _f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def _slug(creature_tag):
    m = _CREATURE_RE.fullmatch(creature_tag) if isinstance(creature_tag, str) else None
    if m is None:
        raise ValueError(f"creature tag must match npcadd_<slug> (lowercase a-z, 0-9, _): {creature_tag!r}")
    if len(creature_tag) > MAX_RESREF:
        raise ValueError(f"creature tag {creature_tag!r} is {len(creature_tag)} chars; ResRef limit is {MAX_RESREF}")
    return m.group(1)


def _slug7(slug):
    """Template key: the slug itself when <= 7 chars, else 4 chars of the slug plus 3 hex chars of its sha1."""
    if len(slug) <= 7:
        return slug
    return slug[:4] + hashlib.sha1(slug.encode("ascii")).hexdigest()[:3]


def _validate_points(points):
    pts = []
    for p in points:
        x, y = p
        x, y = float(x), float(y)
        if not (math.isfinite(x) and math.isfinite(y)):
            raise ValueError(f"route point is not finite: {p!r}")
        pts.append((x, y))
    if not MIN_POINTS <= len(pts) <= MAX_POINTS:
        raise ValueError(f"route needs {MIN_POINTS}..{MAX_POINTS} points, got {len(pts)}")
    for a, b in zip(pts, pts[1:]):
        if a == b:
            raise ValueError(f"consecutive route points are identical (no facing): {a!r}")
    return pts


def _facing(pts, i):
    tx, ty = pts[i + 1] if i + 1 < len(pts) else pts[i - 1]
    dx, dy = tx - pts[i][0], ty - pts[i][1]
    length = math.hypot(dx, dy)
    if length == 0.0:
        raise ValueError(f"no facing for point {i + 1}")
    return dx / length, dy / length


def _utw_bytes(tag, resref):
    """GFF V3.2 'UTW ' with one root struct holding Tag (String) and TemplateResRef (ResRef) only."""
    tag_b, res_b = tag.encode("ascii"), resref.encode("ascii")
    field_data = struct.pack("<I", len(tag_b)) + tag_b + bytes([len(res_b)]) + res_b
    struct_off = _HEADER_SIZE
    field_off = struct_off + 12
    label_off = field_off + 2 * 12
    fd_off = label_off + 2 * 16
    fi_off = fd_off + len(field_data)
    li_off = fi_off + 2 * 4
    header = b"UTW " + b"V3.2" + struct.pack(
        "<IIIIIIIIIIII",
        struct_off, 1,
        field_off, 2,
        label_off, 2,
        fd_off, len(field_data),
        fi_off, 2 * 4,
        li_off, 0,
    )
    struct_entry = struct.pack("<III", 0xFFFFFFFF, 0, 2)          # root struct, 2 fields, indices at offset 0
    field_entries = (struct.pack("<III", _GFF_STRING, 0, 0)       # Tag, data at field-data offset 0
                     + struct.pack("<III", _GFF_RESREF, 1, 4 + len(tag_b)))
    labels = b"Tag".ljust(16, b"\0") + b"TemplateResRef".ljust(16, b"\0")
    field_indices = struct.pack("<II", 0, 1)
    return header + struct_entry + field_entries + labels + field_data + field_indices


def build_route(creature_tag, points, *, existing_tags=(), existing_resrefs=()):
    """Build UTW resources and GIT WaypointList structs for one route.

    creature_tag: 'npcadd_<slug>'. points: 3+ (x, y) pairs; z is 0.0. existing_tags / existing_resrefs: names
    already in the target area or archive; comparisons are case-insensitive, as the engine's tag and ResRef lookups are.
    Raises ValueError on bad input or a name collision.
    """
    slug = _slug(creature_tag)
    pts = _validate_points(points)
    key = _slug7(slug)
    entries = []
    for i, (x, y) in enumerate(pts):
        num = i + 1
        tag = f"WP_{creature_tag}_{num:02d}"
        resref = f"npcwp_{key}_{num:02d}"
        if len(resref) > MAX_RESREF:
            raise ValueError(f"waypoint ResRef {resref!r} is {len(resref)} chars; limit is {MAX_RESREF}")
        ux, uy = _facing(pts, i)
        entries.append((tag, resref, x, y, ux, uy))

    taken_tags = {t.lower() for t in existing_tags}
    taken_resrefs = {r.lower() for r in existing_resrefs}
    tag_clash = [t for t, *_ in entries if t.lower() in taken_tags]
    res_clash = [r for _, r, *_ in entries if r.lower() in taken_resrefs]
    if tag_clash or res_clash:
        raise ValueError(f"waypoint name collision: tags {tag_clash}, resrefs {res_clash}")

    utw_files = {}
    wp_structs = []
    for tag, resref, x, y, ux, uy in entries:
        utw_files[resref] = _utw_bytes(tag, resref)
        wp_structs.append({
            "Appearance": APPEARANCE,
            "LinkedTo": "",
            "TemplateResRef": resref,
            "Tag": tag,
            "LocalizedName": NO_STRREF,
            "Description": NO_STRREF,
            "HasMapNote": 0,
            "MapNote": NO_STRREF,
            "MapNoteEnabled": 1,
            "XPosition": _f32(x),
            "YPosition": _f32(y),
            "ZPosition": 0.0,
            "XOrientation": _f32(ux),
            "YOrientation": _f32(uy),
        })
    return utw_files, wp_structs
