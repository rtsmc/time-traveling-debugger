#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 4096
#define MAX_LINES 100000
#define MAX_FIELD_LENGTH 2048
#define MAX_BREAKPOINTS 100

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

typedef struct {
    TraceEntry *entries;
    int entry_count;
    int current_entry;
    Breakpoint breakpoints[MAX_BREAKPOINTS];
    int breakpoint_count;
} TraceViewer;

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

// Continue to next breakpoint (forward)
void continue_to_breakpoint(TraceViewer *viewer) {
    if (viewer->breakpoint_count == 0) {
        printf("\033[1;33m⚠ No breakpoints set. Use 'b <file> <line>' to set breakpoints.\033[0m\n");
        return;
    }
    
    // Search forward from current position
    for (int i = viewer->current_entry + 1; i < viewer->entry_count; i++) {
        if (is_at_breakpoint(viewer, i)) {
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;31m⚫ BREAKPOINT HIT\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            print_current_entry(viewer);
            return;
        }
    }
    
    // No more breakpoints - go to end
    printf("\033[1;33m⚠ No more breakpoints ahead. Jumping to end of trace.\033[0m\n");
    viewer->current_entry = viewer->entry_count - 1;
    print_current_entry(viewer);
}

// Reverse continue to previous breakpoint (backward)
void reverse_continue_to_breakpoint(TraceViewer *viewer) {
    if (viewer->breakpoint_count == 0) {
        printf("\033[1;33m⚠ No breakpoints set. Use 'b <file> <line>' to set breakpoints.\033[0m\n");
        return;
    }
    
    // Search backward from current position
    for (int i = viewer->current_entry - 1; i >= 0; i--) {
        if (is_at_breakpoint(viewer, i)) {
            viewer->current_entry = i;
            printf("\n\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            printf("\033[1;35m⟲ BREAKPOINT HIT (REVERSE)\033[0m\n");
            printf("\033[1;33m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
            print_current_entry(viewer);
            return;
        }
    }
    
    // No more breakpoints - go to beginning
    printf("\033[1;33m⚠ No more breakpoints behind. Jumping to beginning of trace.\033[0m\n");
    viewer->current_entry = 0;
    print_current_entry(viewer);
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

// View full source file with current line highlighted
void view_file(TraceViewer *viewer) {
    if (viewer->current_entry < 0 || viewer->current_entry >= viewer->entry_count) {
        printf("No current entry\n");
        return;
    }
    
    TraceEntry *current = &viewer->entries[viewer->current_entry];
    const char *filename = current->filename;
    int highlight_line = current->line_number;
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\033[1;31m✗ Cannot open file: %s\033[0m\n", filename);
        return;
    }
    
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mFile: %s\033[0m\n", filename);
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
    
    char line[MAX_LINE_LENGTH];
    int line_num = 1;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        if (line_num == highlight_line) {
            // Highlight current line (green background)
            printf("\033[42m\033[30m>>> [Line %3d] %s\033[0m\n", line_num, line);
        } else {
            // Normal line
            printf("    [Line %3d] %s\n", line_num, line);
        }
        
        line_num++;
    }
    
    fclose(file);
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("Currently at: \033[1;32m[Execution #%ld]\033[0m Line %d\n", 
           current->exec_order, highlight_line);
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
    printf("  \033[1;32m:<number>\033[0m      - Jump to execution number (e.g., :100)\n");
    printf("\n\033[1;35mBreakpoints:\033[0m\n");
    printf("  \033[1;32mb <file> <line>\033[0m - Set breakpoint (e.g., b test.py 25)\n");
    printf("  \033[1;32mlist\033[0m           - List all breakpoints\n");
    printf("  \033[1;32mc\033[0m              - Continue to next breakpoint\n");
    printf("  \033[1;32mrc\033[0m             - Reverse continue to previous breakpoint\n");
    printf("\n\033[1;35mAnalysis:\033[0m\n");
    printf("  \033[1;32mview\033[0m           - View full source file with current line highlighted\n");
    printf("  \033[1;32msummary\033[0m        - Show trace summary\n");
    printf("  \033[1;32mfind <var>\033[0m    - Search for variable usage\n");
    printf("  \033[1;32mjump <line>\033[0m   - Jump to first occurrence of source line\n");
    printf("\n\033[1;35mOther:\033[0m\n");
    printf("  \033[1;32mhelp\033[0m           - Show this help\n");
    printf("  \033[1;32mquit\033[0m or \033[1;32mq\033[0m     - Exit debugger\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n\n");
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
    print_help();
    
    // Print first entry
    print_current_entry(&viewer);

    char input[MAX_LINE_LENGTH];
    while (1) {
        // Show execution number (from exec_order) and position
        long exec_num = viewer.entries[viewer.current_entry].exec_order;
        printf("\n\033[1;32m[Exec #%ld - %d/%d]\033[0m > ", 
               exec_num, viewer.current_entry + 1, viewer.entry_count);
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Remove trailing newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Trim whitespace
        char *cmd = input;
        while (isspace((unsigned char)*cmd)) cmd++;
        
        // Handle empty input
        if (strlen(cmd) == 0) {
            continue;
        }

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
        // Handle 'view' command
        else if (strcmp(cmd, "view") == 0) {
            view_file(&viewer);
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
        // Handle 'q' or 'quit' command
        else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }
        else {
            printf("\033[1;31m✗ Unknown command. Type 'help' for available commands\033[0m\n");
        }
    }

    cleanup(&viewer);
    printf("\n\033[1;36mGoodbye! Happy debugging! 🐛\033[0m\n\n");
    return 0;
}
