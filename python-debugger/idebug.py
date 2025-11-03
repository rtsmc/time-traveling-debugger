#!/usr/bin/env python3
"""
Python Interactive Time-Traveling Debugger
Set breakpoints interactively before execution
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
    CYAN = '\033[1;36m'
    GREEN = '\033[1;32m'
    YELLOW = '\033[1;33m'
    RED = '\033[1;31m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

class DebuggerCLI(cmd.Cmd):
    intro = f"""
{Colors.CYAN}╔════════════════════════════════════════════════════════╗{Colors.RESET}
{Colors.CYAN}║         Python Time-Traveling Debugger v1.0          ║{Colors.RESET}
{Colors.CYAN}╚════════════════════════════════════════════════════════╝{Colors.RESET}

{Colors.YELLOW}Pre-Execution Setup Mode{Colors.RESET}
Set breakpoints before running your program.

{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}
{Colors.GREEN}Commands:{Colors.RESET}
  {Colors.BOLD}break <file> <line>{Colors.RESET}  - Set a breakpoint (short: {Colors.GREEN}b{Colors.RESET})
  {Colors.BOLD}list{Colors.RESET}                 - List all breakpoints (short: {Colors.GREEN}l{Colors.RESET})
  {Colors.BOLD}clear [num]{Colors.RESET}          - Clear breakpoint(s)
  {Colors.BOLD}show <file>{Colors.RESET}          - Show file with line numbers
  {Colors.BOLD}run{Colors.RESET}                  - Start execution (short: {Colors.GREEN}r{Colors.RESET})
  {Colors.BOLD}help{Colors.RESET}                 - Show this help
  {Colors.BOLD}quit{Colors.RESET} or {Colors.BOLD}q{Colors.RESET}           - Exit without running
{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}

{Colors.YELLOW}Example:{Colors.RESET}
  > break test.py 25
  > break test.py 50
  > list
  > run
"""
    prompt = f'{Colors.GREEN}>{Colors.RESET} '
    
    def __init__(self, python_file, trace_file):
        super().__init__()
        self.python_file = python_file
        self.trace_file = trace_file
        self.breakpoints = []  # List of (file, line) tuples
        self.should_run = False
        self.last_command = None
        
        # Set up command history
        self.history_file = os.path.expanduser('~/.idebug_history')
        try:
            readline.read_history_file(self.history_file)
            readline.set_history_length(1000)
        except FileNotFoundError:
            pass
        
        # Save history on exit
        atexit.register(readline.write_history_file, self.history_file)
        
        # Enable tab completion
        readline.set_completer_delims(' \t\n')
        readline.parse_and_bind('tab: complete')
        
        # Disable automatic history - we'll add manually
        readline.set_auto_history(False)
    
    def completedefault(self, text, line, begidx, endidx):
        """Default completion for filenames - only .py files"""
        # Only complete for commands that take file arguments
        words = line.split()
        if len(words) == 0:
            return []
        
        cmd = words[0]
        # Only provide filename completion for 'break', 'b', 'show' commands
        if cmd not in ['break', 'b', 'show']:
            return []
        
        if not text:
            text = ""
        
        # Get list of .py files in current directory
        try:
            files = os.listdir('.')
            py_files = [f for f in files if f.endswith('.py') and f.startswith(text)]
            
            # Sort .py files alphabetically for consistent ordering
            py_files.sort()
            
            return py_files
        except:
            return []
    
    def completenames(self, text, *ignored):
        """Override to provide command completion"""
        commands = ['break', 'b', 'list', 'l', 'clear', 'show', 'run', 'r', 'help', 'quit', 'q']
        return [cmd for cmd in commands if cmd.startswith(text)]
    
    def emptyline(self):
        """Repeat last command when empty line is entered"""
        if self.last_command:
            return self.onecmd(self.last_command)
        return False
    
    def precmd(self, line):
        """Store last non-empty command and add to history"""
        if line.strip():
            self.last_command = line
            # Manually add to history (only executed commands, not tab attempts)
            readline.add_history(line)
        return line
        
    def do_break(self, arg):
        """Set a breakpoint: break <file> <line>"""
        try:
            parts = arg.split()
            if len(parts) != 2:
                print(f"{Colors.RED}Usage:{Colors.RESET} break <file> <line>")
                print(f"{Colors.YELLOW}Example:{Colors.RESET} break test.py 25")
                return
            
            filename = parts[0]
            line = int(parts[1])
            
            # Check if file exists
            if not os.path.exists(filename):
                print(f"{Colors.RED}⚠ Warning:{Colors.RESET} File '{filename}' not found")
                response = input("Set breakpoint anyway? (y/n): ")
                if response.lower() != 'y':
                    return
            
            # Normalize to absolute path for consistent matching
            abs_filename = os.path.abspath(filename)
            
            self.breakpoints.append((abs_filename, line))
            print(f"{Colors.GREEN}✓ Breakpoint set at{Colors.RESET} {abs_filename}:{line}")
            
        except ValueError:
            print(f"{Colors.RED}Error:{Colors.RESET} Line number must be an integer")
        except Exception as e:
            print(f"{Colors.RED}Error:{Colors.RESET} {e}")
    
    def do_b(self, arg):
        """Shorthand for break"""
        self.do_break(arg)
    
    def do_list(self, arg):
        """List all breakpoints"""
        if not self.breakpoints:
            print(f"{Colors.YELLOW}No breakpoints set{Colors.RESET}")
            return
        
        print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"{Colors.YELLOW}Breakpoints:{Colors.RESET}")
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        for i, (file, line) in enumerate(self.breakpoints, 1):
            print(f"  {Colors.GREEN}{i}.{Colors.RESET} {file}:{line}")
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"Total: {Colors.BOLD}{len(self.breakpoints)}{Colors.RESET} breakpoint(s)\n")
    
    def do_l(self, arg):
        """Shorthand for list"""
        self.do_list(arg)
    
    def do_clear(self, arg):
        """Clear breakpoint(s): clear [num] or clear all"""
        if not self.breakpoints:
            print(f"{Colors.YELLOW}No breakpoints to clear{Colors.RESET}")
            return
        
        if not arg or arg == "all":
            self.breakpoints.clear()
            print(f"{Colors.GREEN}✓ All breakpoints cleared{Colors.RESET}")
            return
        
        try:
            num = int(arg)
            if 1 <= num <= len(self.breakpoints):
                bp = self.breakpoints.pop(num - 1)
                print(f"{Colors.GREEN}✓ Cleared breakpoint at{Colors.RESET} {bp[0]}:{bp[1]}")
            else:
                print(f"{Colors.RED}Error:{Colors.RESET} Breakpoint {num} doesn't exist")
        except ValueError:
            print(f"{Colors.RED}Usage:{Colors.RESET} clear [num] or clear all")
    
    def do_show(self, arg):
        """Show file with line numbers: show <file>"""
        if not arg:
            arg = self.python_file
        
        # Try to resolve the file path
        if not os.path.exists(arg):
            # Try as absolute path from breakpoints
            abs_arg = os.path.abspath(arg)
            if os.path.exists(abs_arg):
                arg = abs_arg
            else:
                print(f"{Colors.RED}Error:{Colors.RESET} File '{arg}' not found")
                return
        
        try:
            print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
            print(f"{Colors.YELLOW}File: {arg}{Colors.RESET}")
            print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}\n")
            
            with open(arg, 'r') as f:
                lines = f.readlines()
            
            # Normalize arg path for comparison
            arg_abs = os.path.abspath(arg)
            
            # Show breakpoints - check both relative and absolute paths
            bp_lines = set()
            for file, line in self.breakpoints:
                # Compare absolute paths
                if os.path.abspath(file) == arg_abs:
                    bp_lines.add(line)
            
            for i, line in enumerate(lines, 1):
                marker = f"{Colors.RED}⚫{Colors.RESET}" if i in bp_lines else "  "
                print(f"{marker} {Colors.GREEN}{i:4d}{Colors.RESET} | {line.rstrip()}")
            
            print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
            print(f"Total lines: {Colors.BOLD}{len(lines)}{Colors.RESET}")
            if bp_lines:
                print(f"Breakpoints: {Colors.BOLD}{len(bp_lines)}{Colors.RESET} in this file")
            print()
            
        except Exception as e:
            print(f"{Colors.RED}Error reading file:{Colors.RESET} {e}")
    
    def do_run(self, arg):
        """Start execution with configured breakpoints"""
        print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"{Colors.YELLOW}Starting execution...{Colors.RESET}")
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        
        if self.breakpoints:
            print(f"Breakpoints: {Colors.BOLD}{len(self.breakpoints)}{Colors.RESET}")
            for file, line in self.breakpoints:
                print(f"  {Colors.GREEN}•{Colors.RESET} {file}:{line}")
        else:
            print(f"{Colors.YELLOW}No breakpoints set (will trace only){Colors.RESET}")
        
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}\n")
        
        self.should_run = True
        return True  # Exit the CLI loop
    
    def do_r(self, arg):
        """Shorthand for run"""
        return self.do_run(arg)
    
    def do_quit(self, arg):
        """Exit without running"""
        print(f"{Colors.YELLOW}Exiting without execution{Colors.RESET}")
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
    traceviewer_path = "./traceviewer"
    
    # Check if traceviewer exists and is executable
    needs_compile = False
    if not os.path.exists(traceviewer_path):
        needs_compile = True
    elif not os.access(traceviewer_path, os.X_OK):
        # Exists but not executable - make it executable
        try:
            os.chmod(traceviewer_path, 0o755)
        except:
            needs_compile = True
    
    if needs_compile:
        print(f"\n{Colors.YELLOW}Compiling trace viewer...{Colors.RESET}")
        
        # Compile with readline (mandatory)
        result = subprocess.run(
            ["gcc", "-o", "traceviewer", "traceviewer.c", "-Wall", "-O2", "-lreadline"],
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0:
            print(f"{Colors.RED}Failed to compile traceviewer.{Colors.RESET}")
            print(f"Error: {result.stderr}")
            
            # Check if the error is related to readline
            if "readline" in result.stderr.lower():
                print(f"\n{Colors.RED}✗ Readline library is required but not found!{Colors.RESET}")
                print(f"\n{Colors.YELLOW}Please install readline development library:{Colors.RESET}")
                print(f"  {Colors.CYAN}Fedora/RHEL:{Colors.RESET}  sudo dnf install readline-devel")
                print(f"  {Colors.CYAN}Debian/Ubuntu:{Colors.RESET} sudo apt-get install libreadline-dev")
                print(f"  {Colors.CYAN}macOS:{Colors.RESET}         brew install readline")
            else:
                print(f"\nYou can manually compile with:")
                print(f"  {Colors.BOLD}gcc -o traceviewer traceviewer.c -Wall -O2 -lreadline{Colors.RESET}")
                print(f"  or: {Colors.BOLD}make{Colors.RESET}")
            
            print(f"Then run: {Colors.BOLD}./traceviewer {trace_file}{Colors.RESET}")
            return
        
        # Make executable
        try:
            os.chmod(traceviewer_path, 0o755)
        except Exception as e:
            print(f"{Colors.YELLOW}Warning: Could not make traceviewer executable: {e}{Colors.RESET}")
    
    print("\n")
    subprocess.run([traceviewer_path, trace_file])

def run_with_breakpoints(python_file, trace_file, breakpoints):
    """
    Run a Python file with breakpoints set
    
    Args:
        python_file: Path to the Python file to execute
        trace_file: Path to the trace file to generate
        breakpoints: List of (filename, line) tuples
    """
    
    # Start tracing FIRST (before setting breakpoints)
    print(f"Starting trace to: {Colors.BOLD}{trace_file}{Colors.RESET}")
    cdebugger.start_trace(trace_file)
    
    # Set all breakpoints AFTER tracing starts
    for filename, line in breakpoints:
        try:
            cdebugger.set_breakpoint(filename, line)
            print(f"{Colors.GREEN}✓ Breakpoint active at{Colors.RESET} {filename}:{line}")
        except Exception as e:
            print(f"{Colors.RED}⚠ Warning:{Colors.RESET} Could not set breakpoint at {filename}:{line}: {e}")
    
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
        print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"{Colors.YELLOW}EXECUTION INTERRUPTED BY USER{Colors.RESET}")
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"\nTrace saved to: {Colors.BOLD}{trace_file}{Colors.RESET}")
        return False
        
    except Exception as e:
        # Stop tracing on exception
        cdebugger.stop_trace()
        
        # Print the exception
        print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"{Colors.RED}EXCEPTION OCCURRED:{Colors.RESET}")
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        traceback.print_exc()
        print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print(f"\nTrace saved to: {Colors.BOLD}{trace_file}{Colors.RESET}")
        print(f"{Colors.YELLOW}Launching post-execution debugger...{Colors.RESET}")
        
        launch_trace_viewer(trace_file)
        
        return False
    
    # Normal completion
    cdebugger.stop_trace()
    print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
    print(f"{Colors.GREEN}Execution completed successfully.{Colors.RESET}")
    print(f"{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
    print(f"\nTrace saved to: {Colors.BOLD}{trace_file}{Colors.RESET}")
    print(f"{Colors.YELLOW}Launching post-execution debugger...{Colors.RESET}")
    
    launch_trace_viewer(trace_file)
    
    return True

def main():
    """Main entry point"""
    
    if len(sys.argv) < 2:
        print(f"{Colors.CYAN}╔════════════════════════════════════════════════════════╗{Colors.RESET}")
        print(f"{Colors.CYAN}║         Python Time-Traveling Debugger v1.0          ║{Colors.RESET}")
        print(f"{Colors.CYAN}╚════════════════════════════════════════════════════════╝{Colors.RESET}\n")
        print(f"{Colors.YELLOW}Usage:{Colors.RESET} python3 idebug.py <python_file> [trace_file]")
        print(f"\n{Colors.GREEN}Arguments:{Colors.RESET}")
        print(f"  python_file    Path to the Python file to debug")
        print(f"  trace_file     Path to save trace (default: trace.log)")
        print(f"\n{Colors.YELLOW}Example:{Colors.RESET}")
        print(f"  python3 idebug.py test.py")
        print(f"  python3 idebug.py myapp.py my_trace.log")
        print(f"\n{Colors.CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{Colors.RESET}")
        print("The interactive CLI will open to set breakpoints before execution.")
        sys.exit(1)
    
    python_file = sys.argv[1]
    trace_file = sys.argv[2] if len(sys.argv) > 2 else "trace.log"
    
    # Check if file exists
    if not os.path.exists(python_file):
        print(f"{Colors.RED}Error:{Colors.RESET} File '{python_file}' not found")
        sys.exit(1)
    
    # Check if it's a Python file
    if not python_file.endswith('.py'):
        print(f"{Colors.YELLOW}Warning:{Colors.RESET} '{python_file}' doesn't have a .py extension")
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
