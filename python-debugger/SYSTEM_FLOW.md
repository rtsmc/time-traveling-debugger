# Python Debugger - System Flow Diagram

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         USER INTERACTION                         │
└─────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
                    python runner.py test.py
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                           runner.py                              │
│  • Imports cdebugger module                                      │
│  • Calls cdebugger.start_trace("trace.log")                     │
│  • Executes target Python file                                   │
│  • Catches exceptions                                            │
│  • Calls cdebugger.stop_trace()                                 │
└─────────────────────────────────────────────────────────────────┘
                                 │
                    ┌────────────┴────────────┐
                    │                         │
                    ▼                         ▼
          ┌─────────────────┐      ┌─────────────────┐
          │   Normal Exit   │      │  Exception/     │
          │                 │      │  Crash          │
          └─────────────────┘      └─────────────────┘
                    │                         │
                    │                         ▼
                    │              ┌─────────────────┐
                    │              │  Launch CLI     │
                    │              │  (YOUR CODE)    │
                    │              └─────────────────┘
                    │                         │
                    └────────────┬────────────┘
                                 ▼
                         Show trace file location


                    BACKGROUND: During Execution
┌─────────────────────────────────────────────────────────────────┐
│                      cdebugger.so (C Extension)                  │
│                                                                   │
│  Python Interpreter  ──────────►  trace_callback()              │
│  (executing lines)                    │                          │
│                                       ▼                          │
│                            Extract execution context:            │
│                            • Filename                            │
│                            • Line number                         │
│                            • Source code                         │
│                            • Local variables                     │
│                                       │                          │
│                                       ▼                          │
│                            Write to trace.log:                   │
│                            ORDER|||FILE|||LINE|||CODE|||VARS     │
│                                       │                          │
│                                       ▼                          │
│                            Continue execution                    │
└─────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
                             trace.log
```

## Detailed Execution Flow

```
Step 1: User Runs Python Script
─────────────────────────────────
$ python runner.py test.py
         │
         └──► runner.py starts


Step 2: Initialize Tracing
───────────────────────────
runner.py:
  ├── import cdebugger
  ├── cdebugger.start_trace("trace.log")
  │    └──► Opens trace.log for writing
  │    └──► Registers trace_callback with Python
  │    └──► Sets execution_counter = 0
  └── exec(compile(code, 'test.py', 'exec'))


Step 3: Execute Python Code (test.py runs)
───────────────────────────────────────────
test.py line 1: def calculate_sum(a, b):
                     │
                     ▼
            Python Interpreter
                     │
                     └──► Calls trace_callback()
                              │
                              ▼
                     ┌────────────────────┐
                     │  trace_callback()  │
                     │  in debugger.c     │
                     └────────────────────┘
                              │
                              ├──► Get filename: "test.py"
                              ├──► Get line number: 1
                              ├──► Read source: "def calculate_sum(a, b):"
                              ├──► Get locals: {}
                              │
                              ▼
                     Write to trace.log:
                     "0|||test.py|||1|||def calculate_sum(a, b):|||"
                              │
                              ▼
                     Return to Python (continue execution)


test.py line 5: x = 5
                     │
                     ▼
            trace_callback()
                     │
                     ▼
            Write to trace.log:
            "1|||test.py|||5|||x = 5|||x=5"


test.py line 6: y = 10
                     │
                     ▼
            trace_callback()
                     │
                     ▼
            Write to trace.log:
            "2|||test.py|||6|||y = 10|||x=5;y=10"

... and so on for every line ...


Step 4a: Normal Completion
───────────────────────────
test.py finishes
         │
         ▼
runner.py:
  ├── cdebugger.stop_trace()
  │    └──► Close trace.log
  │    └──► Unregister trace_callback
  └── Print "Execution completed successfully."
         │
         └──► User sees: "Trace file saved to: trace.log"


Step 4b: Exception/Crash
─────────────────────────
test.py: bad = 1 / 0
              │
              ▼
    ZeroDivisionError raised
              │
              ▼
runner.py catches exception:
  ├── cdebugger.stop_trace()
  │    └──► Close trace.log
  ├── traceback.print_exc()
  │    └──► Print error to user
  └── Launch CLI (YOUR CODE HERE)
         │
         └──► from your_cli import DebugCLI
              cli = DebugCLI("trace.log")
              cli.run()
```

## Trace File Generation Flow

```
Every Python Line Executed
           │
           ▼
┌──────────────────────┐
│  trace_callback()    │
│  in debugger.c       │
└──────────────────────┘
           │
           ├──► PyFrameObject *frame (from Python)
           │
           ▼
┌──────────────────────────────────────────┐
│  1. Extract Context                      │
│     ├─ filename ← frame->f_code->co_filename
│     ├─ lineno ← PyFrame_GetLineNumber()  │
│     ├─ code ← get_source_line()          │
│     └─ locals ← PyFrame_GetLocals()      │
└──────────────────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  2. Format Variables                     │
│     ├─ Iterate PyDict (locals)           │
│     ├─ Get name: PyUnicode_AsUTF8(key)  │
│     ├─ Get value: PyObject_Repr(val)     │
│     └─ Format: "name=value;name2=value2" │
└──────────────────────────────────────────┘
           │
           ▼
┌──────────────────────────────────────────┐
│  3. Write to File                        │
│     fprintf(trace_file,                  │
│       "%ld|||%s|||%d|||%s|||%s\n",       │
│       exec_counter, file, line,          │
│       code, variables)                   │
└──────────────────────────────────────────┘
           │
           ▼
       trace.log
```

## CLI Integration Flow

```
Exception Occurs
       │
       ▼
┌────────────────┐
│  runner.py     │
│  exception     │
│  handler       │
└────────────────┘
       │
       ├─► cdebugger.stop_trace()
       ├─► Print traceback
       │
       ▼
┌────────────────────────────┐
│  YOUR CLI INTEGRATION      │
│                            │
│  from your_cli import CLI  │
│  cli = CLI("trace.log")    │
│  cli.run()                 │
└────────────────────────────┘
       │
       ▼
┌────────────────────────────┐
│  Your CLI                  │
│                            │
│  Uses parse_trace.py:      │
│  ├─ TraceParser            │
│  ├─ Read trace.log         │
│  ├─ Navigate entries       │
│  └─ Display to user        │
└────────────────────────────┘
       │
       ▼
   User interacts:
   ├─ "show line 42"
   ├─ "step back"
   ├─ "print variable x"
   └─ etc.
```

## Data Flow Diagram

```
┌──────────┐
│ test.py  │ ──────► Execute
└──────────┘
     │
     ▼
┌─────────────────────┐
│ Python Interpreter  │
└─────────────────────┘
     │
     │ (for each line)
     ▼
┌─────────────────────┐         ┌──────────────┐
│ trace_callback()    │ ───────►│  trace.log   │
│ in debugger.c       │  write  │              │
└─────────────────────┘         │ LINE 1       │
     ▲                           │ LINE 2       │
     │                           │ LINE 3       │
     │ return                    │ ...          │
     │                           └──────────────┘
     │                                  │
┌─────────────────────┐                │
│ Python continues    │                │ read
│ executing           │                │
└─────────────────────┘                ▼
                              ┌──────────────┐
                              │ Your CLI     │
                              │ parse_trace  │
                              └──────────────┘
                                     │
                                     ▼
                              ┌──────────────┐
                              │ User sees    │
                              │ execution    │
                              │ history      │
                              └──────────────┘
```

## Component Interaction

```
┌────────────────────────────────────────────────────────┐
│                     COMPONENTS                         │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐       │
│  │ runner.py│───►│ cdebugger│───►│trace.log │       │
│  │          │    │   .so    │    │          │       │
│  └──────────┘    └──────────┘    └──────────┘       │
│       │               │                  │            │
│       │               │                  │            │
│       │          intercepts          reads           │
│       │               │                  │            │
│       ▼               ▼                  ▼            │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐       │
│  │test.py   │    │ Python   │    │Your CLI  │       │
│  │(target)  │───►│Interpreter│    │          │       │
│  └──────────┘    └──────────┘    └──────────┘       │
│                                                        │
└────────────────────────────────────────────────────────┘
```

## Build Process Flow

```
debugger.c
    │
    │ (compile)
    ▼
┌─────────────┐
│  setup.py   │
└─────────────┘
    │
    │ python setup.py build_ext --inplace
    ▼
┌──────────────────────┐
│  C Compiler (gcc)    │
└──────────────────────┘
    │
    ├─► Include Python.h
    ├─► Compile debugger.c
    └─► Link with Python
    │
    ▼
cdebugger.so (Linux/Mac)
or
cdebugger.pyd (Windows)
    │
    │ (import)
    ▼
runner.py can now use:
  ├─ cdebugger.start_trace()
  ├─ cdebugger.stop_trace()
  └─ cdebugger.set_breakpoint()
```

## Summary: The Big Picture

```
┌─────────────────────────────────────────────────────────┐
│  1. User runs: python runner.py test.py                │
│                                                          │
│  2. runner.py starts tracing via cdebugger              │
│                                                          │
│  3. test.py executes normally                           │
│     • User sees normal output                           │
│     • Can provide input                                 │
│     • Program runs as usual                             │
│                                                          │
│  4. MEANWHILE: cdebugger captures every line            │
│     • Silently writes to trace.log                      │
│     • Logs: order, file, line, code, variables         │
│                                                          │
│  5a. IF program completes:                              │
│      → Show "trace saved to trace.log"                  │
│                                                          │
│  5b. IF program crashes:                                │
│      → Show traceback                                   │
│      → Show "trace saved to trace.log"                  │
│      → Launch YOUR CLI                                  │
│                                                          │
│  6. YOUR CLI reads trace.log                            │
│     • Uses parse_trace.py                               │
│     • Shows execution history                           │
│     • Lets user navigate/debug                          │
└─────────────────────────────────────────────────────────┘
```

This is the complete system! Start with build_and_test.sh to see it in action.
