"""
Test script to demonstrate the Python debugger

This script performs various operations to generate a meaningful trace file.
"""

def calculate_sum(a, b):
    """Calculate the sum of two numbers"""
    result = a + b
    return result

def calculate_product(a, b):
    """Calculate the product of two numbers"""
    result = a * b
    return result

def process_data():
    """Process some data and return results"""
    numbers = [1, 2, 3, 4, 5]
    total = 0
    
    for num in numbers:
        total += num
        square = num * num
        print(f"Number: {num}, Square: {square}, Running Total: {total}")
    
    return total

def main():
    """Main function"""
    print("Starting test program...")
    
    # Basic calculations
    x = 5
    y = 10
    
    sum_result = calculate_sum(x, y)
    print(f"Sum of {x} and {y} is: {sum_result}")
    
    product_result = calculate_product(x, y)
    print(f"Product of {x} and {y} is: {product_result}")
    
    # Process some data
    total = process_data()
    print(f"\nTotal sum: {total}")
    
    # String operations
    message = "Hello, Debugger!"
    upper_message = message.upper()
    print(f"\nOriginal: {message}")
    print(f"Upper: {upper_message}")
    
    # Dictionary operations
    data = {
        'name': 'Python Debugger',
        'version': '1.0',
        'language': 'C'
    }
    
    print("\nDebugger Information:")
    for key, value in data.items():
        print(f"  {key}: {value}")
    
    # This will cause an error - uncomment to test crash handling
    # print("\nAbout to crash...")
    # bad_division = 1 / 0
    
    print("\nTest program completed successfully!")

if __name__ == "__main__":
    main()
