#!/usr/bin/env python3
"""Persistent REA (rea-agents) MCP client over stdio.

One ReaSession is one long-lived `npx rea-agents@6.3.0 mcp` child that keeps
swkotor2.exe open, so repeated probes skip the Ghidra load. Protocol is
newline-delimited JSON-RPC 2.0. Stdlib only.

Library:
    with ReaSession() as s:
        s.call("procedure_info", {"procedure": "0x00776db0"})
CLI:
    rea_session.py guard [--max-live 2]
    rea_session.py probe [--addr 0x776db0 ...]
    rea_session.py call <tool> --args '<json>' [--write]
Every non-cached call appends one row to <cache_dir>/runs.tsv.
"""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import queue
import re
import shlex
import statistics
import subprocess
import sys
import threading
import time
from datetime import datetime
from pathlib import Path

EXE = Path.home() / "Games/KOTOR2_Mod_Workspace/backups/vanilla_clean/swkotor2.exe"
EXE_SHA256 = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
REA_CMD = ["npx", "-y", "rea-agents@6.3.0", "mcp"]
DUMP_ROOT = Path.home() / "Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71"
REA_ENV = {
    "GHIDRA_INSTALL_DIR": "/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC",
    "JAVA_HOME": "/usr/lib/jvm/java-21-openjdk-amd64",
    "REA_GHIDRA_STARTUP_TIMEOUT_MS": "900000",
}
WRITE_TOOLS = frozenset({
    "set_addresses_names", "set_address_name", "annotate_native_function",
    "set_comment", "set_inline_comment", "set_bookmark", "unset_bookmark",
})
PROTOCOL_VERSION = "2025-06-18"
GUARD_RE = re.compile(r"analyzeHeadless|GhidraClassLoader|rea-agents")
TELEMETRY_HEADER = "ts\ttool\taddr\twall_s\tok\trss_kb"
PROBE_ADDRS = ("0x776db0", "0x7680a0")
PROBE_TOOLS = ("procedure_info", "procedure_assembly", "procedure_callers")
_EOF = object()


# First analysis call after open_binary; pays the cold import (~315 s on the Steam exe, D1 probe 10-15).
WARM_CALL = ("procedure_info", {"procedure": "0x00776db0"})


class ReaError(Exception):
    """A REA call or session failure."""


class ReaGuardError(ReaError):
    """Refused to start: too many live REA/Ghidra processes, or the lock is held."""


class _ConnectionLost(ReaError):
    """Internal: the child closed stdout or refused a write."""


def _child_env(extra=None):
    env = dict(os.environ)
    env.update(REA_ENV)
    if extra:
        env.update(extra)
    return env


def preflight(exe=EXE, sha=EXE_SHA256):
    """Refuse Steam installs and verify the exe digest before any REA work."""
    path = Path(exe)
    if "steamapps" in str(path) or "steamapps" in str(path.resolve()):
        raise ReaError(f"refusing Steam install path: {path}")
    if not path.is_file():
        raise ReaError(f"exe not found: {path}")
    digest = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            digest.update(chunk)
    actual = digest.hexdigest()
    if actual != sha.lower():
        raise ReaError(f"sha256 mismatch for {path}: got {actual}, want {sha}")


def _ps_lines():
    out = subprocess.run(["ps", "-eo", "pid,ppid,args"], capture_output=True, text=True, check=False).stdout
    return out.splitlines()


def _parse_ps(ps_lines):
    """Parse `ps -eo pid,ppid,args` lines into {pid: (ppid, line)}; the header and junk are skipped."""
    procs = {}
    for line in ps_lines:
        parts = line.strip().split(None, 2)
        if len(parts) < 2 or not parts[0].isdigit() or not parts[1].isdigit():
            continue
        procs[int(parts[0])] = (int(parts[1]), line.strip())
    return procs


def live_rea_procs(ps_lines=None):
    """Return one ps line per live REA/Ghidra SESSION (own process and rea_session.py excluded).

    ps_lines must be `ps -eo pid,ppid,args` output (header optional). One session is a process
    tree (npm exec -> node rea-agents -> java GhidraClassLoader/analyzeHeadless), so only ROOT
    matches are returned: matching processes with no matching ancestor in the listing.
    """
    if ps_lines is None:
        ps_lines = _ps_lines()
    me = os.getpid()
    procs = _parse_ps(ps_lines)

    def matches(pid):
        line = procs[pid][1]
        return pid != me and "rea_session.py" not in line and bool(GUARD_RE.search(line))

    roots = []
    for pid in sorted(procs):
        if not matches(pid):
            continue
        seen = {pid}
        parent = procs[pid][0]
        has_matching_ancestor = False
        while parent in procs and parent not in seen:
            if matches(parent):
                has_matching_ancestor = True
                break
            seen.add(parent)
            parent = procs[parent][0]
        if not has_matching_ancestor:
            roots.append(procs[pid][1])
    return roots


def guard(max_live=2, ps_lines=None):
    """Raise ReaGuardError when max_live or more REA/Ghidra sessions (process trees) are running."""
    hits = live_rea_procs(ps_lines)
    if len(hits) >= max_live:
        raise ReaGuardError(f"{len(hits)} live REA/Ghidra sessions (max {max_live})")


def _pump(stream, lines):
    try:
        for line in stream:
            lines.put(line)
    except (OSError, ValueError):
        pass
    finally:
        lines.put(_EOF)


def _decode_tool_result(result, tool):
    content = result.get("content") or []
    if result.get("isError"):
        texts = [p.get("text", "") for p in content if isinstance(p, dict) and isinstance(p.get("text"), str)]
        raise ReaError("\n".join(texts) if texts else f"{tool} failed (isError without text)")
    if result.get("structuredContent") is not None:
        return result["structuredContent"]
    first = content[0] if content and isinstance(content[0], dict) else {}
    text = first.get("text")
    if not isinstance(text, str):
        text = ""
    try:
        return json.loads(text)
    except ValueError:
        return {"text": text}


def _vm_hwm_kb(pid):
    if pid is None:
        return None
    try:
        with open(f"/proc/{pid}/status", encoding="utf-8") as fh:
            for line in fh:
                if line.startswith("VmHWM:"):
                    return int(line.split()[1])
    except (OSError, ValueError, IndexError):
        return None
    return None


def telemetry_row(cache_dir, tool, addr, wall_s, ok, pid=None):
    """Append one call row to cache_dir/runs.tsv (header written when new). Returns the row."""
    cache_dir = Path(cache_dir)
    cache_dir.mkdir(parents=True, exist_ok=True)
    path = cache_dir / "runs.tsv"
    rss = _vm_hwm_kb(pid)
    ts = datetime.now().astimezone().isoformat(timespec="seconds")
    fields = [ts, str(tool), str(addr), f"{wall_s:.3f}", "1" if ok else "0", "" if rss is None else str(rss)]
    row = "\t".join(f.replace("\t", " ").replace("\n", " ") for f in fields)
    is_new = not path.exists() or path.stat().st_size == 0
    with path.open("a", encoding="utf-8") as fh:
        if is_new:
            fh.write(TELEMETRY_HEADER + "\n")
        fh.write(row + "\n")
    return row


def _addr_of(args):
    return str(args.get("address") or args.get("procedure") or "")


class ReaSession:
    """One REA MCP child with a flock-guarded lock; use as a context manager."""

    def __init__(self, exe=EXE, *, cmd=REA_CMD, env=None, call_timeout_s=120, open_timeout_s=900,
                 read_only=True, cache_dir=None, lock_dir=Path.home() / ".cache/kotor2_rea",
                 sha=EXE_SHA256, warm=WARM_CALL):
        self.exe = Path(exe)
        self.warm = warm
        self.cmd = list(cmd)
        self.env = _child_env(env)
        self.call_timeout_s = call_timeout_s
        self.open_timeout_s = open_timeout_s
        self.read_only = read_only
        self.cache_dir = Path(cache_dir) if cache_dir is not None else DUMP_ROOT / "rea_cache"
        self.lock_dir = Path(lock_dir)
        self.sha = sha
        self.tools = None
        self.opened = False
        self.open_wall_s = None
        self.warm_wall_s = None
        self._proc = None
        self._lines = None
        self._reader = None
        self._stderr_fh = None
        self._lock_fh = None
        self._next_id = 0

    def __enter__(self):
        try:
            self.start()
            self.open()
        except BaseException:
            self.close()
            raise
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()
        return False

    # ----- lifecycle -------------------------------------------------------

    def start(self):
        if self._proc is not None:
            raise ReaError("REA session already started")
        self._acquire_lock()
        try:
            self._spawn()
            self._handshake()
        except BaseException:
            self._stop_proc(wait_s=5)
            self._release_lock()
            raise

    def open(self):
        t0 = time.monotonic()
        self.call("open_binary", self._open_args(), timeout_s=self.open_timeout_s, use_cache=False)
        self.open_wall_s = time.monotonic() - t0
        self.opened = True
        if self.warm:
            # REA analyses lazily: the first procedure call after open pays the ~5 min import (D1 probe), so it
            # runs here under open_timeout_s instead of failing the caller's call_timeout_s.
            t0 = time.monotonic()
            self.call(self.warm[0], self.warm[1], timeout_s=self.open_timeout_s, use_cache=False)
            self.warm_wall_s = time.monotonic() - t0

    def close(self):
        if self._proc is None and self._lock_fh is None:
            return
        was_opened = self.opened
        self.opened = False
        try:
            if was_opened and self._proc is not None:
                try:
                    self._request("tools/call", {"name": "close_binary", "arguments": {}},
                                  self.call_timeout_s, label="close_binary")
                except (ReaError, OSError) as exc:
                    sys.stderr.write(f"rea_session: close_binary failed: {exc}\n")
        finally:
            try:
                self._stop_proc()
            finally:
                self._release_lock()

    # ----- public call -----------------------------------------------------

    def call(self, tool, args, *, timeout_s=None, use_cache=True):
        args = dict(args or {})
        if self.read_only and tool in WRITE_TOOLS:
            raise ReaError(f"write tool {tool} refused: session is read_only")
        if self.tools is not None and tool not in self.tools:
            raise ReaError(f"tool not offered by server: {tool}")
        cacheable = use_cache and tool not in WRITE_TOOLS
        if cacheable:
            cached = self._cache_read(tool, args)
            if cached is not None:
                return cached
        timeout = self.call_timeout_s if timeout_s is None else timeout_s
        t0 = time.monotonic()
        ok = False
        try:
            result = self._call_with_reconnect(tool, args, timeout)
            ok = True
        finally:
            self._record(tool, _addr_of(args), time.monotonic() - t0, ok)
        if cacheable:
            self._cache_write(tool, args, result)
        return result

    # ----- internals -------------------------------------------------------

    def _open_args(self):
        return {"path": str(self.exe), "provider_id": "ghidra"}

    def _acquire_lock(self):
        self.lock_dir.mkdir(parents=True, exist_ok=True)
        path = self.lock_dir / f"{self.sha[:12]}.lock"
        fh = path.open("a")
        try:
            fcntl.flock(fh.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            fh.close()
            raise ReaGuardError(f"another ReaSession holds {path}")
        self._lock_fh = fh

    def _release_lock(self):
        fh, self._lock_fh = self._lock_fh, None
        if fh is not None:
            fh.close()  # closing the fd releases the flock

    def _spawn(self):
        self.cache_dir.mkdir(parents=True, exist_ok=True)
        self._stderr_fh = (self.cache_dir / "rea_stderr.log").open("a", encoding="utf-8")
        try:
            proc = subprocess.Popen(
                self.cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self._stderr_fh,
                text=True, encoding="utf-8", errors="replace", bufsize=1, env=self.env)
        except OSError as exc:
            self._stderr_fh.close()
            self._stderr_fh = None
            raise ReaError(f"could not start REA command {self.cmd[0]!r}: {exc}") from exc
        self._proc = proc
        self._lines = queue.Queue()
        self._reader = threading.Thread(target=_pump, args=(proc.stdout, self._lines),
                                        name="rea-stdout", daemon=True)
        self._reader.start()

    def _stop_proc(self, wait_s=10):
        proc, self._proc = self._proc, None
        if proc is not None:
            try:
                if proc.stdin and not proc.stdin.closed:
                    proc.stdin.close()
            except OSError:
                pass
            try:
                proc.wait(timeout=wait_s)
            except subprocess.TimeoutExpired:
                proc.terminate()
                try:
                    proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait()
            if self._reader is not None:
                self._reader.join(timeout=5)
            if proc.stdout:
                proc.stdout.close()
        self._reader = None
        self._lines = None
        if self._stderr_fh is not None:
            self._stderr_fh.close()
            self._stderr_fh = None

    def _handshake(self):
        self._request("initialize", {
            "protocolVersion": PROTOCOL_VERSION,
            "capabilities": {},
            "clientInfo": {"name": "rea_session", "version": "1"},
        }, self.open_timeout_s, label="initialize")
        self._notify("notifications/initialized")
        listing = self._request("tools/list", {}, self.open_timeout_s, label="tools/list")
        self.tools = {t["name"] for t in (listing.get("tools") or [])
                      if isinstance(t, dict) and isinstance(t.get("name"), str)}

    def _reconnect(self):
        self._stop_proc()
        self._spawn()
        self._handshake()
        if self.opened:
            self._tools_call("open_binary", self._open_args(), self.open_timeout_s)
            if self.warm:
                self._tools_call(self.warm[0], self.warm[1], self.open_timeout_s)

    def _call_with_reconnect(self, tool, args, timeout_s):
        try:
            return self._tools_call(tool, args, timeout_s)
        except _ConnectionLost:
            pass
        try:
            self._reconnect()
        except ReaError as exc:
            raise ReaError(f"REA connection lost twice: {tool}") from exc
        try:
            return self._tools_call(tool, args, timeout_s)
        except _ConnectionLost as exc:
            raise ReaError(f"REA connection lost twice: {tool}") from exc

    def _tools_call(self, tool, args, timeout_s):
        response = self._request("tools/call", {"name": tool, "arguments": args}, timeout_s, label=tool)
        return _decode_tool_result(response, tool)

    def _send(self, message):
        proc = self._proc
        if proc is None:
            raise ReaError("REA session is not started")
        try:
            proc.stdin.write(json.dumps(message, separators=(",", ":")) + "\n")
            proc.stdin.flush()
        except (OSError, ValueError) as exc:
            raise _ConnectionLost(f"write to REA failed: {exc}") from exc

    def _notify(self, method):
        self._send({"jsonrpc": "2.0", "method": method})

    def _request(self, method, params, timeout_s, label=None):
        self._next_id += 1
        req_id = self._next_id
        self._send({"jsonrpc": "2.0", "id": req_id, "method": method, "params": params})
        response = self._await(req_id, time.monotonic() + timeout_s, label or method, timeout_s)
        if "error" in response:
            err = response["error"]
            message = err.get("message") if isinstance(err, dict) else None
            raise ReaError(message or json.dumps(err))
        return response.get("result") or {}

    def _await(self, req_id, deadline, label, timeout_s):
        lines = self._lines
        timeout_msg = f"timeout after {timeout_s:g}s: {label}"
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise ReaError(timeout_msg)
            try:
                item = lines.get(timeout=remaining)
            except queue.Empty:
                raise ReaError(timeout_msg)
            if item is _EOF:
                raise _ConnectionLost(f"REA stdout closed while waiting for {label}")
            text = item.strip()
            if not text:
                continue
            try:
                message = json.loads(text)
            except ValueError:
                sys.stderr.write(f"rea_session: skipped non-JSON line from REA: {text[:200]}\n")
                continue
            if isinstance(message, dict) and message.get("id") == req_id:
                return message

    def _record(self, tool, addr, wall_s, ok):
        pid = self._proc.pid if self._proc is not None else None
        try:
            telemetry_row(self.cache_dir, tool, addr, wall_s, ok, pid=pid)
        except OSError as exc:
            sys.stderr.write(f"rea_session: telemetry write failed: {exc}\n")

    def _cache_path(self, tool, args):
        blob = tool + "\n" + json.dumps(args, sort_keys=True, separators=(",", ":"))
        key = hashlib.sha1(blob.encode("utf-8")).hexdigest()
        return self.cache_dir / tool / f"{key}.json"

    def _cache_read(self, tool, args):
        try:
            with self._cache_path(tool, args).open(encoding="utf-8") as fh:
                return json.load(fh)
        except (OSError, ValueError):
            return None

    def _cache_write(self, tool, args, result):
        path = self._cache_path(tool, args)
        tmp = path.with_name(f"{path.name}.{os.getpid()}.tmp")
        try:
            path.parent.mkdir(parents=True, exist_ok=True)
            tmp.write_text(json.dumps(result), encoding="utf-8")
            os.replace(tmp, path)
        except OSError as exc:
            sys.stderr.write(f"rea_session: cache write failed: {exc}\n")


# ----- CLI -----------------------------------------------------------------

def _session_kwargs(args):
    kwargs = {}
    if args.cmd:
        kwargs["cmd"] = shlex.split(args.cmd)
    if args.cache_dir is not None:
        kwargs["cache_dir"] = args.cache_dir
    if args.lock_dir is not None:
        kwargs["lock_dir"] = args.lock_dir
    return kwargs


def _safety_checks(args):
    if args.no_preflight:  # tests only
        return
    guard()
    preflight()


def _cmd_guard(args):
    ps = _ps_lines()
    for line in live_rea_procs(ps):
        print(line)
    guard(max_live=args.max_live, ps_lines=ps)
    print("ok")
    return 0


def _cmd_probe(args):
    _safety_checks(args)
    addrs = args.addr or list(PROBE_ADDRS)
    walls = []
    with ReaSession(read_only=True, **_session_kwargs(args)) as session:
        print(f"open_wall_s={session.open_wall_s:.3f}")
        for raw in addrs:
            proc_addr = "0x%08x" % int(raw, 16)
            for tool in PROBE_TOOLS:
                t0 = time.monotonic()
                session.call(tool, {"procedure": proc_addr}, use_cache=False)
                wall = time.monotonic() - t0
                walls.append(wall)
                print(f"{tool} {proc_addr} wall_s={wall:.3f}")
    ordered = sorted(walls)
    nearest_rank = (95 * len(ordered) + 99) // 100  # ceil(0.95 * n), integer arithmetic
    p95 = ordered[nearest_rank - 1]
    print(f"median_s={statistics.median(ordered):.3f} p95_s={p95:.3f} calls={len(ordered)}")
    return 0


def _cmd_call(args):
    _safety_checks(args)
    try:
        payload = json.loads(args.args)
    except ValueError as exc:
        print(f"rea_session: --args is not JSON: {exc}", file=sys.stderr)
        return 2
    with ReaSession(read_only=not args.write, **_session_kwargs(args)) as session:
        result = session.call(args.tool, payload)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


def _build_parser():
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--cmd", help="REA command as a shell string (tests only; default: npx rea-agents@6.3.0 mcp)")
    common.add_argument("--cache-dir", type=Path, default=None, help="default: <DUMP_ROOT>/rea_cache")
    common.add_argument("--lock-dir", type=Path, default=None, help="tests only: default ~/.cache/kotor2_rea")
    common.add_argument("--no-preflight", action="store_true",
                        help="tests only: skip preflight and the process guard")
    parser = argparse.ArgumentParser(prog="rea_session.py", description="Persistent REA MCP client.")
    sub = parser.add_subparsers(dest="command", required=True)

    p_guard = sub.add_parser("guard", parents=[common], help="refuse when REA/Ghidra processes are live")
    p_guard.add_argument("--max-live", type=int, default=2)
    p_guard.set_defaults(func=_cmd_guard)

    p_probe = sub.add_parser("probe", parents=[common], help="time procedure calls on one session")
    p_probe.add_argument("--addr", action="append", default=None, help="procedure address, repeatable")
    p_probe.set_defaults(func=_cmd_probe)

    p_call = sub.add_parser("call", parents=[common], help="call one tool and print the JSON result")
    p_call.add_argument("tool")
    p_call.add_argument("--args", default="{}", help="JSON object of tool arguments")
    p_call.add_argument("--write", action="store_true", help="allow write tools")
    p_call.set_defaults(func=_cmd_call)
    return parser


def main(argv=None):
    args = _build_parser().parse_args(argv)
    try:
        return args.func(args)
    except ReaGuardError as exc:
        print(f"rea_session: guard: {exc}", file=sys.stderr)
        return 3
    except ReaError as exc:
        print(f"rea_session: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
