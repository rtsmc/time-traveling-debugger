"""
Clean Loop Test - For use with idebug.py
No embedded cdebugger calls - set breakpoints interactively
"""

def simple_loop():
    """Test simple for loop with breakpoint inside"""
    print("\n=== Simple Loop Test ===")
    total = 0
    for i in range(5):
        total += i  # Set breakpoint here (line 11)
        print(f"  i={i}, total={total}")
    print(f"Final total: {total}")
    return total

def nested_loops():
    """Test nested loops with breakpoint"""
    print("\n=== Nested Loop Test ===")
    for i in range(3):  # Set breakpoint here (line 20)
        for j in range(3):
            product = i * j
            print(f"  {i} * {j} = {product}")
    print("Nested loops complete")

def while_loop_test():
    """Test while loop with breakpoint"""
    print("\n=== While Loop Test ===")
    count = 0
    while count < 3:  # Set breakpoint here (line 30)
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
    print("║          Clean Loop Test Program                     ║")
    print("╚════════════════════════════════════════════════════════╝")
    print("\nThis program tests breakpoint behavior in loops.")
    print("\nSuggested breakpoints:")
    print("  - Line 11: Inside simple for loop")
    print("  - Line 20: Outer loop of nested loops")
    print("  - Line 30: While loop condition")
    print("\nTips:")
    print("  - Press 'n' to step through each iteration")
    print("  - Press 'c' to continue and see breakpoint hit multiple times")
    print("  - Press 'b' to step back through history")
    
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
