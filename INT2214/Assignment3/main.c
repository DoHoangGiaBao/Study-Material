#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int i;
    int N = 10; 
    pid_t root_pid = getpid();

    // ONLY the very first process prints the header
    printf("digraph ProcessTree {\n");
    printf("    rankdir=LR;\n");
    printf("    node [shape=circle, fontsize=10, style=filled, fillcolor=lightblue];\n");
    fflush(stdout);

    for (i = 0; i < N; i++) {
        pid_t child_pid = fork();

        if (child_pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (child_pid == 0) {
            // CHILD: Print the relationship then CONTINUE the loop
            printf("    \"%d\" -> \"%d\";\n", getppid(), getpid());
            fflush(stdout);
        } else {
            // PARENT: Continues the loop to fork more children
        }
    }

    // Wait a bit to ensure all asynchronous prints finish
    usleep(500000); 

    // ONLY the original root process prints the closing brace
    if (getpid() == root_pid) {
        printf("    label=\"Total Processes: 1024\";\n");
        printf("    fontsize=20;\n");
        printf("}\n");
    }

    return 0;
}