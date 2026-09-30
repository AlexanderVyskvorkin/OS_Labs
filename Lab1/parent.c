#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>


static char CHILD_PROGRAM_NAME[] = "child";

int main(int argc, char **argv) {
    char filename[1024];

    const char msg[] = "Enter filename: ";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    ssize_t len = read(STDIN_FILENO, filename, sizeof(filename) - 1);
    if (len < 0) {
        const char msg[] = "error: failed to read filename\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }
    filename[len] = '\0';
    if (len > 0 && filename[len - 1] == '\n')
        filename[len - 1] = '\0';

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        const char msg[] = "error: failed to open file\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    int child_to_parent[2];
    if (pipe(child_to_parent) == -1) {
        const char msg[] = "error: failed to create pipe\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    const pid_t child = fork();

    switch (child) {
    case -1: {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    } break;

    case 0: {
        dup2(file, STDIN_FILENO);
        dup2(child_to_parent[1], STDOUT_FILENO);
        close(file);
        close(child_to_parent[0]);
        close(child_to_parent[1]);
        {
            char *const args[] = {CHILD_PROGRAM_NAME, NULL};
            int status = execv(CHILD_PROGRAM_NAME, args);
            if (status == -1) {
                const char msg[] = "error: failed to exec\n";
				write(STDERR_FILENO, msg, sizeof(msg) - 1);
				exit(EXIT_FAILURE);
            }
        }
    } break;

    default: {
        close(file);
        close(child_to_parent[1]);
        
        char buf[4096];
        ssize_t bytes;

        while (bytes = read(child_to_parent[0], buf, sizeof(buf))) {
            if (bytes < 0) {
				const char msg[] = "error: failed to read from pipe\n";
				write(STDERR_FILENO, msg, sizeof(msg) - 1);
				exit(EXIT_FAILURE);
            }

            write(STDOUT_FILENO, buf, bytes);
        }

        close(child_to_parent[0]);
        wait(NULL);
    } break;
    }

    return 0;
}
