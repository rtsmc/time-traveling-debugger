#!/usr/bin/env python3
"""
Interactive Debugger CLI
Opens BEFORE execution to set breakpoints and configure debugging
"""

import sys
import os
import cdebugger
import traceback
import subprocess
import cmd

class DebuggerCLI(cmd.Cmd):
    intro = """
╔════════════════════════════════════════════════════════╗
║      Python Interactive Debugger - Setup Mode        ║
╚════════════════════════════════════════════════════════╝

Set breakpoints and configure debugging before execution.

Commands:
  break <file> <line>  - Set a breakpoint
  list                 - List all breakpoints
  clear [num]          - Clear breakpoint(s)
  show <file>          - Show file with line numbers
  run                  - Start execution
  help                 - Show this help
  quit                 - Exit without running

Example:
  > break test.py 25
  > break test.py 50
  > list
  > run
"""
    prompt = '(debugger) > '
    
    def __init__(self, python_file, trace_file):
        super().__init__()
        self.python_file = python_file
        self.trace_file = trace_file
        self.breakpoints = []  # List of (file, line) tuples
        self.should_run = False
        
    def do_break(self, arg):
        """Set a breakpoint: break <file> <line>"""
        try:
            parts = arg.split()
            if len(parts) != 2:
                print("Usage: break <file> <line>")
                print("Example: break test.py 25")
                return
            
            filename = parts[0]
            line = int(parts[1])
            
            # Check if file exists
            if not os.path.exists(filename):
                print(f"⚠ Warning: File '{filename}' not found")
                response = input("Set breakpoint anyway? (y/n): ")
                if response.lower() != 'y':
                    return
            
            self.breakpoints.append((filename, line))
            print(f"✓ Breakpoint set at {filename}:{line}")
            
        except ValueError:
            print("Error: Line number must be an integer")
        except Exception as e:
            print(f"Error: {e}")
    
    def do_b(self, arg):
        """Shorthand for break"""
        self.do_break(arg)
    
    def do_list(self, arg):
        """List all breakpoints"""
        if not self.breakpoints:
            print("No breakpoints set")
            return
        
        print("\nBreakpoints:")
        print("─" * 50)
        for i, (file, line) in enumerate(self.breakpoints, 1):
            print(f"  {i}. {file}:{line}")
        print("─" * 50)
        print(f"Total: {len(self.breakpoints)} breakpoint(s)\n")
    
    def do_l(self, arg):
        """Shorthand for list"""
        self.do_list(arg)
    
    def do_clear(self, arg):
        """Clear breakpoint(s): clear [num] or clear all"""
        if not self.breakpoints:
            print("No breakpoints to clear")
            return
        
        if not arg or arg == "all":
            self.breakpoints.clear()
            print("✓ All breakpoints cleared")
            return
        
        try:
            num = int(arg)
            if 1 <= num <= len(self.breakpoints):
                bp = self.breakpoints.pop(num - 1)
                print(f"✓ Cleared breakpoint at {bp[0]}:{bp[1]}")
            else:
                print(f"Error: Breakpoint {num} doesn't exist")
        except ValueError:
            print("Usage: clear [num] or clear all")
    
    def do_show(self, arg):
        """Show file with line numbers: show <file>"""
        if not arg:
            arg = self.python_file
        
        if not os.path.exists(arg):
            print(f"Error: File '{arg}' not found")
            return
        
        try:
            print(f"\n{'='*60}")
            print(f"File: {arg}")
            print('='*60)
            
            with open(arg, 'r') as f:
                lines = f.readlines()
            
            # Show breakpoints
            bp_lines = set(line for file, line in self.breakpoints if file == arg)
            
            for i, line in enumerate(lines, 1):
                marker = "⚫" if i in bp_lines else "  "
                print(f"{marker} {i:4d} | {line.rstrip()}")
            
            print('='*60)
            print(f"Total lines: {len(lines)}\n")
            
        except Exception as e:
            print(f"Error reading file: {e}")
    
    def do_run(self, arg):
        """Start execution with configured breakpoints"""
        print("\n" + "="*60)
        print("Starting execution...")
        print("="*60)
        
        if self.breakpoints:
            print(f"Breakpoints: {len(self.breakpoints)}")
            for file, line in self.breakpoints:
                print(f"  • {file}:{line}")
        else:
            print("No breakpoints set (will trace only)")
        
        print("="*60 + "\n")
        
        self.should_run = True
        return True  # Exit the CLI loop
    
    def do_r(self, arg):
        """Shorthand for run"""
        return self.do_run(arg)
    
    def do_quit(self, arg):
        """Exit without running"""
        print("Exiting without execution")
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

def launch_trace_viewer(trace_file):
    """Launch the trace viewer CLI"""
    if not os.path.exists("./traceviewer"):
        print("\nCompiling trace viewer...")
        result = subprocess.run(
            ["gcc", "-o", "traceviewer", "traceviewer.c", "-Wall"],
            capture_output=True,
            text=True
        )
        if result.returncode != 0:
            print("Failed to compile traceviewer.")
            print(f"Error: {result.stderr}")
            print("\nYou can manually compile with: gcc -o traceviewer traceviewer.c -Wall")
            print(f"Then run: ./traceviewer {trace_file}")
            return
    
    print("\n")
    subprocess.run(["./traceviewer", trace_file])

def run_with_breakpoints(python_file, trace_file, breakpoints):
    """
    Run a Python file with breakpoints set
    
    Args:
        python_file: Path to the Python file to execute
        trace_file: Path to the trace file to generate
        breakpoints: List of (filename, line) tuples
    """
    
    # Set all breakpoints
    for filename, line in breakpoints:
        try:
            cdebugger.set_breakpoint(filename, line)
            print(f"✓ Breakpoint set at {filename}:{line}")
        except Exception as e:
            print(f"⚠ Warning: Could not set breakpoint at {filename}:{line}: {e}")
    
    # Start tracing
    print(f"\nStarting trace to: {trace_file}")
    cdebugger.start_trace(trace_file)
    
    try:
        # Read and execute the target file
        with open(python_file, 'r') as f:
            code = f.read()
        
        # Set up globals for execution
        globals_dict = {
            '__name__': '__main__',
            '__file__': os.path.abspath(python_file),
        }
        
        # Execute the code
        exec(compile(code, python_file, 'exec'), globals_dict)
        
    except KeyboardInterrupt:
        # User interrupted execution
        cdebugger.stop_trace()
        print(f"\n{'='*60}")
        print(f"EXECUTION INTERRUPTED BY USER")
        print(f"{'='*60}")
        print(f"\nTrace file saved to: {trace_file}")
        return False
        
    except Exception as e:
        # Stop tracing on exception
        cdebugger.stop_trace()
        
        # Print the exception
        print(f"\n{'='*60}")
        print(f"EXCEPTION OCCURRED:")
        print(f"{'='*60}")
        traceback.print_exc()
        print(f"{'='*60}")
        print(f"\nTrace file saved to: {trace_file}")
        print(f"Launching debugger CLI...")
        
        launch_trace_viewer(trace_file)
        
        return False
    
    # Normal completion
    cdebugger.stop_trace()
    print(f"\n{'='*60}")
    print(f"Execution completed successfully.")
    print(f"{'='*60}")
    print(f"\nTrace file saved to: {trace_file}")
    print(f"Launching debugger CLI...")
    
    launch_trace_viewer(trace_file)
    
    return True

def main():
    """Main entry point"""
    
    if len(sys.argv) < 2:
        print("Python Interactive Debugger")
        print("=" * 60)
        print("\nUsage: python idebug.py <python_file> [trace_file]")
        print("\nArguments:")
        print("  python_file    Path to the Python file to execute")
        print("  trace_file     Path to the trace file (default: trace.log)")
        print("\nExample:")
        print("  python idebug.py test.py")
        print("  python idebug.py myapp.py my_trace.log")
        print("\nThe interactive CLI will open to set breakpoints before execution.")
        sys.exit(1)
    
    python_file = sys.argv[1]
    trace_file = sys.argv[2] if len(sys.argv) > 2 else "trace.log"
    
    # Check if file exists
    if not os.path.exists(python_file):
        print(f"Error: File '{python_file}' not found")
        sys.exit(1)
    
    # Check if it's a Python file
    if not python_file.endswith('.py'):
        print(f"Warning: '{python_file}' doesn't have a .py extension")
        response = input("Continue anyway? (y/n): ")
        if response.lower() != 'y':
            sys.exit(1)
    
    # Launch interactive CLI
    cli = DebuggerCLI(python_file, trace_file)
    cli.cmdloop()
    
    # If user chose to run
    if cli.should_run:
        success = run_with_breakpoints(python_file, trace_file, cli.breakpoints)
        sys.exit(0 if success else 1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
