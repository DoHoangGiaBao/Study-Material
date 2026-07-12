#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void) {
    char *args[MAX_LINE/2 + 1]; /* command line arguments */
    char input_buffer[MAX_LINE]; /* where to store the command */
    int should_run = 1; /* flag to determine when to exit program */
    char history[MAX_LINE] = ""; /* store the last command string */
    int has_history = 0; // Flag to check if history is empty

    while (should_run) {
        printf("osh>");
        fflush(stdout);
        // Read user input using fgets() function
        if (fgets(input_buffer, MAX_LINE, stdin) == NULL) {
            break;
        }

        // Remove newline in the buffer
        size_t len = strlen(input_buffer);
        if (len > 0 && input_buffer[len - 1] == '\n') {
            input_buffer[len - 1] = '\0';
        }

        // 2. Skip the loop if the user entered nothing
        if (strlen(input_buffer) == 0) {
            continue; 
        }

        // Check for exit command
        if (strcmp(input_buffer, "exit") == 0) {
            should_run = 0;
            continue;
        }

        // Check if user has input the history command
        if (strcmp(input_buffer, "!!") == 0) {
            if (!has_history) {
                printf("No command in history\n");
                continue;
            }

            // Copy saved history to input buffer and echo the command
            strcpy(input_buffer, history);
            printf("%s\n", input_buffer);
        } else {
            // Update the history buffer to store the new command
            strcpy(history, input_buffer);
            has_history = 1;
        }

        // Split command string into tokens using strtok()
        char *token;
        int i = 0;

        // Get the first token (The command itself)
        token = strtok(input_buffer, " ");

        while (token != NULL) {
            // Store the pointer to the word in the array
            args[i] = token; 
            i++;

            // Get the next token (the arguments)
            // Passing NULL tells strtok to continue with inputBuffer
            token = strtok(NULL, " ");
        }

        // The argument list must be NULL-terminated
        args[i] = NULL;

        // Handle ampersand "&" 
        int run_in_background = 0;

        // Check if the last argument is "&"
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            run_in_background = 1;
            args[i-1] = NULL; // Remove "&" so execvp doesn't think it's a file/command
        }

        // Check for pipe "|"
        int pipe_index = -1;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_index = j;
                args[j] = NULL; // Split the array into two
                break;
            }
        }

        // Create child process to execute user command
        pid_t pid = fork();

        if (pid < 0) {
            // Error occurred
            fprintf(stderr, "Fork Failed\n");
            return 1;
        } else if (pid == 0) {
            // Child process
            // Process pipe logic
            if (pipe_index != -1) {
                int fd[2];
                if (pipe(fd) == -1) {
                    perror("Pipe failed");
                    return 1;
                }

                pid_t p2 = fork();

                if (p2 < 0) {
                    perror("Fork failed");
                    return 1;
                }

                if (p2 == 0) {
                    // GRANDCHILD: Runs the first command
                    dup2(fd[1], STDOUT_FILENO); // Redirect stdout to pipe write-end
                    close(fd[0]); // Close unused read-end
                    close(fd[1]); // Close write-end after dup
                    execvp(args[0], args);
                    exit(1);
                } else {
                    // CHILD: Runs the second command
                    dup2(fd[0], STDIN_FILENO); // Redirect stdin to pipe read-end
                    close(fd[1]); // Close unused write-end
                    close(fd[0]); // Close read-end after dup
                    execvp(args[pipe_index + 1], &args[pipe_index + 1]);
                    exit(1);
                }
            }
            // Process redirection and other commands 
            else {
                for (int i = 0; args[i] != NULL; i++) {
                    // Check for ouput redirection ">"
                    if (strcmp(args[i], ">") == 0) {
                        char *filename = args[i + 1];
                        // Open file: Write only. Create if Missing. Truncate if exists
                        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) {
                            perror("Error opening output file\n");
                            return 1;
                        }
                        dup2(fd, STDOUT_FILENO); // Redirect STDOUT to file
                        close(fd); // Close original descriptor
                        args[i] = NULL; // Chop the args array here
                        break;
                    }
                    // Check for input redirection "<" 
                    else if (strcmp(args[i], "<") == 0) {
                        char *filename = args[i + 1];
                        int fd = open(filename, O_RDONLY); // Open for reading
                        if (fd < 0) {
                            perror("Error opening input file\n");
                            return 1;
                        }
                        dup2(fd, STDIN_FILENO); // Redirect STDIN to file
                        close(fd); // Close original descriptor
                        args[i] = NULL; // Chop the args array here
                        break;
                    }
                }

                // execvp(command, array_of_args)
                if (execvp(args[0], args) <= -1) {
                    printf("Command not found\n");
                }

                exit(1);
            }
        } else {
            // Parrent process
            if (run_in_background == 0) {
                // Wait for child process to complete
                wait(NULL);
            }
        }
    }
    return 0;
}