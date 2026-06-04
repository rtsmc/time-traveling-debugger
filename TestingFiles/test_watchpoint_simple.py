"""
Simple watchpoint test to verify write detection
"""

def main():
    print("Starting test")
    
    # First write to x
    x = 10
    print(f"x is {x}")
    
    # Write to y
    y = 20
    print(f"y is {y}")
    
    # Read x and y
    result = x + y
    print(f"Result: {result}")
    
    # Write to x again
    x = 30
    print(f"x is now {x}")
    
    print("Test complete")

if __name__ == "__main__":
    main()
