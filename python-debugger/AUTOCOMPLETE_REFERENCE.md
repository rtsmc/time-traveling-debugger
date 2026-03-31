# Autocomplete Quick Reference

## 🎯 Tab Completion Features

Both CLIs now have **full readline autocomplete** support!

---

## Pre-Execution CLI (Python - idebug.py)

### Command Completion
Type a partial command and press **TAB**:
```
> b<TAB>          → completes to "break" or shows "b", "back"
> l<TAB>          → completes to "list"
> r<TAB>          → completes to "run"
> s<TAB>          → completes to "show"
```

### Filename Completion
After `break`, `b`, or `show` commands, press **TAB**:
```
> break te<TAB>   → completes to "test_clean_simple.py", "test_variables.py", etc.
> show tes<TAB>   → shows all test*.py files
```

### Example Session
```bash
$ python3 idebug.py test_clean_simple.py

> b<TAB>                    # Shows: break, b, back
> break test<TAB>           # Shows all test*.py files
> break test_clean_simple.py 5
✓ Breakpoint set at test_clean_simple.py:5

> l<TAB>                    # Completes to "list"
> list
Breakpoints:
  1. test_clean_simple.py:5

> r<TAB>                    # Completes to "run"
> run
```

---

## Post-Execution CLI (C - traceviewer)

### Command Completion
Type a partial command and press **TAB**:
```
> n<TAB>          → completes to "n" or "next"
> b<TAB>          → shows "b", "break", "back"
> c<TAB>          → shows "c", "clear", "continue"
> s<TAB>          → shows "show", "summary"
```

### Filename Completion
After `b`, `break`, or `show` commands, press **TAB**:
```
> b test<TAB>     → completes to test*.py files
> show te<TAB>    → completes to test*.py files
```

### Example Session
```bash
[1/20] > n<TAB>             # Completes to "next" or shows "n"
[1/20] > next               # Step forward

[2/20] > b<TAB>             # Shows: b, break, back
[2/20] > b test<TAB>        # Shows all test*.py files
[2/20] > b test_clean_simple.py 10
✓ Breakpoint set at test_clean_simple.py:10

[2/20] > c<TAB>             # Shows: c, clear, continue
[2/20] > c                  # Continue to breakpoint
⚫ BREAKPOINT HIT
```

---

## 🔍 How Autocomplete Works

### Command Completion
- Works at the **beginning** of the line
- Completes to valid debugger commands
- Shows multiple options if ambiguous

### Filename Completion
- Only works after specific commands: `b`, `break`, `show`
- Shows only **.py** files in current directory
- Matches files from the trace (post-CLI only)
- Smart filtering based on what you've typed

---

## 📝 Command History

Both CLIs support readline history:

### Navigation
- **↑** (Up Arrow) - Previous command
- **↓** (Down Arrow) - Next command
- **Ctrl+R** - Reverse search history

### History Files
- Pre-CLI: `~/.idebug_history`
- Post-CLI: `~/.traceviewer_history`

### Empty Line Behavior
Press **Enter** on an empty line to repeat the last command (useful for stepping through execution with `n`).

---

## 💡 Pro Tips

### 1. Quick Stepping
```
[1/20] > n    # Step forward
[2/20] >      # Press Enter to repeat 'n'
[3/20] >      # Press Enter again
[4/20] >      # Keep pressing Enter to step through
```

### 2. Fast Breakpoint Setting
```
> b te<TAB>   # Shows test files
> b test_clean_simple.py 5
✓ Breakpoint set
```

### 3. Tab for Help
When you forget a command, press **TAB** at the start of the line to see all available commands!

---

## 🚀 Keyboard Shortcuts

All standard readline shortcuts work:

### Editing
- **Ctrl+A** - Move to start of line
- **Ctrl+E** - Move to end of line
- **Ctrl+K** - Delete from cursor to end
- **Ctrl+U** - Delete entire line
- **Ctrl+W** - Delete word before cursor

### History
- **Ctrl+R** - Reverse search history
- **Ctrl+P** - Previous command (same as ↑)
- **Ctrl+N** - Next command (same as ↓)

### Completion
- **TAB** - Trigger autocomplete
- **TAB TAB** - Show all possible completions

---

## 🛠️ Troubleshooting

### "Tab completion not enabled" message?
You're missing readline! Install it:
```bash
# Debian/Ubuntu
sudo apt-get install libreadline-dev

# Fedora/RHEL
sudo dnf install readline-devel

# Then rebuild
make clean && make all
```

### Autocomplete not showing filenames?
Make sure you're using it after the right commands:
- ✅ Works: `b <TAB>`, `break <TAB>`, `show <TAB>`
- ❌ Doesn't work: `n <TAB>`, `list <TAB>`, `c <TAB>`

---

## Summary

✅ **TAB** to autocomplete commands
✅ **TAB** to autocomplete .py filenames after `b`/`break`/`show`
✅ **↑/↓** to navigate history
✅ **Enter** on empty line to repeat last command
✅ All standard readline shortcuts work

Enjoy your enhanced debugging experience! 🎉
