Domain checks for Python script claims. Apply the ones the claim touches.
- Subprocess calls: is the return code checked, is output captured and decoded safely, is a shell used with unquoted interpolation, is there a timeout?
- Temp files and directories: created where, cleaned up on every path (including exceptions), possible reuse of a fixed name between runs?
- Paths: absolute or user-specific paths, writes outside a scratch dir, relative paths that depend on the working directory, symlink handling.
- SQL and patterns: string-built SQL, `LIKE` patterns without escaping `%` and `_`, regex built from unescaped input, off-by-one in slices and ranges.
- Error handling: bare `except`, `except Exception: pass`, a swallowed failure that changes the exit status, a return value that is never used.
- Argument and encoding handling: argparse defaults that differ from the documented behaviour, text read without an explicit encoding where the data is non-ASCII.
- Callers: find who calls the function with Grep before deciding a precondition can be violated.
