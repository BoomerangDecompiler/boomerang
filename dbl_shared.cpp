// P  R
// |\/|
// |/\|
// Q  S
//  \/
//   T

#include <iostream>

class P {
public:
  int p1;
      		 P() {p1 = 7;}
virtual void pea() {std::cout << "P::pea\n";}
};

class R {
public:
  int r1;
      		 R() {r1 = 8;}
virtual void ah() {std::cout << "R::ah\n";}
};

class Q : public virtual P, public virtual R {
public:
	int q1;
			 Q() {q1 = 9;}
virtual void pea()	{std::cout << "Q::pea()\n";}
virtual void ah()	{std::cout << "Q::ah()\n";}
};

class S : public virtual P, public virtual R {
public:
	int s1;
			 S() {s1 = 10;}
virtual void pea()	{std::cout << "S::pea()\n";}
virtual void ah()	{std::cout << "S::ah()\n";}
};

class T: virtual public Q, virtual public S {
public:
	int t1;
			 T() {t1 = 11;}
virtual void pea()	{std::cout << "T::pea()\n";}
virtual void ah()	{std::cout << "T::ah()\n";}
virtual      ~T()   {t1 = -1;}
};

void fred() {
	T* t = new T();
	delete t;
}

int main() {
	T* t = new T();
	t->pea();
	t->ah();

	Q* q = (Q*)t;
	q->pea();
	q->ah();
	
	S* s = (S*)t;
	s->pea();
	s->ah();

	((P*)((Q*)t))->pea();
	((P*)((S*)t))->pea();

	((R*)((Q*)t))->ah();
	((R*)((S*)t))->ah();

	//std::cout << "r1 from R from Q from T: " << ((R*)((Q*)t))->r1 << std::endl;
	//std::cout << "r1 from T: " << t->r1 << std::endl;

	return 0;
}
