/* Compile with gcc -O4 -fno-inline -o ...  or cc -xO4 -xinline= -o ...  */

int proc1(int a, int b) {
    return a - b;
}

int main() {
    printf("%i\n", proc1(11, 4));
}


