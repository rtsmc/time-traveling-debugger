# Python File Viewer CLI

A simple C-based CLI tool for viewing Python files line by line with interactive navigation.

## Compilation

```bash
gcc -o pyviewer pyviewer.c -Wall
```

## Usage

```bash
./pyviewer <python_file>
```

Example:
```bash
./pyviewer sample.py
```

## Commands

Once the viewer starts, you can use the following commands:

- **`n`** - Move to the next line
- **`back`** - Move to the previous line
- **`:<number>`** - Jump to a specific line number (e.g., `:10` to go to line 10)
- **`full`** - Display the entire file with line numbers and highlight the current line
- **`quit`** or **`q`** - Exit the viewer

## Example Session

```
$ ./pyviewer sample.py
Python File Viewer - Loaded 19 lines
Commands: 'n' (next), 'back' (previous), ':<number>' (goto line), 'full' (show entire file)
---
[Line 1] import sys
> n
[Line 2] import os
> n
[Line 3] 
> :10
[Line 10]         name = sys.argv[1]
> full

--- Full File View ---
    [Line 1] import sys
    [Line 2] import os
    [Line 3] 
    [Line 4] def greet(name):
    [Line 5]     """Greet someone by name"""
    [Line 6]     return f"Hello, {name}!"
    [Line 7] 
    [Line 8] def main():
    [Line 9]     if len(sys.argv) > 1:
>>> [Line 10]         name = sys.argv[1]
    [Line 11]     else:
    [Line 12]         name = "World"
    [Line 13]     
    [Line 14]     print(greet(name))
    [Line 15]     print("This is a sample Python file")
    [Line 16]     print("Used for testing the CLI viewer")
    [Line 17] 
    [Line 18] if __name__ == "__main__":
    [Line 19]     main()
--- End of File ---

Currently at line 10
> back
[Line 9]     if len(sys.argv) > 1:
> quit
Goodbye!
```

## Features

- Displays one line at a time with line numbers
- Navigate forward and backward through the file
- Jump to any line number
- View the entire file with the current line highlighted (using `full` command)
- Color-coded highlighting for easy identification of current position
- Handles files up to 10,000 lines
- Strips trailing newlines for clean display

## Files Included

- `pyviewer.c` - Source code
- `pyviewer` - Compiled binary (Linux)
- `sample.py` - Sample Python file for testing
