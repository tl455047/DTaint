#include <stdio.h>


float float_test(float a, float b) {

    return a + b;
}

int main() {
    float a;
    float b;
    double c;
    a = 1.111111111;
    b = 2.3;

    c = float_test(a, b);
    fprintf(stderr, "c: %f b: %f a: %f\n", c, b, a);


    return 0;
}