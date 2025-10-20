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
- **`quit`** or **`q`** - Exit the viewer

## Example Session

```
$ ./pyviewer sample.py
Python File Viewer - Loaded 19 lines
Commands: 'n' (next), 'back' (previous), ':<number>' (goto line)
---
[Line 1] import sys
> n
[Line 2] import os
> n
[Line 3] 
> :10
[Line 10]         name = sys.argv[1]
> back
[Line 9]     if len(sys.argv) > 1:
> quit
Goodbye!
```

## Features

- Displays one line at a time with line numbers
- Navigate forward and backward through the file
- Jump to any line number
- Handles files up to 10,000 lines
- Strips trailing newlines for clean display

## Files Included

- `pyviewer.c` - Source code
- `pyviewer` - Compiled binary (Linux)
- `sample.py` - Sample Python file for testing
