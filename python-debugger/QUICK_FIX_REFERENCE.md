# Quick Fix Reference

## ❌ Your Problem
```
[Exec #0 - 1/50] > b test_import_main.py 13
✓ Breakpoint set at test_import_main.py:13
[Exec #0 - 1/50] > c
⚠ No more breakpoints ahead. Jumping to end of trace.
```

## ✅ The Solution

### Fix #1: Use Line 14, Not Line 13
```python
# test_import_main.py
Line 13:  # Test 1: Calculate sum          ← COMMENT (not traced!)
Line 14:  numbers = [1, 2, 3, 4, 5]         ← CODE (traced!) ✓
```

**Corrected Command:**
```bash
[Exec #0 - 1/50] > b test_import_main.py 14  # Use line 14!
✓ Breakpoint set at test_import_main.py:14
[Exec #0 - 1/50] > c
⚫ BREAKPOINT HIT  # ✅ Works!
[Execution #12] Line 14
```

### Fix #2: Jump Command Now 1-Based

**Before (Confusing):**
- Display: `[Exec #12 - 13/50]`
- Command: `:12` ← Off by one!

**After (Intuitive):**
- Display: `[Exec #12 - 13/50]`  
- Command: `:13` ← Matches display! ✓

---

## 🎯 Key Takeaway

**Only set breakpoints on EXECUTABLE lines:**
- ✅ Variable assignments
- ✅ Function calls
- ✅ Control flow (if/for/while)
- ❌ Comments
- ❌ Blank lines
- ❌ Docstrings

**Use `view` or `jump` commands to verify a line is executable before setting a breakpoint.**

---

## 🧪 Test It

```bash
# Test with correct line number
echo -e "b test_import_main.py 14\nc\nq" | ./traceviewer trace.log

# Test 1-based jump indexing  
echo -e ":13\nq" | ./traceviewer trace.log
```
