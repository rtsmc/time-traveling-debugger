# Python Version Compatibility

## ✅ Supported Python Versions

This debugger now works with:
- **Python 3.9** ✓
- **Python 3.10** ✓
- **Python 3.11** ✓
- **Python 3.12** ✓
- **Python 3.13** ✓ (expected)

## 🔧 Technical Details

The code uses compatibility macros to support both old and new Python C API:

### Python 3.9 - 3.10 (Old API)
```c
// Direct struct access
frame->f_code
frame->f_locals
```

### Python 3.11+ (New API)
```c
// Accessor functions
PyFrame_GetCode(frame)
PyFrame_GetLocals(frame)
```

### Our Solution
```c
#if PY_VERSION_HEX < 0x030B0000
  // Use old API for Python 3.9-3.10
  #define COMPAT_PyFrame_GetCode(frame) ((frame)->f_code)
  #define COMPAT_PyFrame_GetLocals(frame) ((frame)->f_locals)
#else
  // Use new API for Python 3.11+
  #define COMPAT_PyFrame_GetCode(frame) PyFrame_GetCode(frame)
  #define COMPAT_PyFrame_GetLocals(frame) PyFrame_GetLocals(frame)
#endif
```

## 🚀 Building

The build process automatically detects your Python version:

```bash
python3 setup.py build_ext --inplace
```

You should see output like:
```
running build_ext
building 'cdebugger' extension
gcc ... -I/usr/include/python3.9 ...
```

The `-I/usr/include/python3.X` flag determines which Python version is used.

## ⚠️ Known Issues

None! The compatibility layer handles all differences transparently.

## 📝 Why This Matters

Python 3.11 changed the frame object structure from a public struct to an opaque type. This broke backward compatibility but improved:
- Internal optimization
- Future flexibility
- Better encapsulation

Our compatibility layer ensures the debugger works on both old and new Python versions without any user action required.
