# CLI Consistency & Visual Breakpoints Update

## ✨ What Changed

### 1. **Command Consistency: `view` → `show`**
Post-CLI now matches Pre-CLI command naming!

**Before:**
```
Pre-CLI:  show <file>   ← Show file with line numbers
Post-CLI: view          ← Different command name!
```

**After:**
```
Pre-CLI:  show <file>   ← Show file with line numbers
Post-CLI: show          ← Now consistent! ✅
```

---

### 2. **Visual Breakpoint Markers**
Post-CLI now shows breakpoint markers just like Pre-CLI!

**Before (Post-CLI):**
```
[Line  13] # Test 1: Calculate sum
[Line  14] numbers = [1, 2, 3, 4, 5]
[Line  15] result = test_helper.calculate_sum(numbers)
```
*No indication of where breakpoints are!*

**After (Post-CLI):**
```
   13 | # Test 1: Calculate sum
⚫  14 | numbers = [1, 2, 3, 4, 5]      ← Breakpoint marker!
   15 | result = test_helper.calculate_sum(numbers)
```
*Red dot (⚫) shows breakpoint location!*

---

## 🎨 Visual Examples

### Pre-CLI Format (Already Had This)
```bash
$ python3 idebug.py test_import_main.py
> b test_import_main.py 14
✓ Breakpoint set at test_import_main.py:14
> show test_import_main.py

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_import_main.py
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

     12 | 
     13 |     # Test 1: Calculate sum
⚫   14 |     numbers = [1, 2, 3, 4, 5]  # Set breakpoint here
     15 |     result = test_helper.calculate_sum(numbers)
     16 |     print(f"Sum of {numbers} = {result}")
```

### Post-CLI Format (NOW MATCHES!)
```bash
$ ./traceviewer trace.log
[1/50] > b test_import_main.py 14
✓ Breakpoint set at test_import_main.py:14
[1/50] > show

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_import_main.py
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

     12 | 
     13 |     # Test 1: Calculate sum
⚫   14 |     numbers = [1, 2, 3, 4, 5]  # Set breakpoint here
     15 |     result = test_helper.calculate_sum(numbers)
     16 |     print(f"Sum of {numbers} = {result}")

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Total lines: 31
Breakpoints: 1 in this file
Currently at: [Execution #0] Line 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🎯 Multiple Breakpoints Example

```bash
[1/50] > b test_import_main.py 14
✓ Breakpoint set at test_import_main.py:14
[1/50] > b test_import_main.py 19
✓ Breakpoint set at test_import_main.py:19
[1/50] > b test_import_main.py 23
✓ Breakpoint set at test_import_main.py:23
[1/50] > show

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_import_main.py
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

     12 |     
     13 |     # Test 1: Calculate sum
⚫   14 |     numbers = [1, 2, 3, 4, 5]          ← Breakpoint 1
     15 |     result = test_helper.calculate_sum(numbers)
     16 |     print(f"Sum of {numbers} = {result}")
     17 |     
     18 |     # Test 2: Calculate product
⚫   19 |     result = test_helper.calculate_product(numbers)  ← Breakpoint 2
     20 |     print(f"Product of {numbers} = {result}")
     21 |     
     22 |     # Test 3: Greeting
⚫   23 |     name = "Alice"                      ← Breakpoint 3
     24 |     greeting = test_helper.greet(name)
     25 |     print(greeting)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Total lines: 31
Breakpoints: 3 in this file
Currently at: [Execution #0] Line 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🔥 Current Line + Breakpoint Together

When you navigate to a breakpoint, you'll see BOTH indicators:

```bash
[1/50] > :13                  # Jump to execution 13 (line 14)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
File: test_import_main.py
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

     12 |     
     13 |     # Test 1: Calculate sum
⚫   14 |     numbers = [1, 2, 3, 4, 5]      ← Green highlight + Red dot!
     15 |     result = test_helper.calculate_sum(numbers)
     16 |     print(f"Sum of {numbers} = {result}")
```
*Line 14 shows both:*
- ⚫ Red dot = Breakpoint set
- Green background = Current execution position

---

## 📋 Complete Command Comparison

### Pre-CLI Commands (idebug.py)
```
break <file> <line>  - Set breakpoint (short: b)
list                 - List all breakpoints
clear [num]          - Clear breakpoint(s)
show <file>          - Show file with line numbers
run                  - Start execution (short: r)
help                 - Show help
quit or q            - Exit
```

### Post-CLI Commands (traceviewer) - NOW CONSISTENT!
```
Navigation:
  n              - Next execution step
  back           - Previous execution step
  :<number>      - Jump to execution (e.g., :5)

Breakpoints:
  b <file> <line> - Set breakpoint ✅ Same as pre-CLI
  list           - List all breakpoints ✅ Same as pre-CLI
  c              - Continue to next breakpoint
  rc             - Reverse continue to previous breakpoint

Analysis:
  show           - Show file with line numbers ✅ RENAMED from 'view'
  summary        - Show trace summary
  find <var>     - Search for variable usage
  jump <line>    - Jump to first occurrence of source line

Other:
  help           - Show help ✅ Same as pre-CLI
  quit or q      - Exit ✅ Same as pre-CLI
```

---

## 💡 Benefits

### 1. **Consistent User Experience**
- Same command names between Pre-CLI and Post-CLI
- No need to remember different commands
- `show` works in both!

### 2. **Visual Clarity**
- Immediately see where breakpoints are set
- Red dots (⚫) stand out clearly
- No confusion about breakpoint locations

### 3. **Better Debugging Workflow**
```bash
# Set breakpoints
b test.py 14
b test.py 19

# Visually verify they're set correctly
show              ← See the red dots!

# Continue to next breakpoint
c                 ← Hit line 14

# Check source again with both markers
show              ← See current line (green) + breakpoint (red dot)
```

---

## 🔧 Technical Implementation

### Changes Made:

1. **Renamed Function**
   - `view_file()` → `show_file()`

2. **Added Breakpoint Detection Loop**
   ```c
   for (int i = 0; i < viewer->breakpoint_count; i++) {
       if (viewer->breakpoints[i].line_number == line_num &&
           filenames_match(viewer->breakpoints[i].filename, filename)) {
           has_breakpoint = 1;
           break;
       }
   }
   ```

3. **Updated Line Format**
   ```c
   // Before: "    [Line  14] code"
   // After:  "⚫   14 | code"  (if breakpoint)
   //         "    14 | code"  (if no breakpoint)
   ```

4. **Updated Command Parser**
   - Changed `strcmp(cmd, "view")` to `strcmp(cmd, "show")`

5. **Updated Help Text**
   - Changed "view" to "show" in documentation

---

## ✅ Summary

**What you requested:** Make post-CLI commands match pre-CLI, and show breakpoints visually.

**What was delivered:**
- ✅ Renamed `view` to `show` for consistency
- ✅ Added visual breakpoint markers (⚫) 
- ✅ Format matches pre-CLI exactly
- ✅ Shows breakpoint count in footer
- ✅ Works with multiple breakpoints
- ✅ Combines with current line highlighting

**Result:** Perfect consistency between Pre-CLI and Post-CLI! 🎉
