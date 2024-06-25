#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int t = isatty(1);

    if (t) {
        printf("Its terminal.\n");
    } else {
        printf("Its not terminal.\n");
    }

    return 0;
}