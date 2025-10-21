# Integration Guide - How Everything Works Together

## Complete System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    YOUR PYTHON PROGRAM                           │
│                                                                   │
│  Your script runs normally → User sees normal output             │
│                           ↓                                       │
│                    Can interact, provide input                    │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ (meanwhile, in background...)
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│                    CDEBUGGER.SO (C Extension)                    │
│                                                                   │
│  Intercepts every line execution                                 │
│  Records: execution order, file, line, code, variables          │
│  Writes to: trace.log                                            │
└─────────────────────────────────────────────────────────────────┘
                            │
                ┌───────────┴────────────┐
                │                        │
                ↓                        ↓
         ┌──────────┐            ┌──────────────┐
         │ Normal   │            │  Exception/  │
         │  Exit    │            │   Crash      │
         └──────────┘            └──────────────┘
                                        │
                                        ↓
                            ┌─────────────────────┐
                            │  AUTO-LAUNCH CLI    │
                            │  ./traceviewer      │
                            └─────────────────────┘
                                        │
                                        ↓
                            ┌─────────────────────┐
                            │  INTERACTIVE DEBUG  │
                            │  • Navigate history │
                            │  • Search variables │
                            │  • Time travel      │
                            └─────────────────────┘
```

## Build Process

```
Source Files               Build Commands              Output
─────────────────────────────────────────────────────────────────

debugger.c        →    python3 setup.py build    →    cdebugger.so
(trace engine)         (or make build)                 (Python extension)

traceviewer.c     →    gcc -o traceviewer ...    →    traceviewer
(CLI debugger)         (or make viewer)                (executable)

                       make all
                       (builds both!)
```

## Execution Flow - Normal Run

```
1. User runs:
   $ python3 runner.py test.py

2. runner.py:
   import cdebugger
   cdebugger.start_trace("trace.log")
   exec(test.py code)
   
3. test.py executes line by line:
   ├─ Line 1: import sys
   │  └─> cdebugger logs to trace.log
   ├─ Line 2: x = 5
   │  └─> cdebugger logs: "1|||test.py|||2|||x = 5|||x=5"
   ├─ Line 3: y = 10
   │  └─> cdebugger logs: "2|||test.py|||3|||y = 10|||x=5;y=10"
   └─ ... continues for every line

4. Program completes:
   cdebugger.stop_trace()
   
5. Output:
   "Trace file saved to: trace.log"
```

## Execution Flow - Crash Scenario

```
1. User runs:
   $ python3 runner.py test_crash.py

2. runner.py starts tracing:
   cdebugger.start_trace("trace.log")
   
3. test_crash.py executes:
   ├─ Line 10: a = 10          logged ✓
   ├─ Line 11: b = 5           logged ✓
   ├─ Line 12: result1 = a/b   logged ✓
   ├─ Line 18: c = 0           logged ✓
   └─ Line 19: result2 = a/c   💥 CRASH!

4. Exception caught by runner.py:
   except Exception as e:
       cdebugger.stop_trace()
       print(traceback)
       
5. Auto-compile CLI (if needed):
   gcc -o traceviewer traceviewer.c
   
6. Auto-launch CLI:
   subprocess.run(["./traceviewer", "trace.log"])
   
7. User sees interactive debugger:
   ╔════════════════════════════════════════════╗
   ║  Python Time-Traveling Debugger v1.0      ║
   ╚════════════════════════════════════════════╝
   
   ✓ Loaded 25 execution steps
   
   [Execution #0]
   File: test_crash.py Line: 10
   Code: a = 10
   Variables:
     • a=10
   
   [1/25] > 

8. User can now:
   - Type 'n' to step forward
   - Type 'back' to step backward
   - Type 'find c' to see where c was used
   - Type ':18' to jump to execution #18
   - etc.
```

## Component Interaction

```
┌──────────────┐
│  runner.py   │ ─────┐
└──────────────┘      │
                      │ imports & calls
                      ↓
              ┌────────────────┐
              │  cdebugger.so  │
              │  (C extension) │
              └────────────────┘
                      │
                      │ writes to
                      ↓
              ┌────────────────┐
              │   trace.log    │
              │   (text file)  │
              └────────────────┘
                      │
                      │ reads from
                      ↓
              ┌────────────────┐
              │  traceviewer   │
              │  (C program)   │
              └────────────────┘
                      │
                      │ displays to
                      ↓
              ┌────────────────┐
              │     User       │
              │  (interactive) │
              └────────────────┘
```

## Files and Their Roles

```
┌─────────────────┬──────────────┬─────────────────────────────┐
│ File            │ Language     │ Purpose                     │
├─────────────────┼──────────────┼─────────────────────────────┤
│ debugger.c      │ C            │ Trace generator (Python ext)│
│ traceviewer.c   │ C            │ Trace navigator (CLI)       │
│ runner.py       │ Python       │ Wrapper to run scripts      │
│ setup.py        │ Python       │ Build config for extension  │
│ Makefile        │ Make         │ Build automation            │
│ test.py         │ Python       │ Example working script      │
│ test_crash.py   │ Python       │ Example crash script        │
│ trace.log       │ Text         │ Generated trace data        │
└─────────────────┴──────────────┴─────────────────────────────┘
```

## Data Flow

```
Python Program Execution:
┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐
│  Line 1  │ → │  Line 2  │ → │  Line 3  │ → │  Line N  │
└──────────┘   └──────────┘   └──────────┘   └──────────┘
     ↓              ↓              ↓              ↓
     │              │              │              │
     └──────────────┴──────────────┴──────────────┘
                         │
                         ↓
                  cdebugger.so
                  (intercepts)
                         │
                         ↓
                    trace.log
                    (writes)
                         
Entry format in trace.log:
0|||test.py|||1|||import sys|||
1|||test.py|||5|||x = 5|||x=5
2|||test.py|||6|||y = 10|||x=5;y=10
3|||test.py|||7|||z = x + y|||x=5;y=10;z=15
...

Read by traceviewer:
┌──────────────────────────────────────┐
│ TraceEntry #0:                       │
│   exec_order = 0                     │
│   filename = "test.py"               │
│   line_number = 1                    │
│   code = "import sys"                │
│   variables = ""                     │
└──────────────────────────────────────┘

Displayed to user:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
[Execution #0]
File: test.py Line: 1
Code: import sys
Variables: (none)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

## Command Flow in CLI

```
User Types Command      →      CLI Processes      →      Action
─────────────────────────────────────────────────────────────────

"n"                     →      current_entry++    →      Show next entry
"back"                  →      current_entry--    →      Show prev entry
":42"                   →      Find exec #42      →      Jump & display
"find x"                →      Search variables   →      List matches
"summary"               →      Calculate stats    →      Show summary
"quit"                  →      cleanup()          →      Exit program
```

## Usage Examples

### Example 1: Quick Test
```bash
$ make all           # Build everything
$ make test-crash    # Run crash test
# CLI launches automatically
> n                  # Step forward
> back              # Step back
> quit              # Exit
```

### Example 2: Debug Your Script
```bash
$ python3 runner.py my_script.py
# If crash occurs, CLI auto-launches
# Otherwise:
$ ./traceviewer trace.log
> :0                # Jump to start
> n                 # Step through
> find my_var       # Search for variable
> summary           # See stats
```

### Example 3: Manual Analysis
```bash
$ python3 runner.py my_script.py my_trace.log
# Program runs...
$ ./traceviewer my_trace.log
> find error
> :500
> back
> back
> back
# Found the bug!
```

## Customization Points

### 1. Modify Trace Format (debugger.c)
```c
// Line ~127
fprintf(trace_file, "%ld|||%s|||%d|||%s",
        execution_counter++, filename, lineno, source_line);

// Add more fields:
fprintf(trace_file, "%ld|||%s|||%d|||%s|||%s|||%ld",
        execution_counter++, filename, lineno, 
        function_name, source_line, timestamp);
```

### 2. Add CLI Commands (traceviewer.c)
```c
// Line ~300, in main() loop
else if (strcmp(cmd, "mycommand") == 0) {
    // Your custom command here
    my_custom_function(&viewer);
}
```

### 3. Filter Traced Files (debugger.c)
```c
// Line ~100, in trace_callback()
if (strstr(filename, "skip_this") != NULL) {
    return 0;  // Don't trace this file
}
```

### 4. Auto-launch Behavior (runner.py)
```c
// Line ~40
# Change to always launch CLI, even on success:
cdebugger.stop_trace()
subprocess.run(["./traceviewer", trace_file])
```

## Summary

**What You Have:**
✅ Complete trace generation system (C)
✅ Interactive CLI debugger (C)
✅ Auto-launch on crash (Python)
✅ Full documentation
✅ Example scripts
✅ Build automation

**How to Use:**
1. `make all` - Build everything
2. `python3 runner.py script.py` - Run with tracing
3. If crash: CLI launches automatically
4. Navigate with: n, back, :N, find, etc.

**Next Steps:**
- Customize trace format for your needs
- Add new CLI commands
- Implement filtering
- Add breakpoint support
- Build more analysis tools

🎉 **You now have a complete time-traveling debugger!**
