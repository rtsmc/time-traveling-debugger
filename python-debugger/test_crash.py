"""
Test script that crashes - for testing debugger error handling
"""

def dangerous_function(x, y):
    """This function will cause a division by zero error"""
    result = x / y
    return result

def main():
    """Main function that will crash"""
    print("Starting crash test...")
    
    a = 10
    b = 5
    
    result1 = dangerous_function(a, b)
    print(f"Result 1: {result1}")
    
    # This will crash
    c = 0
    result2 = dangerous_function(a, c)
    print(f"Result 2: {result2}")  # This line won't be reached
    
    print("Program completed")  # This won't be reached either

if __name__ == "__main__":
    main()
