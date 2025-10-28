#!/usr/bin/env python3
"""
Python Debugger Runner

This script runs a target Python file with C-based tracing enabled.
It captures execution flow and generates a trace file for post-mortem debugging.
"""

import sys
import os
import cdebugger
import traceback
import subprocess

def launch_cli(trace_file):
    """Launch the trace viewer CLI"""
    
    # Check if traceviewer is compiled
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
    
    # Launch the CLI
    print("\n")
    subprocess.run(["./traceviewer", trace_file])

def run_with_trace(python_file, trace_file="trace.log"):
    """
    Run a Python file with tracing enabled
    
    Args:
        python_file: Path to the Python file to execute
        trace_file: Path to the trace file to generate
        
    Returns:
        bool: True if execution completed without errors, False otherwise
    """
    
    # Start tracing
    print(f"Starting trace to: {trace_file}")
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
        
        # Launch the C-based trace viewer CLI
        launch_cli(trace_file)
        
        return False
    
    # Normal completion - also launch CLI
    cdebugger.stop_trace()
    print(f"\n{'='*60}")
    print(f"Execution completed successfully.")
    print(f"{'='*60}")
    print(f"\nTrace file saved to: {trace_file}")
    print(f"Launching debugger CLI...")
    
    # Launch the CLI even on success
    launch_cli(trace_file)
    
    return True

def main():
    """Main entry point"""
    
    if len(sys.argv) < 2:
        print("Python Debugger Runner")
        print("=" * 60)
        print("\nUsage: python runner.py <python_file> [trace_file]")
        print("\nArguments:")
        print("  python_file    Path to the Python file to execute")
        print("  trace_file     Path to the trace file (default: trace.log)")
        print("\nExample:")
        print("  python runner.py test.py")
        print("  python runner.py test.py my_trace.log")
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
    
    # Run with tracing
    success = run_with_trace(python_file, trace_file)
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
