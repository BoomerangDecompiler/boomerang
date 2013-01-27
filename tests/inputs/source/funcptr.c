#include <stdio.h>

void hello() {
  printf("Hello, ");
}

void world() {
  printf("world!\n");
}

typedef void (*FPTR)(void);

int main() {
    FPTR p;
    p = hello;
    (*p)();
    p = world;
    (*p)();
    return 0;
}
