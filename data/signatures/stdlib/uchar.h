
typedef unsigned short char16_t;
typedef unsigned int char32_t;
typedef unsigned int size_t;

typedef struct _mbstate_t mbstate_t;

size_t c16rtomb(char *pmb, char16_t c16, mbstate_t *ps);
size_t c32rtomb(char *pmb, char32_t c32, mbstate_t *ps);

size_t mbrtoc16(char16_t *pc16, const char *pmb, size_t max, mbstate_t *ps);
size_t mbrtoc32(char32_t *pc32, const char *pmb, size_t max, mbstate_t *ps);
