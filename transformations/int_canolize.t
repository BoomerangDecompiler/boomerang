x + 0 becomes x
x - n where (kind(n) == opIntConst) becomes x + (-n)
x - x becomes 0
(x + a) + b where (kind(a) == opIntConst) && (kind(b) == opIntConst) becomes x + (a + b)
(x * k) - x where (kind(k) == opIntConst) becomes x * (k-1)
x + (x * k) where (kind(k) == opIntConst) becomes x * (k+1)
(a*n)*m where (kind(n) == opIntConst) && (kind(m) == opIntConst) becomes a*(n*m)
