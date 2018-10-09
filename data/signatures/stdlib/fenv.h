
// Types
typedef struct _fenv_t fenv_t;
typedef struct _fexcept_t fexcept_t;

// FP exceptions
int feclearexcept(int excepts);
int feraiseexcept(int excepts);
int fegetexceptflag(fexcept_t *flagp, int excepts);
int fesetexceptflag(const fexcept_t *flagp, int excepts);

// rounding mode
int fegetround(void);
int fesetround(int rdir);

// Entire FP environment
int fegetenv(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feholdexcept(fenv_t *envp);
int feupdateenv(const fenv_t *envp);

// Other
int fetestexcept(int excepts);
