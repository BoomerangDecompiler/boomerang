
typedef struct _jmp_buf jmp_buf;

void longjmp(jmp_buf env, int val);
int setjmp(jmp_buf env);
