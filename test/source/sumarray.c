#include <stdio.h>

int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int main() {
    int sum = 0;
    int i;
    for (i=0; i < 10; i++) {
        sum += a[i];
    }
    printf("Sum is %d\n", sum);
    return 0;
}
