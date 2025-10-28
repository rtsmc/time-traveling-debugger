"""
Example: Using Breakpoints
Shows how to set breakpoints and step through code
"""

import cdebugger

# Set breakpoints BEFORE starting trace
cdebugger.set_breakpoint("test_with_breakpoints.py", 20)  # Stop at line 20
cdebugger.set_breakpoint("test_with_breakpoints.py", 30)  # Stop at line 30

# Start tracing
cdebugger.start_trace("trace.log")

def calculate(a, b):
    print(f"Calculating {a} + {b}")
    result = a + b
    print(f"Result: {result}")
    return result

def main():
    print("Starting program...")
    
    x = 10  # LINE 20 - First breakpoint here
    y = 20
    z = 30
    
    result1 = calculate(x, y)
    print(f"First result: {result1}")
    
    result2 = calculate(x, z)  # LINE 30 - Second breakpoint here
    print(f"Second result: {result2}")
    
    print("Program complete!")

if __name__ == "__main__":
    main()
    cdebugger.stop_trace()
