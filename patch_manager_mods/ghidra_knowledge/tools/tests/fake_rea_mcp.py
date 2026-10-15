#!/usr/bin/env python3
"""Fake REA MCP server for tests: newline-delimited JSON-RPC 2.0 on stdin/stdout.

Stdlib only. Diagnostics go to stderr only. Fixtures live in FAKE_REA_FIXTURES
(default: fixtures/rea next to this file): address tools read
<tool>__0x%08x.json, other tools read <tool>.json.

Environment:
  FAKE_REA_FIXTURES    fixture directory
  FAKE_REA_TOOLS_DROP  comma list of tool names left out of tools/list
  FAKE_REA_MODE        comma list of modes: crash_after:N, crash_once_after:N,
                       slow:S, no_init_reply, require_open, garbage_line, stale_reply
  FAKE_REA_STATE       state file (answered count, crash-once marker) kept across restarts
  FAKE_REA_CALL_LOG    file that gets one JSON line per tools/call
"""

import json
import os
import re
import sys
import time

DEFAULT_PROTOCOL = "2025-06-18"
TOOLS = [
    "open_binary",
    "close_binary",
    "procedure_pseudo_code",
    "procedure_info",
    "procedure_assembly",
    "procedure_callers",
    "procedure_callees",
    "procedure_references",
    "xrefs",
    "analyze_function",
    "search_procedures",
    "search_strings",
    "read_bytes",
    "read_function_instructions",
    "resolve_containing_procedure",
    "batch_decompile",
    "resolve_native_call_targets",
    "set_addresses_names",
    "set_address_name",
    "annotate_native_function",
]
ADDRESS_KEYS = ("address", "procedure", "addr")
OPEN_KEYS = ("path", "format", "provider_id", "snapshot_path")
CLOSE_KEYS = ("snapshot_path", "overwrite")
NO_APP_MESSAGE = "No app is open. Call open_binary with its local path first."
GARBAGE_LINE = "this is not json"
STALE_ID = 999999
HERE = os.path.dirname(os.path.abspath(__file__))


class ToolError(Exception):
    """A tool failure; the client receives it as an isError result."""


def log(message):
    sys.stderr.write("fake-rea: %s\n" % message)
    sys.stderr.flush()


def parse_modes(raw):
    modes = {}
    for part in raw.split(","):
        part = part.strip()
        if part:
            name, _, value = part.partition(":")
            modes[name.strip()] = value.strip()
    return modes


def normalise_address(raw):
    """Return the address as an int; raise ToolError if it is not a hex address."""
    if isinstance(raw, int) and not isinstance(raw, bool) and raw >= 0:
        return raw
    if isinstance(raw, str):
        try:
            value = int(raw.strip(), 16)
        except ValueError:
            value = -1
        if value >= 0:
            return value
    raise ToolError("bad address %r" % (raw,))


def address_key(value):
    return "0x%08x" % value


def address_from_arguments(arguments):
    for key in ADDRESS_KEYS:
        if key in arguments:
            return normalise_address(arguments[key])
    return None


def error_result(text):
    return {"content": [{"type": "text", "text": text}], "isError": True}


def unrecognized_keys(arguments, allowed):
    """Keys the real REA schema (a strict object) would reject, sorted."""
    return sorted(key for key in arguments if key not in allowed)


class FakeRea:
    def __init__(self, environ):
        self.modes = parse_modes(environ.get("FAKE_REA_MODE", ""))
        self.fixture_dir = environ.get("FAKE_REA_FIXTURES") or os.path.join(HERE, "fixtures", "rea")
        self.drop = {name.strip() for name in environ.get("FAKE_REA_TOOLS_DROP", "").split(",") if name.strip()}
        self.call_log = environ.get("FAKE_REA_CALL_LOG") or None
        self.state_path = environ.get("FAKE_REA_STATE") or None
        self.require_open = "require_open" in self.modes
        self.crash_after = self._number("crash_after", int)
        self.crash_once_after = self._number("crash_once_after", int)
        self.slow = self._number("slow", float)
        self.state = self._load_state()
        self.answered = int(self.state.get("answered", 0))
        self.crashed = bool(self.state.get("crashed", False))
        self.initialized = False
        self.opened = False
        self.names = {}
        # FAKE_REA_ANNOTATE_FAIL=0x..,0x..: annotate_native_function errors for these (per-item error tests).
        self.unknown_annotate = {int(x, 16) for x in environ.get("FAKE_REA_ANNOTATE_FAIL", "").split(",") if x.strip()}
        self.garbage_done = False
        self.stale_done = False

    def _number(self, name, kind):
        if name not in self.modes:
            return None
        try:
            return kind(self.modes[name])
        except ValueError:
            log("FAKE_REA_MODE %s needs a number, got %r" % (name, self.modes[name]))
            sys.exit(2)

    def _load_state(self):
        if not self.state_path or not os.path.isfile(self.state_path):
            return {}
        try:
            with open(self.state_path, encoding="utf-8") as handle:
                data = json.load(handle)
        except (OSError, ValueError) as exc:
            log("ignoring unreadable state file: %s" % exc)
            return {}
        return data if isinstance(data, dict) else {}

    def save_state(self):
        if not self.state_path:
            return
        with open(self.state_path, "w", encoding="utf-8") as handle:
            json.dump({"answered": self.answered, "crashed": self.crashed}, handle)

    # ----- wire helpers -------------------------------------------------

    def _write(self, message):
        if "garbage_line" in self.modes and not self.garbage_done:
            self.garbage_done = True
            sys.stdout.write(GARBAGE_LINE + "\n")
        sys.stdout.write(json.dumps(message) + "\n")
        sys.stdout.flush()

    def _reply(self, request_id, result):
        self._write({"jsonrpc": "2.0", "id": request_id, "result": result})

    def _error(self, request_id, code, message):
        self._write({"jsonrpc": "2.0", "id": request_id, "error": {"code": code, "message": message}})

    # ----- message handling ---------------------------------------------

    def handle(self, message):
        method = message.get("method")
        if not isinstance(method, str):
            log("ignoring message without a method")
            return
        if "id" not in message:
            if method == "notifications/initialized":
                log("notifications/initialized received")
            else:
                log("ignoring notification %s" % method)
            return

        request_id = message["id"]
        params = message.get("params")
        if not isinstance(params, dict):
            params = {}

        if method == "initialize":
            if "no_init_reply" in self.modes:
                log("no_init_reply: initialize read, not answered")
                return
            self.initialized = True
            self._reply(request_id, {
                "protocolVersion": params.get("protocolVersion") or DEFAULT_PROTOCOL,
                "capabilities": {"tools": {}},
                "serverInfo": {"name": "fake-rea", "version": "0"},
            })
            return

        if not self.initialized:
            self._error(request_id, -32002, "not initialized")
            return

        if method == "tools/list":
            listed = [
                {"name": name, "description": "", "inputSchema": {"type": "object"}}
                for name in TOOLS if name not in self.drop
            ]
            self._reply(request_id, {"tools": listed})
        elif method == "tools/call":
            self._tools_call(request_id, params)
        else:
            self._error(request_id, -32601, "method not found: %s" % method)

    def _tools_call(self, request_id, params):
        name = params.get("name")
        arguments = params.get("arguments", {})
        if arguments is None:
            arguments = {}
        if not isinstance(name, str) or not isinstance(arguments, dict):
            self._error(request_id, -32602, "invalid params: name must be a string, arguments an object")
            return

        self._log_call(name, arguments)
        self._maybe_crash()
        if self.slow:
            time.sleep(self.slow)

        try:
            payload = self._dispatch(name, arguments)
            result = {"content": [{"type": "text", "text": json.dumps(payload)}]}
        except ToolError as exc:
            result = error_result(str(exc))
        except Exception as exc:  # a bug in the fake must not kill the server mid-test
            result = error_result("fake-rea internal error: %s: %s" % (type(exc).__name__, exc))

        if "stale_reply" in self.modes and not self.stale_done:
            self.stale_done = True
            self._write({"jsonrpc": "2.0", "id": STALE_ID,
                         "result": {"content": [{"type": "text", "text": "{}"}]}})

        self._reply(request_id, result)
        self.answered += 1
        self.save_state()

    def _log_call(self, name, arguments):
        if not self.call_log:
            return
        with open(self.call_log, "a", encoding="utf-8") as handle:
            handle.write(json.dumps({"tool": name, "arguments": arguments}) + "\n")

    def _maybe_crash(self):
        if self.crash_after is not None and self.answered >= self.crash_after:
            log("crash_after:%d reached, exiting without reply" % self.crash_after)
            sys.exit(3)
        if (self.crash_once_after is not None and not self.crashed
                and self.answered >= self.crash_once_after):
            self.crashed = True
            self.save_state()
            log("crash_once_after:%d reached, exiting once without reply" % self.crash_once_after)
            sys.exit(3)

    # ----- tools ----------------------------------------------------------

    def _dispatch(self, name, arguments):
        if name not in TOOLS:
            raise ToolError("unknown tool %s" % name)
        if name == "open_binary":
            return self._open_binary(arguments)
        if name == "close_binary":
            return self._close_binary(arguments)
        if self.require_open and not self.opened:
            raise ToolError(NO_APP_MESSAGE)
        if name in ("set_addresses_names", "set_address_name") and "real_names" in self.modes:
            # REA 6.3.0's Ghidra provider declares these tools but rejects them (D1 probe 10-15).
            raise ToolError('{"error":{"code":"capability_unavailable","details":{"provider_id":"ghidra",'
                            '"operation":"%s","reason":"operation is not declared by this provider"}}}' % name)
        if name == "set_addresses_names":
            return self._set_names(arguments)
        if name == "annotate_native_function":
            return self._annotate(arguments)
        if name == "batch_decompile":
            return self._batch(arguments)

        value = address_from_arguments(arguments)
        if value is None:
            return self._fixture("%s.json" % name, "no fixture %s.json" % name)
        key = address_key(value)
        payload = self._fixture("%s__%s.json" % (name, key), "unknown address %s for %s" % (key, name))
        if name == "procedure_pseudo_code" and isinstance(payload, dict):
            if isinstance(payload.get("normalized_result"), str):      # real REA envelope (D1 probe)
                payload["normalized_result"] = self._apply_names(payload["normalized_result"])
            elif isinstance(payload.get("pseudocode"), str):
                payload["pseudocode"] = self._apply_names(payload["pseudocode"])
        return payload

    def _fixture(self, filename, missing_message):
        path = os.path.join(self.fixture_dir, filename)
        if not os.path.isfile(path):
            raise ToolError(missing_message)
        try:
            with open(path, encoding="utf-8") as handle:
                return json.load(handle)
        except ValueError as exc:
            raise ToolError("bad fixture %s: %s" % (filename, exc))

    def _open_binary(self, arguments):
        extra = unrecognized_keys(arguments, OPEN_KEYS)
        if extra:
            raise ToolError("unrecognized key(s) in object: %s" % ", ".join(extra))
        path = arguments.get("path")
        if not isinstance(path, str) or not path:
            raise ToolError("open_binary requires a non-empty path")
        payload = self._fixture("open_binary.json", "missing open_binary.json")
        if not isinstance(payload, dict):
            raise ToolError("open_binary.json is not a JSON object")
        self.opened = True
        payload["path"] = path
        return payload

    def _close_binary(self, arguments):
        extra = unrecognized_keys(arguments, CLOSE_KEYS)
        if extra:
            raise ToolError("unrecognized key(s) in object: %s" % ", ".join(extra))
        if "snapshot_path" in arguments:
            raise ToolError("snapshot_path not allowed in tests")
        self.opened = False
        return self._fixture("close_binary.json", "missing close_binary.json")

    def _annotate(self, arguments):
        if set(arguments) - {"procedure", "name", "comment", "inline_comment"}:
            raise ToolError("unrecognized key(s) in object: %s" % sorted(set(arguments) - {"procedure", "name", "comment", "inline_comment"}))
        value = normalise_address(arguments.get("procedure", ""))
        name = arguments.get("name")
        if not isinstance(name, str) or not name:
            raise ToolError("annotate_native_function requires name")
        if value in self.unknown_annotate:
            raise ToolError("unknown address %s for annotate_native_function" % address_key(value))
        self.names[value] = name
        return {"operation": "annotate_native_function", "parameters": dict(arguments),
                "normalized_result": {"annotations": {"address": "0x%x" % value, "name": name,
                                                      "comment": arguments.get("comment"), "inline_comment": None}}}

    def _set_names(self, arguments):
        names = arguments.get("names")
        if not isinstance(names, dict):
            raise ToolError("set_addresses_names requires names (object: address -> name)")
        results = {}
        for raw, name in names.items():
            try:
                value = normalise_address(raw)
            except ToolError:
                results[str(raw)] = False
                continue
            if not isinstance(name, str) or not name:
                results[address_key(value)] = False
                continue
            self.names[value] = name
            results[address_key(value)] = True
        return results

    def _batch(self, arguments):
        addresses = arguments.get("addresses")
        if not isinstance(addresses, list):
            raise ToolError("batch_decompile requires addresses (list)")
        items = []
        for raw in addresses:
            try:
                key = address_key(normalise_address(raw))
            except ToolError:
                items.append({"address": raw, "status": "error", "error": "bad address"})
                continue
            filename = "procedure_pseudo_code__%s.json" % key
            if not os.path.isfile(os.path.join(self.fixture_dir, filename)):
                items.append({"address": key, "status": "error", "error": "unknown address"})
                continue
            fixture = self._fixture(filename, "missing %s" % filename)
            code = None
            if isinstance(fixture, dict):
                code = fixture.get("normalized_result")
                if not isinstance(code, str):
                    code = fixture.get("pseudocode")
            if not isinstance(code, str):
                items.append({"address": key, "status": "error", "error": "fixture has no pseudocode"})
                continue
            items.append({"address": key, "status": "ok", "pseudocode": self._apply_names(code)})
        succeeded = sum(1 for item in items if item["status"] == "ok")
        # Real REA wraps every result in an evidence envelope (D1 probe 10-15).
        return {"operation": "batch_decompile", "parameters": {"addresses": list(addresses)},
                "normalized_result": {"items": items, "total": len(items), "succeeded": succeeded,
                                      "failed": len(items) - succeeded}}

    def _apply_names(self, text):
        for value, name in self.names.items():
            pattern = r"\bFUN_%08x\b" % value
            text = re.sub(pattern, lambda _match, new=name: new, text)
        return text


def main():
    server = FakeRea(os.environ)
    while True:
        line = sys.stdin.readline()
        if not line:
            return 0
        text = line.strip()
        if not text:
            continue
        try:
            message = json.loads(text)
        except ValueError:
            log("ignoring line that is not JSON: %r" % text[:80])
            continue
        if not isinstance(message, dict):
            log("ignoring JSON that is not an object")
            continue
        server.handle(message)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        sys.exit(0)
