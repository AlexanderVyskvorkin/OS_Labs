#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void stdout_float(float val) {
    char buf[64];
    size_t i = 0;

    if (val < 0) {
        buf[i++] = '-';
        val = -val;
    }

    long long left = (long long)val;

    char temp[64];
    size_t j = 0;
    if (left == 0)
        temp[j++] = '0';
    while (left > 0) {
        temp[j++] = (char)('0' + left % 10);
        left /= 10;
    }
    while (j > 0)
        buf[i++] = temp[--j];

    buf[i++] = '.';
    float frac = val - (long long)val;
    for (int k = 0; k < 6; k++) {
        frac *= 10.0;
        int d = (int)frac;
        buf[i++] = (char)('0' + d);
        frac -= d;
    }

    buf[i++] = '\n';
    write(STDOUT_FILENO, buf, i);
}

void print_sum(char* line, size_t len) {
    line[len] = '\0';

    char* p = line;
    float sum = 0.0;

    while(*p != '\0') {
        char *end;
        float val = strtof(p, &end);

        if (end == p)
            p++;
        else {
            sum += val;
            p = end;
        }
    }
    stdout_float(sum);
}

int main(int argc, char **argv) {
    char buf[4096];
    size_t i = 0;
    ssize_t bytes;

    while (1) {
        bytes = read(STDIN_FILENO, buf + i, sizeof(buf) - 1 - i);
        if (bytes < 0) {
			const char msg[] = "error: failed to read from stdin\n";
			write(STDERR_FILENO, msg, sizeof(msg) - 1);
			exit(EXIT_FAILURE);
        }
        if (bytes == 0) {
            if (i > 0)
                print_sum(buf, i);
            break;
        }

        i += (size_t)bytes;
        size_t start = 0;
        for (size_t j = 0; j < i; j++) {
            if (buf[j] == '\n') {
                print_sum(buf + start, j - start);
                start = j + 1;
            }
        }
        if (start > 0) {
            size_t unfin = i - start;
            if (unfin > 0)
                memmove(buf, buf + start, unfin);
            i = unfin;
        }

        if (i >= sizeof(buf) - 1) {
            print_sum(buf, i);
            i = 0;
        }
    }

    return 0;
}