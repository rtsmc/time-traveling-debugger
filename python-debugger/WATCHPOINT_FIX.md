# Watchpoint Write Detection Fix

## Problem Identified

The watchpoint implementation had TWO critical bugs:

### Bug #1: Comparing Entry With Itself
Write operations were not being detected because we were comparing an entry with itself.

### Bug #2: Stopping One Line Too Late  
Even after fixing bug #1, watchpoints stopped one line AFTER the write, not at the line where the write occurred.

## Bug #1: Root Cause

In the `continue_to_breakpoint()` function, the logic was:

```c
for (int i = viewer->current_entry + 1; i < viewer->entry_count; i++) {
    // BUG: Update prev_vars to entry i
    update_variable_state(viewer, i);
    
    // Then check if watchpoint triggered at entry i
    if (check_watchpoint_triggered(viewer, i, ...)) {
        // This compares prev_vars (now entry i) with entry i
        // Result: No change detected!
    }
}
```

**Fix:** Initialize `prev_vars` before the loop and check before updating:
```c
update_variable_state(viewer, viewer->current_entry);

for (int i = viewer->current_entry + 1; ...) {
    if (check_watchpoint_triggered(viewer, i, ...)) { ... }
    update_variable_state(viewer, i);
}
```

## Bug #2: Root Cause

After fixing bug #1, writes were detected but stopped at the wrong line. Here's why:

In Python's trace model, variables show the state AFTER each line executes:
- Entry 8: Line 23 `x = 10` executes → x is now in variables
- Entry 9: Line 24 `y = 20` → x is still there (unchanged)

When we compared entry 9 vs entry 8:
- Entry 8: no x (before line 23 executed)
- Entry 9: x=10 (after line 23 executed)  
- We detected the change and stopped at entry 9 (line 24) ❌

But the write happened at line 23! We should stop at entry 8, not entry 9.

**The Fix:** When detecting a write at entry `i`, stop at entry `i-1`:

```c
if (check_watchpoint_triggered(viewer, i, ...)) {
    int stop_at = i;
    if (strstr(trigger_type, "write") != NULL) {
        // Write detected - stop at previous line where write occurred
        if (i > 0) {
            stop_at = i - 1;
        }
    }
    viewer->current_entry = stop_at;
    // ...
}
```

**Important:** We only do this for writes, not reads. Read operations happen at the current line, so we stop at entry `i` for reads.

## Complete Fixed Code

### Forward Continue

```c
void continue_to_breakpoint(TraceViewer *viewer) {
    // ... checks ...
    
    // FIX 1: Initialize prev_vars to current_entry BEFORE loop
    update_variable_state(viewer, viewer->current_entry);
    
    for (int i = viewer->current_entry + 1; i < viewer->entry_count; i++) {
        // FIX 1: Check watchpoint FIRST (compares current_entry with i)
        if (check_watchpoint_triggered(viewer, i, ...)) {
            // FIX 2: For writes, stop at i-1 where write happened
            int stop_at = i;
            if (strstr(trigger_type, "write") != NULL) {
                if (i > 0) stop_at = i - 1;
            }
            viewer->current_entry = stop_at;
            return;
        }
        
        if (is_at_breakpoint(viewer, i)) { return; }
        
        // FIX 1: Update state AFTER checking, for next iteration
        update_variable_state(viewer, i);
    }
}
```

### Reverse Continue

Applied the same fixes for backward navigation.

## Verification

### Test Case

```python
def main():
    x = 10          # Line 23 - Write to x
    y = 20          # Line 24 - Write to y  
    result = x + y  # Line 25 - Read x and y
    x = 30          # Line 26 - Write to x again
```

**Before Both Fixes:**
```
> w x
> c
# Skipped line 23 (x = 10) completely
# Stopped at line 25 (result = x + y) - only the read!
```

**After Fix #1 Only:**
```
> w x  
> c
# Stopped at line 24 (y = 20) - one line too late!
```

**After Both Fixes:**
```
> w x
> c
👁 WATCHPOINT HIT
Variable 'x' was write
Line 23: x = 10    # ← Correctly stops at the WRITE! ✓

> c
👁 WATCHPOINT HIT  
Variable 'x' was read
Line 25: result = x + y    # ← Then the read ✓

> c
👁 WATCHPOINT HIT
Variable 'x' was write
Line 26: x = 30    # ← Second write! ✓
```

## Impact

### Fixed
✅ Write watchpoints (`ww`) now detect writes AND stop at correct line
✅ Normal watchpoints (`w`) detect both reads AND writes at correct lines
✅ Read watchpoints (`rw`) continue to work correctly at current line
✅ Variable changes tracked properly between execution steps
✅ Forward continue (`c`) works correctly
✅ Reverse continue (`rc`) works correctly

### Unchanged
✅ Breakpoint functionality unchanged
✅ All other features unaffected

## Testing Recommendations

Test with `test_watchpoint_simple.py`:

```bash
python3 idebug.py test_watchpoint_simple.py
> run

# In trace viewer  
> w x
> c    # Should stop at line with "x = 10" ✓
> c    # Should stop at line with "result = x + y" (read) ✓
> c    # Should stop at line with "x = 30" (second write) ✓
```

## Files Modified

- `traceviewer.c`:
  - `continue_to_breakpoint()` - Fixed state tracking + off-by-one
  - `reverse_continue_to_breakpoint()` - Fixed state tracking + off-by-one

**Lines changed:** ~40 lines modified

## Build Status

✅ Compiles without errors
✅ All existing tests pass  
✅ New behavior verified with test cases

## Conclusion

The watchpoint feature now correctly detects variable writes AND stops at the exact line where the write occurs. Users can reliably track when and where variables are assigned new values.
