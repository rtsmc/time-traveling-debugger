# Quick Rebuild & Test Guide

## ✅ Ready to Test Your Updated Debugger!

All files have been updated to make readline mandatory. Here's how to rebuild and test:

---

## 🔧 Step 1: Ensure Readline is Installed

### Check if you have readline:
```bash
# Try to compile a test program
echo '#include <readline/readline.h>
int main(){return 0;}' | gcc -x c - -lreadline 2>&1
```

If you see no errors, readline is installed! ✅

### If not installed:
```bash
# Debian/Ubuntu
sudo apt-get update
sudo apt-get install libreadline-dev

# Fedora/RHEL/CentOS
sudo dnf install readline-devel

# Arch Linux
sudo pacman -S readline

# macOS
brew install readline
```

---

## 🏗️ Step 2: Rebuild Everything

```bash
# Go to your project directory
cd /path/to/your/python-debugger

# Clean old builds
make clean

# Build with mandatory readline
make all
```

### Expected Output:
```
Checking dependencies...
✓ gcc found
✓ python3 found
✓ python3-dev found
✓ readline found                    ← This should be present!

Building traceviewer (with readline support)...
✓ traceviewer built successfully

Building cdebugger Python C extension...
✓ cdebugger extension built successfully

Verifying build...
✓ traceviewer is ready
✓ cdebugger extension is ready
✓ cdebugger module imports correctly

Build Complete! 🎉
```

### If Readline is Missing:
You'll see:
```
✗ readline not found (required)
Install readline development library:
  Debian/Ubuntu: sudo apt-get install libreadline-dev
  ...
```

Just install readline and rebuild!

---

## 🧪 Step 3: Test Autocomplete in Pre-CLI

Test the Python CLI (runs before execution):

```bash
python3 idebug.py test_clean_simple.py
```

### Try These Tests:

1. **Command completion:**
   ```
   > b<TAB>           # Should show: break, b, back
   > bre<TAB>         # Should complete to "break"
   > r<TAB>           # Should complete to "run"
   ```

2. **Filename completion:**
   ```
   > break <TAB>      # Should show all .py files
   > break test<TAB>  # Should show test*.py files
   ```

3. **Set a breakpoint and run:**
   ```
   > break test_clean_simple.py 5
   > list
   > run
   ```

---

## 🧪 Step 4: Test Autocomplete in Post-CLI

The post-CLI (C traceviewer) launches automatically after execution completes.

### Try These Tests:

1. **Command completion:**
   ```
   [1/20] > n<TAB>    # Should show: n, next
   [1/20] > b<TAB>    # Should show: b, break, back
   [1/20] > s<TAB>    # Should show: show, summary
   ```

2. **Filename completion:**
   ```
   [1/20] > show <TAB>        # Should show .py files
   [1/20] > b test<TAB>       # Should show test*.py files
   ```

3. **Step through execution:**
   ```
   [1/20] > n         # Next step
   [2/20] >           # Press Enter to repeat 'n'
   [3/20] >           # Keep pressing Enter
   ```

4. **Set breakpoint and continue:**
   ```
   [1/20] > b test_clean_simple.py 10
   [1/20] > c         # Continue to breakpoint
   ⚫ BREAKPOINT HIT
   ```

---

## ✨ Step 5: Verify Tab Completion Message

When the post-CLI starts, you should see:
```
✓ Tab completion enabled (press TAB to autocomplete commands and .py files)
```

**You should NOT see:**
```
⚠ Tab completion not available
```

If you see the warning, readline wasn't properly installed.

---

## 🎯 Quick Feature Checklist

Test these features to confirm everything works:

### Pre-CLI (Python)
- [ ] `TAB` completes commands
- [ ] `TAB` after `break` shows .py files  
- [ ] `↑` navigates to previous command
- [ ] History saved in `~/.idebug_history`

### Post-CLI (C Traceviewer)
- [ ] `TAB` completes commands
- [ ] `TAB` after `b` or `show` shows .py files
- [ ] `↑` navigates to previous command
- [ ] History saved in `~/.traceviewer_history`
- [ ] Empty line repeats last command

---

## 🐛 Troubleshooting

### Problem: Build fails with "readline not found"
**Solution:** Install readline:
```bash
# Debian/Ubuntu
sudo apt-get install libreadline-dev

# Fedora/RHEL
sudo dnf install readline-devel
```

### Problem: TAB doesn't do anything
**Check:**
1. Are you in a real terminal? (not piped input)
2. Did the build show "✓ readline found"?
3. Do you see "✓ Tab completion enabled"?

### Problem: "command not found" errors
**Solution:** Make sure you're in the project directory:
```bash
cd /path/to/python-debugger
./traceviewer trace.log  # Should work
```

### Problem: Autocomplete works in pre-CLI but not post-CLI
**Check:** 
```bash
ldd ./traceviewer | grep readline
```
Should show: `libreadline.so.8 => ...`

If not, rebuild:
```bash
make clean && make all
```

---

## 📚 Additional Testing

### Test with Different Files
```bash
# Try with different test files
python3 idebug.py test_variables.py
python3 idebug.py test_clean_loops.py
python3 idebug.py test_import_main.py
```

### Test History Persistence
```bash
# Run debugger
python3 idebug.py test_clean_simple.py
> break test_clean_simple.py 5
> run
[1/20] > n
[2/20] > n
[3/20] > q

# Run again - history should be preserved
python3 idebug.py test_clean_simple.py
> <UP_ARROW>  # Should show "break test_clean_simple.py 5"
```

---

## 🎉 Success Indicators

You'll know everything is working when:

1. ✅ Build completes with "✓ readline found"
2. ✅ Both CLIs show "✓ Tab completion enabled"
3. ✅ `TAB` completes commands in both CLIs
4. ✅ `TAB` shows .py files after `b`, `break`, `show`
5. ✅ Arrow keys navigate command history
6. ✅ No "tab completion not available" warnings

---

## 📖 Next Steps

- Read `AUTOCOMPLETE_REFERENCE.md` for full feature list
- Check `TECHNICAL_DETAILS.md` for implementation details
- See `CHANGES_SUMMARY.md` for overview of all changes

---

## 🚀 Quick Reference

### Essential Commands

**Pre-CLI:**
- `break <file> <line>` or `b <file> <line>` - Set breakpoint
- `list` or `l` - List breakpoints
- `run` or `r` - Start execution
- `quit` or `q` - Exit

**Post-CLI:**
- `n` or `next` - Next step
- `back` - Previous step
- `b <file> <line>` - Set breakpoint
- `c` - Continue to breakpoint
- `show` - Show current file
- `q` - Quit

All commands support **TAB** completion! 🎯

---

Happy debugging with your enhanced autocomplete! 🐛✨
