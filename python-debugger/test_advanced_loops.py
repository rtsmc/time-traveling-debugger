"""
Advanced Loop Breakpoint Test
Tests edge cases and complex loop scenarios
"""

import cdebugger

# Strategic breakpoints
cdebugger.set_breakpoint("test_advanced_loops.py", 25)  # Loop entry
cdebugger.set_breakpoint("test_advanced_loops.py", 27)  # Loop body
cdebugger.set_breakpoint("test_advanced_loops.py", 40)  # Continue statement
cdebugger.set_breakpoint("test_advanced_loops.py", 55)  # List comprehension result

cdebugger.start_trace("trace_advanced_loops.log")

def countdown_loop():
    """Test loop with countdown and conditional"""
    print("\n=== Countdown Loop ===")
    count = 5
    # BREAKPOINT line 25 - entry point
    while count > 0:
        # BREAKPOINT line 27 - body (hits 5 times)
        print(f"  Countdown: {count}")
        count -= 1
        if count == 2:
            print("  Two remaining!")
    print("Blast off!")

def skip_even_numbers():
    """Test loop with continue statement"""
    print("\n=== Skip Even Numbers ===")
    for i in range(10):
        if i % 2 == 0:
            continue  # Skip even numbers
        # BREAKPOINT line 40 - only hits on odd numbers
        print(f"  Odd number: {i}")
    print("Done skipping evens")

def loop_with_else():
    """Test for-else construct"""
    print("\n=== Loop with Else ===")
    for i in range(3):
        print(f"  Iteration {i}")
    else:
        print("  Loop completed normally (else block)")

def list_comprehension_test():
    """Test list comprehension with breakpoint"""
    print("\n=== List Comprehension ===")
    # This creates list in one line
    squares = [x**2 for x in range(5)]
    # BREAKPOINT line 55 - after comprehension
    print(f"  Squares: {squares}")
    
    # Nested comprehension
    matrix = [[i*j for j in range(3)] for i in range(3)]
    print(f"  Matrix: {matrix}")
    
    return squares

def enumerate_test():
    """Test enumerate in loop"""
    print("\n=== Enumerate Test ===")
    fruits = ['apple', 'banana', 'cherry']
    for idx, fruit in enumerate(fruits):
        print(f"  {idx}: {fruit}")

def zip_test():
    """Test zip in loop"""
    print("\n=== Zip Test ===")
    names = ['Alice', 'Bob', 'Charlie']
    ages = [25, 30, 35]
    for name, age in zip(names, ages):
        print(f"  {name} is {age} years old")

def infinite_loop_with_break():
    """Test infinite loop with break condition"""
    print("\n=== Infinite Loop with Break ===")
    counter = 0
    while True:
        counter += 1
        print(f"  Iteration {counter}")
        if counter >= 3:
            print("  Breaking out!")
            break

def nested_break_continue():
    """Test nested loops with break and continue"""
    print("\n=== Nested Break/Continue ===")
    for i in range(3):
        print(f"  Outer loop i={i}")
        for j in range(3):
            if j == 1:
                continue  # Skip j=1
            if i == 2 and j == 2:
                print(f"    Breaking inner loop at i={i}, j={j}")
                break
            print(f"    Inner loop j={j}")

def main():
    print("╔════════════════════════════════════════════════════════╗")
    print("║       Advanced Loop Breakpoint Test Program          ║")
    print("╚════════════════════════════════════════════════════════╝")
    print("\nThis tests complex loop scenarios with breakpoints:")
    print("  • Countdown loops")
    print("  • Continue statements")
    print("  • For-else constructs")
    print("  • List comprehensions")
    print("  • Enumerate and zip")
    print("  • Nested loops with break/continue")
    print("\nBreakpoints at lines: 25, 27, 40, 55")
    print("\nInteresting test cases:")
    print("  - Line 27: Hits multiple times in while loop")
    print("  - Line 40: Only hits when NOT skipped by continue")
    print("  - Line 55: After list comprehension completes")
    
    countdown_loop()
    skip_even_numbers()
    loop_with_else()
    squares = list_comprehension_test()
    enumerate_test()
    zip_test()
    infinite_loop_with_break()
    nested_break_continue()
    
    print("\n" + "="*60)
    print("All advanced tests complete!")
    print(f"Squares result: {squares}")
    print("="*60)

if __name__ == "__main__":
    main()
    cdebugger.stop_trace()
    print("\n✓ Advanced trace saved to: trace_advanced_loops.log")
