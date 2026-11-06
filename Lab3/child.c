#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/mman.h>


#define SHM_SIZE 4096

const char SHM_NAME[] = "SharedMemory";

const char SEM_PUT_NAME[] = "CanPutInSHM";
const char SEM_TAKE_NAME[] = "CanTakeFromSHM";

void print_sum(char* buf, size_t len) {
    buf[len] = '\0';

    char* p = buf;
    float sum = 0.0f;

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
    
    char ans[64];
    int n = snprintf(ans, sizeof(ans), "%f\n", sum);
    if (n < 0) {
        const char msg[] = "error: failed to print sum\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    write(STDOUT_FILENO, ans, n);
}

int main(int argc, char **argv) {

    int shm = shm_open(SHM_NAME, O_RDWR, 0);
	if (shm == -1) {
		const char msg[] = "error: failed to open SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0);
	if (shm_buf == MAP_FAILED) {
		const char msg[] = "error: failed to map SHM\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

    sem_t *sem_put = sem_open(SEM_PUT_NAME, O_RDWR);
    sem_t *sem_take = sem_open(SEM_TAKE_NAME, O_RDWR);
	if (sem_put == SEM_FAILED || sem_take == SEM_FAILED) {
		const char msg[] = "error: failed to open semaphore\n";
		write(STDERR_FILENO, msg, sizeof(msg) - 1);
		exit(EXIT_FAILURE);
	}

    char buf[SHM_SIZE];
    size_t i = 0;

    bool running = true;
    while (running) {
        sem_wait(sem_take);

        uint32_t *length = (uint32_t *)shm_buf;
		const char *text = shm_buf + sizeof(uint32_t);
        uint32_t cur_len = *length;

        if (cur_len == UINT32_MAX)
            running = false;
        else if (cur_len > 0) {
            if (i + cur_len <= sizeof(buf)) {
                memcpy(buf + i, text, cur_len);
                i += cur_len;
            } else {
                sem_post(sem_put);
                const char msg[] = "error: sum buffer overflow\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                exit(EXIT_FAILURE);
            }

        }
        
        sem_post(sem_put);

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
    }

    if (i > 0) {
        print_sum(buf, i);
        i = 0;
    }

    sem_close(sem_put);
    sem_close(sem_take);
    munmap(shm_buf, SHM_SIZE);
    close(shm);

    return 0;
}