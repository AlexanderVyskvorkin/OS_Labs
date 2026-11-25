#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

typedef float sin_integral_func(float a, float b, float e);
typedef char *convert_func(int x);

static sin_integral_func *sin_integral;
static convert_func *convert;

static float sin_integral_stub(float a, float b, float e) {
    (void)a;
    (void)b;
    (void)e;
    return 0.0f;
}

static char *convert_stub(int x) {
    (void)x;
    return NULL;
}

int main() {
    char buf[256];
    ssize_t n;

    void *libOne = dlopen("./lib7One.so", RTLD_LOCAL | RTLD_NOW);
    void *libTwo = dlopen("./lib7Two.so", RTLD_LOCAL | RTLD_NOW);
    
    int libInUse = 1;
    sin_integral = dlsym(libOne, "sin_integral");
    if (sin_integral == NULL) {
        const char msg[] = "error: failed to find sin_integral implementation\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        sin_integral = sin_integral_stub;
    }
    convert = dlsym(libOne, "convert");
    if (convert == NULL) {
        const char msg[] = "error: failed to find convert implementation\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        convert = convert_stub;
    }

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

        if (end == p) {
            const char msg[] = "No command number\n";
            write(STDOUT_FILENO, msg, sizeof(msg));
            continue;
        }
        p = end;

        switch (cmd) {
            case 0:
                {
                if (libInUse == 1) {
                    sin_integral = dlsym(libTwo, "sin_integral");
                    if (sin_integral == NULL) {
                        const char msg[] = "error: failed to find sin_integral implementation\n";
                        write(STDERR_FILENO, msg, sizeof(msg));
                        sin_integral = sin_integral_stub;
                    }
                    convert = dlsym(libTwo, "convert");
                    if (convert == NULL) {
                        const char msg[] = "error: failed to find convert implementation\n";
                        write(STDERR_FILENO, msg, sizeof(msg));
                        convert = convert_stub;
                    }
                    libInUse = 2;
                } else {
                    sin_integral = dlsym(libOne, "sin_integral");
                    if (sin_integral == NULL) {
                        const char msg[] = "error: failed to find sin_integral implementation\n";
                        write(STDERR_FILENO, msg, sizeof(msg));
                        sin_integral = sin_integral_stub;
                    }
                    convert = dlsym(libOne, "convert");
                    if (convert == NULL) {
                        const char msg[] = "error: failed to find convert implementation\n";
                        write(STDERR_FILENO, msg, sizeof(msg));
                        convert = convert_stub;
                    }
                    libInUse = 1;
                }

                char msg[256];
                int len = snprintf(msg, sizeof(msg), "Current implementation: %d\n", libInUse);
                write(STDOUT_FILENO, msg, len);
                }
                break;
            case 1:
                {
                float a = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "failed to read command\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;
                float b = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "failed to read command\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    continue;
                }
                p = end;
                float e = strtof(p, &end);
                if (p == end) {
                    const char msg[] = "failed to read command\n";
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
                    const char msg[] = "failed to read command\n";
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