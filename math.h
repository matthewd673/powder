#include <time.h>
#include <stdlib.h>

#ifndef MATH_H
#define MATH_H
int rand_init = 0;

float clamp(float f, float min, float max) {
    const float c = (f < min) ? min : f;
    return (c > max) ? max : c;
}

float rand_float() {
    if (!rand_init) {
        rand_init = 1;
        srand(time(NULL));
    }
    return rand() / (float)RAND_MAX;
}

float rand_float_range(float min, float max) {
  return min + (max - min) * rand_float();
}

char rand_boolean() {
  return rand_float() >= 0.5f;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}
#endif
