#include "lib7.h"
#include <stdlib.h>

float sinf(float x) {
    float x2 = x * x;
    return x -
           (x2 * x) / 6.0f +
           (x2 * x2 * x) / 120.0f -
           (x2 * x2 * x2 * x) / 5040.0f +
           (x2 * x2 * x2 * x2 * x) / 362880.0f -
           (x2 * x2 * x2 * x2 * x2 * x) / 39916800.0f +
           (x2 * x2 * x2 * x2 * x2 * x2 * x) / 6227020800.0f;
}

float sin_integral(float a, float b, float e) {
    float res = 0.0f;
    float x = a;
    float y;

    while (x < b) {
        float step = e;
        if (x + step > b)
            step = b - x;
        y = sinf(x);
        res += y * step;

        x += step;
    }

    return res;
}

char *convert(int x) {
    char *res = (char *)malloc(4 * 8 + 2);
    
    if (x == 0) {
        res[0] = '0';
        res[1] = '\0';
        return res;
    }

    long long num = x;

    int neg = 0;
    if (num < 0) {
        neg = 1;
        num = -num;
    }

    int i = 0;
    while (num != 0) {
        res[i++] = '0' + (char)(num % 2);
        num /= 2;
    }

    if (neg)
        res[i++] = '-';

    res[i--] = '\0';
    for (int j = 0; j < i; j++) {
        char temp = res[j];
        res[j] = res[i];
        res[i--] = temp;
    }

    return res;
}