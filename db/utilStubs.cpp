#include "util.h"

void loadValue(std::istream &is, bool &b, bool wantlen) {}
void loadValue(std::istream &is, char &ch, bool wantlen) {}
void loadValue(std::istream &is, int& i, bool wantlen) {}
void loadValue(std::istream &is, double &d, bool wantlen) {}
void loadString(std::istream &is, std::string &str) {}
int loadFID(std::istream &is) {return 0;}
int loadLen(std::istream &is) {return 0;}
void saveValue(std::ostream &os, bool b, bool wantlen) {}
void saveValue(std::ostream &os, char ch, bool wantlen) {}
void saveValue(std::ostream &os, int i, bool wantlen) {}
void saveValue(std::ostream &os, double d, bool wantlen) {}
void saveString(std::ostream &os, const std::string &str) {}
void saveFID(std::ostream &os, int fid) {}
void load(Prog *prog, std::string &location) {}
void save(Prog *prog, std::string &location) {}
void skipFID(std::istream &is, int fid) {}
void saveLen(std::ostream &os, int len, bool large) {}
void loadValue(std::istream &is, ADDRESS &a, bool wantlen) {}
void saveValue(std::ostream &os, ADDRESS a, bool wantlen) {}

