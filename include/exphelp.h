/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type sensitive
 */

class Exp;

// A helper file for comparing Exp*'s sensibly
class lessExpStar : public std::binary_function<Exp*, Exp*, bool> {
public:
    bool operator()(const Exp* x, const Exp* y) const;
};


/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type insensitive
 */
class lessTI : public std::binary_function<Exp*, Exp*, bool> {
public:
    bool operator()(const Exp* x, const Exp* y) const;
};


