#include <Python.h>
#include <frameobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Trace file handle
static FILE *trace_file = NULL;
static long execution_counter = 0;
static int is_tracing = 0;

// Configuration
static char *trace_filename = NULL;

// Helper function to write variable values to file
static void
write_variables(FILE *fp, PyObject *locals)
{
    if (locals == NULL || !PyDict_Check(locals)) {
        fprintf(fp, "|||");
        return;
    }

    fprintf(fp, "|||");
    
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    int first = 1;
    
    while (PyDict_Next(locals, &pos, &key, &value)) {
        if (!first) {
            fprintf(fp, ";");
        }
        first = 0;
        
        // Get variable name
        const char *var_name = PyUnicode_AsUTF8(key);
        if (var_name == NULL) {
            PyErr_Clear();
            continue;
        }
        
        // Get variable value as string representation
        PyObject *repr = PyObject_Repr(value);
        const char *var_value = "";
        if (repr != NULL) {
            var_value = PyUnicode_AsUTF8(repr);
            if (var_value == NULL) {
                PyErr_Clear();
                var_value = "<error>";
            }
        }
        
        fprintf(fp, "%s=%s", var_name, var_value);
        
        Py_XDECREF(repr);
    }
}

// Helper function to get the source line
static char*
get_source_line(const char *filename, int lineno)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return strdup("<unavailable>");
    }
    
    char *line = NULL;
    size_t len = 0;
    int current_line = 0;
    
    while (getline(&line, &len, fp) != -1) {
        current_line++;
        if (current_line == lineno) {
            // Remove trailing newline
            size_t line_len = strlen(line);
            if (line_len > 0 && line[line_len - 1] == '\n') {
                line[line_len - 1] = '\0';
            }
            fclose(fp);
            return line;
        }
    }
    
    fclose(fp);
    free(line);
    return strdup("<unavailable>");
}

// Main trace function
static int
trace_callback(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    if (!is_tracing || trace_file == NULL) {
        return 0;
    }
    
    // Only trace LINE events for execution flow
    if (what != PyTrace_LINE) {
        return 0;
    }
    
    // Get filename and line number
    PyObject *filename_obj = frame->f_code->co_filename;
    const char *filename = PyUnicode_AsUTF8(filename_obj);
    int lineno = PyFrame_GetLineNumber(frame);
    
    if (filename == NULL) {
        PyErr_Clear();
        return 0;
    }
    
    // Skip tracing of debugger internals and standard library
    if (strstr(filename, "site-packages") != NULL ||
        strstr(filename, "cdebugger") != NULL) {
        return 0;
    }
    
    // Get the source line
    char *source_line = get_source_line(filename, lineno);
    
    // Get local variables
    PyObject *locals = PyFrame_GetLocals(frame);
    
    // Write trace entry: execution_order|||filename|||line_number|||code|||variables
    fprintf(trace_file, "%ld|||%s|||%d|||%s",
            execution_counter++, filename, lineno, source_line);
    
    // Write variables
    write_variables(trace_file, locals);
    fprintf(trace_file, "\n");
    fflush(trace_file);
    
    // Cleanup
    free(source_line);
    Py_XDECREF(locals);
    
    return 0;
}

// Start tracing
static PyObject*
start_trace(PyObject *self, PyObject *args)
{
    const char *filename = NULL;
    
    if (!PyArg_ParseTuple(args, "s", &filename)) {
        return NULL;
    }
    
    if (is_tracing) {
        PyErr_SetString(PyExc_RuntimeError, "Tracing already active");
        return NULL;
    }
    
    // Open trace file
    trace_file = fopen(filename, "w");
    if (trace_file == NULL) {
        PyErr_SetString(PyExc_IOError, "Cannot open trace file");
        return NULL;
    }
    
    // Write header
    fprintf(trace_file, "EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES\n");
    fflush(trace_file);
    
    // Store filename
    trace_filename = strdup(filename);
    
    // Reset counter
    execution_counter = 0;
    is_tracing = 1;
    
    // Set the trace function
    PyEval_SetTrace((Py_tracefunc)trace_callback, NULL);
    
    Py_RETURN_NONE;
}

// Stop tracing
static PyObject*
stop_trace(PyObject *self, PyObject *args)
{
    if (!is_tracing) {
        Py_RETURN_NONE;
    }
    
    // Disable tracing
    PyEval_SetTrace(NULL, NULL);
    is_tracing = 0;
    
    // Close file
    if (trace_file != NULL) {
        fclose(trace_file);
        trace_file = NULL;
    }
    
    // Free filename
    if (trace_filename != NULL) {
        free(trace_filename);
        trace_filename = NULL;
    }
    
    Py_RETURN_NONE;
}

// Set breakpoint (for future use)
static PyObject*
set_breakpoint(PyObject *self, PyObject *args)
{
    const char *filename;
    int lineno;
    
    if (!PyArg_ParseTuple(args, "si", &filename, &lineno)) {
        return NULL;
    }
    
    // TODO: Implement breakpoint storage
    printf("Breakpoint set at %s:%d\n", filename, lineno);
    
    Py_RETURN_NONE;
}

// Get trace filename
static PyObject*
get_trace_filename(PyObject *self, PyObject *args)
{
    if (trace_filename == NULL) {
        Py_RETURN_NONE;
    }
    return PyUnicode_FromString(trace_filename);
}

// Module methods
static PyMethodDef DebuggerMethods[] = {
    {"start_trace", start_trace, METH_VARARGS, 
     "Start tracing to file"},
    {"stop_trace", stop_trace, METH_NOARGS, 
     "Stop tracing"},
    {"set_breakpoint", set_breakpoint, METH_VARARGS, 
     "Set a breakpoint at file:line"},
    {"get_trace_filename", get_trace_filename, METH_NOARGS,
     "Get the current trace filename"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef debuggermodule = {
    PyModuleDef_HEAD_INIT,
    "cdebugger",
    "C-based Python debugger with trace file generation",
    -1,
    DebuggerMethods
};

// Module initialization
PyMODINIT_FUNC
PyInit_cdebugger(void)
{
    return PyModule_Create(&debuggermodule);
}
