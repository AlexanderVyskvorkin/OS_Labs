#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lib7.h"

int main() {
    char buf[256];
    ssize_t n;

    while(1) {
        n = read(STDIN_FILENO, buf, sizeof(buf) - 1);

        if (n < 0) {
            const char msg[] = "error: failed to read command\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        if (n == 0)
            break;

        buf[n] = '\0';

        char *p = buf;
        char *end;
        while (*p == ' ' || *p == '\t' || *p == '\n')
            p++;

        long cmd = strtol(p, &end, 10);

        if (p == end) {
            const char msg[] = "No command number\n";
            write(STDOUT_FILENO, msg, sizeof(msg));
            continue;
        }
        p = end;

        switch (cmd) {
            case 1:
                {
                float a = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "Command format must be: 1 a b e\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;
                float b = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "Command format must be: 1 a b e\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;
                float e = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "Command format must be: 1 a b e\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;

                float ans = sin_integral(a, b, e);

                char msg[256];
                int len = snprintf(msg, sizeof(msg), "sin_integral = %f\n", ans);
                write(STDOUT_FILENO, msg, len);

                break;
                }

            case 2:
                {
                int x = (int)strtol(p, &end, 10);
                if (p == end) {
                    const char msg[] = "Command format must be: 2 x\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;

                char *ans = convert(x);
                
                char msg[256];
                int len = snprintf(msg, sizeof(msg), "Converted x = %s\n", ans);
                write(STDOUT_FILENO, msg, len);
                free(ans);

                break;
                }

            default:
                {
                const char msg[] = "Unknown command\n";
                write(STDOUT_FILENO, msg, sizeof(msg));
                continue;
                }
        }

    }
    
    return 0;
}