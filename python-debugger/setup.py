import os
import subprocess
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext as _build_ext


def compile_traceviewer():
    """Compile traceviewer.c -> traceviewer binary (requires -lreadline)."""
    src = 'traceviewer.c'
    out = 'traceviewer'

    if not os.path.exists(src):
        print(f"setup.py: {src} not found, skipping traceviewer build", file=sys.stderr)
        return

    cmd = ['gcc', '-o', out, src, '-Wall', '-O2', '-lreadline']
    print('gcc -o traceviewer traceviewer.c -Wall -O2 -lreadline')
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.returncode != 0:
        print(f"setup.py: traceviewer build failed:\n{result.stderr}", file=sys.stderr)
        if 'readline' in result.stderr.lower():
            print(
                "setup.py: readline not found. Install it first:\n"
                "  Fedora/RHEL:   sudo dnf install readline-devel\n"
                "  Debian/Ubuntu: sudo apt-get install libreadline-dev\n"
                "  macOS:         brew install readline",
                file=sys.stderr,
            )
    else:
        try:
            os.chmod(out, 0o755)
        except OSError:
            pass
        # Print any non-error warnings so they're still visible
        for line in result.stderr.splitlines():
            if 'warning:' in line or 'note:' in line:
                pass  # suppress noise; remove this block to restore them
        print(f"traceviewer built successfully")


class build_ext(_build_ext):
    """Custom build_ext that also compiles the traceviewer binary."""

    def run(self):
        compile_traceviewer()
        super().run()


module = Extension(
    'cdebugger',
    sources=['debugger.c'],
    extra_compile_args=['-O3'],
)

setup(
    name='CDebugger',
    version='1.0',
    description='C-based Python debugger with trace file generation',
    ext_modules=[module],
    cmdclass={'build_ext': build_ext},
)
