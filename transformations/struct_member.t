exp + n where (type(exp) == pointer(ty)) && 
              (ty == compound) &&
              (kind(n) == opIntConst) &&
              (m == memberAtOffset(ty, n)) && 
              (r == n - offsetToMember(ty, m))
    becomes a[m[exp].m] + r

