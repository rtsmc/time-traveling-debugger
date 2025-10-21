from setuptools import setup, Extension

module = Extension('cdebugger',
                   sources=['debugger.c'],
                   extra_compile_args=['-O3'])

setup(name='CDebugger',
      version='1.0',
      description='C-based Python debugger with trace file generation',
      ext_modules=[module])
