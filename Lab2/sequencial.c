#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    size_t first_score;
    size_t second_score;
    size_t round;
    size_t K;
} GameState;

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

int main(int argc, char** argv) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (argc != 6) {
        char msg[1024];
        int len = snprintf(msg, sizeof(msg) - 1, "usage: %s K round_number first_points second_points experiment_number\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_SUCCESS);
    }
    
    GameState game = {
        .first_score = (size_t)strtoull(argv[3], NULL, 10),
        .second_score = (size_t)strtoull(argv[4], NULL, 10),
        .K = (size_t)strtoull(argv[1], NULL, 10),
        .round = (size_t)strtoull(argv[2], NULL, 10)
    };
    size_t exp_num = (size_t)strtoull(argv[5], NULL, 10);

    if (game.K < game.round || game.K == 0 || game.round == 0 || exp_num == 0 ||
        game.first_score < 2*(game.round - 1) || game.second_score < 2*(game.round - 1) ||
        game.first_score > 12*(game.round - 1) || game.second_score > 12*(game.round - 1)) {
            char msg[] = "Invalid input";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
    }

    size_t first_win_count = 0; 
    size_t second_win_count = 0;
    int seed = (int)time(NULL);

    for (size_t i = 0; i < exp_num; i++) {
        int winner = DefineWinner(&seed, &game);
        if (winner == 1)
            first_win_count++;
        else if (winner == 2)
            second_win_count++;
    }

    double first_P = (double)first_win_count / (double)exp_num * 100;
    double second_P = (double)second_win_count / (double)exp_num * 100;

    char result[1024];
    int len = snprintf(result, sizeof(result) - 1, "First wins: %.2f%%\n", first_P);
    write(STDOUT_FILENO, result, len);
    len = snprintf(result, sizeof(result) - 1, "Second wins: %.2f%%\n", second_P);
    write(STDOUT_FILENO, result, len);

    clock_gettime(CLOCK_MONOTONIC, &end);
    int run_time = (int)(((end.tv_sec + end.tv_nsec / 1e9) - (start.tv_sec + start.tv_nsec / 1e9)) * 1000);
    len = snprintf(result, sizeof(result) - 1, "Time: %d ms\n", run_time);
    write(STDOUT_FILENO, result, len);

    return 0;
}