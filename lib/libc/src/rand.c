#include "stdlib.h"
#define M 32768
#define A 4099
#define C 997

static unsigned int next=1;

int rand(void) {
    next=(next*A+C)%M;
    return next;    
}

void srand(unsigned int seed){
    next=seed;
}
