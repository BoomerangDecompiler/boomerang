
// globals for pentium
int eax, ebx, ecx, edx, esi, edi, esp, ebp;
int flags;
double st, st1, st2, st3, st4, st5, st6, st7, st8, st9;

// globals for sparc
int g0, g1, g2, g3, g4, g5, g6, g7, g8, g9;
int g10, g11, g12, g13, g14, g15, g16, g17, g18, g19;
int g20, g21, g22, g23, g24, g25, g26, g27, g28, g29;
int g30, g31;
int i0, i1, i2, i3, i4, i5, fp, i7;
int o0, o1, o2, o3, o4, o5, o6, o7;
int l0, l1, l2, l3, l4, l5, l6, l7;
int LR;
float f0, f1, f2, f3, f4, f5, f6, f7;
double f2to3;   // this is evil

// temps
int tmp, tmp1, tmp2, tmp3, tmp4;
double tmpD9;


// target endianness, change if you're not on a little endian machine
int target_endianness = 0;

#define PENTIUMSETUP() { esp = malloc(65536); esp += 65536; \
						 esp -= 4; *(int*)esp = (int)envp; \
						 esp -= 4; *(int*)esp = (int)argv; \
						 esp -= 4; *(int*)esp = (int)argc; \
						 esp -= 4; *(int*)esp = 0; \
					   }
#define SPARCSETUP() { o6 = malloc(65536); o6 += 65400; }
#define SUBFLAGS(x, y, r) ((x) - (y))
#define ADDFLAGS(x, y, r) ((x) + (y))
#define SUBFLAGS32(x, y, r) ((x) - (y))
#define ADDFLAGS32(x, y, r) ((x) + (y))
#define LOGICALFLAGS32(x) ((x))
#define SHRFLAGS32(x, y, r) ((y))
#define SALFLAGS32(x, y, r) ((y))
#define SARFLAGS(x, y, r) ((y))

#define ADDR(x) ((unsigned int)(x) >= start_data &&  \
				 (unsigned int)(x) < start_data + data_size ?  \
				 	(unsigned int)(x) - start_data + (unsigned int)data :  \
				 (unsigned int)(x) >= start_rodata && \
				 (unsigned int)(x) < start_rodata + rodata_size ? \
				 (unsigned int)(x) - start_rodata + (unsigned int)rodata : \
				 (unsigned int)(x) >= start_data1 && \
				 (unsigned int)(x) < start_data1 + data1_size ? \
				 (unsigned int)(x) - start_data1 + (unsigned int)data1 : \
				 (x))
#define MEMASSIGN(x, y) *(int*)(ADDR(x)) = (y)
#define MEMOF(a) (source_endianness != target_endianness && \
				    (a) != ADDR(a) ? \
					(*(unsigned int*)ADDR(a) & 0xff000000) >> 24 | \
					(*(unsigned int*)ADDR(a) & 0x00ff0000) >> 8  | \
					(*(unsigned int*)ADDR(a) & 0x0000ff00) << 8  | \
					(*(unsigned int*)ADDR(a) & 0x000000ff) << 24 \
					: *(unsigned int*)(a))
#define FLOAT_MEMOF(x) (tmp = MEMOF(x), *(float*)&tmp)
#define FLOAT_MEMASSIGN(x, y) *(float*)(ADDR(x)) = (y)
#define DOUBLE_MEMOF(x) *(double*)(ADDR(x))
#define DOUBLE_MEMASSIGN(x, y) *(double*)(ADDR(x)) = (y)

#define CF 0
