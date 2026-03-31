# AGENTS.md - Python Time-Traveling Debugger

This file provides guidance for AI agents working on the Python Time-Traveling Debugger codebase.

## Repository Overview

This is a C-based Python debugger with trace generation and time-travel capabilities. The main code is in the `python-debugger/` subdirectory; `CLITest/` is a legacy utility.

**Core Components:**
- `idebug.py` - Interactive debugger CLI (Python)
- `debugger.c` - C extension for tracing and breakpoints
- `traceviewer.c` - Post-execution trace viewer (C with readline)

## Build Commands

**Setup:**
```bash
cd python-debugger
uv sync
```

**Build everything:**
```bash
uv run make          # Build traceviewer and verify
uv run make rebuild  # Clean and rebuild
```

**Individual components:**
```bash
uv run make traceviewer   # Build only traceviewer
uv run make clean         # Remove build artifacts
uv run make verify        # Verify build components
```

## Test Commands

**Run all tests:**
```bash
uv run make test
```

**Run single test file:**
```bash
uv run python idebug.py test_clean_simple.py
uv run python idebug.py test_watchpoint.py
uv run python idebug.py test_import_main.py
```

**Test traceviewer:**
```bash
./traceviewer trace.log
```

## Lint/Typecheck Commands

**Python linting:**
```bash
uv run ruff check .
uv run ruff check idebug.py              # Single file
uv run ruff check test_clean_simple.py   # Single test
```

**Python type checking:**
```bash
uv run basedpyright .
uv run basedpyright idebug.py            # Single file
```

## Code Style Guidelines

### Python (idebug.py, test_*.py)

**Imports:**
- Standard library imports first (sys, os, cmd, readline, atexit, subprocess, traceback)
- No wildcard imports
- Group imports: stdlib, then third-party, then local

**Formatting:**
- 4 spaces for indentation
- Max line length: follow existing patterns (~100 chars)
- Use f-strings for formatting
- No trailing whitespace

**Types:**
- Type hints encouraged but not mandatory
- Follow existing patterns in the codebase

**Naming:**
- Classes: `PascalCase` (e.g., `DebuggerCLI`, `Colors`)
- Functions/methods: `snake_case` (e.g., `do_break`, `run_with_breakpoints`)
- Constants: `UPPER_CASE` (e.g., `WATCHPOINT_READ`, `WATCHPOINT_BOTH`)
- Private methods: prefixed with `_` (e.g., `_add_watchpoint`)

**Error Handling:**
- Use try/except with specific exceptions
- Print user-friendly error messages
- Handle KeyboardInterrupt gracefully

### C (debugger.c, traceviewer.c)

**Style:**
- Use K&R style braces (opening brace on same line)
- 4 spaces for indentation (no tabs)
- Function declarations: `return_type\nfunction_name(args)`

**Memory Management:**
- Always check malloc/calloc return values
- Free allocated memory in cleanup functions
- Use `strdup()` for string duplication
- Set pointers to NULL after freeing

**Error Handling:**
- Check file operations (fopen, etc.)
- Use perror() for system errors
- Return error codes (0 = success, 1 = failure)

**Compatibility:**
- Support Python 3.9-3.12+ with compatibility macros
- Use `COMPAT_PyFrame_GetCode`, `COMPAT_PyFrame_GetLocals` patterns

## Project Structure

```
python-debugger/
├── idebug.py              # Main entry point (Python CLI)
├── debugger.c             # C extension (cdebugger module)
├── traceviewer.c          # Post-execution viewer (C binary)
├── Makefile               # Build automation
├── pyproject.toml         # Project config (uv/ruff/basedpyright)
├── uv.lock               # Dependency lock
├── test_*.py             # Test files
└── *.md                  # Documentation

CLITest/                   # Legacy utility (less maintained)
├── pyviewer.c
├── sample.py
└── README.md
```

## Key Dependencies

**Required for build:**
- gcc (C compiler)
- python3-dev (Python headers)
- readline-dev (libreadline-dev / readline-devel)

**Python packages (via uv, these are only required for type checking/linting during development):**
- basedpyright (type checking)
- ruff (linting)

## Testing Workflow

1. Build: `uv run make`
2. Test manually: `uv run python idebug.py test_clean_simple.py`
3. Set breakpoints: `> b test_clean_simple.py 8`
4. Run: `> run`
5. In traceviewer: use `n`, `back`, `c`, `rc` to navigate

## Notes for Agents

- Readline is **mandatory** for traceviewer - no fallback without it
- The debugger has two phases: pre-execution (Python CLI) and post-execution (C traceviewer)
- Always use `uv run` prefix for Python commands to ensure correct environment
- Test files should be simple, self-contained Python scripts
- When modifying C code, rebuild with `uv run make rebuild`
