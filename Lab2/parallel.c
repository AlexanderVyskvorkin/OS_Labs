#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    size_t first_score;
    size_t second_score;
    size_t round;
    size_t K;
    size_t exp_num;
    size_t n_threads;
} GameState;

typedef struct {
    size_t number;
    int seed;
} ThreadArgs;

static GameState game;
static size_t first_win_count = 0;
static size_t second_win_count = 0;

static size_t dice_roll(int* seed) {
    return (size_t)(rand_r(seed) % 6 + 1); 
}

static int DefineWinner(int* seed, const GameState* game) {
    size_t scoreOne = game->first_score;
    size_t scoreTwo = game->second_score;
    for (size_t r = game->round; r <= game->K; r++) {
        scoreOne += dice_roll(seed) + dice_roll(seed);
        scoreTwo += dice_roll(seed) + dice_roll(seed);
    }
    if (scoreOne > scoreTwo)
        return 1;
    if (scoreOne < scoreTwo)
        return 2;
    return 0;
}

static void *work(void* _args) {
    ThreadArgs *args = (ThreadArgs *)_args;

    size_t my_exp_num = game.exp_num / game.n_threads;
    size_t exp_remain = game.exp_num % game.n_threads;
    if (args->number < exp_remain)
        my_exp_num++;

    size_t my_first_wins = 0;
    size_t my_second_wins = 0;
    int my_seed = args->seed;

    for (size_t i = 0; i < my_exp_num; i++) {
        int winner = DefineWinner(&my_seed, &game);
        if (winner == 1)
            my_first_wins++;
        else if (winner == 2)
            my_second_wins++;
    }

    pthread_mutex_lock(&mutex);
    first_win_count += my_first_wins;
    second_win_count += my_second_wins;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char** argv) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (argc != 7) {
        char msg[1024];
        int len = snprintf(msg, sizeof(msg) - 1, "usage: %s K round_number first_points second_points experiment_number n_threads\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_SUCCESS);
    }

    game.K = (size_t)strtoull(argv[1], NULL, 10);
    game.round = (size_t)strtoull(argv[2], NULL, 10);
    game.first_score = (size_t)strtoull(argv[3], NULL, 10);
    game.second_score = (size_t)strtoull(argv[4], NULL, 10);
    game.exp_num = (size_t)strtoull(argv[5], NULL, 10);
    game.n_threads = (size_t)strtoull(argv[6], NULL, 10);

    if (game.K < game.round || game.K == 0 || game.round == 0 || game.exp_num == 0 ||
        game.first_score < 2*(game.round - 1) || game.second_score < 2*(game.round - 1) ||
        game.first_score > 12*(game.round - 1) || game.second_score > 12*(game.round - 1) ||
        game.n_threads < 1) {
            char msg[] = "Invalid input";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
    }

    pthread_t* threads = malloc(game.n_threads * sizeof(pthread_t));
    ThreadArgs* thread_args = malloc(game.n_threads * sizeof(ThreadArgs));

    int seed = (int)time(NULL);
    for (size_t i = 0; i < game.n_threads; i++) {
        thread_args[i] = (ThreadArgs){.number = i, .seed = seed + i};
        pthread_create(&threads[i], NULL, work, &thread_args[i]);
    }

    for (size_t i = 0; i < game.n_threads; i++)
        pthread_join(threads[i], NULL);

    double first_P = (double)first_win_count / (double)game.exp_num * 100;
    double second_P = (double)second_win_count / (double)game.exp_num * 100;

    char result[1024];
    int len = snprintf(result, sizeof(result) - 1, "First wins: %.2f%%\n", first_P);
    write(STDOUT_FILENO, result, len);
    len = snprintf(result, sizeof(result) - 1, "Second wins: %.2f%%\n", second_P);
    write(STDOUT_FILENO, result, len);

    clock_gettime(CLOCK_MONOTONIC, &end);
    int run_time = (int)(((end.tv_sec + end.tv_nsec / 1e9) - (start.tv_sec + start.tv_nsec / 1e9)) * 1000);
    len = snprintf(result, sizeof(result) - 1, "Time: %d ms\n", run_time);
    write(STDOUT_FILENO, result, len);

    free(thread_args);
    free(threads);

    return 0;
}