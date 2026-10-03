#!/usr/bin/env python3
"""re_card_prep.py - pre-process dump evidence for small-model reverse-engineering prompts.

The point: the model must never do hex/decimal or ABI math.  This tool does it and prints
ready-to-cite facts.  Read-only: it only reads files under the dump ROOT (stdlib only).

  re_card_prep.py annotate ADDR [--max-lines N]
  re_card_prep.py abi ADDR (--gog-bytes N | --gog-name Class::Func [--gog-addr A]) [--callconv thiscall|stdcall|cdecl|fastcall]
  re_card_prep.py anchors ADDR
  re_card_prep.py prologue ADDR
  (all take --root DUMP_ROOT; default ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/)

annotate
  Prints the decompile with decimal offsets rewritten to hex, decimal kept:
      (param_1 + 100)  ->  (param_1 + 0x64 [dec 100] {int*: byte +0x190})
  The `{T*: byte +0x..}` note is added only when the base variable is declared as a pointer to a known-size
  type (Ghidra scales `ptr + N` by the element size, so `int *p; p + 100` is byte offset 0x190).  Likewise
  `arr[N]` on 4-byte element pointers/arrays becomes `arr[N = byte +0x..]`; other element types are left alone.
  Untouched: numbers already starting 0x, float literals, anything inside strings or comments.
  Ghidra auto labels (FID_conflict, *_deleting_destructor, _atexit, `Library Function`, CRT-like `_x`/`__x`
  names) get ` [GHIDRA-AUTO-LABEL: unreliable]` appended.  Single-forwarding-thunk bodies print a THUNK line.
  The very long callers=/callees= lists of the header are replaced by counts (use `anchors` for callees).

abi
  expected_ret = ceil(N/4)*4 for stdcall/thiscall (x86 promotes every stack arg to >= 4-byte slots; N is the
  GOG total param_size_bytes, thiscall `this` is in ECX and not counted).  cdecl: caller cleans, expected 0.
  fastcall: ASSUMPTION - the first two 4-byte args travel in ECX/EDX, so expected = max(0, ceil(N/4)*4 - 8).
  `--gog-bytes` also takes a comma list (1,4,1): each arg is rounded up to a slot separately.
  Observed: `ret N` of the dump header and every RET instruction in the .asm.

anchors / prologue: see the subcommand output; 16-byte alignment is a strong signal, not proof.
"""
import argparse
import math
import os
import re
import sys

DEFAULT_ROOT = "~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/"
LABEL_TAG = " [GHIDRA-AUTO-LABEL: unreliable]"


# ---------------------------------------------------------------- common helpers
def parse_addr(s):
    s = str(s).strip().lower()
    if s.startswith("0x"):
        s = s[2:]
    return int(s, 16)


def fmt8(a):
    return "%08x" % a


def hx(a):
    return "0x%08X" % a


def expand_root(root):
    return os.path.abspath(os.path.expanduser(root or DEFAULT_ROOT))


def c_path(root, addr):
    h = fmt8(addr)
    return os.path.join(root, "decomp", h[2:4], h + ".c")


def asm_path(root, addr):
    return c_path(root, addr)[:-2] + ".asm"


def read_text(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError:
        return None


_ASM_RE = re.compile(r"^([0-9a-f]{8})  ([0-9a-f]+) *([A-Z].*?)\s*$")


def parse_asm_text(text):
    """-> list of (addr:int, bytes:str, insn:str).  Bytes are lowercase hex, mnemonic upper case."""
    out = []
    for line in text.split("\n"):
        m = _ASM_RE.match(line)
        if m:
            out.append((int(m.group(1), 16), m.group(2), m.group(3)))
    return out


def read_asm(root, addr):
    t = read_text(asm_path(root, addr))
    return None if t is None else parse_asm_text(t)


_HDR_RE = re.compile(r"^//\s*FN\s+([0-9a-fA-F]{8})\s+(\S+)(.*)$")


def parse_header(ctext):
    """First `// FN addr name size=N ret=N callers=... callees=...` line -> dict or None."""
    if not ctext:
        return None
    first = ctext.split("\n", 1)[0]
    m = _HDR_RE.match(first)
    if not m:
        return None
    d = {"addr": int(m.group(1), 16), "name": m.group(2), "size": None, "ret": None, "callers": [], "callees": []}
    rest = m.group(3)
    for key in ("size", "ret"):
        mm = re.search(r"\b%s=(-?\d+)" % key, rest)
        if mm:
            d[key] = int(mm.group(1))
    for key in ("callers", "callees"):
        mm = re.search(r"\b%s=(\S*)" % key, rest)
        if mm and mm.group(1):
            d[key] = [int(x, 16) for x in mm.group(1).split(",") if re.fullmatch(r"[0-9a-fA-F]{1,8}", x)]
    return d


def load_tsv(path):
    """-> list of dicts keyed by the header row ([] when the file is missing)."""
    t = read_text(path)
    if t is None:
        return []
    lines = t.split("\n")
    if not lines or not lines[0]:
        return []
    cols = lines[0].rstrip("\r").split("\t")
    rows = []
    for ln in lines[1:]:
        if not ln.strip():
            continue
        parts = ln.rstrip("\r").split("\t")
        parts += [""] * (len(cols) - len(parts))
        rows.append(dict(zip(cols, parts)))
    return rows


def load_name_map(root):
    names = {}
    for r in load_tsv(os.path.join(root, "inputs", "name_map.tsv")):
        try:
            names[parse_addr(r["steam_addr"])] = r["name"]
        except (KeyError, ValueError):
            pass
    return names


def load_strings(root):
    out = {}
    for r in load_tsv(os.path.join(root, "strings.tsv")):
        try:
            out[int(r["addr"], 16)] = r["string"]
        except (KeyError, ValueError):
            pass
    return out


# ---------------------------------------------------------------- annotate: lexing
_CAST_OR_DECL_SIZES = {
    "int": 4, "uint": 4, "undefined4": 4, "float": 4, "long": 4, "ulong": 4, "dword": 4, "DWORD": 4,
    "float10": 10, "double": 8, "longlong": 8, "ulonglong": 8, "undefined8": 8, "qword": 8,
    "short": 2, "ushort": 2, "undefined2": 2, "word": 2, "WORD": 2,
    "char": 1, "uchar": 1, "byte": 1, "undefined1": 1, "undefined": 1, "bool": 1, "BYTE": 1,
}

_LABEL_KEYWORDS = {"__thiscall", "__stdcall", "__cdecl", "__fastcall", "__declspec", "__restrict", "__unaligned"}
_NOT_KW = {"return", "case", "if", "while", "else", "sizeof", "goto", "do", "for", "switch"}
_IDENT_RE = re.compile(r"FID_conflict:\w+|[A-Za-z_]\w*")


def is_auto_label(name):
    """True for Ghidra FID / library / CRT-like names whose labels cannot be trusted."""
    if "FID_conflict" in name or "deleting_destructor" in name:
        return True
    if name.startswith("_atexit") or name.startswith("atexit"):
        return True
    if name in _LABEL_KEYWORDS:
        return False
    if name.startswith("_"):
        if re.match(r"_(DAT|PTR|LAB|switchD|Var)", name):
            return False
        if re.match(r"_[A-Za-z]", name) or name.startswith("__"):
            return True
    return False


def split_code_segments(line, in_block):
    """Split one C line into [(is_code, text)], tracking /* */ across lines.

    String/char literals and comments are non-code.  Returns (segments, in_block).
    """
    segs = []
    i, n = 0, len(line)
    buf = []

    def flush(code):
        if buf:
            segs.append((code, "".join(buf)))
            buf.clear()

    while i < n:
        if in_block:
            j = line.find("*/", i)
            if j < 0:
                buf.append(line[i:])
                i = n
            else:
                buf.append(line[i:j + 2])
                i = j + 2
                in_block = False
            if not in_block or i >= n:
                flush(False)
            continue
        ch = line[i]
        if line.startswith("//", i):
            flush(True)
            segs.append((False, line[i:]))
            i = n
        elif line.startswith("/*", i):
            flush(True)
            in_block = True
            buf.append("/*")
            i += 2
        elif ch in "\"'":
            flush(True)
            j = i + 1
            while j < n and line[j] != ch:
                j += 2 if line[j] == "\\" else 1
            segs.append((False, line[i:j + 1]))
            i = j + 1
        else:
            buf.append(ch)
            i += 1
    flush(not in_block)
    return segs, in_block


# ---------------------------------------------------------------- annotate: type table
_PTR_DECL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*(\*+)\s*([A-Za-z_]\w*)\s*(?=[;,)=\[])")
_ARR_DECL_RE = re.compile(r"\b([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*\[\s*\w+\s*\]\s*;")


def elem_size(base, stars):
    """Element size in bytes of `base` with `stars` pointer levels, or None when unknown."""
    if len(stars) >= 2:
        return 4
    return _CAST_OR_DECL_SIZES.get(base)


def collect_types(ctext):
    """-> {var: (size, 'int*')} from parameter / local declarations (first declaration wins)."""
    types = {}
    in_block = False
    for line in ctext.split("\n")[1:]:
        segs, in_block = split_code_segments(line, in_block)
        code = "".join(t for c, t in segs if c)
        for m in _PTR_DECL_RE.finditer(code):
            base, stars, name = m.group(1), m.group(2), m.group(3)
            sz = elem_size(base, stars)
            if sz is not None and name not in types and base not in _NOT_KW and base != "return":
                types[name] = (sz, base + stars)
        for m in _ARR_DECL_RE.finditer(code):
            base, name = m.group(1), m.group(2)
            if base in _CAST_OR_DECL_SIZES and name not in types:
                types[name] = (_CAST_OR_DECL_SIZES[base], base + "[]")
    return types


# ---------------------------------------------------------------- annotate: rewriting
_COMBINED_RE = re.compile(
    r"(?P<idx>(?P<iname>[A-Za-z_]\w*)\s*\[\s*(?P<inum>0x[0-9a-fA-F]+|\d+)\s*\])"
    r"|(?P<off>(?<=[\w)\]])(?P<ws1>\s*)(?P<op>[+-])(?P<ws2>\s*)(?P<neg>-?)(?P<num>\d+)(?![\w.]))"
)
_CAST_BEFORE_RE = re.compile(r"\(\s*(?:const\s+|unsigned\s+)?([A-Za-z_]\w*)\s*(\*+)?\s*\)\s*$")
_LEFT_IDENT_RE = re.compile(r"([A-Za-z_]\w*)\s*$")


def _signed_hex(v):
    return "%s0x%x" % ("-" if v < 0 else "+", abs(v))


def _left_info(seg, pos, types):
    """Type info (size, typename) of the operand just left of seg[pos], or None.

    `(int)param_1 + 100` uses the cast (plain int -> byte arithmetic -> None unless a pointer cast).
    """
    pre = seg[:pos].rstrip()
    m = _LEFT_IDENT_RE.search(pre)
    if not m:
        return None
    name = m.group(1)
    if name in _NOT_KW:
        return "kw"
    before = pre[:m.start()]
    cm = _CAST_BEFORE_RE.search(before)
    if cm:
        base, stars = cm.group(1), cm.group(2) or ""
        if not stars:
            return None
        sz = elem_size(base, stars)
        return (sz, base + stars) if sz else None
    return types.get(name)


def _float_exponent_guard(seg, pos):
    pre = seg[:pos]
    return re.search(r"(?<![\w.])\d[\w.]*[eE]$", pre) is not None


def rewrite_code(seg, types):
    def repl(m):
        if m.group("idx"):
            info = types.get(m.group("iname"))
            if not info or info[0] != 4:
                return m.group(0)
            raw = m.group("inum")
            n = int(raw, 0)
            if n == 0:
                return m.group(0)
            return "%s[%s = byte +0x%x]" % (m.group("iname"), raw, n * 4)
        pos = m.start("off")
        info = _left_info(seg, pos, types)
        if info == "kw":
            return m.group(0)
        if _float_exponent_guard(seg, pos):
            return m.group(0)
        n = int(m.group("num"))
        neg = m.group("neg")
        sign = (-1 if m.group("op") == "-" else 1) * (-1 if neg else 1)
        note = ""
        if info and info[0] and info[0] > 1:
            note = " {%s: byte %s}" % (info[1], _signed_hex(sign * n * info[0]))
        if n < 10 and not note:
            return m.group(0)
        return "%s%s%s%s0x%x [dec %s%d]%s" % (m.group("ws1"), m.group("op"), m.group("ws2"), neg, n, neg, n, note)

    return _COMBINED_RE.sub(repl, seg)


def tag_labels(seg):
    """Append LABEL_TAG after each distinct auto-label identifier (once per segment)."""
    seen = set()
    out, last = [], 0
    for m in _IDENT_RE.finditer(seg):
        name = m.group(0)
        if is_auto_label(name) and name not in seen:
            seen.add(name)
            out.append(seg[last:m.end()])
            out.append(LABEL_TAG)
            last = m.end()
    out.append(seg[last:])
    return "".join(out)


def annotate_line(line, in_block, types):
    segs, in_block = split_code_segments(line, in_block)
    out = []
    has_lib = False
    for code, text in segs:
        if code:
            out.append(tag_labels(rewrite_code(text, types)))
        else:
            if "Library Function" in text:
                has_lib = True
            out.append(text)
    res = "".join(out)
    if has_lib and LABEL_TAG not in res:
        res += LABEL_TAG
    return res, in_block


# ---------------------------------------------------------------- thunk detection
_DECL_LINE_RE = re.compile(r"^[\w\s\*\[\]]+;$")


def body_statements(ctext):
    """Statements of the function body with comments, braces and declarations removed."""
    lines = ctext.split("\n")
    in_block = False
    code_lines = []
    for ln in lines[1:]:
        segs, in_block = split_code_segments(ln, in_block)
        code_lines.append("".join(t if c else (" " if t.startswith("/") else t) for c, t in segs).strip())
    # drop prototype lines up to the first lone '{'
    try:
        start = code_lines.index("{") + 1
    except ValueError:
        return None
    stmts = []
    for ln in code_lines[start:]:
        if not ln or ln in ("{", "}"):
            continue
        if _DECL_LINE_RE.match(ln) and "(" not in ln and "=" not in ln and not ln.startswith("return"):
            continue
        stmts.append(ln)
    return stmts


_THUNK_C_RE = re.compile(r"^(?:return\s+)?(?:\(\s*[\w\s\*]+\)\s*)?([A-Za-z_]\w*)\s*\(([^;]*)\)\s*;$")


def c_thunk_callee(ctext):
    """Name of the single forwarded callee when the body is one call (+ optional bare return), else None."""
    stmts = body_statements(ctext)
    if not stmts:
        return None
    if len(stmts) == 2 and stmts[1] == "return;":
        stmts = stmts[:1]
    elif len(stmts) == 2:  # `v = call(..); return v;`
        m = re.match(r"^(\w+)\s*=\s*(.*;)$", stmts[0])
        if m and stmts[1] == "return %s;" % m.group(1):
            stmts = [m.group(2)]
    if len(stmts) != 1:
        return None
    m = _THUNK_C_RE.match(stmts[0])
    if not m:
        return None
    if len(re.findall(r"[A-Za-z_]\w*\s*\(", stmts[0])) != 1:
        return None
    return m.group(1)


_JMP_RE = re.compile(r"^JMP\s+(0x[0-9a-fA-F]+)$")


def asm_thunk_target(asm):
    """Direct-JMP target when the whole function is [no-op prologue/MOV ECX..] + JMP imm, else None."""
    if not asm:
        return None
    mn, _ = split_insn(asm[-1][2])
    m = _JMP_RE.match(asm[-1][2].strip())
    if not m:
        return None
    for _a, _b, insn in asm[:-1]:
        mn, ops = split_insn(insn)
        o = ",".join(x.replace(" ", "").upper() for x in ops)
        if mn in ("PUSH", "POP") and o == "EBP":
            continue
        if mn == "MOV" and o in ("EDI,EDI", "EBP,ESP"):
            continue
        if mn in ("MOV", "LEA", "ADD", "SUB") and ops and ops[0].upper() == "ECX":
            continue
        if mn in ("NOP", "INT3"):
            continue
        return None
    return int(m.group(1), 16)


def split_insn(insn):
    s = insn.strip()
    if s.startswith("LOCK "):
        s = s[5:]
    parts = s.split(None, 1)
    mn = parts[0].upper() if parts else ""
    ops = [o.strip() for o in parts[1].split(",")] if len(parts) > 1 else []
    return mn, ops


def find_thunk(ctext, asm, header):
    """-> target address (int), or 0 when it is a thunk with unknown target, or None when not a thunk."""
    t = asm_thunk_target(asm)
    if t is not None:
        return t
    callee = c_thunk_callee(ctext) if ctext else None
    if callee is None:
        return None
    m = re.fullmatch(r"(?:FUN|thunk_FUN)_([0-9a-fA-F]{8})", callee)
    if m:
        return int(m.group(1), 16)
    if header and len(header["callees"]) == 1:
        return header["callees"][0]
    return 0


def thunk_line(target):
    t = "unknown" if not target else hx(target)
    return ("THUNK -> %s (Ghidra may omit __thiscall on thunks; ECX is passed through)" % t)


# ---------------------------------------------------------------- subcommand: annotate
def header_text(ctext, header):
    first = ctext.split("\n", 1)[0]
    if not header:
        return annotate_line(first, False, {})[0]
    s = "// FN %08x %s size=%s ret=%s callers=%d callees=%d" % (
        header["addr"], header["name"], header["size"], header["ret"], len(header["callers"]), len(header["callees"]))
    return tag_labels(s)


def annotate_text(ctext, asm=None, max_lines=0):
    """Full annotate output (string) for a function's .c text (and optional parsed asm)."""
    header = parse_header(ctext)
    types = collect_types(ctext)
    out = [header_text(ctext, header)]
    tgt = find_thunk(ctext, asm, header)
    if tgt is not None:
        out.append(thunk_line(tgt))
    in_block = False
    body = []
    for ln in ctext.split("\n")[1:]:
        res, in_block = annotate_line(ln, in_block, types)
        body.append(res)
    while body and not body[-1].strip():
        body.pop()
    if max_lines and len(body) > max_lines:
        extra = len(body) - max_lines
        body = body[:max_lines] + ["/* ... %d more lines truncated by --max-lines */" % extra]
    return "\n".join(out + body)


def cmd_annotate(args, root):
    addr = parse_addr(args.addr)
    ctext = read_text(c_path(root, addr))
    if ctext is None:
        print("ERROR: no decompile at %s" % c_path(root, addr), file=sys.stderr)
        return 2
    print(annotate_text(ctext, read_asm(root, addr), args.max_lines))
    return 0


# ---------------------------------------------------------------- subcommand: abi
def slots_bytes(spec):
    """'5' -> 5 ; '1,4,1' -> [1,4,1].  Returns (total, [per-arg sizes] or None)."""
    s = str(spec).strip()
    if "," in s:
        parts = [int(x) for x in s.split(",") if x.strip()]
        return sum(parts), parts
    return int(s, 0), None


def expected_ret(n, callconv, per_arg=None):
    """x86 callee-cleaned bytes for GOG param bytes n (see module docstring for the fastcall assumption)."""
    cc = (callconv or "thiscall").lower().lstrip("_")
    if cc == "cdecl":
        return 0
    if per_arg:
        raw = sum(int(math.ceil(p / 4.0)) * 4 for p in per_arg)
    else:
        raw = int(math.ceil(n / 4.0)) * 4
    if cc == "fastcall":
        return max(0, raw - 8)
    return raw


def asm_rets(asm):
    """Distinct RET immediates (sorted) in the asm; RET with no operand counts as 0."""
    vals = set()
    for _a, _b, insn in asm or []:
        mn, ops = split_insn(insn)
        if mn in ("RET", "RETN"):
            vals.add(int(ops[0], 0) if ops else 0)
    return sorted(vals)


def gog_lookup(root, name, gog_addr=None):
    """Class::Func -> list of gog_functions.tsv rows (filtered by --gog-addr when given)."""
    cls, _, fn = name.rpartition("::")
    rows = [r for r in load_tsv(os.path.join(root, "inputs", "gog_functions.tsv"))
            if r.get("class") == cls and r.get("function") == fn]
    if gog_addr is not None:
        ga = parse_addr(gog_addr)
        rows = [r for r in rows if int(r["gog_addr"], 16) == ga]
    return rows


def abi_report(n, per_arg, callconv, cc_note, header_ret, rets):
    exp = expected_ret(n, callconv, per_arg)
    cc = (callconv or "thiscall").lower().lstrip("_")
    lines = ["GOG param bytes: %d%s; callconv: %s%s" % (n, " (per arg %s)" % per_arg if per_arg else "", cc, cc_note)]
    if cc == "cdecl":
        lines.append("expected_ret = 0 (cdecl: caller cleans the stack)")
    elif cc == "fastcall":
        lines.append("expected_ret = max(0, ceil(%d/4)*4 - 8) = %d (ASSUMPTION: first two 4-byte args in ECX/EDX)" % (n, exp))
    else:
        lines.append("expected_ret = ceil(%d/4)*4 = %d (x86 4-byte stack-slot promotion%s)" % (
            n, exp, "; ECX = this" if cc == "thiscall" else ""))
    lines.append("observed: dump header ret=%s ; asm RET = %s" % (
        "n/a" if header_ret is None else header_ret, "/".join(str(r) for r in rets) if rets else "none"))
    obs = rets if rets else ([header_ret] if header_ret is not None and header_ret >= 0 else [])
    if not obs:
        verdict = "UNKNOWN"
    else:
        verdict = "TRUE" if all(o == exp for o in obs) else "FALSE"
    k = "/".join(str(o) for o in obs) if obs else "none"
    lines.append("Stack Slot Match: %s (GOG %d bytes -> x86 slots %d; ASM RET %s)" % (verdict, n, exp, k))
    if rets and header_ret is not None and header_ret >= 0 and header_ret not in rets:
        lines.append("note: header ret=%d disagrees with asm RET %s" % (header_ret, "/".join(map(str, rets))))
    return lines, verdict


def cmd_abi(args, root):
    addr = parse_addr(args.addr)
    callconv, cc_note = args.callconv, ""
    if args.gog_name:
        rows = gog_lookup(root, args.gog_name, args.gog_addr)
        if not rows:
            print("ERROR: %s not found in inputs/gog_functions.tsv" % args.gog_name, file=sys.stderr)
            return 2
        sig = set((r["param_size_bytes"], r["callconv"]) for r in rows)
        if len(sig) > 1:
            print("ERROR: %s is ambiguous (use --gog-addr):" % args.gog_name, file=sys.stderr)
            for r in rows:
                print("  %s %s::%s %s %s" % (r["gog_addr"], r["class"], r["function"], r["callconv"], r["param_size_bytes"]),
                      file=sys.stderr)
            return 2
        r = rows[0]
        n, per = int(r["param_size_bytes"]), None
        print("gog_functions.tsv: %s %s::%s callconv=%r param_size_bytes=%s (%d row%s)" % (
            r["gog_addr"], r["class"], r["function"], r["callconv"], r["param_size_bytes"], len(rows),
            "" if len(rows) == 1 else "s"))
        if callconv is None:
            callconv = r["callconv"].lstrip("_") or None
            if callconv is None:
                cc_note = " (TSV callconv empty -> assumed thiscall)"
    elif args.gog_bytes is not None:
        n, per = slots_bytes(args.gog_bytes)
    else:
        print("ERROR: give --gog-bytes N or --gog-name Class::Func", file=sys.stderr)
        return 2
    ctext = read_text(c_path(root, addr))
    header = parse_header(ctext)
    asm = read_asm(root, addr)
    if ctext is None and asm is None:
        print("ERROR: no dump files for %s" % hx(addr), file=sys.stderr)
        return 2
    lines, _v = abi_report(n, per, callconv, cc_note, header["ret"] if header else None, asm_rets(asm))
    print("function %s%s" % (hx(addr), " (%s)" % header["name"] if header else ""))
    print("\n".join(lines))
    return 0


# ---------------------------------------------------------------- subcommand: anchors
_HEXTOK_RE = re.compile(r"\b0x([0-9a-fA-F]{1,8})\b")
_MEM_RE = re.compile(r"\[([^\]]*)\]")
_REG_OFF_RE = re.compile(r"^\s*(E[A-D]X|ESI|EDI|[A-D]X|SI|DI)\s*\+\s*(?:E[A-Z]{2}\s*\*\s*\d+\s*\+\s*)?(0x[0-9a-fA-F]+)\s*$")


def string_anchors(asm, strings):
    hits = []
    seen = set()
    for a, _b, insn in asm:
        for m in _HEXTOK_RE.finditer(insn):
            v = int(m.group(1), 16)
            if v in strings and (v, a) not in seen:
                seen.add((v, a))
                hits.append((a, v, strings[v]))
    return hits


def struct_offsets(asm, top=20):
    """[(offset, count, sorted regs)] for [REG + 0xNN] with REG not EBP/ESP, top by count then offset."""
    cnt, regs = {}, {}
    for _a, _b, insn in asm:
        for m in _MEM_RE.finditer(insn):
            mm = _REG_OFF_RE.match(m.group(1))
            if not mm:
                continue
            off = int(mm.group(2), 16)
            if off >= 0x80000000:
                continue
            cnt[off] = cnt.get(off, 0) + 1
            regs.setdefault(off, set()).add(mm.group(1))
    rows = sorted(cnt.items(), key=lambda kv: (-kv[1], kv[0]))[:top]
    return [(o, c, sorted(regs[o])) for o, c in rows]


def asm_call_targets(asm):
    out = []
    for _a, _b, insn in asm:
        m = re.match(r"^CALL\s+(0x[0-9a-fA-F]+)$", insn.strip())
        if m:
            v = int(m.group(1), 16)
            if v not in out:
                out.append(v)
    return out


def cmd_anchors(args, root):
    addr = parse_addr(args.addr)
    asm = read_asm(root, addr)
    if asm is None:
        print("ERROR: no asm at %s" % asm_path(root, addr), file=sys.stderr)
        return 2
    header = parse_header(read_text(c_path(root, addr)))
    names = load_name_map(root)
    strings = load_strings(root)
    print("anchors for %s%s" % (hx(addr), " (%s)" % header["name"] if header else ""))
    hits = string_anchors(asm, strings)
    print("string literals (%d):" % len(hits))
    for a, v, s in hits:
        print("  %s  at %08x  %s" % (hx(v), a, repr(s)))
    callees = header["callees"] if header and header["callees"] else asm_call_targets(asm)
    callees = [c for c in callees if c >= 0x00401000]
    print("direct callees (%d):" % len(callees))
    for c in callees:
        print("  %s  %s" % (hx(c), names.get(c, "(unnamed)")))
    offs = struct_offsets(asm)
    print("struct offsets touched (top %d of [REG + 0xNN], EBP/ESP excluded):" % len(offs))
    for o, c, rg in offs:
        print("  +0x%X [dec %d]  x%d  via %s" % (o, o, c, ",".join(rg)))
    return 0


# ---------------------------------------------------------------- subcommand: prologue
_PROLOGUES = [
    ("8bff558bec", "hot-patch MOV EDI,EDI; PUSH EBP; MOV EBP,ESP (MSVC CRT style)"),
    ("558bec", "55 8B EC  PUSH EBP; MOV EBP,ESP"),
    ("568bf1", "56 8B F1  PUSH ESI; MOV ESI,ECX (thiscall this-save)"),
    ("6aff68", "6A FF 68  SEH frame (PUSH -1; PUSH handler)"),
    ("53", "53  PUSH EBX"),
    ("83ec", "83 EC  SUB ESP,imm8"),
    ("81ec", "81 EC  SUB ESP,imm32"),
    ("e9", "E9  JMP rel32 (thunk)"),
]


def classify_prologue(bytes_hex):
    """List of matched standard-prologue labels for the first instruction bytes (hex string)."""
    found = []
    for pat, label in _PROLOGUES:
        if bytes_hex.startswith(pat):
            found.append(label)
    # SEH push-handler normally follows the frame setup
    for lead in ("558bec", "8bff558bec"):
        if bytes_hex.startswith(lead) and bytes_hex[len(lead):].startswith("6aff68"):
            found.append(_PROLOGUES[3][1] + " after frame setup")
    return found


def cmd_prologue(args, root):
    addr = parse_addr(args.addr)
    asm = read_asm(root, addr)
    if asm is None:
        print("ERROR: no asm at %s" % asm_path(root, addr), file=sys.stderr)
        return 2
    rem = addr % 16
    print("prologue for %s" % hx(addr))
    if rem == 0:
        print("16-byte aligned: YES")
    else:
        print("16-byte aligned: NO (addr %% 16 = %d) - UNALIGNED" % rem)
    print("note: alignment is a strong signal, not proof (1785/1796 high-confidence name_map rows are aligned; "
          "the exceptions are CRT/library functions such as operator delete 0x9196FD).")
    first = asm[:3]
    print("first 3 instructions:")
    for a, b, insn in first:
        print("  %08x  %-14s %s" % (a, " ".join(b[i:i + 2] for i in range(0, len(b), 2)).upper(), insn))
    allbytes = "".join(b for _a, b, _i in asm[:6])
    found = classify_prologue(allbytes)
    if found:
        print("standard prologue: YES - %s" % "; ".join(found))
    else:
        print("standard prologue: NO (none of 55 8B EC / 56 8B F1 / 53 / 83 EC / 81 EC / 6A FF 68 / E9)")
    return 0


# ---------------------------------------------------------------- CLI
def build_parser():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--root", default=None, help="dump root (default %s)" % DEFAULT_ROOT)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("annotate")
    p.add_argument("addr")
    p.add_argument("--max-lines", type=int, default=0)
    p.add_argument("--root", default=argparse.SUPPRESS)
    p = sub.add_parser("abi")
    p.add_argument("addr")
    p.add_argument("--gog-bytes", default=None)
    p.add_argument("--gog-name", default=None)
    p.add_argument("--gog-addr", default=None)
    p.add_argument("--callconv", default=None, choices=["thiscall", "stdcall", "cdecl", "fastcall"])
    p.add_argument("--root", default=argparse.SUPPRESS)
    p = sub.add_parser("anchors")
    p.add_argument("addr")
    p.add_argument("--root", default=argparse.SUPPRESS)
    p = sub.add_parser("prologue")
    p.add_argument("addr")
    p.add_argument("--root", default=argparse.SUPPRESS)
    return ap


def main(argv=None):
    ap = build_parser()
    args = ap.parse_args(argv)
    root = expand_root(args.root)  # --root works before or after the subcommand (subparsers use SUPPRESS)
    fn = {"annotate": cmd_annotate, "abi": cmd_abi, "anchors": cmd_anchors, "prologue": cmd_prologue}[args.cmd]
    return fn(args, root)


if __name__ == "__main__":
    sys.exit(main())
