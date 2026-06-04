# Time-Traveling Debugger

Time-Traveling Debugger is a C-backed Python debugger that records program
execution and opens an interactive trace viewer after the target program runs.
It is designed for inspecting how Python state changes over time without adding
`breakpoint()` calls or debugger-specific imports to the target script.

The active Python debugger implementation lives in `python-debugger/`. Test and
demo programs live in `TestingFiles/`.

## Features

- Pre-execution setup for breakpoints and watchpoints.
- Python tracing through a compiled `cdebugger` extension.
- Post-execution navigation through captured trace entries.
- Forward and reverse movement through execution history.
- Breakpoint and watchpoint navigation inside the trace viewer.
- Terminal UI with source, locals, status, log, and help panes.
- Source display across files that appear in the trace.
- Variable search and basic expression evaluation from captured values.
- Readline command history and tab completion in interactive modes.

## Repository Layout

```text
.
├── python-debugger/
│   ├── idebug.py        # Pre-execution debugger CLI
│   ├── debugger.c       # Python C extension for tracing
│   ├── traceviewer.c    # Post-execution trace viewer
│   ├── Makefile         # Local build and verification targets
│   └── pyproject.toml   # Python package/build metadata
├── TestingFiles/        # Test and demo Python scripts
└── CLITest/             # Legacy standalone file-viewer utility
```

## Requirements

- Python 3.10 or newer
- `uv`
- GCC or a compatible C compiler
- Python development headers
- readline development headers

On Debian or Ubuntu:

```bash
sudo apt-get install gcc python3-dev libreadline-dev
```

On Fedora:

```bash
sudo dnf install gcc python3-devel readline-devel
```

On macOS:

```bash
xcode-select --install
brew install readline
```

## Setup

Run setup commands from the Python debugger directory:

```bash
cd python-debugger
uv sync
uv run make
```

`uv sync` installs the Python project and builds the `cdebugger` extension from
`debugger.c`. The Makefile builds the standalone trace viewer and verifies that
the debugger components are available.

## Basic Usage

Start the debugger with a target Python file:

```bash
cd python-debugger
uv run python idebug.py ../TestingFiles/test_clean_simple.py
```

The pre-execution CLI opens before the program runs. Set breakpoints or
watchpoints, then start execution:

```text
> b ../TestingFiles/test_clean_simple.py 8
> w total
> list
> run
```

When the target program finishes, the trace viewer opens automatically for
post-execution inspection.

You can also choose the trace output path:

```bash
uv run python idebug.py ../TestingFiles/test_clean_simple.py my_trace.log
```

## Pre-Execution Commands

Use these commands before the target program starts:

```text
b <file> <line>      Set a breakpoint
break <file> <line>  Set a breakpoint
list                 List breakpoints
l                    List breakpoints
clear [num]          Clear one breakpoint, or all breakpoints
w <var>              Watch a variable for reads and writes
rw <var>             Watch a variable for reads
ww <var>             Watch a variable for writes
listw                List watchpoints
clearw [num]         Clear one watchpoint, or all watchpoints
show [file]          Show source with line numbers
run                  Start execution
r                    Start execution
help                 Show command help
quit                 Exit without running
q                    Exit without running
```

## Trace Viewer

The trace viewer displays captured execution after the target program exits. In
an interactive terminal it opens as a TUI with source, locals, status, lower
pane, command line, and help view. In non-interactive use it falls back to the
readline command interface.

Common commands:

```text
n, next              Step to the next trace entry
back, prev           Step to the previous trace entry
:<number>            Jump to an execution number
c, continue          Continue to the next breakpoint or watchpoint
rc                   Reverse-continue to the previous breakpoint or watchpoint
jump <line>          Jump to the first trace entry for a source line
b <file> <line>      Set a trace-viewer breakpoint
b, list              List breakpoints
w <var>              Watch variable reads and writes
rw <var>             Watch variable reads
ww <var>             Watch variable writes
listw                List watchpoints
clearw [num]         Clear one watchpoint, or all watchpoints
show [file]          Print source in the log
summary              Print trace summary
find <var>           Search captured variables
eval <expr>          Evaluate an expression from captured values when possible
help                 Open help
q, quit              Exit trace viewer
```

TUI-specific controls:

```text
Tab                  Complete commands and filenames
F2                   Cycle the lower pane
view trace           Show trace history in the lower pane
view breakpoints     Show breakpoints and watchpoints in the lower pane
view log             Show command output in the lower pane
Up / Down            Step backward or forward when the command line is empty
PageUp / PageDown    Scroll the help view
Esc                  Close help
```

You can open an existing trace directly:

```bash
cd python-debugger
build/traceviewer trace.log
```

## Testing and Examples

Example target files are in `TestingFiles/`:

```bash
cd python-debugger
uv run python idebug.py ../TestingFiles/test_clean_simple.py
uv run python idebug.py ../TestingFiles/test_watchpoint.py
uv run python idebug.py ../TestingFiles/bank_sim.py
```

The Makefile also includes a test target:

```bash
uv run make test
```

## Notes and Limitations

- The default trace file is `trace.log`.
- Captured variable values are based on `repr()` and are intentionally capped to
  keep trace files manageable.
- `eval` uses captured representations where possible. It is not a full
  reconstruction of the original live Python process.
- Tracing can generate large trace files for long-running programs or programs
  with many large variables.
