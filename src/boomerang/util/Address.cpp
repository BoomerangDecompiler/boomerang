#include "Address.h"

#include <cassert>


const Address Address::ZERO    = Address(0);
const Address Address::INVALID = Address(-1);


Address Address::g(value_type x)
{
    return Address(x);
}


Address Address::n(value_type x)
{
    assert((x & ~0xFFFFFFFF) == 0);
    return Address(x);
}

