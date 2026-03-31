#!/usr/bin/env python3
"""
Python Interactive Time-Traveling Debugger
Set breakpoints and watchpoints interactively before execution
"""

import sys
import os
import cdebugger
import traceback
import subprocess
import cmd
import readline
import atexit


# ANSI color codes for consistent styling
class Colors:
    CYAN = "\033[1;36m"
    GREEN = "\033[1;32m"
    YELLOW = "\033[1;33m"
    RED = "\033[1;31m"
    MAGENTA = "\033[1;35m"
    RESET = "\033[0m"
    BOLD = "\033[1m"


# Watchpoint types (mirrors traceviewer.c)
WATCHPOINT_READ = "read"
WATCHPOINT_WRITE = "write"
WATCHPOINT_BOTH = "read/write"


class DebuggerCLI(cmd.Cmd):
    intro = f"""
\033[1;36m╔════════════════════════════════════════════════════════╗\033[0m
\033[1;36m║         Python Time-Traveling Debugger v1.0            ║\033[0m
\033[1;36m╚════════════════════════════════════════════════════════╝\033[0m

\033[1;33mPre-Execution Setup Mode\033[0m
Set breakpoints and watchpoints before running your program.

\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m
\033[1;32mBreakpoints:\033[0m
  \033[1mb <file> <line>\033[0m      - Set a breakpoint (long: \033[1;32mbreak\033[0m)
  \033[1mlist\033[0m                 - List all breakpoints (short: \033[1;32ml\033[0m)
  \033[1mclear [num]\033[0m          - Clear breakpoint(s)

\033[1;32mWatchpoints:\033[0m
  \033[1mw <var>\033[0m              - Watch variable (read/write)
  \033[1mrw <var>\033[0m             - Watch variable (read only)
  \033[1mww <var>\033[0m             - Watch variable (write only)
  \033[1mlistw\033[0m                - List all watchpoints
  \033[1mclearw [num]\033[0m         - Clear watchpoint(s)

\033[1;32mSource:\033[0m
  \033[1mshow [file]\033[0m          - Show file with line numbers

\033[1;32mExecution:\033[0m
  \033[1mrun\033[0m                  - Start execution (short: \033[1;32mr\033[0m)
  \033[1mhelp\033[0m                 - Show this help
  \033[1mquit\033[0m or \033[1mq\033[0m           - Exit without running
\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m

\033[1;33mExample:\033[0m
  > b test.py 25
  > w counter
  > ww result
  > list
  > run
"""

    def __init__(self, python_file, trace_file):
        super().__init__()
        if sys.stdout.isatty():
            # Mark ANSI sequences as non-printing for readline/cmd prompt handling
            self.prompt = f"\001{Colors.GREEN}\002>\001{Colors.RESET}\002 "
        else:
            self.prompt = "> "
        self.python_file = python_file
        self.trace_file = trace_file
        self.breakpoints = []  # List of (file, line) tuples
        self.watchpoints = []  # List of (variable, type) tuples
        self.should_run = False
        self.last_command = None

        self.history_file = os.path.expanduser("~/.idebug_history")
        try:
            readline.read_history_file(self.history_file)
            readline.set_history_length(1000)
        except FileNotFoundError:
            pass

        atexit.register(readline.write_history_file, self.history_file)

        readline.set_completer_delims(" \t\n")
        readline.parse_and_bind("tab: complete")
        readline.set_auto_history(False)

    def completedefault(self, text, line, begidx, endidx):
        """Default completion for filenames - only .py files"""
        words = line.split()
        if not words:
            return []
        command = words[0]
        if command not in ["break", "b", "show"]:
            return []
        if not text:
            text = ""
        try:
            files = os.listdir(".")
            py_files = [f for f in files if f.endswith(".py") and f.startswith(text)]
            py_files.sort()
            return py_files
        except:
            return []

    def completenames(self, text, *ignored):
        """Override to provide command completion"""
        commands = [
            "break",
            "b",
            "list",
            "l",
            "clear",
            "w",
            "rw",
            "ww",
            "listw",
            "clearw",
            "show",
            "run",
            "r",
            "help",
            "quit",
            "q",
        ]
        return [c for c in commands if c.startswith(text)]

    def emptyline(self):
        """Repeat last command when empty line is entered"""
        if self.last_command:
            return self.onecmd(self.last_command)
        return False

    def precmd(self, line):
        """Store last non-empty command and add to history"""
        if line.strip():
            self.last_command = line
            readline.add_history(line)
        return line

    # ── Breakpoints ───────────────────────────────────────────────────────────

    def do_break(self, arg):
        """Set a breakpoint: break <file> <line>"""
        try:
            parts = arg.split()
            if len(parts) != 2:
                print("\033[1;31mUsage:\033[0m break <file> <line>")
                print("\033[1;33mExample:\033[0m break test.py 25")
                return
            filename = parts[0]
            line = int(parts[1])
            if not os.path.exists(filename):
                print(f"\033[1;31m⚠ Warning:\033[0m File '{filename}' not found")
                response = input("Set breakpoint anyway? (y/n): ")
                if response.lower() != "y":
                    return
            abs_filename = os.path.abspath(filename)
            if (abs_filename, line) in self.breakpoints:
                print(
                    f"\033[1;33m⚠ Breakpoint already set at\033[0m {abs_filename}:{line}"
                )
                return
            self.breakpoints.append((abs_filename, line))
            print(f"\033[1;32m✓ Breakpoint set at\033[0m {abs_filename}:{line}")
        except ValueError:
            print("\033[1;31mError:\033[0m Line number must be an integer")
        except Exception as e:
            print(f"\033[1;31mError:\033[0m {e}")

    def do_b(self, arg):
        """Shorthand for break"""
        self.do_break(arg)

    def do_list(self, arg):
        """List all breakpoints"""
        if not self.breakpoints:
            print("\033[1;33mNo breakpoints set\033[0m")
            return
        print(
            "\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print("\033[1;33mBreakpoints:\033[0m")
        print(
            "\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        for i, (file, line) in enumerate(self.breakpoints, 1):
            print(f"  \033[1;32m{i}.\033[0m {file}:{line}")
        print(
            "\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"Total: \033[1m{len(self.breakpoints)}\033[0m breakpoint(s)\n")

    def do_l(self, arg):
        """Shorthand for list"""
        self.do_list(arg)

    def do_clear(self, arg):
        """Clear breakpoint(s): clear [num] or clear all"""
        if not self.breakpoints:
            print("\033[1;33mNo breakpoints to clear\033[0m")
            return
        if not arg or arg == "all":
            self.breakpoints.clear()
            print("\033[1;32m✓ All breakpoints cleared\033[0m")
            return
        try:
            num = int(arg)
            if 1 <= num <= len(self.breakpoints):
                bp = self.breakpoints.pop(num - 1)
                print(f"\033[1;32m✓ Cleared breakpoint at\033[0m {bp[0]}:{bp[1]}")
            else:
                print(f"\033[1;31mError:\033[0m Breakpoint {num} doesn't exist")
        except ValueError:
            print("\033[1;31mUsage:\033[0m clear [num] or clear all")

    # ── Watchpoints ───────────────────────────────────────────────────────────

    def _add_watchpoint(self, var_name, wp_type):
        """Internal helper to add a watchpoint."""
        if not var_name:
            return False
        for var, _ in self.watchpoints:
            if var == var_name:
                print(f"\033[1;33m✗ Watchpoint already set on '{var_name}'\033[0m")
                return False
        self.watchpoints.append((var_name, wp_type))
        print(f"\033[1;32m✓ Watchpoint set on '{var_name}' (type: {wp_type})\033[0m")
        return True

    def do_w(self, arg):
        """Set a read/write watchpoint: w <variable>"""
        var = arg.strip()
        if not var:
            print("\033[1;31mUsage:\033[0m w <variable>")
            print("\033[1;33mExample:\033[0m w counter")
            return
        self._add_watchpoint(var, WATCHPOINT_BOTH)

    def do_rw(self, arg):
        """Set a read watchpoint: rw <variable>"""
        var = arg.strip()
        if not var:
            print("\033[1;31mUsage:\033[0m rw <variable>")
            print("\033[1;33mExample:\033[0m rw counter")
            return
        self._add_watchpoint(var, WATCHPOINT_READ)

    def do_ww(self, arg):
        """Set a write watchpoint: ww <variable>"""
        var = arg.strip()
        if not var:
            print("\033[1;31mUsage:\033[0m ww <variable>")
            print("\033[1;33mExample:\033[0m ww counter")
            return
        self._add_watchpoint(var, WATCHPOINT_WRITE)

    def do_listw(self, arg):
        """List all watchpoints"""
        if not self.watchpoints:
            print("\033[1;33mNo watchpoints set\033[0m")
            return
        print(
            "\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print("\033[1;33mWatchpoints:\033[0m")
        print(
            "\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n"
        )
        for i, (var, wp_type) in enumerate(self.watchpoints, 1):
            print(f"  {i}. \033[1;32m{var}\033[0m ({wp_type})")
        print(f"\nTotal: \033[1m{len(self.watchpoints)}\033[0m watchpoint(s)\n")

    def do_clearw(self, arg):
        """Clear watchpoint(s): clearw [num]  (no arg = clear all)"""
        if not self.watchpoints:
            print("\033[1;33mNo watchpoints to clear\033[0m")
            return
        arg = arg.strip()
        if not arg:
            self.watchpoints.clear()
            print("\033[1;32m✓ All watchpoints cleared\033[0m")
            return
        try:
            num = int(arg)
            if 1 <= num <= len(self.watchpoints):
                var, _ = self.watchpoints.pop(num - 1)
                print(f"\033[1;32m✓ Cleared watchpoint on '{var}'\033[0m")
            else:
                print(
                    "\033[1;31m✗ Invalid watchpoint number. Use 'listw' to see watchpoints.\033[0m"
                )
        except ValueError:
            print("\033[1;31mUsage:\033[0m clearw [num]  (no arg = clear all)")

    # ── Source view ───────────────────────────────────────────────────────────

    def do_show(self, arg):
        """Show file with line numbers: show [file]"""
        if not arg:
            arg = self.python_file
        if not os.path.exists(arg):
            abs_arg = os.path.abspath(arg)
            if os.path.exists(abs_arg):
                arg = abs_arg
            else:
                print(f"\033[1;31mError:\033[0m File '{arg}' not found")
                return
        try:
            print(
                f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
            )
            print(f"\033[1;33mFile: {arg}\033[0m")
            print(
                f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n"
            )
            with open(arg, "r") as f:
                lines = f.readlines()
            arg_abs = os.path.abspath(arg)
            bp_lines = set()
            for file, line in self.breakpoints:
                if os.path.abspath(file) == arg_abs:
                    bp_lines.add(line)
            for i, line in enumerate(lines, 1):
                marker = "\033[1;31m⚫\033[0m" if i in bp_lines else "  "
                print(f"{marker} \033[1;32m{i:4d}\033[0m | {line.rstrip()}")
            print(
                f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
            )
            print(f"Total lines: \033[1m{len(lines)}\033[0m")
            if bp_lines:
                print(f"Breakpoints: \033[1m{len(bp_lines)}\033[0m in this file")
            print()
        except Exception as e:
            print(f"\033[1;31mError reading file:\033[0m {e}")

    # ── Execution ─────────────────────────────────────────────────────────────

    def do_run(self, arg):
        """Start execution with configured breakpoints and watchpoints"""
        print(
            f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"\033[1;33mStarting execution...\033[0m")
        print(
            f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        if self.breakpoints:
            print(f"Breakpoints: \033[1m{len(self.breakpoints)}\033[0m")
            for file, line in self.breakpoints:
                print(f"  \033[1;32m•\033[0m {file}:{line}")
        else:
            print(f"\033[1;33mNo breakpoints set (will trace only)\033[0m")
        if self.watchpoints:
            print(f"Watchpoints: \033[1m{len(self.watchpoints)}\033[0m")
            for var, wp_type in self.watchpoints:
                print(f"  \033[1;32m•\033[0m {var} ({wp_type})")
        print(
            f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n"
        )
        self.should_run = True
        return True

    def do_r(self, arg):
        """Shorthand for run"""
        return self.do_run(arg)

    def do_quit(self, arg):
        """Exit without running"""
        print("\033[1;33mExiting without execution\033[0m")
        self.should_run = False
        return True

    def do_q(self, arg):
        """Shorthand for quit"""
        return self.do_quit(arg)

    def do_help(self, arg):
        """Show help"""
        if arg:
            super().do_help(arg)
        else:
            print(self.intro)

    def do_EOF(self, arg):
        """Handle Ctrl+D"""
        print()
        return self.do_quit(arg)


def launch_trace_viewer(trace_file, breakpoints=None, watchpoints=None):
    """Launch the trace viewer CLI, pre-loading breakpoints and watchpoints."""
    traceviewer_path = "./build/traceviewer"

    needs_compile = False
    if not os.path.exists(traceviewer_path):
        needs_compile = True
    elif not os.access(traceviewer_path, os.X_OK):
        try:
            os.chmod(traceviewer_path, 0o755)
        except:
            needs_compile = True

    if needs_compile:
        print(f"\n\033[1;31mError: traceviewer binary not found. Run 'make' to compile traceviewer.\033[0m")

    # Build startup commands to pre-load the user's breakpoints and watchpoints
    # into traceviewer so they carry over from the pre-execution setup.
    startup_cmds = []
    if breakpoints:
        for filename, line in breakpoints:
            startup_cmds.append(f"b {os.path.basename(filename)} {line}\n")
    if watchpoints:
        type_to_cmd = {
            WATCHPOINT_BOTH: "w",
            WATCHPOINT_READ: "rw",
            WATCHPOINT_WRITE: "ww",
        }
        for var, wp_type in watchpoints:
            cmd_prefix = type_to_cmd.get(wp_type, "w")
            startup_cmds.append(f"{cmd_prefix} {var}\n")

    print("\n")

    if startup_cmds:
        # Feed startup commands then hand stdin back to the terminal so the
        # interactive session continues normally afterwards.
        import tempfile

        # Shell script: printf each startup command, then `cat` (pass-through)
        # so the user can keep typing once the initial setup is done.
        script_lines = ["#!/bin/sh\n", "{\n"]
        for sc in startup_cmds:
            # Use printf with %s to avoid interpreting backslashes
            escaped = sc.replace("'", "'\\''")
            script_lines.append(f"  printf '%s' '{escaped}'\n")
        script_lines.append("  cat\n")
        script_lines.append(f"}} | {os.path.abspath(traceviewer_path)} {trace_file}\n")

        with tempfile.NamedTemporaryFile(mode="w", suffix=".sh", delete=False) as tf:
            tf.writelines(script_lines)
            tmp_script = tf.name

        os.chmod(tmp_script, 0o755)
        try:
            subprocess.run(["/bin/sh", tmp_script])
        finally:
            os.unlink(tmp_script)
    else:
        subprocess.run([traceviewer_path, trace_file])


def run_with_breakpoints(python_file, trace_file, breakpoints, watchpoints=None):
    """Run a Python file with breakpoints set, then open the trace viewer."""
    print(f"Starting trace to: \033[1m{trace_file}\033[0m")
    cdebugger.start_trace(trace_file)

    for filename, line in breakpoints:
        try:
            cdebugger.set_breakpoint(filename, line)
            print(f"\033[1;32m✓ Breakpoint active at\033[0m {filename}:{line}")
        except Exception as e:
            print(
                f"\033[1;31m⚠ Warning:\033[0m Could not set breakpoint at {filename}:{line}: {e}"
            )

    try:
        with open(python_file, "r") as f:
            code = f.read()
        globals_dict = {
            "__name__": "__main__",
            "__file__": os.path.abspath(python_file),
        }
        exec(compile(code, python_file, "exec"), globals_dict)

    except KeyboardInterrupt:
        cdebugger.stop_trace()
        print(
            f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"\033[1;33mEXECUTION INTERRUPTED BY USER\033[0m")
        print(
            f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"\nTrace saved to: \033[1m{trace_file}\033[0m")
        return False

    except Exception as e:
        cdebugger.stop_trace()
        print(
            f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"\033[1;31mEXCEPTION OCCURRED:\033[0m")
        print(
            f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        traceback.print_exc()
        print(
            f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(f"\nTrace saved to: \033[1m{trace_file}\033[0m")
        print(f"\033[1;33mLaunching post-execution debugger...\033[0m")
        launch_trace_viewer(trace_file, breakpoints, watchpoints)
        return False

    cdebugger.stop_trace()
    print(
        f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
    )
    print(f"\033[1;32mExecution completed successfully.\033[0m")
    print(f"\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m")
    print(f"\nTrace saved to: \033[1m{trace_file}\033[0m")
    print(f"\033[1;33mLaunching post-execution debugger...\033[0m")
    launch_trace_viewer(trace_file, breakpoints, watchpoints)
    return True


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print(
            f"\033[1;36m╔════════════════════════════════════════════════════════╗\033[0m"
        )
        print(
            f"\033[1;36m║         Python Time-Traveling Debugger v1.0          ║\033[0m"
        )
        print(
            f"\033[1;36m╚════════════════════════════════════════════════════════╝\033[0m\n"
        )
        print(f"\033[1;33mUsage:\033[0m python3 idebug.py <python_file> [trace_file]")
        print(f"\n\033[1;32mArguments:\033[0m")
        print(f"  python_file    Path to the Python file to debug")
        print(f"  trace_file     Path to save trace (default: trace.log)")
        print(f"\n\033[1;33mExample:\033[0m")
        print(f"  python3 idebug.py test.py")
        print(f"  python3 idebug.py myapp.py my_trace.log")
        print(
            f"\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m"
        )
        print(
            "The interactive CLI will open to set breakpoints and watchpoints before execution."
        )
        sys.exit(1)

    python_file = sys.argv[1]
    trace_file = sys.argv[2] if len(sys.argv) > 2 else "trace.log"

    if not os.path.exists(python_file):
        print(f"\033[1;31mError:\033[0m File '{python_file}' not found")
        sys.exit(1)

    if not python_file.endswith(".py"):
        print(f"\033[1;33mWarning:\033[0m '{python_file}' doesn't have a .py extension")
        response = input("Continue anyway? (y/n): ")
        if response.lower() != "y":
            sys.exit(1)

    cli = DebuggerCLI(python_file, trace_file)
    cli.cmdloop()

    if cli.should_run:
        success = run_with_breakpoints(
            python_file, trace_file, cli.breakpoints, cli.watchpoints
        )
        sys.exit(0 if success else 1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()
