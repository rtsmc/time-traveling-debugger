#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 10000

typedef struct {
    char **lines;
    int line_count;
    int current_line;
} FileViewer;

// Read all lines from file into memory
int read_file(const char *filename, FileViewer *viewer) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 0;
    }

    viewer->lines = malloc(MAX_LINES * sizeof(char *));
    if (!viewer->lines) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 0;
    }

    viewer->line_count = 0;
    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), file) && viewer->line_count < MAX_LINES) {
        // Remove trailing newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        viewer->lines[viewer->line_count] = malloc(strlen(buffer) + 1);
        if (!viewer->lines[viewer->line_count]) {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(file);
            return 0;
        }
        strcpy(viewer->lines[viewer->line_count], buffer);
        viewer->line_count++;
    }

    fclose(file);
    viewer->current_line = 0;
    return 1;
}

// Print the current line
void print_current_line(FileViewer *viewer) {
    if (viewer->current_line >= 0 && viewer->current_line < viewer->line_count) {
        printf("[Line %d] %s\n", viewer->current_line + 1, viewer->lines[viewer->current_line]);
    }
}

// Free allocated memory
void cleanup(FileViewer *viewer) {
    for (int i = 0; i < viewer->line_count; i++) {
        free(viewer->lines[i]);
    }
    free(viewer->lines);
}

// Trim whitespace from string (not used but kept for potential future use)
void trim(char *str) {
    char *end;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <python_file>\n", argv[0]);
        return 1;
    }

    FileViewer viewer;
    
    if (!read_file(argv[1], &viewer)) {
        return 1;
    }

    if (viewer.line_count == 0) {
        printf("File is empty\n");
        cleanup(&viewer);
        return 0;
    }

    printf("Python File Viewer - Loaded %d lines\n", viewer.line_count);
    printf("Commands: 'n' (next), 'back' (previous), ':<number>' (goto line)\n");
    printf("---\n");

    // Print first line
    print_current_line(&viewer);

    char input[MAX_LINE_LENGTH];
    while (1) {
        printf("> ");
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

        // Handle 'n' command
        if (strcmp(cmd, "n") == 0) {
            if (viewer.current_line < viewer.line_count - 1) {
                viewer.current_line++;
                print_current_line(&viewer);
            } else {
                printf("Already at last line\n");
            }
        }
        // Handle 'back' command
        else if (strcmp(cmd, "back") == 0) {
            if (viewer.current_line > 0) {
                viewer.current_line--;
                print_current_line(&viewer);
            } else {
                printf("Already at first line\n");
            }
        }
        // Handle ':<number>' command
        else if (cmd[0] == ':') {
            int line_num = atoi(cmd + 1);
            if (line_num < 1 || line_num > viewer.line_count) {
                printf("Invalid line number. Valid range: 1-%d\n", viewer.line_count);
            } else {
                viewer.current_line = line_num - 1;
                print_current_line(&viewer);
            }
        }
        // Handle 'q' or 'quit' command
        else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        }
        else {
            printf("Unknown command. Use 'n', 'back', ':<number>', or 'quit'\n");
        }
    }

    cleanup(&viewer);
    printf("Goodbye!\n");
    return 0;
}
