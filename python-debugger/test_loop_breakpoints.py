"""
Breakpoint Test with Loops
Tests breakpoint behavior in various loop scenarios
"""

import cdebugger

# Set breakpoints at strategic locations
cdebugger.set_breakpoint("test_loop_breakpoints.py", 22)  # Before loop
cdebugger.set_breakpoint("test_loop_breakpoints.py", 24)  # Inside loop
cdebugger.set_breakpoint("test_loop_breakpoints.py", 32)  # Nested loop
cdebugger.set_breakpoint("test_loop_breakpoints.py", 42)  # After loops

# Start tracing
cdebugger.start_trace("trace_loops.log")

def simple_loop():
    """Test simple for loop with breakpoint inside"""
    print("\n=== Simple Loop Test ===")
    total = 0
    # BREAKPOINT at line 22
    for i in range(5):
        # BREAKPOINT at line 24 - will hit 5 times!
        total += i
        print(f"  i={i}, total={total}")
    print(f"Final total: {total}")
    return total

def nested_loops():
    """Test nested loops with breakpoint"""
    print("\n=== Nested Loop Test ===")
    # BREAKPOINT at line 32
    for i in range(3):
        for j in range(3):
            product = i * j
            print(f"  {i} * {j} = {product}")
    print("Nested loops complete")

def while_loop_test():
    """Test while loop with breakpoint"""
    print("\n=== While Loop Test ===")
    count = 0
    # BREAKPOINT at line 42
    while count < 3:
        count += 1
        print(f"  Count: {count}")
    print(f"Final count: {count}")
    return count

def loop_with_break():
    """Test loop with early exit"""
    print("\n=== Loop with Break Test ===")
    for i in range(10):
        if i == 5:
            print(f"  Breaking at i={i}")
            break
        print(f"  i={i}")
    print("Loop with break complete")

def main():
    print("╔════════════════════════════════════════════════════════╗")
    print("║          Breakpoint Loop Test Program                ║")
    print("╚════════════════════════════════════════════════════════╝")
    print("\nThis program tests breakpoint behavior in loops.")
    print("Breakpoints set at lines: 22, 24, 32, 42")
    print("\nTips:")
    print("  - Press 'n' at line 24 to step through each iteration")
    print("  - Press 'c' to continue and see breakpoint hit multiple times")
    print("  - Watch how breakpoints behave in nested loops")
    
    # Run tests
    result1 = simple_loop()
    
    nested_loops()
    
    result2 = while_loop_test()
    
    loop_with_break()
    
    print("\n" + "="*60)
    print("All tests complete!")
    print(f"Simple loop result: {result1}")
    print(f"While loop result: {result2}")
    print("="*60)

if __name__ == "__main__":
    main()
    cdebugger.stop_trace()
    print("\n✓ Trace saved to: trace_loops.log")
