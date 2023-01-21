#include <time.h>
#include <stdlib.h>

int randInit = 0;

float clamp(float f, float min, float max) {
    const float c = (f < min) ? min : f;
    return (c > max) ? max : c;
}

float randFloat() {
    if (!randInit) {
        randInit = 1;
        srand(time(NULL));
    }
    return rand() / (float)RAND_MAX;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}