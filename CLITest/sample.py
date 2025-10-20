import sys
import os

def greet(name):
    """Greet someone by name"""
    return f"Hello, {name}!"

def main():
    if len(sys.argv) > 1:
        name = sys.argv[1]
    else:
        name = "World"
    
    print(greet(name))
    print("This is a sample Python file")
    print("Used for testing the CLI viewer")

if __name__ == "__main__":
    main()
