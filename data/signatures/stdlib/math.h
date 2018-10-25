// Note: The C parser currently does not support "long double",
// so the functions requiring an argument of type "long double"
// are commented out

typedef double double_t;
typedef float float_t;

// Trigonometric functions
double cos(double x);
double sin(double x);
double tan(double x);
double acos(double x);
double asin(double x);
double atan(double x);
double atan2(double y, double x);
float cosf(float x);
float sinf(float x);
float tanf(float x);
float acosf(float x);
float asinf(float x);
float atanf(float x);
float atan2f(float y, float x);
// long double cosl(long double x);
// long double sinl(long double x);
// long double tanl(long double x);
// long double acosl(long double x);
// long double asinl(long double x);
// long double atanl(long double x);
// long double atan2l (long double y, long double x);

// Hyperbolic functions
double cosh(double x);
double sinh(double x);
double tanh(double x);
double acosh(double x);
double asinh(double x);
double atanh(double x);
float coshf(float x);
float sinhf(float x);
float tanhf(float x);
float acoshf(float x);
float asinhf(float x);
float atanhf(float x);
// long double coshl(long double x);
// long double sinhl(long double x);
// long double tanhl(long double x);
// long double acoshl(long double x);
// long double asinhl(long double x);
// long double atanhl(long double x);

// exp /log functions
double exp(double x);
double frexp(double x, int* exp);
double ldexp(double x, int exp);
double log(double x);
double log10(double x);
double modf(double x, double* intpart);
double exp2(double x);
double expm1(double x);
int ilogb(double x);
double log1p(double x);
double log2(double x);
double logb(double x);
double scalbn(double x, int n);
double scalbln(double x, long n);
float expf(float x);
float frexpf(float x, int* exp);
float ldexpf(float x, int exp);
float logf(float x);
float log10f(float x);
float modff(float x, float *intpart);
float exp2f(float x);
float expm1f(float x);
int ilogbf(float x);
float log1pf(float x);
float log2f(float x);
float logbf(float x);
float scalbnf(float x, int n);
float scalblnf(float x, long n);
// long double expl(long double x);
// long double frexpl(long double x, int* exp);
// long double ldexpl(long double x, int exp);
// long double logl(long double x);
// long double log10l(long double x);
// long double modfl(long double x, long double* intpart);
// long double exp2l(long double x);
// long double expm1l(long double x);
// int ilogbl(long double x);
// long double log1pl(long double x);
// long double log2l(long double x);
// long double logbl(long double x);
// long double scalbnl(long double x, int n);
// long double scalblnl(long double x, long int n);

// power functions
double pow(double base, double exponent);
double sqrt(double x);
double cbrt(double x);
double hypot(double x, double y);
float powf(float base, float exponent);
float sqrtf(float x);
float cbrtf(float x);
float hypotf(float x, float y);
// long double powl(long double base, long double exponent);
// long double sqrtl(long double x);
// long double cbrtl(long double x);
// long double hypotl(long double x, long double y);

// Error and gamma functions
double erf(double x);
double erfc(double x);
double tgamma(double x);
double lgamma(double x);
float erff(float x);
float erfcf(float x);
float tgammaf(float x);
float lgammaf(float x);
// long double erfl(long double x);
// long double erfcl(long double x);
// long double tgammal(long double x);
// long double lgammal(long double x);

// rounding and remainder functions
double ceil(double x);
double floor(double x);
double fmod(double numer, double denom);
double trunc(double x);
double round(double x);
long lround(double x);
long long llround(double x);
double rint(double x);
long lrint(double x);
long long llrint(double x);
double nearbyint(double x);
double remainder(double numer, double denom);
double remquo(double numer, double denom, int* quot);
float ceilf(float x);
float floorf(float x);
float fmodf(float numer, float denom);
float truncf(float x);
float roundf(float x);
long lroundf(float x);
long long llroundf(float x);
float rintf(float x);
long lrintf(float x);
long long llrintf(float x);
float nearbyintf(float x);
float remainderf(float numer, float denom);
float remquof(float numer, float denom, int* quot);
// long double rintl(long double x);
// long int lrintl(long double x);
// long double ceill(long double x);
// long double floorl(long double x);
// long double fmodl(long double numer, long double denom);
// long double truncl (long double x);
// long double roundl(long double x);
// long int lroundl(long double x);
// long long int llroundl(long double x);
// long long int llrintl(long double x);
// long double nearbyintl(long double x);
// long double remainderl(long double numer, long double denom);
// long double remquol(long double numer, long double denom, int* quot);

// FP manipulation functions
double copysign(double x, double y);
double nan(const char* tagp);
double nextafter(double x, double y);
// double nexttoward  (double x, long double y);
float copysignf(float x, float y);
float nextafterf(float x, float y);
// float nexttowardf(float x, long double y);
// long double copysignl(long double x, long double y);
// long double nextafterl(long double x, long double y);
// long double nexttowardl(long double x, long double y);

// min, max, diff functions
double fdim(double x, double y);
double fmax(double x, double y);
double fmin(double x, double y);
float fdimf(float x, float y);
float fmaxf(float x, float y);
float fminf(float x, float y);
// long double fdiml(long double x, long double y);
// long double fmaxl(long double x, long double y);
// long double fminl(long double x, long double y);

// Other functions
double fabs(double x);
double abs(double x);
double fma(double x, double y, double z);
float fabsf(float x);
float abs(float x);
float fmaf(float x, float y, float z);
// long double fabsl(long double x);
// long double abs(long double x);
// long double fmal(long double x, long double y, long double z);
