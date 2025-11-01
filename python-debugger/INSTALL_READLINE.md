# Installing Readline Library

## Quick Fix

Your system needs the readline development library. Install it based on your Linux distribution:

### Fedora / RHEL / CentOS / Rocky Linux
```bash
sudo dnf install readline-devel
# or on older versions:
sudo yum install readline-devel
```

### Debian / Ubuntu
```bash
sudo apt-get install libreadline-dev
```

### Arch Linux
```bash
sudo pacman -S readline
```

### Alpine Linux
```bash
sudo apk add readline-dev
```

### openSUSE
```bash
sudo zypper install readline-devel
```

---

## After Installing

Then compile with:
```bash
gcc -o traceviewer traceviewer.c -Wall -O2 -lreadline
```

Or use the provided Makefile:
```bash
make
```

---

## Verify Installation

Check if readline is installed:
```bash
# Check for header files
ls /usr/include/readline/

# Check for library
ldconfig -p | grep readline
```

---

## If You Can't Install Readline

If you don't have sudo access or can't install readline, you can build a version without the advanced CLI features (no history, no tab completion, but still works):

```bash
# Use the basic version
gcc -o traceviewer traceviewer.c -Wall -O2 -DNO_READLINE
```

This will disable readline features but the debugger will still work with basic input.
