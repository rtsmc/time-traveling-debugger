# 🚀 Quick Start Guide

## Step 1: Extract

```bash
unzip python-debugger.zip
cd python-debugger/
```

---

## Step 2: Build

### Option A: Use Makefile (Recommended)
```bash
make
```

The Makefile will:
- ✅ Auto-detect if readline is available
- ✅ Build with readline features if available
- ✅ Fall back to basic version if not available
- ✅ Show clear warnings if dependencies are missing

### Option B: Manual Build

**If you have readline installed:**
```bash
gcc -o traceviewer traceviewer.c -Wall -O2 -lreadline
python3 setup.py build_ext --inplace
```

**If you DON'T have readline (basic version):**
```bash
gcc -o traceviewer traceviewer.c -Wall -O2 -DNO_READLINE
python3 setup.py build_ext --inplace
```

---

## Step 3: Install Readline (If Needed)

If build fails with "readline/readline.h: No such file or directory":

### Fedora / RHEL / CentOS / Rocky Linux
```bash
sudo dnf install readline-devel
```

### Debian / Ubuntu
```bash
sudo apt-get install libreadline-dev
```

### Arch Linux
```bash
sudo pacman -S readline
```

### Then rebuild:
```bash
make
```

See **INSTALL_READLINE.md** for more details.

---

## Step 4: Run!

### Pre-Execution Debugging
```bash
python3 idebug.py test_import_main.py

> b test_import_main.py 14     # Set breakpoint
> show test_import_main.py     # See file with ⚫ markers
> run                          # Execute!
```

### Post-Execution Analysis
```bash
./traceviewer trace.log

[1/50] > b test_import_main.py 14    # Set breakpoint
[1/50] > show                        # See file with ⚫ markers
[1/50] > c                           # Continue to breakpoint
[13/50] > n                          # Next step
[14/50] > ⏎                          # Press Enter - repeats!
```

---

## 🎯 Key Features

### With Readline (Full Features)
- ✅ Command history (↑/↓ arrows)
- ✅ Line editing (←/→ arrows)
- ✅ Tab completion
- ✅ Enter aliasing (repeat last command)
- ✅ All other features

### Without Readline (Basic Version)
- ✅ All core debugging features work
- ✅ Enter aliasing (repeat last command)
- ⚠️ No arrow key navigation
- ⚠️ No tab completion

**Recommendation:** Install readline for the best experience!

---

## 📊 Build Options

```bash
make              # Build everything (auto-detect readline)
make traceviewer  # Build only traceviewer
make cdebugger    # Build only Python extension
make basic        # Force build without readline
make clean        # Clean build files
make rebuild      # Clean and rebuild
make test         # Build and run test
make help         # Show all options
```

---

## ✅ Verify Installation

```bash
# Check traceviewer built successfully
./traceviewer --help 2>&1 | head -1

# Check Python extension
python3 -c "import cdebugger; print('✓ cdebugger imported')"

# Run quick test
make test
```

---

## 🐛 Troubleshooting

### Problem: readline/readline.h not found
**Solution:** Install readline-devel or build without it:
```bash
# Install readline (preferred)
sudo dnf install readline-devel   # Fedora/RHEL
sudo apt-get install libreadline-dev  # Debian/Ubuntu

# OR build basic version
make basic
```

### Problem: Python.h not found
**Solution:** Install Python development headers:
```bash
sudo dnf install python3-devel    # Fedora/RHEL
sudo apt-get install python3-dev  # Debian/Ubuntu
```

### Problem: Permission denied when running
**Solution:** Make executable:
```bash
chmod +x traceviewer
```

---

## 📚 Next Steps

1. **Learn the basics:** Read `README.md`
2. **See new features:** Read `CLI_ENHANCEMENTS.md`
3. **Quick keyboard reference:** Read `QUICK_REFERENCE_CARD.md`
4. **Visual examples:** Read `VISUAL_BEFORE_AFTER.md`

---

## 💡 Pro Tips

- Use `make` instead of manual compilation
- Install readline for the best experience
- Press `Tab` twice to see all commands
- Press `↑` to recall previous commands
- Press Enter on empty line to repeat last command

**You're all set! Happy debugging!** 🎉
