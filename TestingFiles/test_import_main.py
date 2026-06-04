"""
Main test file that imports and uses helper module
Tests breakpoints across multiple files
"""

import test_helper

def main():
    print("=" * 50)
    print("Testing Breakpoints Across Files")
    print("=" * 50)
    
    # Test 1: Calculate sum
    numbers = [1, 2, 3, 4, 5]  # Set breakpoint here (line 13)
    result = test_helper.calculate_sum(numbers)
    print(f"Sum of {numbers} = {result}")
    
    # Test 2: Calculate product
    result = test_helper.calculate_product(numbers)  # Set breakpoint here (line 18)
    print(f"Product of {numbers} = {result}")
    
    # Test 3: Greeting
    name = "Alice"  # Set breakpoint here (line 22)
    greeting = test_helper.greet(name)
    print(greeting)
    
    print("=" * 50)
    print("Test complete!")

if __name__ == "__main__":
    main()
