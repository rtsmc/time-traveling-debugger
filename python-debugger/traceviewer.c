#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 4096
#define MAX_LINES 100000
#define MAX_FIELD_LENGTH 2048

typedef struct {
    long exec_order;
    char filename[512];
    int line_number;
    char code[MAX_FIELD_LENGTH];
    char variables[MAX_FIELD_LENGTH];
} TraceEntry;

typedef struct {
    TraceEntry *entries;
    int entry_count;
    int current_entry;
} TraceViewer;

// Parse a trace line into a TraceEntry
int parse_trace_line(char *line, TraceEntry *entry) {
    // Format: EXECUTION_ORDER|||FILENAME|||LINE_NUMBER|||CODE|||VARIABLES
    char *token;
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';
    
    // Parse execution order
    token = strtok(line_copy, "|");
    if (!token) return 0;
    
    // Skip the ||| separator (strtok will return empty strings)
    while (token && strlen(token) == 0) {
        token = strtok(NULL, "|");
    }
    if (!token) return 0;
    entry->exec_order = atol(token);
    
    // Parse filename
    token = strtok(NULL, "|");
    while (token && strlen(token) == 0) {
        token = strtok(NULL, "|");
    }
    if (!token) return 0;
    strncpy(entry->filename, token, sizeof(entry->filename) - 1);
    entry->filename[sizeof(entry->filename) - 1] = '\0';
    
    // Parse line number
    token = strtok(NULL, "|");
    while (token && strlen(token) == 0) {
        token = strtok(NULL, "|");
    }
    if (!token) return 0;
    entry->line_number = atoi(token);
    
    // Parse code
    token = strtok(NULL, "|");
    while (token && strlen(token) == 0) {
        token = strtok(NULL, "|");
    }
    if (!token) {
        entry->code[0] = '\0';
    } else {
        strncpy(entry->code, token, sizeof(entry->code) - 1);
        entry->code[sizeof(entry->code) - 1] = '\0';
    }
    
    // Parse variables (rest of the line)
    token = strtok(NULL, "");
    if (!token) {
        entry->variables[0] = '\0';
    } else {
        // Skip leading |||
        while (*token == '|') token++;
        strncpy(entry->variables, token, sizeof(entry->variables) - 1);
        entry->variables[sizeof(entry->variables) - 1] = '\0';
    }
    
    return 1;
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

// Free allocated memory
void cleanup(TraceViewer *viewer) {
    free(viewer->entries);
}

// Print help
void print_help() {
    printf("\n\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("\033[1;33mTrace Debugger Commands\033[0m\n");
    printf("\033[1;36m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf("  \033[1;32mn\033[0m              - Next execution step\n");
    printf("  \033[1;32mback\033[0m           - Previous execution step\n");
    printf("  \033[1;32m:<number>\033[0m      - Jump to execution number (e.g., :100)\n");
    printf("  \033[1;32msummary\033[0m        - Show trace summary\n");
    printf("  \033[1;32mfind <var>\033[0m    - Search for variable usage\n");
    printf("  \033[1;32mjump <line>\033[0m   - Jump to specific source line\n");
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
        printf("\n\033[1;32m[%d/%d]\033[0m > ", viewer.current_entry + 1, viewer.entry_count);
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
        // Handle 'help' command
        else if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        // Handle ':<number>' command (jump to execution)
        else if (cmd[0] == ':') {
            long exec_num = atol(cmd + 1);
            int found = 0;
            
            for (int i = 0; i < viewer.entry_count; i++) {
                if (viewer.entries[i].exec_order == exec_num) {
                    viewer.current_entry = i;
                    print_current_entry(&viewer);
                    found = 1;
                    break;
                }
            }
            
            if (!found) {
                printf("\033[1;31m✗ Execution #%ld not found. Valid range: 0-%ld\033[0m\n", 
                       exec_num, viewer.entries[viewer.entry_count - 1].exec_order);
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
        // Handle 'jump <line>' command
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
