
struct entry
{
    char    *key;
    void    *data;
};

typedef struct entry ENTRY;


/*enum ACTION { FIND, ENTER };
 *enum VISIT { preorder, postorder, endorder, leaf }; */
typedef int ACTION;
typedef int VISIT;


int    hcreate(size_t nel);
void   hdestroy(void);
ENTRY *hsearch(ENTRY item, ACTION action);
void   insque(void *element, void *pred);
void  *lfind(const void *key, const void *base, size_t *nelp,
             size_t width, int (*compar)(const void *lhs, const void *rhs));
void  *lsearch(const void *key, void *base, size_t *nelp, size_t width,
               int (*compar)(const void *lhs, const void *rhs));
void   remque(void *element);
void  *tdelete(const void *key, void **rootp, int (*compar)(const void *lhs, const void *rhs));
void  *tfind(const void *key, void **rootp, int (*compar)(const void *lhs, const void *rhs));
void  *tsearch(const void *key, void **rootp, int (*compar)(const void *lhs, const void *rhs));
void   twalk(const void *root, void (*action)(const void *node, VISIT type, int level));
