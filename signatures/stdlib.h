void *malloc(unsigned int size);
void free(void* ptr);
void exit(int code);

typedef int main(int argc, char **argv, char **envp);

int stat(const char* filename, struct stat* st);

