#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

// Readline support (mandatory)
#include <readline/readline.h>
#include <readline/history.h>
#define HAS_READLINE 1

#define MAX_LINE_LENGTH 4096
#define MAX_LINES 100000
#define MAX_FIELD_LENGTH 2048
#define MAX_BREAKPOINTS 100
#define MAX_WATCHPOINTS 50
#define MAX_VARS 100

typedef struct {
    long exec_order;
    char filename[512];
    int line_number;
    char code[MAX_FIELD_LENGTH];
    char variables[MAX_FIELD_LENGTH];
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
    char name[256];
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
} TraceViewer;

// Global pointer for autocomplete (needs access to trace files)
static TraceViewer *g_viewer = NULL;

// Forward declarations
void print_current_entry(TraceViewer *viewer);

// Parse a trace line into a TraceEntry
int parse_trace_line(char *line, TraceEntry *entry) {
    // Format: EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';
    
    // Split by ||| delimiter
    char *parts[5] = {NULL};
    int part_count = 0;
    char *ptr = line_copy;
    char *start = ptr;
    
    while (*ptr && part_count < 5) {
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
    strncpy(entry->code, parts[3], sizeof(entry->code) - 1);
    entry->code[sizeof(entry->code) - 1] = '\0';
    
    // Parse variables (5th part, may be empty)
    if (part_count >= 5 && parts[4] && strlen(parts[4]) > 0) {
        strncpy(entry->variables, parts[4], sizeof(entry->variables) - 1);
        entry->variables[sizeof(entry->variables) - 1] = '\0';
    } else {
        entry->variables[0] = '\0';
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
    
    char vars_copy[MAX_FIELD_LENGTH];
    strncpy(vars_copy, vars_str, MAX_FIELD_LENGTH - 1);
    vars_copy[MAX_FIELD_LENGTH - 1] = '\0';
    
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
    char buffer[MAX_LINE_LENGTH];
    int first_line = 1;

    while (fgets(buffer, sizeof(buffer), file) && viewer->entry_count < MAX_LINES) {
        // Skip header line
        if (first_line) {
            first_line = 0;
            continue;
        }
        
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
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

    fclose(file);
    viewer->current_entry = 0;
    return 1;
}

// Print the current trace entry
void print_current_entry(TraceViewer *viewer) {
    if (viewer->current_entry >= 0 && viewer->current_entry < viewer->entry_count) {
        TraceEntry *entry = &viewer->entries[viewer->current_entry];
        printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
        printf("\033[1;33m[Execution #%ld]\033[0m\n", entry->exec_order);
        printf("\033[1;32mFile:\033[0m %s \033[1;32mLine:\033[0m %d\n", entry->filename, entry->line_number);
        printf("\033[1;35mCode:\033[0m %s\n", entry->code);
        
        if (strlen(entry->variables) > 0) {
            printf("\033[1;34mVariables:\033[0m\n");
            
            // Parse and display variables nicely
            char vars_copy[MAX_FIELD_LENGTH];
            strncpy(vars_copy, entry->variables, sizeof(vars_copy) - 1);
            vars_copy[sizeof(vars_copy) - 1] = '\0';
            
            char *var = strtok(vars_copy, ";");
            while (var) {
                // Trim leading whitespace
                while (isspace((unsigned char)*var)) var++;
                if (strlen(var) > 0) {
                    printf("  • %s\n", var);
                }
                var = strtok(NULL, ";");
            }
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
            char vars_copy[MAX_FIELD_LENGTH];
            strncpy(vars_copy, entry->variables, sizeof(vars_copy) - 1);
            vars_copy[sizeof(vars_copy) - 1] = '\0';
            
            char *var = strtok(vars_copy, ";");
            while (var) {
                while (isspace((unsigned char)*var)) var++;
                if (strstr(var, var_name) == var || 
                    (strstr(var, var_name) && *(strstr(var, var_name) - 1) == ' ')) {
                    printf("  → %s\n", var);
                }
                var = strtok(NULL, ";");
            }
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
            char try_path[1024];
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
    char resolved_path[1024] = {0};
    
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
    printf("\n\033[1;35mOther:\033[0m\n");
    printf("  \033[1;32mhelp\033[0m           - Show this help\n");
    printf("  \033[1;32mquit\033[0m or \033[1;32mq\033[0m     - Exit debugger\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
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
    static const char* commands[] = {
        "n", "next", "back", "b", "break", "list", "clear", "c", "continue",
        "rc", "show", "summary", "find", "jump", "help", "quit", "q", NULL
    };
    
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    
    while (commands[list_index]) {
        const char* name = commands[list_index];
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
    
    // Initialize readline for command history and tab completion
    init_readline();
    
    print_help();
    
    printf("\n\033[1;32m✓ Tab completion enabled\033[0m (press TAB to autocomplete commands and .py files)\n");
    
    // Print first entry
    print_current_entry(&viewer);

    char *last_command = NULL;
    while (1) {
        char input[MAX_LINE_LENGTH];
        char *cmd;
        
        // Build prompt with user-friendly 1-based position
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "\n\033[1;32m[%d/%d]\033[0m > ", 
                 viewer.current_entry + 1, viewer.entry_count);
        
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
                cmd = last_command;
                // Note: don't add to history again
            } else {
                continue;
            }
        } else {
            // Add non-empty command to history
            add_history(line);
            
            // Update last command
            if (last_command) {
                free(last_command);
            }
            last_command = strdup(cmd);
        }
        
        // Copy command to input buffer for processing
        strncpy(input, cmd, MAX_LINE_LENGTH - 1);
        input[MAX_LINE_LENGTH - 1] = '\0';
        
        // Free the readline buffer (but not last_command if we're using it)
        if (cmd == line) {
            free(line);
        }
        
        // Point cmd to input buffer for command processing
        cmd = input;
        while (isspace((unsigned char)*cmd)) cmd++;

        // Handle 'n' command (next)
        if (strcmp(cmd, "n") == 0) {
            if (viewer.current_entry < viewer.entry_count - 1) {
                viewer.current_entry++;
                print_current_entry(&viewer);
            } else {
                printf("\033[1;31m✗ Already at last execution step\033[0m\n");
            }
        }
        // Handle 'back' command (previous)
        else if (strcmp(cmd, "back") == 0) {
            if (viewer.current_entry > 0) {
                viewer.current_entry--;
                print_current_entry(&viewer);
            } else {
                printf("\033[1;31m✗ Already at first execution step\033[0m\n");
            }
        }
        // Handle 'summary' command
        else if (strcmp(cmd, "summary") == 0) {
            print_summary(&viewer);
        }
        // Handle 'show' command (with optional filename)
        else if (strncmp(cmd, "show", 4) == 0) {
            if (cmd[4] == '\0') {
                // Just 'show' - show current file
                show_file(&viewer, NULL);
            } else if (cmd[4] == ' ') {
                // 'show <filename>' - show specific file
                char *filename = cmd + 5;
                while (isspace((unsigned char)*filename)) filename++;
                
                if (strlen(filename) > 0) {
                    show_file(&viewer, filename);
                } else {
                    show_file(&viewer, NULL);
                }
            } else {
                show_file(&viewer, NULL);
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
            if (user_num < 1 || user_num > viewer.entry_count) {
                printf("\033[1;31m✗ Execution #%ld out of range. Valid range: 1-%d\033[0m\n", 
                       user_num, viewer.entry_count);
            } else {
                for (int i = 0; i < viewer.entry_count; i++) {
                    if (viewer.entries[i].exec_order == exec_num) {
                        viewer.current_entry = i;
                        print_current_entry(&viewer);
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
                search_variable(&viewer, var_name);
            } else {
                printf("\033[1;31m✗ Usage: find <variable_name>\033[0m\n");
            }
        }
        // Handle 'b <file> <line>' command (set breakpoint)
        else if (cmd[0] == 'b' && (cmd[1] == ' ' || cmd[1] == '\0')) {
            if (cmd[1] == '\0') {
                // Just 'b' - list breakpoints
                list_breakpoints(&viewer);
            } else {
                // 'b <file> <line>' - set breakpoint
                char *args = cmd + 2;
                while (isspace((unsigned char)*args)) args++;
                
                char filename[512];
                int line_num;
                
                if (sscanf(args, "%s %d", filename, &line_num) == 2) {
                    add_breakpoint(&viewer, filename, line_num);
                } else {
                    printf("\033[1;31m✗ Usage: b <file> <line>\033[0m\n");
                    printf("Example: b test.py 25\n");
                }
            }
        }
        // Handle 'list' command (list breakpoints)
        else if (strcmp(cmd, "list") == 0) {
            list_breakpoints(&viewer);
        }
        // Handle 'c' command (continue to next breakpoint)
        else if (strcmp(cmd, "c") == 0) {
            continue_to_breakpoint(&viewer);
        }
        // Handle 'rc' command (reverse continue to previous breakpoint)
        else if (strcmp(cmd, "rc") == 0) {
            reverse_continue_to_breakpoint(&viewer);
        }
        // Handle 'jump <line>' command (jump to source line - old 'break' behavior)
        else if (strncmp(cmd, "jump ", 5) == 0) {
            int line_num = atoi(cmd + 5);
            int found = 0;
            
            printf("\nSearching for line %d...\n\n", line_num);
            for (int i = 0; i < viewer.entry_count; i++) {
                if (viewer.entries[i].line_number == line_num) {
                    viewer.current_entry = i;
                    print_current_entry(&viewer);
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
            for (int i = 0; i < viewer.entry_count; i++) {
                if (viewer.entries[i].line_number == line_num) {
                    viewer.current_entry = i;
                    print_current_entry(&viewer);
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
                    add_watchpoint(&viewer, var_name, WATCHPOINT_BOTH);
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
                add_watchpoint(&viewer, var_name, WATCHPOINT_READ);
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
                add_watchpoint(&viewer, var_name, WATCHPOINT_WRITE);
            } else {
                printf("\033[1;31m✗ Usage: ww <variable>\033[0m\n");
                printf("Example: ww counter\n");
            }
        }
        // Handle 'listw' command (list watchpoints)
        else if (strcmp(cmd, "listw") == 0) {
            list_watchpoints(&viewer);
        }
        // Handle 'clearw [num]' command (clear watchpoint(s))
        else if (strncmp(cmd, "clearw", 6) == 0) {
            if (cmd[6] == '\0') {
                // Clear all watchpoints
                clear_watchpoint(&viewer, -1);
            } else if (cmd[6] == ' ') {
                // Clear specific watchpoint
                int wp_num = atoi(cmd + 7);
                if (wp_num > 0 && wp_num <= viewer.watchpoint_count) {
                    clear_watchpoint(&viewer, wp_num - 1);
                } else {
                    printf("\033[1;31m✗ Invalid watchpoint number. Use 'listw' to see watchpoints.\033[0m\n");
                }
            }
        }
        // Handle 'q' or 'quit' command
        else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }
        else {
            printf("\033[1;31m✗ Unknown command. Type 'help' for available commands\033[0m\n");
        }
    }

    // Save command history
    save_readline_history();
    
    // Free last command
    if (last_command) {
        free(last_command);
    }
    
    cleanup(&viewer);
    printf("\n\033[1;36mGoodbye! Happy debugging! 🐛\033[0m\n\n");
    return 0;
}
