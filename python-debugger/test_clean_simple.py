"""
Simple Clean Test - For use with idebug.py
No embedded cdebugger calls
"""

def add(a, b):
    """Add two numbers"""
    result = a + b  # Set breakpoint here (line 8)
    return result

def multiply(a, b):
    """Multiply two numbers"""
    result = a * b  # Set breakpoint here (line 13)
    return result

def main():
    print("Simple Calculator Test")
    print("="*40)
    
    x = 10  # Set breakpoint here (line 20)
    y = 20
    
    sum_result = add(x, y)
    print(f"{x} + {y} = {sum_result}")
    
    product = multiply(x, y)  # Set breakpoint here (line 26)
    print(f"{x} * {y} = {product}")
    
    print("="*40)
    print("Test complete!")

if __name__ == "__main__":
    main()
