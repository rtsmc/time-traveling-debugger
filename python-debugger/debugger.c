#include <Python.h>
#include <frameobject.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Compatibility for Python 3.9-3.10 vs 3.11+
#if PY_VERSION_HEX < 0x030B0000
// Python 3.9-3.10: Use direct struct access
#define COMPAT_PyFrame_GetCode(frame) ((frame)->f_code)
#define COMPAT_PyFrame_GetLocals(frame) ((frame)->f_locals)
#define COMPAT_Py_XDECREF_Code(code) ((void)0)  // No-op for old API
#else
// Python 3.11+: Use accessor functions
#define COMPAT_PyFrame_GetCode(frame) PyFrame_GetCode(frame)
#define COMPAT_PyFrame_GetLocals(frame) PyFrame_GetLocals(frame)
#define COMPAT_Py_XDECREF_Code(code) Py_XDECREF(code)
#endif

// Breakpoint structure
typedef struct Breakpoint {
    char *filename;
    int lineno;
    int enabled;
    int hit_count;
    struct Breakpoint *next;
} Breakpoint;

// Trace file handle
static FILE *trace_file = NULL;
static long execution_counter = 0;
static int is_tracing = 0;
static int is_paused = 0;  // For breakpoint pausing
static int step_mode = 0;   // 0=continue, 1=step_next, 2=step_into

// Configuration
static char *trace_filename = NULL;
static Breakpoint *breakpoints = NULL;

// Trace history for step back
#define MAX_TRACE_HISTORY 1000
typedef struct TraceEntry {
    char *filename;
    int lineno;
    char *code;
    char *variables;
    long exec_num;
} TraceEntry;

static TraceEntry trace_history[MAX_TRACE_HISTORY];
static int trace_history_count = 0;
static int trace_history_index = 0;

// Helper function to add entry to trace history
static void add_trace_entry(const char *filename, int lineno, const char *code, const char *variables) {
    if (trace_history_count < MAX_TRACE_HISTORY) {
        TraceEntry *entry = &trace_history[trace_history_count];
        entry->filename = strdup(filename);
        entry->lineno = lineno;
        entry->code = strdup(code);
        entry->variables = strdup(variables ? variables : "");
        entry->exec_num = execution_counter - 1;
        trace_history_count++;
        trace_history_index = trace_history_count;
    }
}

// Helper function to show trace entry
static void show_trace_entry(int index) {
    if (index < 0 || index >= trace_history_count) {
        return;
    }
    
    TraceEntry *entry = &trace_history[index];
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;35m⟲ TRACE HISTORY [%d/%d]\033[0m\n", index + 1, trace_history_count);
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("Execution: \033[1;32m#%ld\033[0m\n", entry->exec_num);
    printf("File: \033[1;32m%s\033[0m Line: \033[1;32m%d\033[0m\n", entry->filename, entry->lineno);
    printf("Code: %s\n", entry->code);
    if (entry->variables && strlen(entry->variables) > 0) {
        printf("Variables: %s\n", entry->variables);
    }
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
}

// Helper function to free trace history
static void free_trace_history() {
    for (int i = 0; i < trace_history_count; i++) {
        free(trace_history[i].filename);
        free(trace_history[i].code);
        free(trace_history[i].variables);
    }
    trace_history_count = 0;
    trace_history_index = 0;
}

// Helper function to add a breakpoint
static int add_breakpoint(const char *filename, int lineno) {
    Breakpoint *bp = (Breakpoint *)malloc(sizeof(Breakpoint));
    if (bp == NULL) {
        return 0;
    }
    
    bp->filename = strdup(filename);
    bp->lineno = lineno;
    bp->enabled = 1;
    bp->hit_count = 0;
    bp->next = breakpoints;
    breakpoints = bp;
    
    return 1;
}

// Helper function to check if we're at a breakpoint
static Breakpoint* check_breakpoint(const char *filename, int lineno) {
    Breakpoint *bp = breakpoints;
    while (bp != NULL) {
        if (bp->enabled && 
            bp->lineno == lineno && 
            strcmp(bp->filename, filename) == 0) {
            bp->hit_count++;
            return bp;
        }
        bp = bp->next;
    }
    return NULL;
}

// Helper function to write variable values to file
static void
write_variables(FILE *fp, PyObject *locals)
{
    if (locals == NULL || !PyDict_Check(locals)) {
        return;
    }
    
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    int first = 1;
    
    while (PyDict_Next(locals, &pos, &key, &value)) {
        if (!first) {
            fprintf(fp, ";");
        }
        first = 0;
        
        const char *var_name = PyUnicode_AsUTF8(key);
        if (var_name == NULL) {
            PyErr_Clear();
            continue;
        }
        
        PyObject *repr = PyObject_Repr(value);
        const char *var_value = "";
        if (repr != NULL) {
            var_value = PyUnicode_AsUTF8(repr);
            if (var_value == NULL) {
                PyErr_Clear();
                var_value = "<e>";
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

// Main trace function with breakpoint support
static int
trace_callback(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
    if (!is_tracing || trace_file == NULL) {
        return 0;
    }
    
    // Only trace LINE events
    if (what != PyTrace_LINE) {
        return 0;
    }
    
    // Get filename and line number
    PyCodeObject *code = COMPAT_PyFrame_GetCode(frame);
    PyObject *filename_obj = code->co_filename;
    const char *filename = PyUnicode_AsUTF8(filename_obj);
    int lineno = PyFrame_GetLineNumber(frame);
    COMPAT_Py_XDECREF_Code(code);
    
    if (filename == NULL) {
        PyErr_Clear();
        return 0;
    }
    
    // Skip system files
    if (strstr(filename, "site-packages") != NULL ||
        strstr(filename, "/usr/lib") != NULL ||
        strstr(filename, "/usr/local/lib") != NULL ||
        strstr(filename, "python3.") != NULL ||
        strstr(filename, "<frozen") != NULL ||
        strstr(filename, "importlib") != NULL ||
        strstr(filename, "cdebugger") != NULL ||
        strstr(filename, "runner.py") != NULL ||
        strstr(filename, "idebug.py") != NULL ||
        filename[0] == '<') {
        return 0;
    }
    
    // Check for breakpoint
    Breakpoint *bp = check_breakpoint(filename, lineno);
    if (bp != NULL) {
        is_paused = 1;
        printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\033[1;31m⚫ BREAKPOINT HIT\033[0m\n");
        printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("File: \033[1;32m%s\033[0m Line: \033[1;32m%d\033[0m\n", filename, lineno);
        printf("Hit count: %d\n", bp->hit_count);
        
        // Show current code
        char *current_code = get_source_line(filename, lineno);
        printf("Code: %s\n", current_code);
        free(current_code);
        
        printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\nCommands:\n");
        printf("  \033[1;32mc\033[0m - Continue to next breakpoint\n");
        printf("  \033[1;32mn\033[0m - Step to next line\n");
        printf("  \033[1;32mb\033[0m - Step back to previous line\n");
        printf("  \033[1;32mh\033[0m - Show trace history\n");
        printf("  \033[1;32mq\033[0m - Quit execution\n");
        printf("\n> ");
        
        char input[256];
        while (1) {
            if (fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = 0;  // Remove newline
                
                if (strcmp(input, "c") == 0) {
                    // Continue - breakpoint will hit again in loop
                    is_paused = 0;
                    step_mode = 0;
                    break;
                } else if (strcmp(input, "n") == 0) {
                    // Step next
                    is_paused = 0;
                    step_mode = 1;
                    break;
                } else if (strcmp(input, "b") == 0) {
                    // Step back
                    if (trace_history_index > 1) {
                        trace_history_index--;
                        show_trace_entry(trace_history_index - 1);
                        printf("\n> ");
                    } else {
                        printf("Already at the beginning of trace history.\n");
                        printf("\n> ");
                    }
                } else if (strcmp(input, "h") == 0) {
                    // Show history
                    printf("\nTrace History (%d entries):\n", trace_history_count);
                    int start = trace_history_count > 10 ? trace_history_count - 10 : 0;
                    for (int i = start; i < trace_history_count; i++) {
                        char marker = (i == trace_history_index - 1) ? '>' : ' ';
                        printf("  %c [%d] #%ld %s:%d\n", 
                               marker, i + 1, trace_history[i].exec_num,
                               trace_history[i].filename, trace_history[i].lineno);
                    }
                    printf("\n> ");
                } else if (strcmp(input, "q") == 0) {
                    // Quit
                    printf("Exiting...\n");
                    exit(0);
                } else {
                    printf("Unknown command. Use: c, n, b, h, or q\n");
                    printf("\n> ");
                }
            } else {
                break;
            }
        }
    }
    
    // If in step mode, pause after one line
    if (step_mode == 1 && !is_paused) {
        is_paused = 1;
        step_mode = 0;  // Reset step mode
        
        char *source_line = get_source_line(filename, lineno);
        printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\033[1;33m➜ STEP\033[0m\n");
        printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("File: \033[1;32m%s\033[0m Line: \033[1;32m%d\033[0m\n", filename, lineno);
        printf("Code: %s\n", source_line);
        printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\nCommands: c (continue), n (step), b (back), h (history), q (quit)\n");
        printf("\n> ");
        free(source_line);
        
        char input[256];
        while (1) {
            if (fgets(input, sizeof(input), stdin)) {
                input[strcspn(input, "\n")] = 0;
                
                if (strcmp(input, "c") == 0) {
                    is_paused = 0;
                    step_mode = 0;
                    break;
                } else if (strcmp(input, "n") == 0) {
                    is_paused = 0;
                    step_mode = 1;
                    break;
                } else if (strcmp(input, "b") == 0) {
                    // Step back
                    if (trace_history_index > 1) {
                        trace_history_index--;
                        show_trace_entry(trace_history_index - 1);
                        printf("\n> ");
                    } else {
                        printf("Already at the beginning of trace history.\n");
                        printf("\n> ");
                    }
                } else if (strcmp(input, "h") == 0) {
                    // Show history
                    printf("\nTrace History (%d entries):\n", trace_history_count);
                    int start = trace_history_count > 10 ? trace_history_count - 10 : 0;
                    for (int i = start; i < trace_history_count; i++) {
                        char marker = (i == trace_history_index - 1) ? '>' : ' ';
                        printf("  %c [%d] #%ld %s:%d\n", 
                               marker, i + 1, trace_history[i].exec_num,
                               trace_history[i].filename, trace_history[i].lineno);
                    }
                    printf("\n> ");
                } else if (strcmp(input, "q") == 0) {
                    exit(0);
                } else {
                    printf("Unknown command. Use: c, n, b, h, or q\n");
                    printf("\n> ");
                }
            } else {
                break;
            }
        }
    }
    
    // Write to trace file
    char *source_line = get_source_line(filename, lineno);
    
    PyObject *locals = COMPAT_PyFrame_GetLocals(frame);
    if (locals == NULL) {
        locals = PyDict_New();
    } else {
        Py_INCREF(locals);
    }
    
    fprintf(trace_file, "%ld|||%s|||%d|||%s|||",
            execution_counter++, filename, lineno, source_line);
    
    write_variables(trace_file, locals);
    fprintf(trace_file, "\n");
    fflush(trace_file);
    
    // Add to trace history for step back
    // Build variables string
    char var_buffer[4096] = {0};
    if (locals && PyDict_Check(locals)) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        int first = 1;
        
        while (PyDict_Next(locals, &pos, &key, &value) && strlen(var_buffer) < 3900) {
            if (!first) {
                strcat(var_buffer, "; ");
            }
            first = 0;
            
            const char *var_name = PyUnicode_AsUTF8(key);
            if (var_name) {
                strcat(var_buffer, var_name);
                strcat(var_buffer, "=");
                
                PyObject *repr = PyObject_Repr(value);
                if (repr) {
                    const char *var_value = PyUnicode_AsUTF8(repr);
                    if (var_value) {
                        strncat(var_buffer, var_value, 100);
                    }
                    Py_XDECREF(repr);
                }
            }
        }
    }
    
    add_trace_entry(filename, lineno, source_line, var_buffer);
    
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
    
    trace_file = fopen(filename, "w");
    if (trace_file == NULL) {
        PyErr_SetString(PyExc_IOError, "Cannot open trace file");
        return NULL;
    }
    
    fprintf(trace_file, "EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES\n");
    fflush(trace_file);
    
    trace_filename = strdup(filename);
    execution_counter = 0;
    is_tracing = 1;
    is_paused = 0;
    step_mode = 0;
    
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
    
    PyEval_SetTrace(NULL, NULL);
    is_tracing = 0;
    
    if (trace_file != NULL) {
        fclose(trace_file);
        trace_file = NULL;
    }
    
    if (trace_filename != NULL) {
        free(trace_filename);
        trace_filename = NULL;
    }
    
    // Free trace history
    free_trace_history();
    
    Py_RETURN_NONE;
}

// Set breakpoint
static PyObject*
set_breakpoint(PyObject *self, PyObject *args)
{
    const char *filename;
    int lineno;
    
    if (!PyArg_ParseTuple(args, "si", &filename, &lineno)) {
        return NULL;
    }
    
    if (add_breakpoint(filename, lineno)) {
        printf("✓ Breakpoint set at %s:%d\n", filename, lineno);
        Py_RETURN_TRUE;
    } else {
        PyErr_SetString(PyExc_RuntimeError, "Failed to set breakpoint");
        return NULL;
    }
}

// Clear all breakpoints
static PyObject*
clear_breakpoints(PyObject *self, PyObject *args)
{
    Breakpoint *bp = breakpoints;
    while (bp != NULL) {
        Breakpoint *next = bp->next;
        free(bp->filename);
        free(bp);
        bp = next;
    }
    breakpoints = NULL;
    
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
    {"start_trace", start_trace, METH_VARARGS, "Start tracing to file"},
    {"stop_trace", stop_trace, METH_NOARGS, "Stop tracing"},
    {"set_breakpoint", set_breakpoint, METH_VARARGS, "Set a breakpoint at file:line"},
    {"clear_breakpoints", clear_breakpoints, METH_NOARGS, "Clear all breakpoints"},
    {"get_trace_filename", get_trace_filename, METH_NOARGS, "Get trace filename"},
    {NULL, NULL, 0, NULL}
};

// Module definition
static struct PyModuleDef debuggermodule = {
    PyModuleDef_HEAD_INIT,
    "cdebugger",
    "C-based Python debugger with breakpoints and trace generation",
    -1,
    DebuggerMethods
};

// Module initialization
PyMODINIT_FUNC
PyInit_cdebugger(void)
{
    return PyModule_Create(&debuggermodule);
}
