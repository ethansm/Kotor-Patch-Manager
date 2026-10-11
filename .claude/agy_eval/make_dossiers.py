#!/usr/bin/env python3
"""Freeze the blinded agy-vs-Haiku comparison inputs. No model calls; stdlib only; deterministic.

Run:  python3 -I .claude/agy_eval/make_dossiers.py

Read only:
  .claude/review/work/verify_queue.json, verify_key.json
  .claude/review/work/fix/calib_key.json, calib_tasks.json
  .claude/review/fix/patches/<ctid>.a1.patch
  .claude/review/verify/verify_schema.json, .claude/review/fix/review_schema.json
  .claude/agy_eval/prompts/PROMPT_verify.txt, PROMPT_fix.txt
  the repo source files named by queue/calibration entries

Written (frozen/ is rebuilt from scratch on every run):
  frozen/verify/<id>.prompt.txt   gold verify items (decoys + controls)
  frozen/fix/<ctid>.prompt.txt    fix calibration items
  frozen/real/<id>.prompt.txt     seeded sample of real verify items
  frozen/KEY.json                 answer key (never print or paste)
  frozen/MANIFEST.json            sha256 of every prompt, the sources, misses, redactions, drift
"""
from __future__ import annotations

import hashlib
import json
import random
import re
import shutil
import sys
from pathlib import Path

SELF = Path(__file__).resolve()
REPO = SELF.parents[2]
REVIEW = REPO / ".claude" / "review"
AGY = REPO / ".claude" / "agy_eval"
PROMPTS = AGY / "prompts"
FROZEN = AGY / "frozen"

VERIFY_QUEUE = REVIEW / "work" / "verify_queue.json"
VERIFY_KEY = REVIEW / "work" / "verify_key.json"
CALIB_KEY = REVIEW / "work" / "fix" / "calib_key.json"
CALIB_TASKS = REVIEW / "work" / "fix" / "calib_tasks.json"
PATCH_DIR = REVIEW / "fix" / "patches"
VERIFY_SCHEMA = REVIEW / "verify" / "verify_schema.json"
REVIEW_SCHEMA = REVIEW / "fix" / "review_schema.json"
PROMPT_VERIFY = PROMPTS / "PROMPT_verify.txt"
PROMPT_FIX = PROMPTS / "PROMPT_fix.txt"

SAMPLE_SEED = 20261010
SAMPLE_N = 30
CTX = 15
EXCERPT_HDR = "EXCERPT (±15 lines, line numbers shown):"
ORIG_HDR = "ORIGINAL EXCERPT (±15 lines, line numbers shown):"
KIND_ROLE = {"seeded-bad": "seeded_bad", "known-good": "known_good"}

BLIND_WORDS = ["decoy", "control", "seeded", "calib", "gold", "label", "mode",
               "known_good", "good patch", "bad patch", "expected"]
BLIND_WORD_RE = [(w, re.compile(r"\b" + re.escape(w) + r"\b", re.IGNORECASE)) for w in BLIND_WORDS]
BLIND_SUB_RE = [(w, re.compile(re.escape(w), re.IGNORECASE)) for w in BLIND_WORDS]
ABS_RE = re.compile(r"/workspaces/|orca/workspaces|/home/|/tmp/|/var/|/root/|/Users/|claude-1000"
                    r"|(?<![A-Za-z0-9])[A-Za-z]:\\[A-Za-z0-9_]")
HOME_RE = re.compile(r"/home/[^/\s\"'`]+")
TMP_RE = re.compile(r"/tmp/")
HUNK_RE = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")

REDACTIONS: list[tuple[str, str, str, int]] = []  # (id, field, kind, count)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def rel(p: Path) -> str:
    return p.resolve().relative_to(REPO).as_posix()


def load(p: Path):
    return json.loads(p.read_text(encoding="utf-8"))


def dump(p: Path, obj) -> bytes:
    data = (json.dumps(obj, indent=2, sort_keys=True, ensure_ascii=True) + "\n").encode("utf-8")
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_bytes(data)
    return data


def repo_rel(path_text) -> str | None:
    if not isinstance(path_text, str):
        return None
    s = path_text.strip().replace("\\", "/")
    if not s or s.startswith("/") or re.match(r"^[A-Za-z]:", s):
        return None
    parts = [x for x in s.split("/") if x not in ("", ".")]
    if not parts or ".." in parts:
        return None
    return "/".join(parts)


def source_lines(path_text) -> list[str] | None:
    """Lines of a repo-relative source file, or None when it cannot be read as UTF-8 text."""
    r = repo_rel(path_text)
    if r is None:
        return None
    try:
        real = (REPO / r).resolve()
        if REPO not in real.parents or not real.is_file():
            return None
        data = real.read_bytes()
    except OSError:
        return None
    if b"\x00" in data[:4096]:
        return None
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        return None
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    return [x[:-1] if x.endswith("\r") else x for x in lines]


def span_for(lines_field, src: list[str] | None):
    """(lo, hi) inside the file, or (None, reason). Nothing is guessed."""
    if src is None:
        return None, "file-unreadable"
    nums = [int(x) for x in re.findall(r"\d+", str(lines_field or ""))]
    if not nums:
        return None, "range-unparsable"
    lo, hi = min(nums), max(nums)
    if lo < 1 or hi > len(src):
        return None, "range-outside-file"
    return (lo, hi), None


def redact(text: str, pid: str, field: str) -> str:
    """Absolute path prefixes are shortened (prompts must not carry absolute paths).
    /home/<user>/ -> ~/ and /tmp/ -> <tmp>/ ; every substitution is logged in MANIFEST.json."""
    new, n_home = HOME_RE.subn("~", text)
    if n_home:
        REDACTIONS.append((pid, field, "home", n_home))
    new, n_tmp = TMP_RE.subn("<tmp>/", new)
    if n_tmp:
        REDACTIONS.append((pid, field, "tmp", n_tmp))
    return new


def excerpt(src: list[str], lo: int, hi: int) -> str:
    a, b = max(1, lo - CTX), min(len(src), hi + CTX)
    return "\n".join(f"{n:>5}: {src[n - 1]}" for n in range(a, b + 1))


def patch_hunks(patch_text: str):
    """(repo-rel path, old-side start, old-side lines) for each hunk of a unified diff."""
    out, cur = [], None
    lines = patch_text.split("\n")
    i = 0
    while i < len(lines):
        ln = lines[i]
        if ln.startswith("--- a/"):
            cur = ln[6:].strip()
            i += 1
            continue
        m = HUNK_RE.match(ln)
        if m and cur is not None:
            start = int(m.group(1))
            want_old = int(m.group(2)) if m.group(2) is not None else 1
            want_new = int(m.group(4)) if m.group(4) is not None else 1
            old, n_old, n_new = [], 0, 0
            i += 1
            while (n_old < want_old or n_new < want_new) and i < len(lines):
                t = lines[i]
                tag = t[:1]
                if tag == "\\":
                    pass
                elif tag == " " or t == "":
                    old.append(t[1:])
                    n_old += 1
                    n_new += 1
                elif tag == "-":
                    old.append(t[1:])
                    n_old += 1
                elif tag == "+":
                    n_new += 1
                else:
                    break
                i += 1
            out.append((cur, start, [x[:-1] if x.endswith("\r") else x for x in old]))
            continue
        i += 1
    return out


def patch_drift(patch_text: str) -> list[str]:
    """Hunks whose old-side lines no longer match the current source (patch base drift)."""
    bad = []
    for path_text, start, old in patch_hunks(patch_text):
        src = source_lines(path_text)
        if src is None:
            bad.append(f"{path_text}:unreadable")
            continue
        s0 = max(start - 1, 0)
        if src[s0:s0 + len(old)] != old:
            bad.append(f"{path_text}:{start}")
    return bad


def build_verify(pid: str, item: dict):
    src = source_lines(item["file"])
    span, why = span_for(item["lines"], src)
    if span is None:
        return None, why
    lo, hi = span
    claim = redact(str(item["claim"]).strip(), pid, "claim")
    exc = redact(excerpt(src, lo, hi), pid, "excerpt")
    body = (PROMPT_VERIFY_TEXT.rstrip("\n") + "\n\n"
            + f"CLAIM: {claim}\n"
            + f"FILE: {item['file']}\n"
            + f"LINES: {item['lines']}\n"
            + f"{EXCERPT_HDR}\n{exc}\n")
    return body, None


def build_fix(pid: str, claim: str, file_text: str, lines_text: str, patch_text: str):
    src = source_lines(file_text)
    span, why = span_for(lines_text, src)
    if span is None:
        return None, why
    lo, hi = span
    claim_r = redact(claim.strip(), pid, "claim")
    patch_r = redact(patch_text if patch_text.endswith("\n") else patch_text + "\n", pid, "patch")
    exc = redact(excerpt(src, lo, hi), pid, "excerpt")
    body = (PROMPT_FIX_TEXT.rstrip("\n") + "\n\n"
            + f"CLAIM: {claim_r}\n"
            + "PATCH:\n"
            + patch_r + "\n"
            + f"{ORIG_HDR}\n{exc}\n")
    return body, None


def blind_scan(name: str, text: str):
    words = [(name, w) for w, rx in BLIND_WORD_RE if rx.search(text)]
    have = {w for _, w in words}
    subs = [(name, w) for w, rx in BLIND_SUB_RE if w not in have and rx.search(text)]
    return words, subs


def main() -> int:
    misses: list[dict] = []
    drift: list[str] = []
    outputs: list[tuple[str, str, str, str]] = []  # (set, subdir, id, text)
    key: dict[str, dict] = {}

    queue = {it["id"]: it for it in load(VERIFY_QUEUE)["items"]}
    vkey = load(VERIFY_KEY)
    gold: dict[str, tuple[str, str]] = {}
    for group in ("decoys", "controls"):
        for gid, meta in vkey[group].items():
            gold[gid] = (meta["role"], meta["label"])
    assert len(gold) == 15, f"expected 15 gold verify ids, found {len(gold)}"

    # 1. gold verify items
    for gid in sorted(gold):
        role, label = gold[gid]
        item = queue.get(gid)
        body, why = build_verify(gid, item) if item else (None, "not-in-queue")
        if body is None:
            misses.append({"id": gid, "set": "verify_gold", "reason": why})
            continue
        outputs.append(("verify_gold", "verify", gid, body))
        key[gid] = {"set": "verify_gold", "role": role, "label": label}

    # 2. fix calibration items
    ck = load(CALIB_KEY)["items"]
    ctasks = {t["task_id"]: t for t in load(CALIB_TASKS)["tasks"]}
    for ctid in sorted(ck):
        meta = ck[ctid]
        task = ctasks.get(ctid)
        patch_p = PATCH_DIR / f"{ctid}.a1.patch"
        if task is None or len(task.get("findings") or []) != 1 or not patch_p.is_file():
            misses.append({"id": ctid, "set": "fix_gold", "reason": "task-or-patch-missing"})
            continue
        f = task["findings"][0]
        patch_text = patch_p.read_text(encoding="utf-8")
        for d in patch_drift(patch_text):
            drift.append(f"{ctid}:{d}")
        body, why = build_fix(ctid, str(f["claim"]), f["file"], f["lines"], patch_text)
        if body is None:
            misses.append({"id": ctid, "set": "fix_gold", "reason": why})
            continue
        outputs.append(("fix_gold", "fix", ctid, body))
        key[ctid] = {"set": "fix_gold", "role": KIND_ROLE[meta["kind"]], "label": meta["expected"]}

    # 3. real verify sample (queue minus gold; fixed seed; no resampling on MISS)
    real_ids = sorted(i for i in queue if i not in gold)
    sample = random.Random(SAMPLE_SEED).sample(real_ids, SAMPLE_N)
    for rid in sample:
        body, why = build_verify(rid, queue[rid])
        if body is None:
            misses.append({"id": rid, "set": "real", "reason": why})
            continue
        outputs.append(("real", "real", rid, body))
        key[rid] = {"set": "real", "role": "real", "label": None}

    # 4. write frozen/ from scratch
    if FROZEN.exists():
        shutil.rmtree(FROZEN)
    entries = []
    texts_for_scan: list[tuple[str, str]] = []
    for set_name, sub, pid, body in outputs:
        p = FROZEN / sub / f"{pid}.prompt.txt"
        p.parent.mkdir(parents=True, exist_ok=True)
        data = body.encode("utf-8")
        p.write_bytes(data)
        entries.append({"id": pid, "set": set_name, "path": rel(p),
                        "sha256": sha256_bytes(data), "bytes": len(data)})
        texts_for_scan.append((rel(p), body))
    entries.sort(key=lambda e: e["path"])
    key_bytes = dump(FROZEN / "KEY.json", dict(sorted(key.items())))

    sources = {}
    for p in (PROMPT_VERIFY, PROMPT_FIX, VERIFY_SCHEMA, REVIEW_SCHEMA, SELF):
        sources[rel(p)] = sha256_bytes(p.read_bytes())
    manifest = {
        "prompts": entries,
        "sources": sources,
        "misses": misses,
        "redactions": [{"id": a, "field": b, "kind": k, "count": c} for a, b, k, c in REDACTIONS],
        "patch_drift": drift,
    }
    man_bytes = dump(FROZEN / "MANIFEST.json", manifest)

    # 5. checks (report only; blinding hits are listed, never edited away)
    texts_for_scan += [(rel(PROMPT_VERIFY), PROMPT_VERIFY_TEXT), (rel(PROMPT_FIX), PROMPT_FIX_TEXT)]
    word_hits, sub_hits, abs_hits = [], [], []
    for name, text in texts_for_scan:
        w, s = blind_scan(name, text)
        word_hits += w
        sub_hits += s
        abs_hits += [(name, m.group(0)) for m in ABS_RE.finditer(text)]

    by_set: dict[str, int] = {}
    by_role: dict[str, int] = {}
    for e in key.values():
        by_set[e["set"]] = by_set.get(e["set"], 0) + 1
        by_role[e["role"]] = by_role.get(e["role"], 0) + 1
    print(f"written: {len(entries)} prompts; per set {dict(sorted(by_set.items()))}; roles {dict(sorted(by_role.items()))}")
    print(f"real pool: {len(real_ids)} (queue {len(queue)} minus {len(gold)} gold); sample seed {SAMPLE_SEED} n {SAMPLE_N}")
    print(f"misses: {len(misses)} {json.dumps(misses, sort_keys=True)}")
    print(f"patch drift: {len(drift)} {json.dumps(drift)}")
    print(f"redactions (home path -> ~): {json.dumps(manifest['redactions'], sort_keys=True)}")
    print(f"blinding word hits (word boundary): {len(word_hits)} {json.dumps(word_hits)}")
    print(f"blinding substring-only hits: {len(sub_hits)} {json.dumps(sub_hits)}")
    print(f"absolute-path hits after redaction: {len(abs_hits)} {json.dumps(abs_hits)}")
    print(f"MANIFEST.json sha256: {sha256_bytes(man_bytes)}")
    print(f"KEY.json sha256: {sha256_bytes(key_bytes)}")
    return 2 if abs_hits else 0


PROMPT_VERIFY_TEXT = PROMPT_VERIFY.read_text(encoding="utf-8")
PROMPT_FIX_TEXT = PROMPT_FIX.read_text(encoding="utf-8")

if __name__ == "__main__":
    sys.exit(main())
