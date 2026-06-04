#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

// Readline support (mandatory)
#include <readline/readline.h>
#include <readline/history.h>
#define HAS_READLINE 1



#define MAX_LINE_LENGTH 10000
#define MAX_LINES 100000
#define MAX_BREAKPOINTS 100
#define MAX_WATCHPOINTS 50
#define MAX_VARS 100

typedef struct {
    long exec_order;
    char filename[512];
    int line_number;
    char *code;
    char *variables;
} TraceEntry;

// Breakpoint structure for post-execution navigation
typedef struct {
    char filename[512];
    int line_number;
} Breakpoint;

// Watchpoint types
typedef enum {
    WATCHPOINT_READ,
    WATCHPOINT_WRITE,
    WATCHPOINT_BOTH
} WatchpointType;

// Watchpoint structure
typedef struct {
    char variable[256];
    WatchpointType type;
} Watchpoint;

// Variable state for tracking changes
typedef struct {
    char name[513];
    char value[512];
} VarState;

typedef struct {
    TraceEntry *entries;
    int entry_count;
    int current_entry;
    Breakpoint breakpoints[MAX_BREAKPOINTS];
    int breakpoint_count;
    Watchpoint watchpoints[MAX_WATCHPOINTS];
    int watchpoint_count;
    VarState prev_vars[MAX_VARS];
    int prev_var_count;
    char eval_temp_file[PATH_MAX];
    int eval_temp_file_ready;
} TraceViewer;

// Global pointer for autocomplete (needs access to trace files)
static TraceViewer *g_viewer = NULL;
static int g_tui_mode = 0;

static const char* g_commands[] = {
    "n", "next", "back", "prev", "b", "break", "list", "c", "continue",
    "rc", "show", "summary", "find", "jump", "eval", "w", "rw", "ww",
    "listw", "clearw", "view", "help", "quit", "q", NULL
};

static const char* g_lower_views[] = {
    "trace", "breakpoints", "log", NULL
};

// Forward declarations
void print_current_entry(TraceViewer *viewer);
void update_variable_state(TraceViewer *viewer, int entry_index);
int check_watchpoint_triggered(TraceViewer *viewer, int entry_index, char *triggered_var, char *trigger_type, WatchpointType *wp_type);

static char*
xstrdup(const char *value) {
    char *copy = strdup(value ? value : "");
    if (!copy) {
        fprintf(stderr, "Memory allocation failed\n");
    }
    return copy;
}

// Parse a trace line into a TraceEntry
int parse_trace_line(char *line, TraceEntry *entry) {
    // Format: EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
    memset(entry, 0, sizeof(*entry));
    
    // Split by ||| delimiter
    char *parts[5] = {NULL};
    int part_count = 0;
    char *ptr = line;
    char *start = ptr;
    
    while (*ptr && part_count < 4) {
        if (ptr[0] == '|' && ptr[1] == '|' && ptr[2] == '|') {
            // Found delimiter
            *ptr = '\0';
            parts[part_count++] = start;
            ptr += 3;  // Skip |||
            start = ptr;
        } else {
            ptr++;
        }
    }
    // Get last part (or rest of string)
    if (part_count < 5) {
        parts[part_count++] = start;
    }
    
    // Must have at least 4 parts (order, file, line, code)
    if (part_count < 4) {
        return 0;
    }
    
    // Parse execution order
    entry->exec_order = atol(parts[0]);
    
    // Parse filename
    strncpy(entry->filename, parts[1], sizeof(entry->filename) - 1);
    entry->filename[sizeof(entry->filename) - 1] = '\0';
    
    // Parse line number
    entry->line_number = atoi(parts[2]);
    
    // Parse code
    entry->code = xstrdup(parts[3]);
    if (!entry->code) {
        return 0;
    }
    
    // Parse variables (5th part, may be empty)
    if (part_count >= 5 && parts[4] && strlen(parts[4]) > 0) {
        entry->variables = xstrdup(parts[4]);
    } else {
        entry->variables = xstrdup("");
    }
    if (!entry->variables) {
        free(entry->code);
        entry->code = NULL;
        return 0;
    }
    
    return 1;
}

// Helper function to extract basename from path
const char* get_basename(const char *path) {
    const char *basename = strrchr(path, '/');
    if (basename != NULL) {
        return basename + 1;  // Skip the '/'
    }
    return path;  // No slash found, return as-is
}

// Helper function to check if two filenames match (supports basename matching)
int filenames_match(const char *bp_filename, const char *trace_filename) {
    // Exact match
    if (strcmp(bp_filename, trace_filename) == 0) {
        return 1;
    }
    
    // Check if trace_filename contains bp_filename
    if (strstr(trace_filename, bp_filename) != NULL) {
        return 1;
    }
    
    // Check basename match
    const char *bp_base = get_basename(bp_filename);
    const char *trace_base = get_basename(trace_filename);
    if (strcmp(bp_base, trace_base) == 0) {
        return 1;
    }
    
    return 0;
}

// Check if current entry matches any breakpoint
int is_at_breakpoint(TraceViewer *viewer, int entry_index) {
    if (entry_index < 0 || entry_index >= viewer->entry_count) {
        return 0;
    }
    
    TraceEntry *entry = &viewer->entries[entry_index];
    
    for (int i = 0; i < viewer->breakpoint_count; i++) {
        if (viewer->breakpoints[i].line_number == entry->line_number &&
            filenames_match(viewer->breakpoints[i].filename, entry->filename)) {
            return 1;
        }
    }
    
    return 0;
}

// Add a breakpoint
void add_breakpoint(TraceViewer *viewer, const char *filename, int line_number) {
    if (viewer->breakpoint_count >= MAX_BREAKPOINTS) {
        printf("\033[1;31m✗ Maximum breakpoints (%d) reached\033[0m\n", MAX_BREAKPOINTS);
        return;
    }
    
    // Check for duplicates
    for (int i = 0; i < viewer->breakpoint_count; i++) {
        if (viewer->breakpoints[i].line_number == line_number &&
            strcmp(viewer->breakpoints[i].filename, filename) == 0) {
            printf("\033[1;33m⚠ Breakpoint already set at %s:%d\033[0m\n", filename, line_number);
            return;
        }
    }
    
    Breakpoint *bp = &viewer->breakpoints[viewer->breakpoint_count];
    strncpy(bp->filename, filename, sizeof(bp->filename) - 1);
    bp->filename[sizeof(bp->filename) - 1] = '\0';
    bp->line_number = line_number;
    viewer->breakpoint_count++;
    
    printf("\033[1;32m✓ Breakpoint set at %s:%d\033[0m\n", filename, line_number);
}

// List all breakpoints
void list_breakpoints(TraceViewer *viewer) {
    if (viewer->breakpoint_count == 0) {
        printf("\033[1;33mNo breakpoints set\033[0m\n");
        return;
    }
    
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mBreakpoints:\033[0m\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    for (int i = 0; i < viewer->breakpoint_count; i++) {
        printf("  \033[1;32m%d.\033[0m %s:%d\n", i + 1, 
               viewer->breakpoints[i].filename, 
               viewer->breakpoints[i].line_number);
    }
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("Total: \033[1;32m%d\033[0m breakpoint(s)\n\n", viewer->breakpoint_count);
}

// Continue to next breakpoint or watchpoint (forward)
void continue_to_breakpoint(TraceViewer *viewer) {
    if (viewer->breakpoint_count == 0 && viewer->watchpoint_count == 0) {
        // No breakpoints or watchpoints set - just jump to end
        printf("\033[1;33m⚠ No breakpoints or watchpoints set. Jumping to end of trace.\033[0m\n");
        viewer->current_entry = viewer->entry_count - 1;
        print_current_entry(viewer);
        return;
    }
    
    // Initialize previous state to current entry before searching
    update_variable_state(viewer, viewer->current_entry);
    
    // Search forward from current position
    for (int i = viewer->current_entry + 1; i < viewer->entry_count; i++) {
        // Check watchpoints first (compares prev_vars with entry i)
        char triggered_var[256];
        char trigger_type[64];
        WatchpointType wp_type;
        if (check_watchpoint_triggered(viewer, i, triggered_var, trigger_type, &wp_type)) {
            // For writes: the write happened at line shown in entry i-1, but we can only detect  
            // it at entry i (where we see the new value). We stop at entry i.
            // This means line number will be one past the write, but variables will show new value.
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;35m👁 WATCHPOINT HIT\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;36mVariable '%s' was %s\033[0m\n", triggered_var, trigger_type);
            print_current_entry(viewer);
            return;
        }
        
        // Then check breakpoints
        if (is_at_breakpoint(viewer, i)) {
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;31m⚫ BREAKPOINT HIT\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            print_current_entry(viewer);
            return;
        }
        
        // Update variable state for next iteration
        update_variable_state(viewer, i);
    }
    
    // No more breakpoints or watchpoints - go to end
    printf("\033[1;33m⚠ No more breakpoints or watchpoints ahead. Jumping to end of trace.\033[0m\n");
    viewer->current_entry = viewer->entry_count - 1;
    print_current_entry(viewer);
}

// Reverse continue to previous breakpoint or watchpoint (backward)
void reverse_continue_to_breakpoint(TraceViewer *viewer) {
    if (viewer->breakpoint_count == 0 && viewer->watchpoint_count == 0) {
        // No breakpoints or watchpoints set - just jump to beginning
        printf("\033[1;33m⚠ No breakpoints or watchpoints set. Jumping to beginning of trace.\033[0m\n");
        viewer->current_entry = 0;
        print_current_entry(viewer);
        return;
    }
    
    // Search backward from current position
    // For reverse, we need to check each position with the previous position as context
    for (int i = viewer->current_entry - 1; i >= 0; i--) {
        // Initialize previous state to one before i (or empty if i is 0)
        if (i > 0) {
            update_variable_state(viewer, i - 1);
        } else {
            viewer->prev_var_count = 0;  // No previous state for first entry
        }
        
        // Check watchpoints first (compares prev_vars with entry i)
        char triggered_var[256];
        char trigger_type[64];
        WatchpointType wp_type;
        if (check_watchpoint_triggered(viewer, i, triggered_var, trigger_type, &wp_type)) {
            // Stop at entry i where we detected the change
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;35m⟲ WATCHPOINT HIT (REVERSE)\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;36mVariable '%s' was %s\033[0m\n", triggered_var, trigger_type);
            print_current_entry(viewer);
            return;
        }
        
        // Then check breakpoints
        if (is_at_breakpoint(viewer, i)) {
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;35m⟲ BREAKPOINT HIT (REVERSE)\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            print_current_entry(viewer);
            return;
        }
    }
    
    // No more breakpoints or watchpoints - go to beginning
    printf("\033[1;33m⚠ No more breakpoints or watchpoints behind. Jumping to beginning of trace.\033[0m\n");
    viewer->current_entry = 0;
    print_current_entry(viewer);
}

// Parse variables from trace entry
// Format: var1=value1;var2=value2;...
void parse_variables(const char *vars_str, VarState *vars, int *count, int max_vars) {
    *count = 0;
    if (!vars_str || strlen(vars_str) == 0) {
        return;
    }
    
    char *vars_copy = xstrdup(vars_str);
    if (!vars_copy) {
        return;
    }
    
    char *ptr = vars_copy;
    char *start = ptr;
    
    while (*ptr && *count < max_vars) {
        // Find next semicolon or end
        if (*ptr == ';' || *(ptr + 1) == '\0') {
            if (*(ptr + 1) == '\0' && *ptr != ';') {
                ptr++;
            }
            
            // Extract variable=value pair
            char temp = *ptr;
            *ptr = '\0';
            
            // Find the '=' sign
            char *eq = strchr(start, '=');
            if (eq) {
                *eq = '\0';
                strncpy(vars[*count].name, start, sizeof(vars[*count].name) - 1);
                vars[*count].name[sizeof(vars[*count].name) - 1] = '\0';
                
                strncpy(vars[*count].value, eq + 1, sizeof(vars[*count].value) - 1);
                vars[*count].value[sizeof(vars[*count].value) - 1] = '\0';
                
                (*count)++;
            }
            
            *ptr = temp;
            if (*ptr == ';') {
                ptr++;
                start = ptr;
            } else {
                break;
            }
        } else {
            ptr++;
        }
    }

    free(vars_copy);
}

// Check if variable was written (value changed)
int variable_written(const char *var_name, VarState *prev_vars, int prev_count, 
                     VarState *curr_vars, int curr_count) {
    char *prev_value = NULL;
    char *curr_value = NULL;
    
    // Find in previous state
    for (int i = 0; i < prev_count; i++) {
        if (strcmp(prev_vars[i].name, var_name) == 0) {
            prev_value = prev_vars[i].value;
            break;
        }
    }
    
    // Find in current state
    for (int i = 0; i < curr_count; i++) {
        if (strcmp(curr_vars[i].name, var_name) == 0) {
            curr_value = curr_vars[i].value;
            break;
        }
    }
    
    // Variable is written if:
    // 1. It didn't exist before but exists now (new variable)
    // 2. Its value changed
    if (!prev_value && curr_value) {
        return 1;
    }
    if (prev_value && curr_value && strcmp(prev_value, curr_value) != 0) {
        return 1;
    }
    
    return 0;
}

// Check if variable was read (appears in code)
int variable_read(const char *var_name, const char *code, VarState *curr_vars, int curr_count) {
    // First check if variable exists in current state
    int exists = 0;
    for (int i = 0; i < curr_count; i++) {
        if (strcmp(curr_vars[i].name, var_name) == 0) {
            exists = 1;
            break;
        }
    }
    
    if (!exists) {
        return 0;
    }
    
    // Check if variable name appears in code
    // We need to be careful to match whole words only
    const char *ptr = code;
    int var_len = strlen(var_name);
    
    while ((ptr = strstr(ptr, var_name)) != NULL) {
        // Check if it's a whole word (not part of another identifier)
        int is_whole_word = 1;
        
        // Check character before
        if (ptr > code) {
            char before = *(ptr - 1);
            if (isalnum(before) || before == '_') {
                is_whole_word = 0;
            }
        }
        
        // Check character after
        if (is_whole_word) {
            char after = *(ptr + var_len);
            if (isalnum(after) || after == '_') {
                is_whole_word = 0;
            }
        }
        
        if (is_whole_word) {
            return 1;
        }
        
        ptr++;
    }
    
    return 0;
}

// Add a watchpoint
void add_watchpoint(TraceViewer *viewer, const char *variable, WatchpointType type) {
    if (viewer->watchpoint_count >= MAX_WATCHPOINTS) {
        printf("\033[1;31m✗ Maximum watchpoints (%d) reached\033[0m\n", MAX_WATCHPOINTS);
        return;
    }
    
    // Check if watchpoint already exists
    for (int i = 0; i < viewer->watchpoint_count; i++) {
        if (strcmp(viewer->watchpoints[i].variable, variable) == 0) {
            printf("\033[1;33m✗ Watchpoint already set on '%s'\033[0m\n", variable);
            return;
        }
    }
    
    Watchpoint *wp = &viewer->watchpoints[viewer->watchpoint_count];
    strncpy(wp->variable, variable, sizeof(wp->variable) - 1);
    wp->variable[sizeof(wp->variable) - 1] = '\0';
    wp->type = type;
    viewer->watchpoint_count++;
    
    const char *type_str = (type == WATCHPOINT_READ) ? "read" : 
                          (type == WATCHPOINT_WRITE) ? "write" : "read/write";
    printf("\033[1;32m✓ Watchpoint set on '%s' (type: %s)\033[0m\n", variable, type_str);
}

// List all watchpoints
void list_watchpoints(TraceViewer *viewer) {
    if (viewer->watchpoint_count == 0) {
        printf("\033[1;33mNo watchpoints set\033[0m\n");
        return;
    }
    
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mWatchpoints:\033[0m\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
    
    for (int i = 0; i < viewer->watchpoint_count; i++) {
        const char *type_str = (viewer->watchpoints[i].type == WATCHPOINT_READ) ? "read" :
                              (viewer->watchpoints[i].type == WATCHPOINT_WRITE) ? "write" : "read/write";
        printf("  %d. \033[1;32m%s\033[0m (%s)\n", i + 1, viewer->watchpoints[i].variable, type_str);
    }
    
    printf("\nTotal: \033[1;32m%d\033[0m watchpoint(s)\n\n", viewer->watchpoint_count);
}

// Clear a specific watchpoint or all watchpoints
void clear_watchpoint(TraceViewer *viewer, int index) {
    if (index < 0) {
        // Clear all
        viewer->watchpoint_count = 0;
        printf("\033[1;32m✓ All watchpoints cleared\033[0m\n");
    } else if (index >= 0 && index < viewer->watchpoint_count) {
        // Clear specific watchpoint
        printf("\033[1;32m✓ Cleared watchpoint on '%s'\033[0m\n", viewer->watchpoints[index].variable);
        
        // Shift remaining watchpoints
        for (int i = index; i < viewer->watchpoint_count - 1; i++) {
            viewer->watchpoints[i] = viewer->watchpoints[i + 1];
        }
        viewer->watchpoint_count--;
    } else {
        printf("\033[1;31m✗ Invalid watchpoint number\033[0m\n");
    }
}

// Check if watchpoint is triggered at current entry
int check_watchpoint_triggered(TraceViewer *viewer, int entry_index, char *triggered_var, 
                               char *trigger_type, WatchpointType *wp_type) {
    if (viewer->watchpoint_count == 0) {
        return 0;
    }
    
    TraceEntry *entry = &viewer->entries[entry_index];
    
    // Parse current variables
    VarState curr_vars[MAX_VARS];
    int curr_count = 0;
    parse_variables(entry->variables, curr_vars, &curr_count, MAX_VARS);
    
    // Check each watchpoint
    for (int i = 0; i < viewer->watchpoint_count; i++) {
        Watchpoint *wp = &viewer->watchpoints[i];
        int read_triggered = 0;
        int write_triggered = 0;
        
        // Check for write
        if (wp->type == WATCHPOINT_WRITE || wp->type == WATCHPOINT_BOTH) {
            if (variable_written(wp->variable, viewer->prev_vars, viewer->prev_var_count, 
                               curr_vars, curr_count)) {
                write_triggered = 1;
            }
        }
        
        // Check for read
        if (wp->type == WATCHPOINT_READ || wp->type == WATCHPOINT_BOTH) {
            if (variable_read(wp->variable, entry->code, curr_vars, curr_count)) {
                read_triggered = 1;
            }
        }
        
        if (read_triggered || write_triggered) {
            strncpy(triggered_var, wp->variable, 255);
            triggered_var[255] = '\0';
            
            if (read_triggered && write_triggered) {
                strcpy(trigger_type, "read/write");
            } else if (read_triggered) {
                strcpy(trigger_type, "read");
            } else {
                strcpy(trigger_type, "write");
            }
            
            *wp_type = wp->type;
            return 1;
        }
    }
    
    return 0;
}

// Update variable state tracking
void update_variable_state(TraceViewer *viewer, int entry_index) {
    TraceEntry *entry = &viewer->entries[entry_index];
    parse_variables(entry->variables, viewer->prev_vars, &viewer->prev_var_count, MAX_VARS);
}

// Read trace file into memory
int read_trace_file(const char *filename, TraceViewer *viewer) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening trace file");
        return 0;
    }

    viewer->entries = malloc(MAX_LINES * sizeof(TraceEntry));
    if (!viewer->entries) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 0;
    }

    viewer->entry_count = 0;
    viewer->breakpoint_count = 0;  // Initialize breakpoint count
    viewer->watchpoint_count = 0;  // Initialize watchpoint count
    viewer->prev_var_count = 0;    // Initialize variable state count
    viewer->eval_temp_file[0] = '\0';
    viewer->eval_temp_file_ready = 0;
    char *buffer = NULL;
    size_t buffer_size = 0;
    int first_line = 1;

    while (getline(&buffer, &buffer_size, file) != -1 && viewer->entry_count < MAX_LINES) {
        // Skip header line
        if (first_line) {
            first_line = 0;
            continue;
        }
        
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        if (len > 0 && buffer[len - 1] == '\r') {
            buffer[len - 1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(buffer) == 0) {
            continue;
        }

        TraceEntry entry;
        if (parse_trace_line(buffer, &entry)) {
            viewer->entries[viewer->entry_count] = entry;
            viewer->entry_count++;
        }
    }

    free(buffer);
    fclose(file);
    viewer->current_entry = 0;
    return 1;
}

// Print the current trace entry
void print_current_entry(TraceViewer *viewer) {
    if (g_tui_mode) {
        return;
    }

    if (viewer->current_entry >= 0 && viewer->current_entry < viewer->entry_count) {
        TraceEntry *entry = &viewer->entries[viewer->current_entry];
        printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\033[1;33m[Execution #%ld]\033[0m\n", entry->exec_order);
        printf("\033[1;32mFile:\033[0m %s \033[1;32mLine:\033[0m %d\n", entry->filename, entry->line_number);
        printf("\033[1;35mCode:\033[0m %s\n", entry->code);
        
        if (strlen(entry->variables) > 0) {
            printf("\033[1;34mVariables:\033[0m\n");
            
            // Parse and display variables nicely
            char *vars_copy = xstrdup(entry->variables);
            if (!vars_copy) {
                return;
            }
            
            char *var = strtok(vars_copy, ";");
            while (var) {
                // Trim leading whitespace
                while (isspace((unsigned char)*var)) var++;
                if (strlen(var) > 0) {
                    printf("  • %s\n", var);
                }
                var = strtok(NULL, ";");
            }
            free(vars_copy);
        } else {
            printf("\033[1;34mVariables:\033[0m (none)\n");
        }
        printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    }
}

// Print summary statistics
void print_summary(TraceViewer *viewer) {
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mTrace Summary\033[0m\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("Total Executions: %d\n", viewer->entry_count);
    
    if (viewer->entry_count > 0) {
        printf("First Entry: [%ld] %s:%d\n", 
               viewer->entries[0].exec_order,
               viewer->entries[0].filename,
               viewer->entries[0].line_number);
        printf("Last Entry:  [%ld] %s:%d\n", 
               viewer->entries[viewer->entry_count - 1].exec_order,
               viewer->entries[viewer->entry_count - 1].filename,
               viewer->entries[viewer->entry_count - 1].line_number);
        printf("\nCurrent Position: [%ld] (Entry %d of %d)\n",
               viewer->entries[viewer->current_entry].exec_order,
               viewer->current_entry + 1,
               viewer->entry_count);
    }
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
}

// Search for a variable in the trace
void search_variable(TraceViewer *viewer, const char *var_name) {
    int found = 0;
    printf("\n\033[1;33mSearching for variable '%s'...\033[0m\n\n", var_name);
    
    for (int i = 0; i < viewer->entry_count; i++) {
        TraceEntry *entry = &viewer->entries[i];
        if (strstr(entry->variables, var_name)) {
            printf("[%ld] %s:%d\n", entry->exec_order, entry->filename, entry->line_number);
            
            // Parse and find the specific variable
            char *vars_copy = xstrdup(entry->variables);
            if (!vars_copy) {
                return;
            }
            
            char *var = strtok(vars_copy, ";");
            while (var) {
                while (isspace((unsigned char)*var)) var++;
                if (strstr(var, var_name) == var || 
                    (strstr(var, var_name) && *(strstr(var, var_name) - 1) == ' ')) {
                    printf("  → %s\n", var);
                }
                var = strtok(NULL, ";");
            }
            free(vars_copy);
            printf("\n");
            found++;
            
            if (found >= 10) {
                printf("... showing first 10 results. Use 'find <var>' for more specific search.\n");
                break;
            }
        }
    }
    
    if (found == 0) {
        printf("Variable '%s' not found in trace.\n", var_name);
    } else {
        printf("Found %d occurrence(s).\n", found);
    }
}

// Helper: try to open a file, discovering directory from trace if needed
static FILE* try_open_with_trace_dir(const char *basename, TraceViewer *viewer, char *resolved_path, size_t path_size) {
    FILE *f;
    
    // Strategy 1: Try basename in current directory
    f = fopen(basename, "r");
    if (f) {
        strncpy(resolved_path, basename, path_size - 1);
        return f;
    }
    
    // Strategy 2: Extract directory from trace entries and try there
    for (int i = 0; i < viewer->entry_count; i++) {
        const char *trace_file = viewer->entries[i].filename;
        
        // If this entry has a directory path, extract it
        const char *last_slash = strrchr(trace_file, '/');
        if (last_slash && last_slash > trace_file) {
            // Extract directory
            size_t dir_len = last_slash - trace_file;
            char dir_path[1024];
            strncpy(dir_path, trace_file, dir_len);
            dir_path[dir_len] = '\0';
            
            // Try: directory/basename
            char try_path[1025];
            snprintf(try_path, sizeof(try_path), "%s/%s", dir_path, basename);
            f = fopen(try_path, "r");
            if (f) {
                strncpy(resolved_path, try_path, path_size - 1);
                return f;
            }
        }
    }
    
    return NULL;
}

// View full source file with current line highlighted
void show_file(TraceViewer *viewer, const char *requested_file) {
    if (viewer->current_entry < 0 || viewer->current_entry >= viewer->entry_count) {
        printf("No current entry\n");
        return;
    }
    
    TraceEntry *current = &viewer->entries[viewer->current_entry];
    const char *filename = NULL;
    int highlight_line = -1;
    char resolved_path[1026] = {0};
    
    // If a specific file was requested
    if (requested_file && strlen(requested_file) > 0) {
        const char *basename_to_use = get_basename(requested_file);
        
        // Search trace for matching file
        const char *found_in_trace = NULL;
        for (int i = 0; i < viewer->entry_count; i++) {
            if (filenames_match(requested_file, viewer->entries[i].filename)) {
                found_in_trace = viewer->entries[i].filename;
                break;
            }
        }
        
        if (found_in_trace) {
            // Try trace path directly
            FILE *test = fopen(found_in_trace, "r");
            if (test) {
                fclose(test);
                filename = found_in_trace;
            } else {
                // Try with directory discovery
                test = try_open_with_trace_dir(get_basename(found_in_trace), viewer, resolved_path, sizeof(resolved_path));
                if (test) {
                    fclose(test);
                    filename = resolved_path;
                } else {
                    filename = basename_to_use;
                }
            }
        } else {
            // Not in trace, try with directory discovery
            FILE *test = try_open_with_trace_dir(basename_to_use, viewer, resolved_path, sizeof(resolved_path));
            if (test) {
                fclose(test);
                filename = resolved_path;
            } else {
                filename = basename_to_use;
            }
        }
        
        // Check if we should highlight
        if (filenames_match(requested_file, current->filename)) {
            highlight_line = current->line_number;
        }
    } else {
        // Default to current file
        const char *current_path = current->filename;
        
        // Try current path directly
        FILE *test = fopen(current_path, "r");
        if (test) {
            fclose(test);
            filename = current_path;
        } else {
            // Try with directory discovery
            test = try_open_with_trace_dir(get_basename(current_path), viewer, resolved_path, sizeof(resolved_path));
            if (test) {
                fclose(test);
                filename = resolved_path;
            } else {
                filename = get_basename(current_path);
            }
        }
        
        highlight_line = current->line_number;
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\033[1;31m✗ Cannot open file: %s\033[0m\n", get_basename(filename));
        if (requested_file && strlen(requested_file) > 0) {
            printf("\033[1;33mTip: File not found in trace or on disk.\033[0m\n");
            printf("\033[1;33mFiles in trace:\033[0m\n");
            for (int i = 0; i < viewer->entry_count; i++) {
                int already_shown = 0;
                for (int j = 0; j < i; j++) {
                    if (strcmp(get_basename(viewer->entries[i].filename), 
                               get_basename(viewer->entries[j].filename)) == 0) {
                        already_shown = 1;
                        break;
                    }
                }
                if (!already_shown) {
                    printf("  - %s\n", get_basename(viewer->entries[i].filename));
                }
            }
        }
        return;
    }
    
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mFile: %s\033[0m\n", get_basename(filename));
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
    
    char line[MAX_LINE_LENGTH];
    int line_num = 1;
    int bp_count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // Check if this line has a breakpoint
        int has_breakpoint = 0;
        for (int i = 0; i < viewer->breakpoint_count; i++) {
            if (viewer->breakpoints[i].line_number == line_num &&
                filenames_match(viewer->breakpoints[i].filename, filename)) {
                has_breakpoint = 1;
                bp_count++;
                break;
            }
        }
        
        // Format: marker + line number + separator + code
        const char *marker = has_breakpoint ? "\033[1;31m⚫\033[0m" : "  ";
        
        if (highlight_line > 0 && line_num == highlight_line) {
            // Highlight current line (green background)
            printf("%s \033[42m\033[30m%4d\033[0m \033[1;32m|\033[0m \033[42m\033[30m%s\033[0m\n", 
                   marker, line_num, line);
        } else {
            // Normal line
            printf("%s \033[1;32m%4d\033[0m | %s\n", marker, line_num, line);
        }
        
        line_num++;
    }
    
    fclose(file);
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("Total lines: \033[1m%d\033[0m\n", line_num - 1);
    if (bp_count > 0) {
        printf("Breakpoints: \033[1m%d\033[0m in this file\n", bp_count);
    }
    if (highlight_line > 0) {
        printf("Currently at: \033[1;32m[Execution #%ld]\033[0m Line %d\n", 
               current->exec_order, highlight_line);
    }
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
}

// Free allocated memory
void cleanup(TraceViewer *viewer) {
    if (viewer->eval_temp_file_ready) {
        unlink(viewer->eval_temp_file);
        viewer->eval_temp_file_ready = 0;
    }

    for (int i = 0; i < viewer->entry_count; i++) {
        free(viewer->entries[i].code);
        free(viewer->entries[i].variables);
    }
    free(viewer->entries);
}

// Print help
void print_help() {
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mTrace Debugger Commands\033[0m\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\n\033[1;35mNavigation:\033[0m\n");
    printf("  \033[1;32mn\033[0m              - Next execution step\n");
    printf("  \033[1;32mback\033[0m           - Previous execution step\n");
    printf("  \033[1;32m:<number>\033[0m      - Jump to execution (e.g., :5 jumps to [5/50])\n");
    printf("\n\033[1;35mBreakpoints:\033[0m\n");
    printf("  \033[1;32mb <file> <line>\033[0m - Set breakpoint (e.g., b test.py 25)\n");
    printf("  \033[1;32mlist\033[0m           - List all breakpoints\n");
    printf("  \033[1;32mc\033[0m              - Continue to next breakpoint/watchpoint\n");
    printf("  \033[1;32mrc\033[0m             - Reverse continue to previous breakpoint/watchpoint\n");
    printf("\n\033[1;35mWatchpoints:\033[0m\n");
    printf("  \033[1;32mw <var>\033[0m        - Set watchpoint on variable (read/write)\n");
    printf("  \033[1;32mrw <var>\033[0m       - Set read watchpoint on variable\n");
    printf("  \033[1;32mww <var>\033[0m       - Set write watchpoint on variable\n");
    printf("  \033[1;32mlistw\033[0m          - List all watchpoints\n");
    printf("  \033[1;32mclearw [num]\033[0m   - Clear watchpoint(s) (no arg = clear all)\n");
    printf("\n\033[1;35mAnalysis:\033[0m\n");
    printf("  \033[1;32mshow [file]\033[0m    - Show full source file (current or specified)\n");
    printf("  \033[1;32msummary\033[0m        - Show trace summary\n");
    printf("  \033[1;32mfind <var>\033[0m    - Search for variable usage\n");
    printf("  \033[1;32mjump <line>\033[0m   - Jump to first occurrence of source line\n");
    printf("  \033[1;32meval <expression>\033[0m   - Evaluates a python command and prints the result\n");
    printf("\n\033[1;35mOther:\033[0m\n");
    printf("  \033[1;32mhelp\033[0m           - Show this help\n");
    printf("  \033[1;32mquit\033[0m or \033[1;32mq\033[0m     - Exit debugger\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
}

static void rstrip(char *value) {
    size_t len = strlen(value);
    while (len > 0 && isspace((unsigned char)value[len - 1])) {
        value[len - 1] = '\0';
        len--;
    }
}

static int is_python_identifier(const char *value) {
    if (!value || value[0] == '\0') {
        return 0;
    }
    if (!(isalpha((unsigned char)value[0]) || value[0] == '_')) {
        return 0;
    }
    for (const char *p = value + 1; *p; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '_')) {
            return 0;
        }
    }
    return 1;
}

static char* lookup_variable_repr(const char *vars_str, const char *name) {
    char *vars_copy = xstrdup(vars_str);
    if (!vars_copy) {
        return NULL;
    }

    char *saveptr = NULL;
    char *var = strtok_r(vars_copy, ";", &saveptr);
    while (var) {
        while (isspace((unsigned char)*var)) var++;
        char *eq = strchr(var, '=');
        if (eq) {
            *eq = '\0';
            rstrip(var);
            if (strcmp(var, name) == 0) {
                char *result = xstrdup(eq + 1);
                free(vars_copy);
                return result;
            }
        }
        var = strtok_r(NULL, ";", &saveptr);
    }

    free(vars_copy);
    return NULL;
}

static char* lookup_direct_captured_identifier(TraceEntry *entry, const char *expression) {
    if (!is_python_identifier(expression)) {
        return NULL;
    }
    return lookup_variable_repr(entry->variables, expression);
}

static void write_python_string(FILE *f, const char *value) {
    fputc('\'', f);
    for (const char *p = value; *p; p++) {
        switch (*p) {
        case '\\':
            fputs("\\\\", f);
            break;
        case '\'':
            fputs("\\'", f);
            break;
        case '\n':
            fputs("\\n", f);
            break;
        case '\r':
            fputs("\\r", f);
            break;
        default:
            fputc(*p, f);
            break;
        }
    }
    fputc('\'', f);
}

static void write_literal_loads(FILE *f, const char *vars_str) {
    char *vars_copy = xstrdup(vars_str);
    if (!vars_copy) {
        return;
    }

    fprintf(f, "import ast as __trace_ast\n");
    fprintf(f, "__trace_loaded_names = set()\n");
    fprintf(f, "def __trace_load(name, value):\n");
    fprintf(f, "    try:\n");
    fprintf(f, "        globals()[name] = __trace_ast.literal_eval(value)\n");
    fprintf(f, "        __trace_loaded_names.add(name)\n");
    fprintf(f, "    except Exception:\n");
    fprintf(f, "        pass\n");

    char *saveptr = NULL;
    char *var = strtok_r(vars_copy, ";", &saveptr);
    while (var) {
        while (isspace((unsigned char)*var)) var++;
        char *eq = strchr(var, '=');
        if (eq) {
            *eq = '\0';
            rstrip(var);
            if (is_python_identifier(var)) {
                fprintf(f, "__trace_load(");
                write_python_string(f, var);
                fprintf(f, ", ");
                write_python_string(f, eq + 1);
                fprintf(f, ")\n");
            }
        }
        var = strtok_r(NULL, ";", &saveptr);
    }

    free(vars_copy);
}

static int command_failed(int status) {
    return status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0;
}

static int append_output(char **buffer, size_t *length, size_t *capacity, const char *text) {
    size_t text_len = strlen(text);
    size_t needed = *length + text_len + 1;

    if (needed > *capacity) {
        size_t new_capacity = *capacity ? *capacity : MAX_LINE_LENGTH;
        while (new_capacity < needed) {
            new_capacity *= 2;
        }

        char *new_buffer = realloc(*buffer, new_capacity);
        if (!new_buffer) {
            return 0;
        }

        *buffer = new_buffer;
        *capacity = new_capacity;
    }

    memcpy(*buffer + *length, text, text_len);
    *length += text_len;
    (*buffer)[*length] = '\0';
    return 1;
}

static void print_eval_header(const char *expression) {
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mEvaluating:\033[0m %s\n", expression);
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
}

static void print_eval_footer() {
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
}

static void print_captured_repr_result(const char *captured_value) {
    printf("Result: %s\n", captured_value);
    printf("Type: captured repr\n");
}

static int ensure_eval_temp_file(TraceViewer *viewer) {
    char template[PATH_MAX];
    int fd;

    if (viewer->eval_temp_file_ready) {
        return 1;
    }

    snprintf(template, sizeof(template), "/tmp/traceviewer_eval_XXXXXX");
    fd = mkstemp(template);
    if (fd == -1) {
        return 0;
    }

    close(fd);
    strncpy(viewer->eval_temp_file, template, sizeof(viewer->eval_temp_file) - 1);
    viewer->eval_temp_file[sizeof(viewer->eval_temp_file) - 1] = '\0';
    viewer->eval_temp_file_ready = 1;
    return 1;
}

void eval_expression(TraceViewer *viewer, const char *expression) {
    if (viewer->current_entry < 0 || viewer->current_entry >= viewer->entry_count) {
        printf("\033[1;31m✗ No current entry\033[0m\n");
        return;
    }

    TraceEntry *entry = &viewer->entries[viewer->current_entry];

    // is an import, just add it to the file and return
    if(strncmp(expression, "import ", 7) == 0 || strncmp(expression, "from ", 5) == 0){
        if (!ensure_eval_temp_file(viewer)) {
            printf("\033[1;31m✗ Failed to create temp file\033[0m\n");
            return;
        }

        const char *temp_file = viewer->eval_temp_file;
        FILE *f = fopen(temp_file, "a");
        if (!f) {
            printf("\033[1;31m✗ Failed to open temp file\033[0m\n");
            return;
        }
        long pos_before = ftell(f);
        fprintf(f, "%s\n", expression);
        fclose(f);
  // hopefully this works
        char command[PATH_MAX + 32];
        snprintf(command, sizeof(command), "python3 %s 2>&1", temp_file);
        FILE *output = popen(command, "r");
        int failed = 0;

        if (output) {
            int status = pclose(output);
            if (command_failed(status)) failed = 1;
        }

        if (failed) {
            int fd = open(temp_file, O_WRONLY);
            ftruncate(fd, pos_before);
            close(fd);
            printf("\033[1;31m✗ Import failed\033[0m\n");
        } else {
            printf("\033[1;32m✓ Import added to session\033[0m\n\n");
        }
        return;
    }

    // Eval and print
    char *direct_captured_value = lookup_direct_captured_identifier(entry, expression);

    if (!ensure_eval_temp_file(viewer)) {
        if (direct_captured_value) {
            print_eval_header(expression);
            print_captured_repr_result(direct_captured_value);
            print_eval_footer();
            free(direct_captured_value);
            return;
        }
        free(direct_captured_value);
        printf("\033[1;31m✗ Failed to create temp file\033[0m\n");
        return;
    }

    const char *temp_file = viewer->eval_temp_file;
    FILE *f = fopen(temp_file, "a");
    if (!f) {
        if (direct_captured_value) {
            print_eval_header(expression);
            print_captured_repr_result(direct_captured_value);
            print_eval_footer();
            free(direct_captured_value);
            return;
        }
        free(direct_captured_value);
        printf("\033[1;31m✗ Failed to open temp file\033[0m\n");
        return;
    }

    long pos_before = ftell(f);

    write_literal_loads(f, entry->variables);

    if (direct_captured_value) {
        fprintf(f, "if ");
        write_python_string(f, expression);
        fprintf(f, " not in __trace_loaded_names:\n");
        fprintf(f, "    raise NameError(");
        write_python_string(f, expression);
        fprintf(f, ")\n");
    }

    fprintf(f, "__r=%s\n", expression);
    fprintf(f, "print(f'Result: {__r!r}')\n");
    fprintf(f, "print(f'Type: {type(__r).__name__}')\n");
    fclose(f);

    char command[PATH_MAX + 32];
    snprintf(command, sizeof(command), "python3 %s 2>&1", temp_file);

    print_eval_header(expression);

    FILE *output = popen(command, "r");
    if (output) {
        char line[MAX_LINE_LENGTH];
        char *captured_output = NULL;
        size_t output_length = 0;
        size_t output_capacity = 0;

        while (fgets(line, sizeof(line), output)) {
            if (!append_output(&captured_output, &output_length, &output_capacity, line)) {
                free(captured_output);
                pclose(output);
                printf("\033[1;31m✗ Failed to capture evaluation output\033[0m\n");
                goto cleanup_eval;
            }
        }

        int status = pclose(output);
        if (command_failed(status)) {
            if (direct_captured_value) {
                print_captured_repr_result(direct_captured_value);
            } else {
                if (captured_output) {
                    printf("%s", captured_output);
                }
                printf("\033[1;31m✗ Evaluation failed\033[0m\n");
            }
        } else if (captured_output) {
            printf("%s", captured_output);
        }
        free(captured_output);
    } else {
        printf("\033[1;31m✗ Failed to execute Python\033[0m\n");
    }

    print_eval_footer();

    int fd = open(temp_file, O_WRONLY);
    ftruncate(fd, pos_before);
    close(fd);
    free(direct_captured_value);
    return;

cleanup_eval:
    print_eval_footer();
    int cleanup_fd = open(temp_file, O_WRONLY);
    ftruncate(cleanup_fd, pos_before);
    close(cleanup_fd);
    free(direct_captured_value);
}

// Filename completion support - shows .py files from trace and current directory
static char* filename_generator(const char* text, int state) {
    static DIR *dir = NULL;
    static int len;
    static int trace_index = 0;
    static int seen_count = 0;
    static char seen_files[100][256];  // Track files we've already shown
    struct dirent *entry;
    
    if (!state) {
        // First call - reset state
        if (dir) {
            closedir(dir);
            dir = NULL;
        }
        trace_index = 0;
        seen_count = 0;
        len = strlen(text);
    }
    
    // Phase 1: Suggest files from trace (most relevant)
    if (g_viewer && trace_index < g_viewer->entry_count) {
        while (trace_index < g_viewer->entry_count) {
            const char *trace_file = g_viewer->entries[trace_index].filename;
            trace_index++;
            
            // Get basename
            const char *basename = get_basename(trace_file);
            size_t name_len = strlen(basename);
            
            // Check if it's a .py file
            if (name_len > 3 && strcmp(basename + name_len - 3, ".py") == 0) {
                // Check if it matches the text
                if (strncmp(basename, text, len) == 0) {
                    // Check if we've already shown this file
                    int already_seen = 0;
                    for (int i = 0; i < seen_count; i++) {
                        if (strcmp(seen_files[i], basename) == 0) {
                            already_seen = 1;
                            break;
                        }
                    }
                    
                    if (!already_seen) {
                        // Add to seen list
                        if (seen_count < 100) {
                            strncpy(seen_files[seen_count], basename, 255);
                            seen_files[seen_count][255] = '\0';
                            seen_count++;
                        }
                        return strdup(basename);
                    }
                }
            }
        }
    }
    
    // Phase 2: Suggest files from current directory
    if (!dir) {
        dir = opendir(".");
    }
    
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            const char *name = entry->d_name;
            size_t name_len = strlen(name);
            
            // Only .py files
            if (name_len > 3 && strcmp(name + name_len - 3, ".py") == 0) {
                // Check if it matches the text
                if (strncmp(name, text, len) == 0) {
                    // Check if we've already shown this file
                    int already_seen = 0;
                    for (int i = 0; i < seen_count; i++) {
                        if (strcmp(seen_files[i], name) == 0) {
                            already_seen = 1;
                            break;
                        }
                    }
                    
                    if (!already_seen) {
                        // Add to seen list
                        if (seen_count < 100) {
                            strncpy(seen_files[seen_count], name, 255);
                            seen_files[seen_count][255] = '\0';
                            seen_count++;
                        }
                        return strdup(name);
                    }
                }
            }
        }
        
        // Done with directory
        closedir(dir);
        dir = NULL;
    }
    
    // No more matches
    return NULL;
}

// Tab completion support
static char* command_generator(const char* text, int state) {
    static int list_index, len;
    
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while (g_commands[list_index]) {
        const char* name = g_commands[list_index];
        list_index++;
        
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }
    
    return NULL;
}

static char** command_completion(const char* text, int start, int end) {
    // Don't use default filename completion
    rl_attempted_completion_over = 1;
    
    // Complete commands at the start of the line
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    
    // For arguments, check what command was typed
    // Get the line buffer to check the command
    const char *line = rl_line_buffer;
    
    // Check if this is 'show' or 'b'/'break' command - complete .py filenames
    if ((strncmp(line, "show ", 5) == 0 && start >= 5) ||
        (strncmp(line, "b ", 2) == 0 && start >= 2) ||
        (strncmp(line, "break ", 6) == 0 && start >= 6)) {
        return rl_completion_matches(text, filename_generator);
    }
    
    // No completion for other arguments
    return NULL;
}

// Initialize readline
void init_readline() {
    // Set up tab completion
    rl_attempted_completion_function = command_completion;
    
    // Set completer delimiters to space, tab, and newline only
    // This prevents readline from breaking on special characters
    rl_completer_word_break_characters = " \t\n";
    
    // Configure readline behavior
    rl_bind_key('\t', rl_complete);  // Ensure TAB triggers completion
    
    // Load history from file
    char* home = getenv("HOME");
    if (home) {
        char history_file[512];
        snprintf(history_file, sizeof(history_file), "%s/.traceviewer_history", home);
        read_history(history_file);
        stifle_history(1000);  // Limit to 1000 entries
    }
}

// Save history on exit
void save_readline_history() {
    char* home = getenv("HOME");
    if (home) {
        char history_file[512];
        snprintf(history_file, sizeof(history_file), "%s/.traceviewer_history", home);
        write_history(history_file);
    }
}

static int execute_command(TraceViewer *viewer, const char *raw_cmd) {
    char input[MAX_LINE_LENGTH];
    char *cmd;

    strncpy(input, raw_cmd, sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';

    cmd = input;
    while (isspace((unsigned char)*cmd)) cmd++;
    rstrip(cmd);

    if (strlen(cmd) == 0) {
        return 0;
    }

    // Handle 'n' command (next)
    if (strcmp(cmd, "n") == 0 || strcmp(cmd, "next") == 0) {
        if (viewer->current_entry < viewer->entry_count - 1) {
            viewer->current_entry++;
            print_current_entry(viewer);
        } else {
            printf("\033[1;31m✗ Already at last execution step\033[0m\n");
        }
    }
    // Handle 'back' command (previous)
    else if (strcmp(cmd, "back") == 0 || strcmp(cmd, "prev") == 0) {
        if (viewer->current_entry > 0) {
            viewer->current_entry--;
            print_current_entry(viewer);
        } else {
            printf("\033[1;31m✗ Already at first execution step\033[0m\n");
        }
    }
    // Handle 'summary' command
    else if (strcmp(cmd, "summary") == 0) {
        print_summary(viewer);
    }
    // Handle 'show' command (with optional filename)
    else if (strncmp(cmd, "show", 4) == 0) {
        if (cmd[4] == '\0') {
            // Just 'show' - show current file
            show_file(viewer, NULL);
        } else if (cmd[4] == ' ') {
            // 'show <filename>' - show specific file
            char *filename = cmd + 5;
            while (isspace((unsigned char)*filename)) filename++;

            if (strlen(filename) > 0) {
                show_file(viewer, filename);
            } else {
                show_file(viewer, NULL);
            }
        } else {
            show_file(viewer, NULL);
        }
    }
    // Handle 'help' command
    else if (strcmp(cmd, "help") == 0) {
        print_help();
    }
    // Handle ':<number>' command (jump to execution)
    else if (cmd[0] == ':') {
        long user_num = atol(cmd + 1);
        // Convert from 1-based user input to 0-based internal execution order
        long exec_num = user_num - 1;
        int found = 0;

        // Validate range (user sees 1 to N, we search for 0 to N-1)
        if (user_num < 1 || user_num > viewer->entry_count) {
            printf("\033[1;31m✗ Execution #%ld out of range. Valid range: 1-%d\033[0m\n",
                   user_num, viewer->entry_count);
        } else {
            for (int i = 0; i < viewer->entry_count; i++) {
                if (viewer->entries[i].exec_order == exec_num) {
                    viewer->current_entry = i;
                    print_current_entry(viewer);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                printf("\033[1;31m✗ Execution #%ld not found in trace\033[0m\n", user_num);
            }
        }
    }
    // Handle 'find <var>' command
    else if (strncmp(cmd, "find ", 5) == 0) {
        char *var_name = cmd + 5;
        while (isspace((unsigned char)*var_name)) var_name++;
        if (strlen(var_name) > 0) {
            search_variable(viewer, var_name);
        } else {
            printf("\033[1;31m✗ Usage: find <variable_name>\033[0m\n");
        }
    }
    // Handle 'b <file> <line>' command (set breakpoint)
    else if (cmd[0] == 'b' && (cmd[1] == ' ' || cmd[1] == '\0')) {
        if (cmd[1] == '\0') {
            // Just 'b' - list breakpoints
            list_breakpoints(viewer);
        } else {
            // 'b <file> <line>' - set breakpoint
            char *args = cmd + 2;
            while (isspace((unsigned char)*args)) args++;

            char filename[512];
            int line_num;

            if (sscanf(args, "%s %d", filename, &line_num) == 2) {
                add_breakpoint(viewer, filename, line_num);
            } else {
                printf("\033[1;31m✗ Usage: b <file> <line>\033[0m\n");
                printf("Example: b test.py 25\n");
            }
        }
    }
    // Handle 'list' command (list breakpoints)
    else if (strcmp(cmd, "list") == 0) {
        list_breakpoints(viewer);
    }
    // Handle 'c' command (continue to next breakpoint)
    else if (strcmp(cmd, "c") == 0 || strcmp(cmd, "continue") == 0) {
        continue_to_breakpoint(viewer);
    }
    // Handle 'rc' command (reverse continue to previous breakpoint)
    else if (strcmp(cmd, "rc") == 0) {
        reverse_continue_to_breakpoint(viewer);
    }
    // Handle 'jump <line>' command (jump to source line - old 'break' behavior)
    else if (strncmp(cmd, "jump ", 5) == 0) {
        int line_num = atoi(cmd + 5);
        int found = 0;

        printf("\nSearching for line %d...\n\n", line_num);
        for (int i = 0; i < viewer->entry_count; i++) {
            if (viewer->entries[i].line_number == line_num) {
                viewer->current_entry = i;
                print_current_entry(viewer);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("\033[1;31m✗ Line %d not found in trace\033[0m\n", line_num);
        }
    }
    // Handle old 'break <line>' command (redirect to 'jump')
    else if (strncmp(cmd, "break ", 6) == 0) {
        int line_num = atoi(cmd + 6);
        int found = 0;

        printf("\n\033[1;33mNote: 'break <line>' is deprecated. Use 'jump <line>' or 'b <file> <line>'\033[0m\n");
        printf("Searching for line %d...\n\n", line_num);
        for (int i = 0; i < viewer->entry_count; i++) {
            if (viewer->entries[i].line_number == line_num) {
                viewer->current_entry = i;
                print_current_entry(viewer);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("\033[1;31m✗ Line %d not found in trace\033[0m\n", line_num);
        }
    }
    // Handle 'w <var>' command (set watchpoint - both read and write)
    else if (cmd[0] == 'w' && (cmd[1] == ' ' || cmd[1] == '\0')) {
        if (cmd[1] == '\0') {
            printf("\033[1;31m✗ Usage: w <variable>\033[0m\n");
            printf("Example: w counter\n");
        } else {
            char *var_name = cmd + 2;
            while (isspace((unsigned char)*var_name)) var_name++;
            if (strlen(var_name) > 0) {
                add_watchpoint(viewer, var_name, WATCHPOINT_BOTH);
            } else {
                printf("\033[1;31m✗ Usage: w <variable>\033[0m\n");
            }
        }
    }
    // Handle 'rw <var>' command (set read watchpoint)
    else if (strncmp(cmd, "rw ", 3) == 0) {
        char *var_name = cmd + 3;
        while (isspace((unsigned char)*var_name)) var_name++;
        if (strlen(var_name) > 0) {
            add_watchpoint(viewer, var_name, WATCHPOINT_READ);
        } else {
            printf("\033[1;31m✗ Usage: rw <variable>\033[0m\n");
            printf("Example: rw counter\n");
        }
    }
    // Handle 'ww <var>' command (set write watchpoint)
    else if (strncmp(cmd, "ww ", 3) == 0) {
        char *var_name = cmd + 3;
        while (isspace((unsigned char)*var_name)) var_name++;
        if (strlen(var_name) > 0) {
            add_watchpoint(viewer, var_name, WATCHPOINT_WRITE);
        } else {
            printf("\033[1;31m✗ Usage: ww <variable>\033[0m\n");
            printf("Example: ww counter\n");
        }
    }
    // Handle 'listw' command (list watchpoints)
    else if (strcmp(cmd, "listw") == 0) {
        list_watchpoints(viewer);
    }
    // Handle 'clearw [num]' command (clear watchpoint(s))
    else if (strncmp(cmd, "clearw", 6) == 0) {
        if (cmd[6] == '\0') {
            // Clear all watchpoints
            clear_watchpoint(viewer, -1);
        } else if (cmd[6] == ' ') {
            // Clear specific watchpoint
            int wp_num = atoi(cmd + 7);
            if (wp_num > 0 && wp_num <= viewer->watchpoint_count) {
                clear_watchpoint(viewer, wp_num - 1);
            } else {
                printf("\033[1;31m✗ Invalid watchpoint number. Use 'listw' to see watchpoints.\033[0m\n");
            }
        }
    }
    // Handle 'q' or 'quit' command
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
        return 1;
    }
    // Eval
    else if (strncmp(cmd, "eval ", 5) == 0) {
        char *expression = cmd + 5;
        while (isspace((unsigned char)*expression)) expression++;
        if (strlen(expression) > 0) {
            eval_expression(viewer, expression);
        } else {
            printf("\033[1;31m✗ Usage: eval <expression>\033[0m\n");
            printf("Example: eval x + y\n");
            printf("Example: eval len(my_list)\n");
        }
    } else {
        printf("\033[1;31m✗ Unknown command. Type 'help' for available commands\033[0m\n");
    }

    return 0;
}

static void run_readline_viewer(TraceViewer *viewer) {
    char *last_command = NULL;

    // Initialize readline for command history and tab completion
    init_readline();

    print_help();

    printf("\n\033[1;32m✓ Tab completion enabled\033[0m (press TAB to autocomplete commands and .py files)\n");

    // Print first entry
    print_current_entry(viewer);

    while (1) {
        char *cmd;
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "\n\033[1;32m[%d/%d]\033[0m > ",
                 viewer->current_entry + 1, viewer->entry_count);

        // Use readline for better line editing and history
        char *line = readline(prompt);

        // Check for EOF (Ctrl+D)
        if (!line) {
            printf("\n");
            break;
        }

        // Trim whitespace
        cmd = line;
        while (isspace((unsigned char)*cmd)) cmd++;

        // Handle empty input - repeat last command
        if (strlen(cmd) == 0) {
            free(line);
            if (last_command) {
                if (execute_command(viewer, last_command)) {
                    break;
                }
            }
            continue;
        }

        // Add non-empty command to history
        add_history(line);

        if (last_command) {
            free(last_command);
        }
        last_command = xstrdup(cmd);

        if (execute_command(viewer, cmd)) {
            free(line);
            break;
        }

        free(line);
    }

    // Save command history
    save_readline_history();

    if (last_command) {
        free(last_command);
    }
}

#define TUI_MAX_LOG_LINES 200
#define TUI_LOG_LINE_LENGTH 1024

typedef enum {
    TUI_LOWER_TRACE,
    TUI_LOWER_BREAKPOINTS,
    TUI_LOWER_LOG
} TuiLowerView;

typedef struct {
    TraceViewer *viewer;
    char command[MAX_LINE_LENGTH];
    int command_len;
    int command_cursor;
    char last_command[MAX_LINE_LENGTH];
    char status[512];
    char log_lines[TUI_MAX_LOG_LINES][TUI_LOG_LINE_LENGTH];
    int log_count;
    TuiLowerView lower_view;
    int help_open;
    int help_scroll;
    int needs_render;
    int running;
} TuiState;

static const char* g_tui_help_lines[] = {
    "Traceviewer TUI",
    "",
    "Navigation",
    "  n, next              step to next execution",
    "  back, prev           step to previous execution",
    "  :<number>            jump to execution number",
    "  c, continue          continue to next breakpoint or watchpoint",
    "  rc                   reverse-continue to previous breakpoint or watchpoint",
    "  jump <line>          jump to first trace entry for a source line",
    "",
    "Breakpoints and Watchpoints",
    "  b <file> <line>      set a breakpoint",
    "  b, list              list breakpoints",
    "  w <var>              watch variable reads and writes",
    "  rw <var>             watch variable reads",
    "  ww <var>             watch variable writes",
    "  listw                list watchpoints",
    "  clearw [num]         clear one watchpoint or all watchpoints",
    "",
    "Inspection",
    "  show [file]          print source file in the log",
    "  summary              print trace summary in the log",
    "  find <var>           search captured variables",
    "  eval <expr>          evaluate a Python expression from captured values",
    "",
    "TUI",
    "  help                 open this help view",
    "  view trace           show trace history in the lower pane",
    "  view breakpoints     show breakpoints and watchpoints in the lower pane",
    "  view log             show command output in the lower pane",
    "  F2                   cycle lower pane",
    "  Tab                  complete command or filename",
    "  Up/Down              step back/forward when the command line is empty",
    "  PageUp/PageDown      scroll this help view",
    "  Esc                  close help",
    "  q, quit              exit traceviewer",
    NULL
};

static struct termios g_original_termios;
static int g_raw_mode_enabled = 0;

static void tui_disable_raw_mode() {
    if (g_raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_original_termios);
        g_raw_mode_enabled = 0;
    }
}

static int tui_enable_raw_mode() {
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &g_original_termios) == -1) {
        return 0;
    }

    raw = g_original_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return 0;
    }

    g_raw_mode_enabled = 1;
    return 1;
}

static void tui_get_size(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        *rows = 24;
        *cols = 80;
        return;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
}

static void tui_move(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

static void tui_clear() {
    printf("\033[2J\033[H");
}

static void tui_write_clipped(int row, int col, int width, const char *text) {
    char buffer[4096];
    int out = 0;
    int used = 0;

    if (width <= 0) {
        return;
    }

    for (const char *p = text; *p && used < width && out < (int)sizeof(buffer) - 1; p++) {
        unsigned char ch = (unsigned char)*p;

        if (ch == '\n' || ch == '\r') {
            break;
        }
        if (ch == '\t') {
            int spaces = 4 - (used % 4);
            while (spaces-- > 0 && used < width && out < (int)sizeof(buffer) - 1) {
                buffer[out++] = ' ';
                used++;
            }
            continue;
        }
        if (iscntrl(ch)) {
            ch = ' ';
        }

        buffer[out++] = (char)ch;
        used++;
    }
    buffer[out] = '\0';

    tui_move(row, col);
    printf("%-*s", width, buffer);
}

static void tui_printf_clipped(int row, int col, int width, const char *fmt, ...) {
    char buffer[4096];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    tui_write_clipped(row, col, width, buffer);
}

static void tui_draw_horizontal(int row, int col, int width) {
    if (width <= 0) {
        return;
    }

    tui_move(row, col);
    for (int i = 0; i < width; i++) {
        putchar('-');
    }
}

static void tui_draw_box(int row, int col, int height, int width, const char *title) {
    if (height < 2 || width < 2) {
        return;
    }

    tui_move(row, col);
    putchar('+');
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    putchar('+');

    for (int r = row + 1; r < row + height - 1; r++) {
        tui_move(r, col);
        putchar('|');
        for (int i = 0; i < width - 2; i++) {
            putchar(' ');
        }
        putchar('|');
    }

    tui_move(row + height - 1, col);
    putchar('+');
    for (int i = 0; i < width - 2; i++) {
        putchar('-');
    }
    putchar('+');

    if (title && title[0] != '\0' && width > 4) {
        char title_buffer[512];
        snprintf(title_buffer, sizeof(title_buffer), " %s ", title);
        tui_write_clipped(row, col + 2, width - 4, title_buffer);
    }
}

static void tui_strip_ansi_line(const char *input, char *output, size_t output_size) {
    size_t out = 0;

    if (output_size == 0) {
        return;
    }

    for (const char *p = input; *p && out < output_size - 1; p++) {
        unsigned char ch = (unsigned char)*p;

        if (ch == '\033' && p[1] == '[') {
            p += 2;
            while (*p && !isalpha((unsigned char)*p)) {
                p++;
            }
            continue;
        }
        if (ch == '\n' || ch == '\r') {
            break;
        }
        if (ch == '\t') {
            ch = ' ';
        }
        if (ch >= 128) {
            if (out == 0 || output[out - 1] != '-') {
                output[out++] = '-';
            }
            continue;
        }
        if (iscntrl(ch)) {
            continue;
        }

        output[out++] = (char)ch;
    }

    output[out] = '\0';
}

static int tui_is_separator_log_line(const char *line) {
    int saw_separator = 0;

    for (const char *p = line; *p; p++) {
        if (isspace((unsigned char)*p)) {
            continue;
        }
        if (*p == '-' || *p == '=') {
            saw_separator = 1;
            continue;
        }
        return 0;
    }

    return saw_separator;
}

static void tui_add_log_line(TuiState *state, const char *line) {
    char clean[TUI_LOG_LINE_LENGTH];
    size_t clean_len;

    tui_strip_ansi_line(line, clean, sizeof(clean));
    if (clean[0] == '\0' || tui_is_separator_log_line(clean)) {
        return;
    }

    if (state->log_count == TUI_MAX_LOG_LINES) {
        memmove(state->log_lines, state->log_lines + 1,
                sizeof(state->log_lines[0]) * (TUI_MAX_LOG_LINES - 1));
        state->log_count--;
    }

    clean_len = strnlen(clean, TUI_LOG_LINE_LENGTH - 1);
    memcpy(state->log_lines[state->log_count], clean, clean_len);
    state->log_lines[state->log_count][clean_len] = '\0';
    state->log_count++;
}

static void tui_add_output(TuiState *state, const char *output) {
    char line[TUI_LOG_LINE_LENGTH];
    int pos = 0;

    if (!output || output[0] == '\0') {
        return;
    }

    for (const char *p = output; ; p++) {
        if (*p == '\n' || *p == '\0' || pos >= TUI_LOG_LINE_LENGTH - 1) {
            line[pos] = '\0';
            tui_add_log_line(state, line);
            pos = 0;
            if (*p == '\0') {
                break;
            }
            continue;
        }
        line[pos++] = *p;
    }
}

static int tui_line_has_breakpoint(TraceViewer *viewer, const char *filename, int line_number) {
    for (int i = 0; i < viewer->breakpoint_count; i++) {
        if (viewer->breakpoints[i].line_number == line_number &&
            filenames_match(viewer->breakpoints[i].filename, filename)) {
            return 1;
        }
    }

    return 0;
}

static FILE* tui_open_current_source(TraceViewer *viewer, char *resolved_path, size_t resolved_size) {
    TraceEntry *entry = &viewer->entries[viewer->current_entry];
    FILE *file = fopen(entry->filename, "r");

    if (file) {
        strncpy(resolved_path, entry->filename, resolved_size - 1);
        resolved_path[resolved_size - 1] = '\0';
        return file;
    }

    file = try_open_with_trace_dir(get_basename(entry->filename), viewer, resolved_path, resolved_size);
    if (file) {
        resolved_path[resolved_size - 1] = '\0';
    }

    return file;
}

static void tui_render_header(TuiState *state, int cols) {
    TraceViewer *viewer = state->viewer;
    TraceEntry *entry = &viewer->entries[viewer->current_entry];
    const char *stop_reason = is_at_breakpoint(viewer, viewer->current_entry) ? "breakpoint" : "line";

    tui_draw_box(1, 1, 3, cols, "");
    tui_printf_clipped(2, 3, cols - 4,
                       "target %s / execution %d of %d / frame %s:%d / status paused / stop %s",
                       get_basename(entry->filename),
                       viewer->current_entry + 1,
                       viewer->entry_count,
                       get_basename(entry->filename),
                       entry->line_number,
                       stop_reason);
}

static void tui_render_source(TuiState *state, int row, int col, int height, int width) {
    TraceViewer *viewer = state->viewer;
    TraceEntry *entry = &viewer->entries[viewer->current_entry];
    char resolved_path[1026] = {0};
    FILE *file;
    int inner_rows = height - 2;
    int start_line;
    int end_line;
    int line_num = 1;
    int out_row = row + 1;

    tui_draw_box(row, col, height, width, "source / current line centered");

    if (inner_rows <= 0) {
        return;
    }

    file = tui_open_current_source(viewer, resolved_path, sizeof(resolved_path));
    if (!file) {
        tui_printf_clipped(row + 1, col + 2, width - 4, "%s:%d", entry->filename, entry->line_number);
        tui_printf_clipped(row + 2, col + 2, width - 4, "%s", entry->code);
        return;
    }

    start_line = entry->line_number - inner_rows / 2;
    if (start_line < 1) {
        start_line = 1;
    }
    end_line = start_line + inner_rows - 1;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        if (line_num >= start_line && line_num <= end_line && out_row < row + height - 1) {
            char marker = ' ';
            char rendered[MAX_LINE_LENGTH + 64];
            int current = line_num == entry->line_number;

            if (current) {
                marker = '>';
            } else if (tui_line_has_breakpoint(viewer, resolved_path, line_num)) {
                marker = 'B';
            }

            snprintf(rendered, sizeof(rendered), "%c %5d | %s", marker, line_num, line);
            if (current) {
                printf("\033[7m");
                tui_write_clipped(out_row, col + 1, width - 2, rendered);
                printf("\033[0m");
            } else if (marker == 'B') {
                printf("\033[31m");
                tui_write_clipped(out_row, col + 1, width - 2, rendered);
                printf("\033[0m");
            } else {
                tui_write_clipped(out_row, col + 1, width - 2, rendered);
            }
            out_row++;
        }

        if (line_num > end_line) {
            break;
        }
        line_num++;
    }

    fclose(file);
}

static const char* tui_watchpoint_type_name(WatchpointType type) {
    if (type == WATCHPOINT_READ) {
        return "read";
    }
    if (type == WATCHPOINT_WRITE) {
        return "write";
    }
    return "read/write";
}

static const char* tui_find_var_value(VarState *vars, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }

    return NULL;
}

static int tui_help_line_count() {
    int count = 0;

    while (g_tui_help_lines[count]) {
        count++;
    }

    return count;
}

static void tui_render_help(TuiState *state, int row, int col, int height, int width) {
    int content_rows = height - 2;
    int help_lines = tui_help_line_count();
    int max_scroll = help_lines - content_rows;

    if (max_scroll < 0) {
        max_scroll = 0;
    }
    if (state->help_scroll > max_scroll) {
        state->help_scroll = max_scroll;
    }

    tui_draw_box(row, col, height, width, "help / command reference");

    for (int i = 0; i < content_rows && i + state->help_scroll < help_lines; i++) {
        tui_write_clipped(row + 1 + i, col + 2, width - 4,
                          g_tui_help_lines[i + state->help_scroll]);
    }

    if (max_scroll > 0) {
        tui_printf_clipped(row + height - 1, col + width - 24, 20,
                           " %d/%d ",
                           state->help_scroll + 1,
                           max_scroll + 1);
    }
}

static void tui_append_text(char *dest, size_t dest_size, size_t *dest_len, const char *text) {
    size_t available;
    size_t copy_len;

    if (*dest_len >= dest_size - 1) {
        return;
    }

    available = dest_size - *dest_len - 1;
    copy_len = strnlen(text, available);
    memcpy(dest + *dest_len, text, copy_len);
    *dest_len += copy_len;
    dest[*dest_len] = '\0';
}

static void tui_render_locals(TuiState *state, int row, int col, int height, int width) {
    TraceViewer *viewer = state->viewer;
    TraceEntry *entry = &viewer->entries[viewer->current_entry];
    VarState curr_vars[MAX_VARS];
    VarState prev_vars[MAX_VARS];
    int curr_count = 0;
    int prev_count = 0;
    int out_row = row + 1;
    int last_row = row + height - 1;

    tui_draw_box(row, col, height, width, "locals / watches / diff-highlighted");

    parse_variables(entry->variables, curr_vars, &curr_count, MAX_VARS);
    if (viewer->current_entry > 0) {
        parse_variables(viewer->entries[viewer->current_entry - 1].variables,
                        prev_vars, &prev_count, MAX_VARS);
    }

    tui_printf_clipped(out_row++, col + 2, width - 4,
                       "locals %d  watches %d  changed marked *",
                       curr_count, viewer->watchpoint_count);
    tui_draw_horizontal(out_row++, col + 1, width - 2);

    if (curr_count == 0 && out_row < last_row) {
        tui_printf_clipped(out_row++, col + 2, width - 4, "(no locals captured)");
    }

    for (int i = 0; i < curr_count && out_row < last_row; i++) {
        char rendered[1200];
        size_t rendered_len = 0;
        int changed = variable_written(curr_vars[i].name, prev_vars, prev_count,
                                       curr_vars, curr_count);

        rendered[rendered_len++] = changed ? '*' : ' ';
        rendered[rendered_len++] = ' ';
        rendered[rendered_len] = '\0';
        tui_append_text(rendered, sizeof(rendered), &rendered_len, curr_vars[i].name);
        tui_append_text(rendered, sizeof(rendered), &rendered_len, " = ");
        tui_append_text(rendered, sizeof(rendered), &rendered_len, curr_vars[i].value);

        if (changed) {
            printf("\033[33m");
            tui_write_clipped(out_row, col + 2, width - 4, rendered);
            printf("\033[0m");
        } else {
            tui_write_clipped(out_row, col + 2, width - 4, rendered);
        }
        out_row++;
    }

    if (viewer->watchpoint_count > 0 && out_row + 1 < last_row) {
        tui_draw_horizontal(out_row++, col + 1, width - 2);
        tui_printf_clipped(out_row++, col + 2, width - 4, "watches");
        for (int i = 0; i < viewer->watchpoint_count && out_row < last_row; i++) {
            const char *value = tui_find_var_value(curr_vars, curr_count,
                                                   viewer->watchpoints[i].variable);
            tui_printf_clipped(out_row++, col + 2, width - 4,
                               "%s (%s) = %s",
                               viewer->watchpoints[i].variable,
                               tui_watchpoint_type_name(viewer->watchpoints[i].type),
                               value ? value : "(not captured)");
        }
    }
}

static void tui_render_lower_trace(TuiState *state, int row, int col, int rows, int width) {
    TraceViewer *viewer = state->viewer;
    int start = viewer->current_entry - rows / 2;

    if (start < 0) {
        start = 0;
    }
    if (start + rows > viewer->entry_count) {
        start = viewer->entry_count - rows;
        if (start < 0) {
            start = 0;
        }
    }

    for (int i = 0; i < rows && start + i < viewer->entry_count; i++) {
        int index = start + i;
        TraceEntry *entry = &viewer->entries[index];
        char rendered[MAX_LINE_LENGTH + 128];
        char marker = index == viewer->current_entry ? '>' : ' ';

        snprintf(rendered, sizeof(rendered), "%c [%ld] %s:%d  %s",
                 marker, entry->exec_order, get_basename(entry->filename),
                 entry->line_number, entry->code);
        if (index == viewer->current_entry) {
            printf("\033[7m");
            tui_write_clipped(row + i, col, width, rendered);
            printf("\033[0m");
        } else {
            tui_write_clipped(row + i, col, width, rendered);
        }
    }
}

static void tui_render_lower_breakpoints(TuiState *state, int row, int col, int rows, int width) {
    TraceViewer *viewer = state->viewer;
    int out_row = row;

    if (viewer->breakpoint_count == 0 && viewer->watchpoint_count == 0) {
        tui_write_clipped(out_row, col, width, "no breakpoints or watchpoints");
        return;
    }

    if (viewer->breakpoint_count > 0 && out_row < row + rows) {
        tui_write_clipped(out_row++, col, width, "breakpoints");
        for (int i = 0; i < viewer->breakpoint_count && out_row < row + rows; i++) {
            tui_printf_clipped(out_row++, col, width, "%2d  %s:%d",
                               i + 1,
                               viewer->breakpoints[i].filename,
                               viewer->breakpoints[i].line_number);
        }
    }

    if (viewer->watchpoint_count > 0 && out_row < row + rows) {
        tui_write_clipped(out_row++, col, width, "watchpoints");
        for (int i = 0; i < viewer->watchpoint_count && out_row < row + rows; i++) {
            tui_printf_clipped(out_row++, col, width, "%2d  %s (%s)",
                               i + 1,
                               viewer->watchpoints[i].variable,
                               tui_watchpoint_type_name(viewer->watchpoints[i].type));
        }
    }
}

static void tui_render_lower_log(TuiState *state, int row, int col, int rows, int width) {
    int start = state->log_count - rows;

    if (start < 0) {
        start = 0;
    }

    for (int i = 0; i < rows && start + i < state->log_count; i++) {
        tui_write_clipped(row + i, col, width, state->log_lines[start + i]);
    }
}

static void tui_render_lower(TuiState *state, int row, int col, int height, int width) {
    const char *selected;
    char title[128];
    int content_rows = height - 2;

    if (state->lower_view == TUI_LOWER_TRACE) {
        selected = "F2: [trace] breakpoints log";
    } else if (state->lower_view == TUI_LOWER_BREAKPOINTS) {
        selected = "F2: trace [breakpoints] log";
    } else {
        selected = "F2: trace breakpoints [log]";
    }

    snprintf(title, sizeof(title), "%s", selected);
    tui_draw_box(row, col, height, width, title);

    if (content_rows <= 0) {
        return;
    }

    if (state->lower_view == TUI_LOWER_TRACE) {
        tui_render_lower_trace(state, row + 1, col + 2, content_rows, width - 4);
    } else if (state->lower_view == TUI_LOWER_BREAKPOINTS) {
        tui_render_lower_breakpoints(state, row + 1, col + 2, content_rows, width - 4);
    } else {
        tui_render_lower_log(state, row + 1, col + 2, content_rows, width - 4);
    }
}

static void tui_render_command(TuiState *state, int row, int cols) {
    char prompt[MAX_LINE_LENGTH + 16];
    int cursor_col;

    tui_draw_box(row, 1, 3, cols, "");
    snprintf(prompt, sizeof(prompt), ": %s", state->command);
    tui_write_clipped(row + 1, 3, cols - 4, prompt);

    if (state->status[0] != '\0') {
        tui_write_clipped(row, 3, cols - 4, state->status);
    } else {
        tui_write_clipped(row, 3, cols - 4,
                          "Tab completes / F2 switches lower pane / help opens reference / Ctrl-C quits");
    }

    cursor_col = 5 + state->command_cursor;
    if (cursor_col > cols - 2) {
        cursor_col = cols - 2;
    }
    tui_move(row + 1, cursor_col);
}

static void tui_render(TuiState *state) {
    int rows;
    int cols;
    int header_h = 3;
    int command_h = 3;
    int lower_h;
    int middle_h;
    int middle_row;
    int lower_row;
    int command_row;
    int source_w;
    int locals_w;

    tui_get_size(&rows, &cols);

    if (rows < 20 || cols < 70) {
        tui_clear();
        tui_write_clipped(1, 1, cols, "Terminal too small for traceviewer TUI. Resize to at least 70x20.");
        fflush(stdout);
        return;
    }

    lower_h = rows >= 32 ? 8 : 6;
    middle_h = rows - header_h - lower_h - command_h;
    if (middle_h < 8) {
        lower_h = 5;
        middle_h = rows - header_h - lower_h - command_h;
    }

    middle_row = header_h + 1;
    lower_row = middle_row + middle_h;
    command_row = rows - command_h + 1;
    locals_w = cols / 3;
    if (locals_w < 28) {
        locals_w = 28;
    }
    source_w = cols - locals_w;

    tui_clear();
    printf("\033[?25l");

    tui_render_header(state, cols);
    if (state->help_open) {
        tui_render_help(state, middle_row, 1, middle_h + lower_h, cols);
    } else {
        tui_render_source(state, middle_row, 1, middle_h, source_w);
        tui_render_locals(state, middle_row, source_w, middle_h, locals_w + 1);
        tui_render_lower(state, lower_row, 1, lower_h, cols);
    }
    tui_render_command(state, command_row, cols);

    printf("\033[?25h");
    fflush(stdout);
}

static char* tui_capture_command(TraceViewer *viewer, const char *command, int *quit) {
    FILE *tmp;
    int saved_stdout;
    long size;
    char *output;

    *quit = 0;
    tmp = tmpfile();
    if (!tmp) {
        *quit = execute_command(viewer, command);
        return xstrdup("");
    }

    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout == -1 || dup2(fileno(tmp), STDOUT_FILENO) == -1) {
        if (saved_stdout != -1) {
            close(saved_stdout);
        }
        fclose(tmp);
        *quit = execute_command(viewer, command);
        return xstrdup("");
    }

    *quit = execute_command(viewer, command);

    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    fseek(tmp, 0, SEEK_END);
    size = ftell(tmp);
    if (size < 0) {
        size = 0;
    }
    rewind(tmp);

    output = malloc((size_t)size + 1);
    if (!output) {
        fclose(tmp);
        return NULL;
    }

    if (size > 0) {
        fread(output, 1, (size_t)size, tmp);
    }
    output[size] = '\0';

    fclose(tmp);
    return output;
}

static void tui_set_status(TuiState *state, const char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vsnprintf(state->status, sizeof(state->status), fmt, args);
    va_end(args);
}

#define TUI_MAX_COMPLETION_MATCHES 64
#define TUI_COMPLETION_LENGTH 256

static int tui_current_token(TuiState *state, int *token_start, int *token_end, int *token_index) {
    int p = 0;
    int index = 0;

    while (p < state->command_len) {
        while (p < state->command_len && isspace((unsigned char)state->command[p])) {
            p++;
        }
        if (p >= state->command_len) {
            break;
        }

        int start = p;
        while (p < state->command_len && !isspace((unsigned char)state->command[p])) {
            p++;
        }
        int end = p;

        if (state->command_cursor >= start && state->command_cursor <= end) {
            *token_start = start;
            *token_end = end;
            *token_index = index;
            return 1;
        }

        index++;
    }

    *token_start = state->command_cursor;
    *token_end = state->command_cursor;
    *token_index = index;
    return 1;
}

static void tui_first_token(TuiState *state, char *token, size_t token_size) {
    int p = 0;
    int out = 0;

    while (p < state->command_len && isspace((unsigned char)state->command[p])) {
        p++;
    }

    while (p < state->command_len &&
           !isspace((unsigned char)state->command[p]) &&
           out < (int)token_size - 1) {
        token[out++] = state->command[p++];
    }
    token[out] = '\0';
}

static int tui_collect_static_matches(const char *prefix, const char **values,
                                      char matches[][TUI_COMPLETION_LENGTH], int max_matches) {
    int count = 0;
    size_t prefix_len = strlen(prefix);

    for (int i = 0; values[i] && count < max_matches; i++) {
        if (strncmp(values[i], prefix, prefix_len) == 0) {
            strncpy(matches[count], values[i], TUI_COMPLETION_LENGTH - 1);
            matches[count][TUI_COMPLETION_LENGTH - 1] = '\0';
            count++;
        }
    }

    return count;
}

static int tui_collect_file_matches(const char *prefix,
                                    char matches[][TUI_COMPLETION_LENGTH], int max_matches) {
    int count = 0;
    int state = 0;
    char *match;

    while (count < max_matches && (match = filename_generator(prefix, state)) != NULL) {
        strncpy(matches[count], match, TUI_COMPLETION_LENGTH - 1);
        matches[count][TUI_COMPLETION_LENGTH - 1] = '\0';
        free(match);
        count++;
        state++;
    }

    return count;
}

static void tui_common_completion_prefix(char matches[][TUI_COMPLETION_LENGTH], int count,
                                         char *common, size_t common_size) {
    size_t common_len;

    if (count <= 0 || common_size == 0) {
        if (common_size > 0) {
            common[0] = '\0';
        }
        return;
    }

    common_len = strnlen(matches[0], common_size - 1);
    memcpy(common, matches[0], common_len);
    common[common_len] = '\0';
    common_len = strlen(common);

    for (int i = 1; i < count; i++) {
        size_t j = 0;
        while (j < common_len && matches[i][j] && common[j] == matches[i][j]) {
            j++;
        }
        common[j] = '\0';
        common_len = j;
    }
}

static void tui_show_completion_matches(TuiState *state,
                                        char matches[][TUI_COMPLETION_LENGTH], int count) {
    char status[512] = "matches:";
    size_t status_len = strlen(status);
    int shown = count < 6 ? count : 6;

    for (int i = 0; i < shown; i++) {
        tui_append_text(status, sizeof(status), &status_len, " ");
        tui_append_text(status, sizeof(status), &status_len, matches[i]);
    }
    if (count > shown) {
        tui_append_text(status, sizeof(status), &status_len, " ...");
    }

    tui_set_status(state, "%s", status);
}

static void tui_replace_command_range(TuiState *state, int start, int end,
                                      const char *replacement, int append_space) {
    char updated[MAX_LINE_LENGTH];
    int replacement_len = strlen(replacement);
    int new_len = state->command_len - (end - start) + replacement_len + append_space;

    if (new_len >= MAX_LINE_LENGTH) {
        tui_set_status(state, "completion is too long");
        return;
    }

    memcpy(updated, state->command, start);
    memcpy(updated + start, replacement, replacement_len);
    if (append_space) {
        updated[start + replacement_len] = ' ';
    }
    memcpy(updated + start + replacement_len + append_space,
           state->command + end,
           (size_t)(state->command_len - end + 1));

    memcpy(state->command, updated, (size_t)new_len + 1);
    state->command_len = new_len;
    state->command_cursor = start + replacement_len + append_space;
}

static void tui_complete_command(TuiState *state) {
    char first[TUI_COMPLETION_LENGTH];
    char prefix[TUI_COMPLETION_LENGTH];
    char matches[TUI_MAX_COMPLETION_MATCHES][TUI_COMPLETION_LENGTH];
    char common[TUI_COMPLETION_LENGTH];
    int token_start;
    int token_end;
    int token_index;
    int prefix_len;
    int match_count = 0;
    int append_space = 0;

    tui_current_token(state, &token_start, &token_end, &token_index);
    prefix_len = state->command_cursor - token_start;
    if (prefix_len < 0) {
        prefix_len = 0;
    }
    if (prefix_len >= TUI_COMPLETION_LENGTH) {
        prefix_len = TUI_COMPLETION_LENGTH - 1;
    }
    memcpy(prefix, state->command + token_start, (size_t)prefix_len);
    prefix[prefix_len] = '\0';

    if (token_index == 0) {
        match_count = tui_collect_static_matches(prefix, g_commands, matches,
                                                 TUI_MAX_COMPLETION_MATCHES);
        append_space = 1;
    } else {
        tui_first_token(state, first, sizeof(first));
        if ((strcmp(first, "show") == 0 ||
             strcmp(first, "b") == 0 ||
             strcmp(first, "break") == 0) &&
            token_index == 1) {
            match_count = tui_collect_file_matches(prefix, matches,
                                                   TUI_MAX_COMPLETION_MATCHES);
            append_space = 1;
        } else if (strcmp(first, "view") == 0 && token_index == 1) {
            match_count = tui_collect_static_matches(prefix, g_lower_views, matches,
                                                     TUI_MAX_COMPLETION_MATCHES);
            append_space = 0;
        }
    }

    if (match_count == 0) {
        tui_set_status(state, "no completions");
        return;
    }

    tui_common_completion_prefix(matches, match_count, common, sizeof(common));
    if (match_count == 1) {
        tui_replace_command_range(state, token_start, token_end, matches[0], append_space);
        tui_set_status(state, "completed %s", matches[0]);
        return;
    }

    if (strlen(common) > strlen(prefix)) {
        tui_replace_command_range(state, token_start, token_end, common, 0);
    }
    tui_show_completion_matches(state, matches, match_count);
}

static int tui_handle_view_command(TuiState *state, const char *command) {
    if (strcmp(command, "view trace") == 0 || strcmp(command, "view backtrace") == 0) {
        state->lower_view = TUI_LOWER_TRACE;
    } else if (strcmp(command, "view breakpoints") == 0 || strcmp(command, "view b") == 0) {
        state->lower_view = TUI_LOWER_BREAKPOINTS;
    } else if (strcmp(command, "view log") == 0) {
        state->lower_view = TUI_LOWER_LOG;
    } else {
        return 0;
    }

    tui_set_status(state, "lower view changed");
    return 1;
}

static void tui_submit_command(TuiState *state) {
    char command[MAX_LINE_LENGTH];
    char *trimmed;
    char *output;
    int quit = 0;

    strncpy(command, state->command, sizeof(command) - 1);
    command[sizeof(command) - 1] = '\0';
    trimmed = command;
    while (isspace((unsigned char)*trimmed)) trimmed++;
    rstrip(trimmed);

    if (trimmed[0] == '\0') {
        if (state->last_command[0] == '\0') {
            return;
        }
        strncpy(command, state->last_command, sizeof(command) - 1);
        command[sizeof(command) - 1] = '\0';
        trimmed = command;
    } else {
        strncpy(state->last_command, trimmed, sizeof(state->last_command) - 1);
        state->last_command[sizeof(state->last_command) - 1] = '\0';
    }

    tui_add_log_line(state, "");
    char log_command[MAX_LINE_LENGTH + 4];
    snprintf(log_command, sizeof(log_command), ": %s", trimmed);
    tui_add_log_line(state, log_command);

    if (strcmp(trimmed, "help") == 0) {
        state->help_open = 1;
        state->help_scroll = 0;
        tui_set_status(state, "help opened; Esc closes, PageUp/PageDown scroll");
    } else if (tui_handle_view_command(state, trimmed)) {
        state->help_open = 0;
    } else {
        state->help_open = 0;
        output = tui_capture_command(state->viewer, trimmed, &quit);
        if (output) {
            if (output[0] != '\0') {
                tui_add_output(state, output);
                state->lower_view = TUI_LOWER_LOG;
            }
            free(output);
        }
        tui_set_status(state, "last command: %s", trimmed);
        if (quit) {
            state->running = 0;
        }
    }

    state->command[0] = '\0';
    state->command_len = 0;
    state->command_cursor = 0;
}

static void tui_cycle_lower_view(TuiState *state) {
    if (state->lower_view == TUI_LOWER_LOG) {
        state->lower_view = TUI_LOWER_TRACE;
    } else {
        state->lower_view++;
    }
    tui_set_status(state, "lower view changed");
}

static void tui_insert_char(TuiState *state, char ch) {
    if (state->command_len >= MAX_LINE_LENGTH - 1) {
        return;
    }

    memmove(state->command + state->command_cursor + 1,
            state->command + state->command_cursor,
            (size_t)(state->command_len - state->command_cursor + 1));
    state->command[state->command_cursor] = ch;
    state->command_len++;
    state->command_cursor++;
}

static void tui_delete_before_cursor(TuiState *state) {
    if (state->command_cursor <= 0) {
        return;
    }

    memmove(state->command + state->command_cursor - 1,
            state->command + state->command_cursor,
            (size_t)(state->command_len - state->command_cursor + 1));
    state->command_cursor--;
    state->command_len--;
}

static void tui_scroll_help(TuiState *state, int delta) {
    if (!state->help_open) {
        return;
    }

    state->help_scroll += delta;
    if (state->help_scroll < 0) {
        state->help_scroll = 0;
    }
    tui_set_status(state, "help opened; Esc closes, PageUp/PageDown scroll");
}

static void tui_handle_escape(TuiState *state) {
    char seq[4];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) {
        if (state->help_open) {
            state->help_open = 0;
            tui_set_status(state, "help closed");
        }
        return;
    }
    if (read(STDIN_FILENO, &seq[1], 1) != 1) {
        if (state->help_open) {
            state->help_open = 0;
            tui_set_status(state, "help closed");
        }
        return;
    }

    if (seq[0] == 'O' && seq[1] == 'Q') {
        tui_cycle_lower_view(state);
    } else if (seq[0] == '[') {
        if (seq[1] == 'C' && state->command_cursor < state->command_len) {
            state->command_cursor++;
        } else if (seq[1] == 'D' && state->command_cursor > 0) {
            state->command_cursor--;
        } else if (seq[1] == 'A' && state->command_len == 0) {
            if (state->help_open) {
                tui_scroll_help(state, -1);
            } else {
                strncpy(state->command, "back", sizeof(state->command) - 1);
                state->command[sizeof(state->command) - 1] = '\0';
                state->command_len = strlen(state->command);
                state->command_cursor = state->command_len;
                tui_submit_command(state);
            }
        } else if (seq[1] == 'B' && state->command_len == 0) {
            if (state->help_open) {
                tui_scroll_help(state, 1);
            } else {
                strncpy(state->command, "n", sizeof(state->command) - 1);
                state->command[sizeof(state->command) - 1] = '\0';
                state->command_len = strlen(state->command);
                state->command_cursor = state->command_len;
                tui_submit_command(state);
            }
        } else if ((seq[1] == '5' || seq[1] == '6') &&
                   read(STDIN_FILENO, &seq[2], 1) == 1 &&
                   seq[2] == '~') {
            tui_scroll_help(state, seq[1] == '5' ? -8 : 8);
        } else if (seq[1] == '1' &&
                   read(STDIN_FILENO, &seq[2], 1) == 1 &&
                   seq[2] == '2' &&
                   read(STDIN_FILENO, &seq[3], 1) == 1 &&
                   seq[3] == '~') {
            tui_cycle_lower_view(state);
        }
    }
}

static void run_tui_viewer(TraceViewer *viewer) {
    TuiState state;

    memset(&state, 0, sizeof(state));
    state.viewer = viewer;
    state.lower_view = TUI_LOWER_TRACE;
    state.running = 1;
    state.needs_render = 1;
    snprintf(state.status, sizeof(state.status),
             "Tab completes / F2 switches lower pane / help opens reference / Ctrl-C quits");
    tui_add_log_line(&state, "traceviewer TUI ready");

    if (!tui_enable_raw_mode()) {
        fprintf(stderr, "Failed to enter raw terminal mode; falling back to readline.\n");
        run_readline_viewer(viewer);
        return;
    }

    g_tui_mode = 1;
    printf("\033[?1049h");

    while (state.running) {
        char ch;

        if (state.needs_render) {
            tui_render(&state);
            state.needs_render = 0;
        }
        if (read(STDIN_FILENO, &ch, 1) != 1) {
            continue;
        }

        if (ch == 3 || ch == 4) {
            state.running = 0;
        } else if (ch == '\r' || ch == '\n') {
            tui_submit_command(&state);
        } else if (ch == 127 || ch == 8) {
            tui_delete_before_cursor(&state);
        } else if (ch == '\t') {
            tui_complete_command(&state);
        } else if (ch == 27) {
            tui_handle_escape(&state);
        } else if (isprint((unsigned char)ch)) {
            tui_insert_char(&state, ch);
        }
        state.needs_render = 1;
    }

    printf("\033[?1049l\033[?25h\033[0m");
    fflush(stdout);
    tui_disable_raw_mode();
    g_tui_mode = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <trace_file>\n", argv[0]);
        fprintf(stderr, "Example: %s trace.log\n", argv[0]);
        return 1;
    }

    TraceViewer viewer;

    printf("\033[1;36m╔════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║         Python Time-Traveling Debugger v1.0          ║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════╝\033[0m\n\n");

    printf("Loading trace file: %s\n", argv[1]);

    if (!read_trace_file(argv[1], &viewer)) {
        return 1;
    }

    if (viewer.entry_count == 0) {
        printf("Trace file is empty or invalid\n");
        cleanup(&viewer);
        return 0;
    }

    printf("✓ Loaded %d execution steps\n", viewer.entry_count);
    
    // Set global pointer for autocomplete access
    g_viewer = &viewer;

    if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
        run_tui_viewer(&viewer);
    } else {
        run_readline_viewer(&viewer);
    }
    
    cleanup(&viewer);
    printf("\n\033[1;36mGoodbye.\033[0m\n\n");
    return 0;
}
