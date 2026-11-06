#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <string.h>


#define SHM_SIZE 4096

const char SHM_NAME[] = "SharedMemory";

const char SEM_PUT_NAME[] = "CanPutInSHM";
const char SEM_TAKE_NAME[] = "CanTakeFromSHM";

int main(void) {
    int shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (shm == -1) {
        const char msg[] = "error: failed to create SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm, SHM_SIZE) == -1) {
        const char msg[] = "error: failed to resize SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    char *shm_buf = mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm, 0);
    if (shm_buf == MAP_FAILED) {
        const char msg[] = "error: failed to map SHM\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_put = sem_open(SEM_PUT_NAME, O_RDWR | O_CREAT, 0600, 1);
    sem_t *sem_take = sem_open(SEM_TAKE_NAME, O_RDWR | O_CREAT, 0600, 0);
    if (sem_put == SEM_FAILED || sem_take == SEM_FAILED) {
        const char msg[] = "error: failed to create semaphore\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

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

    const pid_t child = fork();
    if (child == 0) {
        char *const args[] = {"./child", NULL};
        int status = execv("./child", args);
        if (status == -1) {
            const char msg[] = "error: failed to exec\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
    } else if (child == -1) {
        const char msg[] = "error: failed to spawn new process\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }


    bool running = true;
    while (running) {
        char buf[SHM_SIZE - sizeof(uint32_t)];
        ssize_t bytes = read(file, buf, sizeof(buf));
        if (bytes == -1) {
            const char msg[] = "error: failed to read from file\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }

        sem_wait(sem_put);

        uint32_t *length = (uint32_t *)shm_buf;
        char *text = shm_buf + sizeof(uint32_t);

        if (bytes > 0) {
            *length = (uint32_t)bytes;
            memcpy(text, buf, (size_t)bytes);
        } else {
            *length = UINT32_MAX;
            running = false;
        }

        sem_post(sem_take);
    }

    close(file);

    waitpid(child, NULL, 0);

    sem_unlink(SEM_PUT_NAME);
    sem_unlink(SEM_TAKE_NAME);
    sem_close(sem_put);
    sem_close(sem_take);
    munmap(shm_buf, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm);

    return 0;
}