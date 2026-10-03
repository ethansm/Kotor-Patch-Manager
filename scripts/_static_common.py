"""Shared library for the doc 20 Phase 1 static checks (check_hook_bytes, check_hook_spans, check_exports).

Everything takes explicit paths or bytes, so tests can feed tiny fixtures; the module-level constants are only
the defaults the command-line wrappers use.

  KOTOR2_ROOT          game dir (via _common.GAME_DIR)
  KPM_DIR              Kotor Patch Manager checkout (default ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager)
  KOTOR2_VANILLA_EXE   pristine swkotor2.exe (default <workspace>/backups/vanilla_clean/swkotor2.exe)
  TMPDIR               scratch for objdump input (default /tmp/claude-<uid>)

Mods: patch_manager_mods/**/ dirs holding manifest.toml + kotor2-steam-aspyr.hooks.toml (see load_mods).
Hook spans (HOOKS) come from that toml; byte patches the DLLs write themselves (CODE_PATCHES) from the per-mod
code_patches.toml sidecar (see load_code_patches for the schema).
"""
import hashlib
import os
import struct
import subprocess
import sys
import tempfile
import tomllib
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
try:
    import _common  # noqa: E402  GAME_DIR, WORKSPACE (same env overrides as the other scripts)
    _GAME, _WORKSPACE = Path(_common.GAME_DIR), Path(_common.WORKSPACE)
except ImportError:  # pragma: no cover - _common is optional (same defaults and env vars)
    _GAME = Path(os.environ.get("KOTOR2_ROOT")
                 or "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II").expanduser()
    _WORKSPACE = Path(os.environ.get("KOTOR2_WORKSPACE") or "~/Games/KOTOR2_Mod_Workspace").expanduser()

REPO = Path(__file__).resolve().parent.parent
GAME_DIR = _GAME
KPM_DIR = Path(os.environ.get("KPM_DIR") or _WORKSPACE / "tools" / "kotor-patch-manager").expanduser()
VANILLA_EXE = Path(os.environ.get("KOTOR2_VANILLA_EXE")
                   or _WORKSPACE / "backups" / "vanilla_clean" / "swkotor2.exe").expanduser()
LIVE_CONFIG = GAME_DIR / "patch_config.toml"
OBJDUMP = "i686-w64-mingw32-objdump"

HOOKS_TOML = "kotor2-steam-aspyr.hooks.toml"
CODE_PATCHES_TOML = "code_patches.toml"
SKIP_DIRS = {"testdata", "retired", ".venv", "__pycache__", ".git", "node_modules"}


def tmp_dir() -> Path:
    """Scratch directory ($TMPDIR, default /tmp/claude-<uid>), created on demand."""
    d = Path(os.environ.get("TMPDIR") or f"/tmp/claude-{os.getuid()}")
    d.mkdir(parents=True, exist_ok=True)
    return d


# ---------------------------------------------------------------- reporting

SEVERITIES = ("FAIL", "WARN", "INFO", "PASS", "SKIP")


@dataclass
class Finding:
    sev: str        # FAIL | WARN | INFO | PASS | SKIP
    check: str      # short check name, e.g. "bytes", "boundary"
    msg: str

    def __str__(self):
        return f"{self.sev} {self.check}: {self.msg}"


class Reporter:
    """Collects Findings and prints them as `FAIL|WARN|INFO|PASS|SKIP <check>: <msg>`.

    out=None collects silently (tests). quiet hides PASS and INFO lines (FAIL, WARN, SKIP and the summary stay).
    exit_code() is 1 if any FAIL was reported, else 0.
    """

    def __init__(self, quiet=False, out=sys.stdout):
        self.quiet = quiet
        self.out = out
        self.findings: list[Finding] = []

    def add(self, sev, check, msg=""):
        f = Finding(sev, check, msg)
        self.findings.append(f)
        if self.out is not None and not (self.quiet and sev in ("PASS", "INFO")):
            print(f, file=self.out)
        return f

    def fail(self, check, msg=""): return self.add("FAIL", check, msg)
    def warn(self, check, msg=""): return self.add("WARN", check, msg)
    def info(self, check, msg=""): return self.add("INFO", check, msg)
    def passed(self, check, msg=""): return self.add("PASS", check, msg)
    def skip(self, check, msg=""): return self.add("SKIP", check, msg)

    def count(self, sev) -> int:
        return sum(1 for f in self.findings if f.sev == sev)

    def of(self, sev):
        return [f for f in self.findings if f.sev == sev]

    def summary(self) -> str:
        return " ".join(f"{s}={self.count(s)}" for s in SEVERITIES)

    def exit_code(self) -> int:
        return 1 if self.count("FAIL") else 0

    def finish(self, title="") -> int:
        """Print the summary line and return the process exit code."""
        if self.out is not None:
            print(f"{title + ': ' if title else ''}{self.summary()}", file=self.out)
        return self.exit_code()


def sha256_file(path) -> str:
    """Upper-case hex sha256 of a file."""
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest().upper()


def require_version(exe, expected, rep: Reporter, check="version") -> bool:
    """True if `exe` exists and its sha256 equals `expected` (case-insensitive).

    A missing exe is a SKIP (the caller exits 0), a different hash is a FAIL; both return False.
    """
    exe = Path(exe)
    if not exe.is_file():
        rep.skip(check, f"{exe} not found")
        return False
    got = sha256_file(exe)
    if got != str(expected).upper():
        rep.fail(check, f"{exe} sha256 {got[:8]}... != expected {str(expected).upper()[:8]}...")
        return False
    return True


# ---------------------------------------------------------------- hooks / code patches / mods

def _to_int(v) -> int:
    return v if isinstance(v, int) else int(str(v), 16) if str(v).lower().startswith("0x") else int(str(v))


def _to_bytes(v) -> bytes:
    """original_bytes as written in a hooks toml: an int list (ours, KPM) or a hex string ("55 8b ec")."""
    if isinstance(v, str):
        return bytes.fromhex(v.replace(" ", ""))
    return bytes(v)


@dataclass
class Hook:
    address: int
    type: str                        # "detour" | "simple"
    orig: bytes                      # original_bytes: the stolen window, its length is the span
    function: str | None = None      # detour only
    skip_original: bool = False      # skip_original_bytes (detour only)
    replacement: bytes | None = None  # replacement_bytes (simple only)
    mod: str = ""                    # owning patch id (filled by the loaders)
    source: str = ""                 # where it came from (path), for messages

    @property
    def end(self) -> int:
        return self.address + len(self.orig)


@dataclass
class CodePatch:
    name: str
    address: int
    orig: bytes
    kind: str = "code"               # "code" | "data"
    verifies_original: bool = False  # the DLL memcmp's the site before writing
    enabled_by: str = ""
    source: str = ""
    mod: str = ""

    @property
    def end(self) -> int:
        return self.address + len(self.orig)


@dataclass
class Mod:
    id: str
    version: str
    dir: Path
    kpm_dir: Path | None             # KPM Patches/<Name>/ (matched by manifest id), None if absent
    hooks: list                      # [Hook]
    target_versions: list            # upper-case sha256 strings; empty = applies to every exe
    exports: list                    # names in exports.def (empty if no file)
    code_patches: list               # [CodePatch]
    hooks_path: Path | None = None
    has_def: bool = False


def parse_hooks(data, mod="", source="") -> tuple[list, list]:
    """(target_versions, [Hook]) from hooks-toml text or bytes."""
    if isinstance(data, bytes):
        data = data.decode("utf-8")
    doc = tomllib.loads(data)
    tv = [str(s).upper() for s in doc.get("metadata", {}).get("target_versions", [])]
    hooks = []
    for h in doc.get("hooks", []):
        hooks.append(Hook(
            address=_to_int(h["address"]), type=h.get("type", "detour"), orig=_to_bytes(h["original_bytes"]),
            function=h.get("function"), skip_original=bool(h.get("skip_original_bytes", False)),
            replacement=_to_bytes(h["replacement_bytes"]) if "replacement_bytes" in h else None,
            mod=mod, source=source))
    return tv, hooks


def parse_def(text) -> list:
    """Export names from a module-definition file (EXPORTS section; `;` comments, `@ordinal`, `=alias` stripped)."""
    names, in_exports = [], False
    for line in text.splitlines():
        line = line.split(";", 1)[0].strip()
        if not line:
            continue
        word = line.split()[0]
        if word.upper() in ("LIBRARY", "NAME", "DESCRIPTION", "STACKSIZE", "VERSION", "HEAPSIZE"):
            continue
        if word.upper() == "EXPORTS":
            in_exports = True
            rest = line[len(word):].split()
            line = rest[0] if rest else ""
            if not line:
                continue
        if in_exports:
            names.append(line.split()[0].split("=")[0].split("@")[0])
    return names


def load_code_patches(moddir) -> list:
    """The `code_patches.toml` sidecar of a mod dir (or a direct path to one); [] if there is none.

    Schema ([[code_patches]] tables): name (str), address (int), original_bytes (int list, length = span, the
    vanilla bytes), kind ("code" default | "data"), verifies_original (bool, the DLL memcmp's before writing),
    enabled_by (free text), source (file:line in the mod's C++). Missing name/address/original_bytes, an unknown
    kind or an empty span raise ValueError.
    """
    p = Path(moddir)
    if p.is_dir():
        p = p / CODE_PATCHES_TOML
    if not p.is_file():
        return []
    out = []
    for i, t in enumerate(tomllib.loads(p.read_text(encoding="utf-8")).get("code_patches", [])):
        for key in ("name", "address", "original_bytes"):
            if key not in t:
                raise ValueError(f"{p}: code_patches[{i}] lacks '{key}'")
        cp = CodePatch(name=t["name"], address=_to_int(t["address"]), orig=_to_bytes(t["original_bytes"]),
                       kind=t.get("kind", "code"), verifies_original=bool(t.get("verifies_original", False)),
                       enabled_by=t.get("enabled_by", ""), source=t.get("source", ""))
        if cp.kind not in ("code", "data"):
            raise ValueError(f"{p}: code_patches[{i}] '{cp.name}': kind must be 'code' or 'data', not {cp.kind!r}")
        if not cp.orig:
            raise ValueError(f"{p}: code_patches[{i}] '{cp.name}': empty original_bytes")
        out.append(cp)
    return out


def _kpm_index(kpm_dir) -> dict:
    """{patch id: [(Patches/<Name> dir, version)]} for a KPM checkout (empty if it is missing)."""
    idx = {}
    root = Path(kpm_dir) / "Patches" if kpm_dir else None
    if root and root.is_dir():
        for d in sorted(root.iterdir()):
            m = d / "manifest.toml"
            if d.is_dir() and m.is_file():
                try:
                    pt = tomllib.loads(m.read_text(encoding="utf-8")).get("patch", {})
                except tomllib.TOMLDecodeError:
                    continue
                if "id" in pt:
                    idx.setdefault(pt["id"], []).append((d, str(pt.get("version", ""))))
    return idx


def load_mods(repo=REPO, kpm_dir=KPM_DIR, variants=False) -> list:
    """The mods under <repo>/patch_manager_mods: every dir with manifest.toml + kotor2-steam-aspyr.hooks.toml.

    Skips testdata, retired, .venv, __pycache__ (SKIP_DIRS). Each Mod gets its hooks, target_versions, exports.def
    names, code_patches sidecar and the KPM `Patches/<Name>/` dir with the same manifest id (the one with the same
    version when several share the id; None if kpm_dir is missing or has no match).

    Two dirs may share an id (dialogue_letterbox_fix_probe_src is a staged 1.0.1-probe build of
    k2-dialogue-letterbox-fix). By default only the primary is returned: the one with a plain version (no `-suffix`),
    else the first by path. variants=True returns every dir (for matching a .kpatch by id + version).
    """
    root = Path(repo) / "patch_manager_mods"
    found = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = sorted(d for d in dirnames if d not in SKIP_DIRS)
        if "manifest.toml" in filenames and HOOKS_TOML in filenames:
            found.append(Path(dirpath))
    idx = _kpm_index(kpm_dir)
    mods = []
    for d in sorted(found):
        pt = tomllib.loads((d / "manifest.toml").read_text(encoding="utf-8")).get("patch", {})
        pid, ver = pt.get("id", ""), str(pt.get("version", ""))
        tv, hooks = parse_hooks((d / HOOKS_TOML).read_bytes(), mod=pid, source=str(d / HOOKS_TOML))
        cands = idx.get(pid, [])
        kd = next((c for c, v in cands if v == ver), cands[0][0] if cands else None)
        defp = d / "exports.def"
        cps = load_code_patches(d)
        for cp in cps:
            cp.mod = pid
        mods.append(Mod(id=pid, version=ver, dir=d, kpm_dir=kd, hooks=hooks, target_versions=tv,
                        exports=parse_def(defp.read_text(encoding="utf-8")) if defp.is_file() else [],
                        code_patches=cps, hooks_path=d / HOOKS_TOML, has_def=defp.is_file()))
    if not variants:
        best = {}
        for m in mods:
            cur = best.get(m.id)
            if cur is None or ("-" in cur.version and "-" not in m.version):
                best[m.id] = m
        mods = [m for m in mods if best[m.id] is m]
    return mods


def live_target_sha(path=LIVE_CONFIG) -> str | None:
    """`target_version_sha` of the live patch_config.toml (upper-case), or None if absent."""
    p = Path(path)
    if not p.is_file():
        return None
    s = tomllib.loads(p.read_text(encoding="utf-8")).get("target_version_sha")
    return s.upper() if s else None


def load_live_config(path=LIVE_CONFIG) -> dict:
    """{patch id: [Hook]} from a live patch_config.toml (decimal address/original_bytes). DLL-only ids map to [].

    Read-only; raises FileNotFoundError if `path` is missing.
    """
    doc = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    out = {}
    for pt in doc.get("patches", []):
        pid = pt.get("id", "?")
        out[pid] = [Hook(address=_to_int(h["address"]), type=h.get("type", "detour"), orig=_to_bytes(h["original_bytes"]),
                         function=h.get("function"), skip_original=bool(h.get("skip_original_bytes", False)),
                         replacement=_to_bytes(h["replacement_bytes"]) if "replacement_bytes" in h else None,
                         mod=pid, source=str(path))
                    for h in pt.get("hooks", [])]
    return out


# ---------------------------------------------------------------- .kpatch zips

@dataclass
class KpatchMembers:
    path: str
    names: list
    manifest: bytes | None = None
    hooks_name: str | None = None
    hooks: bytes | None = None
    dll: bytes | None = None        # binaries/windows_x86.dll

    @property
    def manifest_doc(self) -> dict:
        return tomllib.loads(self.manifest.decode("utf-8")).get("patch", {}) if self.manifest else {}


def kpatch_members(path) -> KpatchMembers:
    """Read a .kpatch zip fully into memory: manifest.toml, the *.hooks.toml and binaries/windows_x86.dll.

    Absent members are None (the caller decides whether that matters). `path` may also be a file-like object.
    """
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        hn = next((n for n in names if n.endswith(".hooks.toml")), None)
        return KpatchMembers(
            path=str(path), names=names,
            manifest=z.read("manifest.toml") if "manifest.toml" in names else None,
            hooks_name=hn, hooks=z.read(hn) if hn else None,
            dll=z.read("binaries/windows_x86.dll") if "binaries/windows_x86.dll" in names else None)


# ---------------------------------------------------------------- PE images

@dataclass
class Section:
    name: str
    va: int          # absolute (image base + RVA)
    vsize: int
    raw_off: int
    raw_size: int
    flags: int

    @property
    def end(self) -> int:
        return self.va + max(self.vsize, self.raw_size)

    @property
    def is_code(self) -> bool:
        return bool(self.flags & 0x20000000 or self.flags & 0x20)

    def contains(self, va) -> bool:
        return self.va <= va < self.end


class PeImage:
    """A 32-bit PE file parsed in pure Python: section table, VA reads, export names.

    PeImage(path) or PeImage(data=bytes) (e.g. a DLL read out of a .kpatch zip). Attributes: data, image_base,
    sections ([Section]). Raises ValueError if the bytes are not a PE32 image.
    """

    def __init__(self, path=None, data: bytes | None = None):
        self.path = str(path) if path is not None else "<memory>"
        self.data = bytes(data) if data is not None else Path(path).read_bytes()
        d = self.data
        if len(d) < 0x40 or d[:2] != b"MZ":
            raise ValueError(f"{self.path}: not a PE file (no MZ header)")
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        if d[pe:pe + 4] != b"PE\0\0":
            raise ValueError(f"{self.path}: bad PE signature")
        nsec, = struct.unpack_from("<H", d, pe + 6)
        optsize, = struct.unpack_from("<H", d, pe + 20)
        opt = pe + 24
        if struct.unpack_from("<H", d, opt)[0] != 0x10B:
            raise ValueError(f"{self.path}: not a PE32 image")
        self.image_base, = struct.unpack_from("<I", d, opt + 28)
        ndirs, = struct.unpack_from("<I", d, opt + 92)
        self._dirs = [struct.unpack_from("<II", d, opt + 96 + 8 * i) for i in range(min(ndirs, 16))]
        self.sections = []
        for i in range(nsec):
            o = opt + optsize + 40 * i
            name = d[o:o + 8].rstrip(b"\0").decode("ascii", "replace")
            vsize, rva, rawsize, rawoff = struct.unpack_from("<IIII", d, o + 8)
            flags, = struct.unpack_from("<I", d, o + 36)
            self.sections.append(Section(name, self.image_base + rva, vsize, rawoff, rawsize, flags))

    def section_of(self, va) -> Section | None:
        """The section containing absolute address `va`, or None."""
        return next((s for s in self.sections if s.contains(va)), None)

    def va_to_offset(self, va) -> int | None:
        """File offset of `va`, or None if it is outside every section's raw data."""
        s = self.section_of(va)
        if s is None or va - s.va >= s.raw_size:
            return None
        return s.raw_off + (va - s.va)

    def read_va(self, va, n) -> bytes:
        """`n` bytes at absolute address `va`. Raises ValueError if the range is not backed by file data."""
        s = self.section_of(va)
        if s is None or va + n > s.va + s.raw_size:
            raise ValueError(f"{self.path}: 0x{va:08X}+{n} is not inside a section's file data")
        off = s.raw_off + (va - s.va)
        return self.data[off:off + n]

    def exports(self) -> list:
        """Names in the export directory, in table order ([] if there is none). No objdump needed."""
        if not self._dirs or self._dirs[0][0] == 0:
            return []
        base = self.image_base

        def off(rva):
            o = self.va_to_offset(base + rva)
            if o is None:
                raise ValueError(f"{self.path}: export RVA 0x{rva:X} outside the image")
            return o
        e = off(self._dirs[0][0])
        nnames, = struct.unpack_from("<I", self.data, e + 24)
        names_rva, = struct.unpack_from("<I", self.data, e + 32)
        out = []
        for i in range(nnames):
            nrva, = struct.unpack_from("<I", self.data, off(names_rva) + 4 * i)
            o = off(nrva)
            out.append(self.data[o:self.data.index(b"\0", o)].decode("ascii", "replace"))
        return out


# ---------------------------------------------------------------- disassembly

@dataclass
class Insn:
    va: int
    length: int
    mnemonic: str       # first word, e.g. "call", "jne", "rep"
    operands: str       # the rest of the text, e.g. "0x8a7e40", "*%eax", "%al,%es:(%edi)"
    raw: bytes = b""

    @property
    def end(self) -> int:
        return self.va + self.length

    @property
    def text(self) -> str:
        return f"{self.mnemonic} {self.operands}".strip()


def disasm(data: bytes, va: int) -> list:
    """Disassemble raw x86-32 bytes at virtual address `va` with objdump (AT&T syntax) -> [Insn].

    Runs `i686-w64-mingw32-objdump -D -b binary -m i386 --adjust-vma=VA` on a temp file under $TMPDIR. Undecodable
    bytes come back as mnemonic "(bad)". Raises RuntimeError if objdump is missing or fails.
    """
    fd, name = tempfile.mkstemp(prefix="disasm_", suffix=".bin", dir=tmp_dir())
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data)
        try:
            r = subprocess.run([OBJDUMP, "-D", "-b", "binary", "-m", "i386", f"--adjust-vma=0x{va:x}", name],
                               capture_output=True, text=True, timeout=60)
        except FileNotFoundError as e:
            raise RuntimeError(f"{OBJDUMP} not found") from e
    finally:
        os.unlink(name)
    if r.returncode != 0:
        raise RuntimeError(f"{OBJDUMP} failed: {r.stderr.strip()}")
    out = []
    for line in r.stdout.splitlines():
        parts = line.split("\t")
        head = parts[0].strip()
        if len(parts) < 2 or not head.endswith(":"):
            continue
        try:
            addr = int(head[:-1], 16)
        except ValueError:
            continue
        raw = bytes.fromhex(parts[1].replace(" ", "")) if parts[1].strip() else b""
        if len(parts) >= 3 and parts[2].strip():
            text = parts[2].strip()
            mn, _, ops = text.partition(" ")
            out.append(Insn(addr, len(raw), mn, ops.strip(), raw))
        elif out and out[-1].end == addr:   # continuation line of a long instruction
            out[-1].length += len(raw)
            out[-1].raw += raw
    return out


_REL_BRANCHES = ("call", "jmp", "loop", "loope", "loopne", "loopz", "loopnz", "jecxz", "jcxz")


def branch_target(insn: Insn) -> int | None:
    """Absolute target of a relative call/jmp/jcc/loop/jecxz with an immediate operand, else None.

    Indirect forms (`call *%eax`, `jmp *0x1234(,%eax,4)`) and far/segment forms return None.
    """
    mn = insn.mnemonic
    if not (mn in _REL_BRANCHES or (mn.startswith("j") and mn not in ("ljmp",))):
        return None
    ops = insn.operands.split("<", 1)[0].strip()
    if not ops.startswith("0x") or "*" in ops or "," in ops or "(" in ops:
        return None
    try:
        return int(ops, 16)
    except ValueError:
        return None
