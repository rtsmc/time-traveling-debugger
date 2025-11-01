"""
Test script to verify trace filtering and variable capture
"""

def calculate(a, b, c):
    """Function with multiple arguments"""
    # All these should be captured: a, b, c, result
    result = a + b + c
    return result

def loop_test():
    """Test loop variables"""
    total = 0
    numbers = [1, 2, 3, 4, 5]
    
    # Loop variable 'num' should be captured at each iteration
    for num in numbers:
        total += num
        square = num * num
    
    return total

def nested_function():
    """Test nested scope"""
    outer_var = "outer"
    
    def inner():
        inner_var = "inner"
        return inner_var
    
    result = inner()
    return result

def main():
    print("Testing variable capture...")
    
    # Test 1: Function arguments
    x = 10
    y = 20
    z = 30
    sum_result = calculate(x, y, z)
    print(f"Sum: {sum_result}")
    
    # Test 2: Loop variables
    loop_result = loop_test()
    print(f"Loop total: {loop_result}")
    
    # Test 3: Nested functions
    nested_result = nested_function()
    print(f"Nested: {nested_result}")
    
    # Test 4: Dictionary and list
    data = {'name': 'Test', 'value': 42}
    items = [1, 2, 3]
    
    print("All tests complete!")

if __name__ == "__main__":
    main()
