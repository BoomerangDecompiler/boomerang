a + b where (kind(a) == opIntConst) && (kind(b) == opIntConst) && (c == plus(a, b)) becomes c
a - b where (kind(a) == opIntConst) && (kind(b) == opIntConst) && (c == plus(a, neg(b))) becomes c
(-a) where (kind(a) == opIntConst) && (b == neg(a)) becomes b
