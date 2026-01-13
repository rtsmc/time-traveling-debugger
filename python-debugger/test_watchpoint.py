"""
Test script for watchpoint functionality
Demonstrates read and write watchpoints
"""

def calculate(x, y):
    result = x + y  # Write to result
    print(f"Sum: {result}")  # Read from result
    return result

def modify_values():
    counter = 0  # Write to counter
    for i in range(5):
        counter = counter + 1  # Read counter, then write
        temp = counter * 2  # Read counter, write temp
        print(f"Iteration {i}: counter={counter}, temp={temp}")
    return counter

def main():
    print("Watchpoint Test")
    print("=" * 40)
    
    x = 10  # Write to x
    y = 20  # Write to y
    
    sum_result = calculate(x, y)  # Read x and y
    print(f"Result: {sum_result}")  # Read sum_result
    
    final_count = modify_values()
    print(f"Final count: {final_count}")  # Read final_count
    
    print("=" * 40)
    print("Test complete!")

if __name__ == "__main__":
    main()
