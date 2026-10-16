#!/usr/bin/env python3
"""Generate the embedded GFF V3.2 'GUI ' blob for the holster editor's hse_view layout (build-time bytes).

Default: write patch_manager_mods/holster_editor_src/HseViewGui.inc as `static const unsigned char kHseViewGui[]`.
--check: regenerate in memory, compare with the .inc on disk, exit 1 (message on stderr) if they differ.
Output is deterministic: no timestamps, fixed table order.

Blob content: root struct 0 -> List "CONTROLS" (type 15), two elements:
  [0] 3D_MODEL control, TAG "HSE_VIEW": ID (Int), TAG (String), CONTROLTYPE (Int, 4), EXTENT (Struct: LEFT, TOP, WIDTH, HEIGHT Int)
  [1] label template LBL_TITLE: same field set, ID 3, CONTROLTYPE 4, EXTENT 90,57,252,24 (vanilla, optionsingame_p)

Field set: scripts/add_equip_3dview_control.py (ID, TAG, CONTROLTYPE asserted == 4, EXTENT via get_rect/set_rect).
LBL_TITLE values: .claude/waves/w1-HSE-plan/u10.md:8. LBL_BAR1 is not emitted: no EXTENT for it in any source.
GFF writer: header size, type codes, 16-byte labels and field-data layout from scripts/npc_add_waypoints.py _utw_bytes,
extended with struct, field-index and list tables. Build-time only; the runtime loader is not touched here.
"""
import argparse
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import npc_add_waypoints as naw  # noqa: E402  (GFF V3.2 header size and type codes)

INC_PATH = HERE.parent / "patch_manager_mods" / "holster_editor_src" / "HseViewGui.inc"

_HEADER_SIZE = naw._HEADER_SIZE      # 56 = "GUI " + "V3.2" + 12 x uint32 table offsets/counts
_GFF_STRING = naw._GFF_STRING        # 10 (CExoString)
_GFF_INT, _GFF_STRUCT, _GFF_LIST = 5, 14, 15
_ROOT_STRUCT_ID = 0xFFFFFFFF
_ELEMENT_SIZE = 4                    # list element = uint32 struct index
_EXT_KEYS = ("LEFT", "TOP", "WIDTH", "HEIGHT")
_CONTROLTYPE_3D = 4                  # add_equip_3dview_control.py asserts CONTROLTYPE == 4 on its 3D_MODEL

# PLACEHOLDER: the DLL calls setCtlRect at runtime with the measured pane, so this rect is not final.
# 216x336 is the equip 3D view size (add_equip_3dview_control.py LAYOUTS "R"), used only as a sane default.
# ID -1 on purpose: sceneview::createView runs InitControl with addToList=1, and an ID below the panel's control count would
# OVERWRITE that vanilla list slot (d2e section 2/4 risk 4). With -1 InitControl lists nothing; the DLL appends the view itself
# (gui::appendToPanel sets ID = count).
HSE_VIEW_ID = -1
HSE_VIEW_EXTENT = (0, 0, 216, 336)   # LEFT, TOP, WIDTH, HEIGHT
LBL_TITLE_ID = 3                     # optionsingame_p LBL_TITLE (u10.md:8)
LBL_TITLE_CONTROLTYPE = 4            # "T4" in u10.md:8
LBL_TITLE_EXTENT = (90, 57, 252, 24) # vanilla rect, u10.md:8


class _Gff:
    """Minimal GFF V3.2 writer. Tables are appended in build order; field data and list indices are byte-addressed."""

    def __init__(self):
        self.labels = []
        self.structs = [None]        # (struct_id, data_or_offset, field_count); slot 0 is the root
        self.fields = []             # (type, label index, data_or_offset)
        self.field_data = bytearray()
        self.field_indices = bytearray()
        self.list_indices = bytearray()
        self._lists = []             # (offset in list_indices, declared element count) for the bounds guard

    def _label(self, name):
        if len(name.encode("ascii")) > 16:
            raise ValueError(f"GFF label longer than 16 bytes: {name}")
        if name not in self.labels:
            self.labels.append(name)
        return self.labels.index(name)

    def add_struct(self, pairs, *, struct_id=0, slot=None):
        """pairs: (label, gff_type, value). INT: int. STRING: str. STRUCT: struct index.
        LIST: (element struct indices, declared count). Returns the struct index."""
        field_ids = []
        for label, ftype, value in pairs:
            if ftype == _GFF_INT:
                data = value & 0xFFFFFFFF
            elif ftype == _GFF_STRING:
                raw = value.encode("ascii")
                data = len(self.field_data)
                self.field_data += struct.pack("<I", len(raw)) + raw
            elif ftype == _GFF_STRUCT:
                data = value
            elif ftype == _GFF_LIST:
                elements, declared = value
                data = len(self.list_indices)
                self.list_indices += struct.pack("<I", declared)
                self.list_indices += b"".join(struct.pack("<I", e) for e in elements)
                self._lists.append((data, declared))
            else:
                raise ValueError(f"unsupported GFF field type {ftype}")
            field_ids.append(len(self.fields))
            self.fields.append((ftype, self._label(label), data))
        if len(field_ids) == 1:
            entry = (struct_id, field_ids[0], 1)          # single field: data is the field index itself
        else:
            entry = (struct_id, len(self.field_indices), len(field_ids))
            self.field_indices += b"".join(struct.pack("<I", f) for f in field_ids)
        if slot is None:
            self.structs.append(entry)
            return len(self.structs) - 1
        self.structs[slot] = entry
        return slot

    def pack(self):
        struct_off = _HEADER_SIZE
        field_off = struct_off + 12 * len(self.structs)
        label_off = field_off + 12 * len(self.fields)
        fd_off = label_off + 16 * len(self.labels)
        fi_off = fd_off + len(self.field_data)
        li_off = fi_off + len(self.field_indices)
        total = li_off + len(self.list_indices)
        # Bounds guard: list_offset + count * element_size must fit inside the blob. Forged counts fail here.
        for off, declared in self._lists:
            first = li_off + off + 4
            if first + declared * _ELEMENT_SIZE > total:
                fits = (total - first) // _ELEMENT_SIZE
                raise AssertionError(f"list at {first:#x} declares {declared} elements; only {fits} fit in {total} bytes")
        header = b"GUI " + b"V3.2" + struct.pack(
            "<IIIIIIIIIIII",
            struct_off, len(self.structs), field_off, len(self.fields), label_off, len(self.labels),
            fd_off, len(self.field_data), fi_off, len(self.field_indices), li_off, len(self.list_indices),
        )
        body = b"".join(struct.pack("<III", *s) for s in self.structs)
        body += b"".join(struct.pack("<III", *f) for f in self.fields)
        body += b"".join(n.encode("ascii").ljust(16, b"\0") for n in self.labels)
        body += bytes(self.field_data) + bytes(self.field_indices) + bytes(self.list_indices)
        blob = header + body
        assert len(blob) == total, "section offsets disagree with section sizes"
        return blob


def _control(g, ident, tag, controltype, extent):
    """One control struct with the 3D_MODEL field set: ID, TAG, CONTROLTYPE, EXTENT (LEFT, TOP, WIDTH, HEIGHT)."""
    ext = g.add_struct([(k, _GFF_INT, v) for k, v in zip(_EXT_KEYS, extent)])
    return g.add_struct([
        ("ID", _GFF_INT, ident),
        ("TAG", _GFF_STRING, tag),
        ("CONTROLTYPE", _GFF_INT, controltype),
        ("EXTENT", _GFF_STRUCT, ext),
    ])


def build_gui_bytes(list_count=None):
    """Return the GFF V3.2 'GUI ' bytes. list_count overrides the CONTROLS count written in the list header.
    Only the bounds-guard test passes it; a count larger than the elements present raises AssertionError."""
    g = _Gff()
    hse = _control(g, HSE_VIEW_ID, "HSE_VIEW", _CONTROLTYPE_3D, HSE_VIEW_EXTENT)
    lbl = _control(g, LBL_TITLE_ID, "LBL_TITLE", LBL_TITLE_CONTROLTYPE, LBL_TITLE_EXTENT)
    elements = [hse, lbl]
    declared = len(elements) if list_count is None else list_count
    g.add_struct([("CONTROLS", _GFF_LIST, (elements, declared))], struct_id=_ROOT_STRUCT_ID, slot=0)
    return g.pack()


def render_inc(blob):
    rows = []
    for i in range(0, len(blob), 12):
        hexes = ", ".join(f"0x{b:02x}" for b in blob[i:i + 12])
        rows.append(f"    {hexes},")
    head = [
        "// Generated by scripts/gen_hse_layout.py. Do not edit by hand.",
        f"// Embedded GFF V3.2 'GUI ' blob for layout hse_view, {len(blob)} bytes.",
        "// PLACEHOLDER: HSE_VIEW ID and EXTENT are not measured; the DLL sets the rect at runtime (setCtlRect).",
        "static const unsigned char kHseViewGui[] = {",
    ]
    return "\n".join(head + rows + ["};", ""])


def main(argv=None):
    ap = argparse.ArgumentParser(description="Generate HseViewGui.inc (embedded hse_view GUI blob).")
    ap.add_argument("--check", action="store_true",
                    help="regenerate in memory and compare with --inc; exit 1 if they differ")
    ap.add_argument("--inc", type=Path, default=INC_PATH, help="target .inc (default: holster_editor_src/HseViewGui.inc)")
    a = ap.parse_args(argv)
    blob = build_gui_bytes()
    text = render_inc(blob).encode("ascii")
    if a.check:
        try:
            on_disk = a.inc.read_bytes()
        except OSError as e:
            print(f"gen_hse_layout: cannot read {a.inc}: {e}", file=sys.stderr)
            return 1
        if on_disk != text:
            print(f"gen_hse_layout: {a.inc} is stale; run scripts/gen_hse_layout.py to regenerate", file=sys.stderr)
            return 1
        return 0
    a.inc.write_bytes(text)
    print(f"wrote {a.inc} ({len(blob)} blob bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
