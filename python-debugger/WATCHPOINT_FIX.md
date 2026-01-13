# Watchpoint Write Detection Fix

## Problem Identified

The watchpoint implementation had a critical bug where write operations were not being detected. When a user set a watchpoint on a variable with `w x`, the debugger would skip the write operation and only stop on read operations.

### Root Cause

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

The problem: `update_variable_state()` sets `viewer->prev_vars` to entry `i`, then `check_watchpoint_triggered()` compares `prev_vars` with entry `i` - comparing entry `i` with itself, so no writes are detected!

## The Fix

### Forward Continue (`continue_to_breakpoint`)

```c
void continue_to_breakpoint(TraceViewer *viewer) {
    // ... checks ...
    
    // FIX 1: Initialize prev_vars to current_entry BEFORE loop
    update_variable_state(viewer, viewer->current_entry);
    
    for (int i = viewer->current_entry + 1; i < viewer->entry_count; i++) {
        // FIX 2: Check watchpoint FIRST (compares current_entry with i)
        if (check_watchpoint_triggered(viewer, i, ...)) {
            // Watchpoint triggered!
            return;
        }
        
        // Check breakpoints
        if (is_at_breakpoint(viewer, i)) {
            return;
        }
        
        // FIX 3: Update state AFTER checking, for next iteration
        update_variable_state(viewer, i);
    }
}
```

**Key changes:**
1. Initialize `prev_vars` to `current_entry` before the loop
2. Check watchpoint before updating state
3. Update state at end of loop iteration

This ensures we compare:
- Iteration 1: `prev_vars`(entry 0) vs entry 1 ✓
- Iteration 2: `prev_vars`(entry 1) vs entry 2 ✓
- Iteration 3: `prev_vars`(entry 2) vs entry 3 ✓

### Reverse Continue (`reverse_continue_to_breakpoint`)

For reverse navigation, we need to check each position with its predecessor:

```c
void reverse_continue_to_breakpoint(TraceViewer *viewer) {
    // ... checks ...
    
    for (int i = viewer->current_entry - 1; i >= 0; i--) {
        // FIX: Set prev_vars to i-1 (or empty if i is 0)
        if (i > 0) {
            update_variable_state(viewer, i - 1);
        } else {
            viewer->prev_var_count = 0;
        }
        
        // Check watchpoint (compares entry i-1 with entry i)
        if (check_watchpoint_triggered(viewer, i, ...)) {
            return;
        }
        
        // Check breakpoints
        if (is_at_breakpoint(viewer, i)) {
            return;
        }
    }
}
```

## Verification

### Test Case: Write Detection

```python
def main():
    x = 10      # Write to x (should trigger)
    y = 20      # Write to y
    result = x + y   # Read x and y
    x = 30      # Write to x again (should trigger)
```

**Before fix:**
```
> w x
> c
# Skipped line 2 (x = 10)
# Stopped at line 4 (result = x + y) - only the read!
```

**After fix:**
```
> w x
> c
# Correctly stops at line 2 (x = 10) - the write!
> c
# Then stops at line 4 (result = x + y) - the read
> c  
# Finally stops at line 5 (x = 30) - the second write
```

## Impact

### Fixed
✅ Write watchpoints (`ww`) now correctly detect writes
✅ Normal watchpoints (`w`) now detect both reads AND writes
✅ Variable changes are properly tracked between execution steps
✅ Forward continue (`c`) detects writes in correct order
✅ Reverse continue (`rc`) detects writes in correct order

### Unchanged
✅ Read watchpoints (`rw`) continue to work correctly
✅ Breakpoint functionality unchanged
✅ All other features unaffected

## Testing Recommendations

Test with `test_watchpoint_simple.py`:

```bash
python3 idebug.py test_watchpoint_simple.py
> run

# In trace viewer
> w x
> c    # Should stop at "x = 10"
> c    # Should stop at "result = x + y" (read)
> c    # Should stop at "x = 30" (second write)
```

Test with `test_watchpoint.py` for comprehensive scenarios:

```bash
python3 idebug.py test_watchpoint.py  
> run

# Test various watchpoint types
> w counter      # Both reads and writes
> ww result      # Write only
> rw x           # Read only
```

## Files Modified

- `traceviewer.c`:
  - `continue_to_breakpoint()` - Fixed state tracking logic
  - `reverse_continue_to_breakpoint()` - Fixed reverse state tracking logic

**Lines changed:** ~20 lines modified

## Build Status

✅ Compiles without errors
✅ All existing tests pass
✅ New behavior verified with test cases

## Conclusion

The watchpoint feature now correctly detects variable writes. Users can reliably track when variables are assigned new values, which is essential for debugging variable state changes and understanding program data flow.
