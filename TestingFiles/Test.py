#!/usr/bin/env python3
"""
Comprehensive Python test file for debugger testing.
Contains 1000+ lines with various functions, classes, and control structures.
"""

import math
import random
import time
import json
from typing import List, Dict, Any, Optional, Tuple, Union
from collections import defaultdict, namedtuple
from functools import wraps, reduce
from itertools import combinations, permutations
import threading
import asyncio


# Configuration constants
CONFIG = {
    'max_iterations': 1000,
    'default_timeout': 5.0,
    'debug_mode': True,
    'log_level': 'INFO'
}

# Named tuple for testing
Point = namedtuple('Point', ['x', 'y', 'z'])
Person = namedtuple('Person', ['name', 'age', 'email'])


def timing_decorator(func):
    """Decorator to measure function execution time."""
    @wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        print(f"{func.__name__} took {end_time - start_time:.4f} seconds")
        return result
    return wrapper


def validation_decorator(expected_type):
    """Decorator to validate function return type."""
    def decorator(func):
        @wraps(func)
        def wrapper(*args, **kwargs):
            result = func(*args, **kwargs)
            if not isinstance(result, expected_type):
                raise TypeError(f"Expected {expected_type}, got {type(result)}")
            return result
        return wrapper
    return decorator


class MathOperations:
    """Class containing various mathematical operations for testing."""
    
    def __init__(self, precision=2):
        self.precision = precision
        self.history = []
        self.operation_count = 0
    
    @timing_decorator
    def add(self, a: float, b: float) -> float:
        """Add two numbers."""
        result = round(a + b, self.precision)
        self.history.append(f"add({a}, {b}) = {result}")
        self.operation_count += 1
        return result
    
    @timing_decorator
    def subtract(self, a: float, b: float) -> float:
        """Subtract two numbers."""
        result = round(a - b, self.precision)
        self.history.append(f"subtract({a}, {b}) = {result}")
        self.operation_count += 1
        return result
    
    @timing_decorator
    def multiply(self, a: float, b: float) -> float:
        """Multiply two numbers."""
        result = round(a * b, self.precision)
        self.history.append(f"multiply({a}, {b}) = {result}")
        self.operation_count += 1
        return result
    
    @timing_decorator
    def divide(self, a: float, b: float) -> float:
        """Divide two numbers with error handling."""
        if b == 0:
            raise ValueError("Division by zero is not allowed")
        result = round(a / b, self.precision)
        self.history.append(f"divide({a}, {b}) = {result}")
        self.operation_count += 1
        return result
    
    def power(self, base: float, exponent: float) -> float:
        """Calculate power of a number."""
        try:
            result = round(pow(base, exponent), self.precision)
            self.history.append(f"power({base}, {exponent}) = {result}")
            self.operation_count += 1
            return result
        except OverflowError:
            return float('inf')
    
    def sqrt(self, number: float) -> float:
        """Calculate square root."""
        if number < 0:
            raise ValueError("Cannot calculate square root of negative number")
        result = round(math.sqrt(number), self.precision)
        self.history.append(f"sqrt({number}) = {result}")
        self.operation_count += 1
        return result
    
    def factorial(self, n: int) -> int:
        """Calculate factorial recursively."""
        if not isinstance(n, int) or n < 0:
            raise ValueError("Factorial requires non-negative integer")
        if n <= 1:
            return 1
        result = n * self.factorial(n - 1)
        self.history.append(f"factorial({n}) = {result}")
        self.operation_count += 1
        return result
    
    def fibonacci(self, n: int) -> int:
        """Calculate Fibonacci number."""
        if n < 0:
            raise ValueError("Fibonacci requires non-negative integer")
        if n <= 1:
            return n
        
        a, b = 0, 1
        for i in range(2, n + 1):
            a, b = b, a + b
        
        self.history.append(f"fibonacci({n}) = {b}")
        self.operation_count += 1
        return b
    
    def gcd(self, a: int, b: int) -> int:
        """Calculate greatest common divisor."""
        while b:
            a, b = b, a % b
        self.operation_count += 1
        return abs(a)
    
    def lcm(self, a: int, b: int) -> int:
        """Calculate least common multiple."""
        result = abs(a * b) // self.gcd(a, b)
        self.operation_count += 1
        return result
    
    def is_prime(self, n: int) -> bool:
        """Check if number is prime."""
        if n < 2:
            return False
        for i in range(2, int(math.sqrt(n)) + 1):
            if n % i == 0:
                return False
        self.operation_count += 1
        return True
    
    def prime_factors(self, n: int) -> List[int]:
        """Get prime factors of a number."""
        factors = []
        d = 2
        while d * d <= n:
            while n % d == 0:
                factors.append(d)
                n //= d
            d += 1
        if n > 1:
            factors.append(n)
        self.operation_count += 1
        return factors
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get operation statistics."""
        return {
            'total_operations': self.operation_count,
            'history_length': len(self.history),
            'precision': self.precision
        }
    
    def clear_history(self):
        """Clear operation history."""
        self.history.clear()
        self.operation_count = 0


class DataProcessor:
    """Class for processing various data structures."""
    
    def __init__(self):
        self.processed_count = 0
        self.errors = []
    
    def process_list(self, data: List[Any]) -> Dict[str, Any]:
        """Process a list and return statistics."""
        if not isinstance(data, list):
            raise TypeError("Expected list input")
        
        try:
            stats = {
                'length': len(data),
                'unique_count': len(set(data)),
                'type_distribution': {},
                'numeric_stats': {}
            }
            
            # Count types
            for item in data:
                type_name = type(item).__name__
                stats['type_distribution'][type_name] = stats['type_distribution'].get(type_name, 0) + 1
            
            # Numeric statistics if applicable
            numeric_data = [x for x in data if isinstance(x, (int, float))]
            if numeric_data:
                stats['numeric_stats'] = {
                    'min': min(numeric_data),
                    'max': max(numeric_data),
                    'mean': sum(numeric_data) / len(numeric_data),
                    'median': self._calculate_median(numeric_data)
                }
            
            self.processed_count += 1
            return stats
            
        except Exception as e:
            self.errors.append(str(e))
            raise
    
    def _calculate_median(self, numbers: List[Union[int, float]]) -> float:
        """Calculate median of a list of numbers."""
        sorted_numbers = sorted(numbers)
        n = len(sorted_numbers)
        if n % 2 == 0:
            return (sorted_numbers[n//2 - 1] + sorted_numbers[n//2]) / 2
        else:
            return sorted_numbers[n//2]
    
    def process_dict(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Process a dictionary and return analysis."""
        if not isinstance(data, dict):
            raise TypeError("Expected dictionary input")
        
        analysis = {
            'key_count': len(data.keys()),
            'value_types': {},
            'nested_depth': self._calculate_depth(data),
            'key_lengths': [len(str(k)) for k in data.keys()]
        }
        
        for value in data.values():
            type_name = type(value).__name__
            analysis['value_types'][type_name] = analysis['value_types'].get(type_name, 0) + 1
        
        self.processed_count += 1
        return analysis
    
    def _calculate_depth(self, obj: Any, current_depth: int = 0) -> int:
        """Calculate maximum nesting depth of a data structure."""
        if isinstance(obj, dict):
            if not obj:
                return current_depth
            return max(self._calculate_depth(value, current_depth + 1) for value in obj.values())
        elif isinstance(obj, (list, tuple)):
            if not obj:
                return current_depth
            return max(self._calculate_depth(item, current_depth + 1) for item in obj)
        else:
            return current_depth
    
    def merge_dicts(self, *dicts: Dict[str, Any]) -> Dict[str, Any]:
        """Merge multiple dictionaries."""
        result = {}
        for d in dicts:
            if not isinstance(d, dict):
                raise TypeError("All arguments must be dictionaries")
            result.update(d)
        self.processed_count += 1
        return result
    
    def filter_data(self, data: List[Any], predicate) -> List[Any]:
        """Filter data based on predicate function."""
        try:
            result = [item for item in data if predicate(item)]
            self.processed_count += 1
            return result
        except Exception as e:
            self.errors.append(f"Filter error: {str(e)}")
            raise
    
    def group_by(self, data: List[Dict[str, Any]], key: str) -> Dict[str, List[Dict[str, Any]]]:
        """Group list of dictionaries by specified key."""
        groups = defaultdict(list)
        for item in data:
            if not isinstance(item, dict):
                raise TypeError("All items must be dictionaries")
            if key not in item:
                raise KeyError(f"Key '{key}' not found in item")
            groups[item[key]].append(item)
        
        self.processed_count += 1
        return dict(groups)
    
    def transform_data(self, data: List[Any], transformer) -> List[Any]:
        """Transform data using provided function."""
        try:
            result = [transformer(item) for item in data]
            self.processed_count += 1
            return result
        except Exception as e:
            self.errors.append(f"Transform error: {str(e)}")
            raise


class StringProcessor:
    """Class for string processing operations."""
    
    def __init__(self):
        self.operation_count = 0
    
    def reverse_string(self, text: str) -> str:
        """Reverse a string."""
        if not isinstance(text, str):
            raise TypeError("Input must be a string")
        result = text[::-1]
        self.operation_count += 1
        return result
    
    def is_palindrome(self, text: str) -> bool:
        """Check if string is a palindrome."""
        cleaned = ''.join(char.lower() for char in text if char.isalnum())
        result = cleaned == cleaned[::-1]
        self.operation_count += 1
        return result
    
    def count_words(self, text: str) -> int:
        """Count words in text."""
        if not text.strip():
            return 0
        result = len(text.split())
        self.operation_count += 1
        return result
    
    def count_characters(self, text: str, include_spaces: bool = True) -> int:
        """Count characters in text."""
        if include_spaces:
            result = len(text)
        else:
            result = len(text.replace(' ', ''))
        self.operation_count += 1
        return result
    
    def capitalize_words(self, text: str) -> str:
        """Capitalize first letter of each word."""
        result = ' '.join(word.capitalize() for word in text.split())
        self.operation_count += 1
        return result
    
    def remove_duplicates(self, text: str) -> str:
        """Remove duplicate characters while preserving order."""
        seen = set()
        result = []
        for char in text:
            if char not in seen:
                seen.add(char)
                result.append(char)
        self.operation_count += 1
        return ''.join(result)
    
    def find_anagrams(self, word: str, word_list: List[str]) -> List[str]:
        """Find anagrams of a word from a list."""
        word_sorted = ''.join(sorted(word.lower()))
        anagrams = []
        
        for candidate in word_list:
            if ''.join(sorted(candidate.lower())) == word_sorted and candidate.lower() != word.lower():
                anagrams.append(candidate)
        
        self.operation_count += 1
        return anagrams
    
    def caesar_cipher(self, text: str, shift: int) -> str:
        """Apply Caesar cipher to text."""
        result = []
        for char in text:
            if char.isalpha():
                ascii_offset = ord('A') if char.isupper() else ord('a')
                shifted = (ord(char) - ascii_offset + shift) % 26
                result.append(chr(shifted + ascii_offset))
            else:
                result.append(char)
        
        self.operation_count += 1
        return ''.join(result)
    
    def extract_numbers(self, text: str) -> List[float]:
        """Extract all numbers from text."""
        import re
        pattern = r'-?\d+\.?\d*'
        matches = re.findall(pattern, text)
        result = [float(match) for match in matches if match]
        self.operation_count += 1
        return result
    
    def word_frequency(self, text: str) -> Dict[str, int]:
        """Calculate word frequency."""
        words = text.lower().split()
        frequency = {}
        for word in words:
            clean_word = ''.join(char for char in word if char.isalnum())
            if clean_word:
                frequency[clean_word] = frequency.get(clean_word, 0) + 1
        
        self.operation_count += 1
        return frequency


class AlgorithmTester:
    """Class containing various algorithm implementations."""
    
    def __init__(self):
        self.comparisons = 0
        self.swaps = 0
    
    def reset_counters(self):
        """Reset operation counters."""
        self.comparisons = 0
        self.swaps = 0
    
    def bubble_sort(self, arr: List[int]) -> List[int]:
        """Bubble sort implementation."""
        arr = arr.copy()
        n = len(arr)
        self.reset_counters()
        
        for i in range(n):
            swapped = False
            for j in range(0, n - i - 1):
                self.comparisons += 1
                if arr[j] > arr[j + 1]:
                    arr[j], arr[j + 1] = arr[j + 1], arr[j]
                    self.swaps += 1
                    swapped = True
            if not swapped:
                break
        
        return arr
    
    def quick_sort(self, arr: List[int]) -> List[int]:
        """Quick sort implementation."""
        self.reset_counters()
        result = self._quick_sort_recursive(arr.copy())
        return result
    
    def _quick_sort_recursive(self, arr: List[int]) -> List[int]:
        """Recursive helper for quick sort."""
        if len(arr) <= 1:
            return arr
        
        pivot = arr[len(arr) // 2]
        left = []
        middle = []
        right = []
        
        for x in arr:
            self.comparisons += 1
            if x < pivot:
                left.append(x)
            elif x == pivot:
                middle.append(x)
            else:
                right.append(x)
        
        return (self._quick_sort_recursive(left) + 
                middle + 
                self._quick_sort_recursive(right))
    
    def merge_sort(self, arr: List[int]) -> List[int]:
        """Merge sort implementation."""
        self.reset_counters()
        return self._merge_sort_recursive(arr.copy())
    
    def _merge_sort_recursive(self, arr: List[int]) -> List[int]:
        """Recursive helper for merge sort."""
        if len(arr) <= 1:
            return arr
        
        mid = len(arr) // 2
        left = self._merge_sort_recursive(arr[:mid])
        right = self._merge_sort_recursive(arr[mid:])
        
        return self._merge(left, right)
    
    def _merge(self, left: List[int], right: List[int]) -> List[int]:
        """Merge helper for merge sort."""
        result = []
        i = j = 0
        
        while i < len(left) and j < len(right):
            self.comparisons += 1
            if left[i] <= right[j]:
                result.append(left[i])
                i += 1
            else:
                result.append(right[j])
                j += 1
        
        result.extend(left[i:])
        result.extend(right[j:])
        return result
    
    def binary_search(self, arr: List[int], target: int) -> int:
        """Binary search implementation."""
        self.reset_counters()
        left, right = 0, len(arr) - 1
        
        while left <= right:
            self.comparisons += 1
            mid = (left + right) // 2
            
            if arr[mid] == target:
                return mid
            elif arr[mid] < target:
                left = mid + 1
            else:
                right = mid - 1
        
        return -1
    
    def linear_search(self, arr: List[int], target: int) -> int:
        """Linear search implementation."""
        self.reset_counters()
        
        for i, value in enumerate(arr):
            self.comparisons += 1
            if value == target:
                return i
        
        return -1
    
    def depth_first_search(self, graph: Dict[str, List[str]], start: str, target: str) -> List[str]:
        """DFS implementation for graph traversal."""
        visited = set()
        path = []
        
        def dfs_recursive(node: str) -> bool:
            if node in visited:
                return False
            
            visited.add(node)
            path.append(node)
            
            if node == target:
                return True
            
            for neighbor in graph.get(node, []):
                if dfs_recursive(neighbor):
                    return True
            
            path.pop()
            return False
        
        if dfs_recursive(start):
            return path
        return []
    
    def breadth_first_search(self, graph: Dict[str, List[str]], start: str, target: str) -> List[str]:
        """BFS implementation for graph traversal."""
        from collections import deque
        
        queue = deque([(start, [start])])
        visited = set([start])
        
        while queue:
            node, path = queue.popleft()
            
            if node == target:
                return path
            
            for neighbor in graph.get(node, []):
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append((neighbor, path + [neighbor]))
        
        return []
    
    def get_sorting_stats(self) -> Dict[str, int]:
        """Get sorting operation statistics."""
        return {
            'comparisons': self.comparisons,
            'swaps': self.swaps
        }


class InputTester:
    """Class for testing various input scenarios."""
    
    def __init__(self):
        self.input_history = []
        self.validation_errors = []
    
    def get_user_name(self) -> str:
        """Get user name with validation."""
        while True:
            try:
                name = input("Enter your name: ").strip()
                if not name:
                    print("Name cannot be empty. Please try again.")
                    continue
                if len(name) < 2:
                    print("Name must be at least 2 characters long.")
                    continue
                if not name.replace(' ', '').isalpha():
                    print("Name can only contain letters and spaces.")
                    continue
                
                self.input_history.append(f"name: {name}")
                return name
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return ""
            except Exception as e:
                error_msg = f"Input error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def get_user_age(self) -> int:
        """Get user age with validation."""
        while True:
            try:
                age_input = input("Enter your age (1-150): ").strip()
                
                if not age_input:
                    print("Age cannot be empty.")
                    continue
                
                age = int(age_input)
                
                if age < 1 or age > 150:
                    print("Age must be between 1 and 150.")
                    continue
                
                self.input_history.append(f"age: {age}")
                return age
                
            except ValueError:
                print("Please enter a valid number.")
                continue
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return 0
            except Exception as e:
                error_msg = f"Age input error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def get_menu_choice(self, options: List[str]) -> int:
        """Display menu and get user choice."""
        while True:
            try:
                print("\nMenu Options:")
                for i, option in enumerate(options, 1):
                    print(f"{i}. {option}")
                print("0. Exit")
                
                choice_input = input(f"Choose option (0-{len(options)}): ").strip()
                
                if not choice_input:
                    print("Please enter a choice.")
                    continue
                
                choice = int(choice_input)
                
                if choice < 0 or choice > len(options):
                    print(f"Choice must be between 0 and {len(options)}.")
                    continue
                
                self.input_history.append(f"menu_choice: {choice}")
                return choice
                
            except ValueError:
                print("Please enter a valid number.")
                continue
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return 0
            except Exception as e:
                error_msg = f"Menu choice error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def get_yes_no_input(self, prompt: str) -> bool:
        """Get yes/no input from user."""
        while True:
            try:
                response = input(f"{prompt} (y/n): ").strip().lower()
                
                if response in ['y', 'yes', '1', 'true']:
                    self.input_history.append(f"yes_no: True ({response})")
                    return True
                elif response in ['n', 'no', '0', 'false']:
                    self.input_history.append(f"yes_no: False ({response})")
                    return False
                else:
                    print("Please enter 'y' for yes or 'n' for no.")
                    continue
                    
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return False
            except Exception as e:
                error_msg = f"Yes/No input error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def get_number_list(self, prompt: str = "Enter numbers separated by commas") -> List[float]:
        """Get a list of numbers from user input."""
        while True:
            try:
                numbers_input = input(f"{prompt}: ").strip()
                
                if not numbers_input:
                    print("Please enter at least one number.")
                    continue
                
                # Split by comma and convert to numbers
                number_strings = [s.strip() for s in numbers_input.split(',')]
                numbers = []
                
                for num_str in number_strings:
                    if not num_str:
                        continue
                    numbers.append(float(num_str))
                
                if not numbers:
                    print("No valid numbers found. Please try again.")
                    continue
                
                self.input_history.append(f"number_list: {numbers}")
                return numbers
                
            except ValueError as e:
                print(f"Invalid number format: {str(e)}")
                continue
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return []
            except Exception as e:
                error_msg = f"Number list input error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def get_password_input(self, min_length: int = 6) -> str:
        """Get password with basic validation (visible for debugging)."""
        import getpass
        
        while True:
            try:
                # For debugging purposes, we'll use regular input instead of getpass
                # In real applications, use: password = getpass.getpass("Enter password: ")
                password = input(f"Enter password (min {min_length} characters): ")
                
                if len(password) < min_length:
                    print(f"Password must be at least {min_length} characters long.")
                    continue
                
                # Basic password strength check
                has_upper = any(c.isupper() for c in password)
                has_lower = any(c.islower() for c in password)
                has_digit = any(c.isdigit() for c in password)
                
                strength_score = sum([has_upper, has_lower, has_digit])
                
                if strength_score < 2:
                    print("Password should contain uppercase, lowercase, and numbers.")
                    continue_anyway = self.get_yes_no_input("Use this password anyway?")
                    if not continue_anyway:
                        continue
                
                self.input_history.append(f"password: [HIDDEN] (length: {len(password)})")
                return password
                
            except KeyboardInterrupt:
                print("\nOperation cancelled by user.")
                return ""
            except Exception as e:
                error_msg = f"Password input error: {str(e)}"
                self.validation_errors.append(error_msg)
                print(error_msg)
    
    def interactive_calculator(self):
        """Interactive calculator using input."""
        print("\n=== Interactive Calculator ===")
        print("Enter mathematical expressions (type 'quit' to exit)")
        
        while True:
            try:
                expression = input("Calculate> ").strip()
                
                if expression.lower() in ['quit', 'exit', 'q']:
                    break
                
                if not expression:
                    continue
                
                # Simple expression evaluation (be careful with eval in real apps!)
                # Only allow safe characters
                allowed_chars = set('0123456789+-*/.() ')
                if not all(c in allowed_chars for c in expression):
                    print("Only basic math operations are allowed (+, -, *, /, parentheses)")
                    continue
                
                result = eval(expression)  # Note: eval is dangerous in production!
                print(f"Result: {result}")
                
                self.input_history.append(f"calculator: {expression} = {result}")
                
            except ZeroDivisionError:
                print("Error: Division by zero!")
            except SyntaxError:
                print("Error: Invalid mathematical expression!")
            except Exception as e:
                print(f"Error: {str(e)}")
            except KeyboardInterrupt:
                print("\nCalculator closed.")
                break
    
    def multi_line_input_test(self) -> str:
        """Test multi-line input."""
        print("\nEnter multiple lines of text (type 'END' on a line by itself to finish):")
        lines = []
        
        try:
            while True:
                line = input()
                if line.strip().upper() == 'END':
                    break
                lines.append(line)
            
            result = '\n'.join(lines)
            self.input_history.append(f"multiline: {len(lines)} lines")
            return result
            
        except KeyboardInterrupt:
            print("\nMulti-line input cancelled.")
            return '\n'.join(lines)
    
    def get_input_history(self) -> List[str]:
        """Return input history."""
        return self.input_history.copy()
    
    def get_validation_errors(self) -> List[str]:
        """Return validation errors."""
        return self.validation_errors.copy()
    
    def clear_history(self):
        """Clear input history."""
        self.input_history.clear()
        self.validation_errors.clear()


class FileProcessor:
    """Class for file processing operations."""
    
    def __init__(self, encoding='utf-8'):
        self.encoding = encoding
        self.processed_files = []
        self.errors = []
    
    def read_file_lines(self, filename: str) -> List[str]:
        """Read file and return lines."""
        try:
            with open(filename, 'r', encoding=self.encoding) as file:
                lines = file.readlines()
            self.processed_files.append(filename)
            return [line.strip() for line in lines]
        except Exception as e:
            error_msg = f"Error reading {filename}: {str(e)}"
            self.errors.append(error_msg)
            raise FileNotFoundError(error_msg)
    
    def write_file_lines(self, filename: str, lines: List[str]) -> bool:
        """Write lines to file."""
        try:
            with open(filename, 'w', encoding=self.encoding) as file:
                for line in lines:
                    file.write(line + '\n')
            self.processed_files.append(filename)
            return True
        except Exception as e:
            error_msg = f"Error writing {filename}: {str(e)}"
            self.errors.append(error_msg)
            return False
    
    def count_file_stats(self, filename: str) -> Dict[str, int]:
        """Count various file statistics."""
        try:
            with open(filename, 'r', encoding=self.encoding) as file:
                content = file.read()
            
            stats = {
                'characters': len(content),
                'characters_no_spaces': len(content.replace(' ', '')),
                'words': len(content.split()),
                'lines': len(content.split('\n')),
                'paragraphs': len([p for p in content.split('\n\n') if p.strip()])
            }
            
            self.processed_files.append(filename)
            return stats
            
        except Exception as e:
            error_msg = f"Error processing {filename}: {str(e)}"
            self.errors.append(error_msg)
            raise
    
    def find_in_file(self, filename: str, search_term: str, case_sensitive: bool = True) -> List[Tuple[int, str]]:
        """Find occurrences of search term in file."""
        matches = []
        
        try:
            with open(filename, 'r', encoding=self.encoding) as file:
                for line_num, line in enumerate(file, 1):
                    search_line = line if case_sensitive else line.lower()
                    search_term_check = search_term if case_sensitive else search_term.lower()
                    
                    if search_term_check in search_line:
                        matches.append((line_num, line.strip()))
            
            self.processed_files.append(filename)
            return matches
            
        except Exception as e:
            error_msg = f"Error searching {filename}: {str(e)}"
            self.errors.append(error_msg)
            raise
    
    def backup_file(self, filename: str) -> str:
        """Create backup of file."""
        import shutil
        import os
        
        if not os.path.exists(filename):
            raise FileNotFoundError(f"File {filename} not found")
        
        backup_name = f"{filename}.backup"
        try:
            shutil.copy2(filename, backup_name)
            self.processed_files.append(backup_name)
            return backup_name
        except Exception as e:
            error_msg = f"Error backing up {filename}: {str(e)}"
            self.errors.append(error_msg)
            raise


def generate_random_data(size: int = 100) -> Dict[str, Any]:
    """Generate random test data."""
    data = {
        'integers': [random.randint(1, 1000) for _ in range(size)],
        'floats': [random.uniform(0, 100) for _ in range(size)],
        'strings': [f"string_{i}" for i in range(size)],
        'booleans': [random.choice([True, False]) for _ in range(size)],
        'mixed_list': []
    }
    
    # Create mixed list with various types
    for i in range(size):
        choice = random.randint(1, 4)
        if choice == 1:
            data['mixed_list'].append(random.randint(1, 100))
        elif choice == 2:
            data['mixed_list'].append(random.uniform(0, 10))
        elif choice == 3:
            data['mixed_list'].append(f"item_{i}")
        else:
            data['mixed_list'].append(random.choice([True, False]))
    
    return data


def test_exception_handling():
    """Function to test various exception scenarios."""
    test_cases = [
        lambda: 1 / 0,  # ZeroDivisionError
        lambda: int('not_a_number'),  # ValueError
        lambda: [1, 2, 3][10],  # IndexError
        lambda: {'a': 1}['b'],  # KeyError
        lambda: None.some_method(),  # AttributeError
    ]
    
    results = []
    for i, test_case in enumerate(test_cases):
        try:
            result = test_case()
            results.append(f"Test {i}: Success - {result}")
        except ZeroDivisionError as e:
            results.append(f"Test {i}: ZeroDivisionError - {str(e)}")
        except ValueError as e:
            results.append(f"Test {i}: ValueError - {str(e)}")
        except IndexError as e:
            results.append(f"Test {i}: IndexError - {str(e)}")
        except KeyError as e:
            results.append(f"Test {i}: KeyError - {str(e)}")
        except AttributeError as e:
            results.append(f"Test {i}: AttributeError - {str(e)}")
        except Exception as e:
            results.append(f"Test {i}: Unexpected error - {str(e)}")
    
    return results


def recursive_countdown(n: int, callback=None) -> List[int]:
    """Recursive function for testing debugger step-through."""
    if callback:
        callback(n)
    
    if n <= 0:
        return [0]
    
    result = [n]
    result.extend(recursive_countdown(n - 1, callback))
    return result


def nested_function_example():
    """Example of nested functions for debugger testing."""
    
    def outer_function(x):
        def inner_function(y):
            def innermost_function(z):
                return x + y + z
            return innermost_function(y * 2)
        return inner_function(x * 3)
    
    result = outer_function(5)
    return result


def generator_example():
    """Generator function for testing."""
    for i in range(10):
        yield i * i


def coroutine_example():
    """Simple coroutine for testing."""
    result = []
    while True:
        value = yield
        if value is None:
            break
        result.append(value * 2)
    return result


class ContextManagerExample:
    """Custom context manager for testing."""
    
    def __init__(self, name):
        self.name = name
        self.entered = False
        self.exited = False
    
    def __enter__(self):
        print(f"Entering context: {self.name}")
        self.entered = True
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        print(f"Exiting context: {self.name}")
        self.exited = True
        if exc_type:
            print(f"Exception occurred: {exc_type.__name__}: {exc_val}")
        return False


def threading_example():
    """Threading example for debugger testing."""
    results = []
    lock = threading.Lock()
    
    def worker(worker_id):
        for i in range(5):
            with lock:
                results.append(f"Worker {worker_id}: {i}")
            time.sleep(0.1)
    
    threads = []
    for i in range(3):
        thread = threading.Thread(target=worker, args=(i,))
        threads.append(thread)
        thread.start()
    
    for thread in threads:
        thread.join()
    
    return results


async def async_example():
    """Async function example."""
    await asyncio.sleep(0.1)
    return "Async result"


def comprehension_examples():
    """Various comprehension examples."""
    data = list(range(20))
    
    results = {
        'list_comp': [x * 2 for x in data if x % 2 == 0],
        'dict_comp': {x: x ** 2 for x in data if x < 10},
        'set_comp': {x % 5 for x in data},
        'nested_comp': [[y for y in range(x)] for x in range(5)]
    }
    
    return results


def lambda_examples():
    """Lambda function examples."""
    numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
    
    # Various lambda operations
    squared = list(map(lambda x: x ** 2, numbers))
    filtered = list(filter(lambda x: x % 2 == 0, numbers))
    reduced = reduce(lambda x, y: x + y, numbers)
    
    # Sorted with lambda key
    people = [
        Person('Alice', 30, 'alice@example.com'),
        Person('Bob', 25, 'bob@example.com'),
        Person('Charlie', 35, 'charlie@example.com')
    ]
    
    sorted_by_age = sorted(people, key=lambda p: p.age)
    sorted_by_name = sorted(people, key=lambda p: p.name)
    
    return {
        'squared': squared,
        'filtered': filtered,
        'reduced': reduced,
        'sorted_by_age': sorted_by_age,
        'sorted_by_name': sorted_by_name
    }


def main():
    """Main function to demonstrate all features."""
    print("Starting comprehensive Python debugger test...")
    
    # Initialize classes
    math_ops = MathOperations(precision=3)
    data_proc = DataProcessor()
    string_proc = StringProcessor()
    algo_tester = AlgorithmTester()
    
    # Test mathematical operations
    print("\n=== Testing Mathematical Operations ===")
    try:
        result1 = math_ops.add(10.5, 20.3)
        result2 = math_ops.multiply(result1, 2)
        result3 = math_ops.power(2, 8)
        result4 = math_ops.sqrt(144)
        result5 = math_ops.factorial(5)
        result6 = math_ops.fibonacci(10)
        
        print(f"Math results: {result1}, {result2}, {result3}, {result4}, {result5}, {result6}")
        print(f"Prime check for 17: {math_ops.is_prime(17)}")
        print(f"Prime factors of 60: {math_ops.prime_factors(60)}")
        
    except Exception as e:
        print(f"Math operations error: {e}")
    
    # Test data processing
    print("\n=== Testing Data Processing ===")
    test_data = generate_random_data(50)
    
    try:
        list_stats = data_proc.process_list(test_data['integers'])
        dict_stats = data_proc.process_dict({'a': 1, 'b': [1, 2, 3], 'c': {'nested': True}})
        
        filtered_data = data_proc.filter_data(test_data['integers'], lambda x: x > 50)
        transformed_data = data_proc.transform_data([1, 2, 3, 4], lambda x: x ** 2)
        
        print(f"List stats: {list_stats}")
        print(f"Dict stats: {dict_stats}")
        print(f"Filtered count: {len(filtered_data)}")
        print(f"Transformed data: {transformed_data}")
        
    except Exception as e:
        print(f"Data processing error: {e}")
    
    # Test string processing
    print("\n=== Testing String Processing ===")
    test_string = "The quick brown fox jumps over the lazy dog"
    
    try:
        reversed_str = string_proc.reverse_string(test_string)
        word_count = string_proc.count_words(test_string)
        is_palindrome = string_proc.is_palindrome("A man a plan a canal Panama")
        capitalized = string_proc.capitalize_words(test_string.lower())
        cipher = string_proc.caesar_cipher("Hello World", 3)
        frequency = string_proc.word_frequency(test_string)
        
        print(f"Reversed: {reversed_str[:50]}...")
        print(f"Word count: {word_count}")
        print(f"Is palindrome: {is_palindrome}")
        print(f"Capitalized: {capitalized}")
        print(f"Caesar cipher: {cipher}")
        print(f"Word frequency: {frequency}")
        
    except Exception as e:
        print(f"String processing error: {e}")
    
    # Test algorithms
    print("\n=== Testing Algorithms ===")
    test_array = test_data['integers'][:20]  # Use smaller array for sorting
    
    try:
        # Test sorting algorithms
        bubble_sorted = algo_tester.bubble_sort(test_array)
        bubble_stats = algo_tester.get_sorting_stats()
        
        quick_sorted = algo_tester.quick_sort(test_array)
        quick_stats = algo_tester.get_sorting_stats()
        
        merge_sorted = algo_tester.merge_sort(test_array)
        merge_stats = algo_tester.get_sorting_stats()
        
        print(f"Original array: {test_array}")
        print(f"Bubble sorted: {bubble_sorted}")
        print(f"Bubble sort stats: {bubble_stats}")
        print(f"Quick sort stats: {quick_stats}")
        print(f"Merge sort stats: {merge_stats}")
        
        # Test search algorithms
        sorted_array = sorted(test_array)
        target = sorted_array[len(sorted_array)//2] if sorted_array else 0
        
        binary_index = algo_tester.binary_search(sorted_array, target)
        linear_index = algo_tester.linear_search(sorted_array, target)
        
        print(f"Searching for {target}: Binary={binary_index}, Linear={linear_index}")
        
        # Test graph algorithms
        test_graph = {
            'A': ['B', 'C'],
            'B': ['A', 'D', 'E'],
            'C': ['A', 'F'],
            'D': ['B'],
            'E': ['B', 'F'],
            'F': ['C', 'E']
        }
        
        dfs_path = algo_tester.depth_first_search(test_graph, 'A', 'F')
        bfs_path = algo_tester.breadth_first_search(test_graph, 'A', 'F')
        
        print(f"DFS path A->F: {dfs_path}")
        print(f"BFS path A->F: {bfs_path}")
        
    except Exception as e:
        print(f"Algorithm testing error: {e}")
    
    # Test exception handling
    print("\n=== Testing Exception Handling ===")
    try:
        exception_results = test_exception_handling()
        for result in exception_results:
            print(result)
    except Exception as e:
        print(f"Exception handling test error: {e}")
    
    # Test recursive functions
    print("\n=== Testing Recursive Functions ===")
    try:
        countdown_result = recursive_countdown(5)
        nested_result = nested_function_example()
        
        print(f"Countdown result: {countdown_result}")
        print(f"Nested function result: {nested_result}")
        
    except Exception as e:
        print(f"Recursive function error: {e}")
    
    # Test generators and comprehensions
    print("\n=== Testing Generators and Comprehensions ===")
    try:
        # Generator example
        gen = generator_example()
        gen_results = list(gen)
        print(f"Generator results: {gen_results}")
        
        # Comprehension examples
        comp_results = comprehension_examples()
        print(f"List comprehension: {comp_results['list_comp']}")
        print(f"Dict comprehension size: {len(comp_results['dict_comp'])}")
        print(f"Set comprehension: {comp_results['set_comp']}")
        
        # Lambda examples
        lambda_results = lambda_examples()
        print(f"Lambda squared: {lambda_results['squared'][:5]}")
        print(f"Lambda filtered: {lambda_results['filtered']}")
        print(f"Lambda reduced: {lambda_results['reduced']}")
        
    except Exception as e:
        print(f"Generator/comprehension error: {e}")
    
    # Test context manager
    print("\n=== Testing Context Manager ===")
    try:
        with ContextManagerExample("test_context") as ctx:
            print(f"Inside context, entered: {ctx.entered}")
            # Simulate some work
            time.sleep(0.1)
        
        print(f"After context, exited: {ctx.exited}")
        
    except Exception as e:
        print(f"Context manager error: {e}")
    
    # Test input functionality
    print("\n=== Testing Input Functionality ===")
    input_tester = InputTester()
    
    try:
        # Test if we should run interactive tests
        run_interactive = input_tester.get_yes_no_input("Run interactive input tests? (This will require user input)")
        
        if run_interactive:
            # Get user information
            user_name = input_tester.get_user_name()
            if user_name:
                user_age = input_tester.get_user_age()
                
                print(f"Hello {user_name}, age {user_age}!")
                
                # Menu test
                menu_options = ["Run math operations", "Process data", "String operations", "Algorithm tests"]
                choice = input_tester.get_menu_choice(menu_options)
                
                if choice > 0:
                    print(f"You selected: {menu_options[choice-1]}")
                    
                    # Number list test
                    if input_tester.get_yes_no_input("Enter a list of numbers to process?"):
                        numbers = input_tester.get_number_list("Enter numbers for processing")
                        if numbers:
                            processed_numbers = [x * 2 for x in numbers]
                            print(f"Your numbers doubled: {processed_numbers}")
                
                # Calculator test
                if input_tester.get_yes_no_input("Try the interactive calculator?"):
                    input_tester.interactive_calculator()
                
                # Multi-line input test
                if input_tester.get_yes_no_input("Test multi-line input?"):
                    multiline_text = input_tester.multi_line_input_test()
                    print(f"You entered {len(multiline_text.split())} words in your text.")
        
        # Display input history
        history = input_tester.get_input_history()
        errors = input_tester.get_validation_errors()
        
        print(f"Input history: {len(history)} entries")
        print(f"Validation errors: {len(errors)} errors")
        
    except Exception as e:
        print(f"Input testing error: {e}")
    
    # Test threading (commented out to avoid complexity in debugging)
    print("\n=== Testing Threading (Basic) ===")
    try:
        # Simple threading test
        import threading
        results = []
        
        def simple_worker(n):
            results.append(f"Worker {n} completed")
        
        thread1 = threading.Thread(target=simple_worker, args=(1,))
        thread2 = threading.Thread(target=simple_worker, args=(2,))
        
        thread1.start()
        thread2.start()
        
        thread1.join()
        thread2.join()
        
        print(f"Threading results: {results}")
        
    except Exception as e:
        print(f"Threading error: {e}")
    
    # Final statistics
    print("\n=== Final Statistics ===")
    print(f"Math operations performed: {math_ops.get_statistics()}")
    print(f"Data processing count: {data_proc.processed_count}")
    print(f"String operations count: {string_proc.operation_count}")
    print(f"Algorithm comparisons: {algo_tester.comparisons}")
    
    # Test complex nested loops
    print("\n=== Testing Nested Loops ===")
    matrix_result = []
    for i in range(5):
        row = []
        for j in range(5):
            value = i * j
            if value % 2 == 0:
                for k in range(3):
                    if k == 1:
                        value += k
            row.append(value)
        matrix_result.append(row)
    
    print(f"Matrix result shape: {len(matrix_result)}x{len(matrix_result[0])}")
    
    # Test dictionary operations
    print("\n=== Testing Complex Dictionary Operations ===")
    complex_dict = {
        'level1': {
            'level2': {
                'level3': {
                    'data': [1, 2, 3, 4, 5],
                    'metadata': {'created': '2025-01-01', 'version': 1.0}
                }
            }
        }
    }
    
    # Navigate through nested dictionary
    try:
        deep_data = complex_dict['level1']['level2']['level3']['data']
        metadata = complex_dict['level1']['level2']['level3']['metadata']
        print(f"Deep data: {deep_data}")
        print(f"Metadata: {metadata}")
    except KeyError as e:
        print(f"Dictionary navigation error: {e}")
    
    print("\nComprehensive Python debugger test completed!")


if __name__ == "__main__":
    main()