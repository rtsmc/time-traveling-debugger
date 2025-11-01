"""
Helper module for testing breakpoints in imported files
"""

def calculate_sum(numbers):
    """Calculate sum of numbers"""
    total = 0  # Set breakpoint here (line 7)
    for num in numbers:
        total += num  # Set breakpoint here (line 9)
    return total

def calculate_product(numbers):
    """Calculate product of numbers"""
    result = 1  # Set breakpoint here (line 14)
    for num in numbers:
        result *= num  # Set breakpoint here (line 16)
    return result

def greet(name):
    """Simple greeting function"""
    message = f"Hello, {name}!"  # Set breakpoint here (line 21)
    return message
