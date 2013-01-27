struct pair {
	int a;
	int b;
};

struct pair twofib(int n) {
	struct pair r;
	if (n == 0) {
		r.a = 0;
		r.b = 1;
	} else {
		r = twofib(n-1);
		int a = r.a;
		r.a = r.b;
		r.b += a;
	}
	return r;
}

int main() {
	int n;
	printf("Enter number: ");
	scanf("%d", &n);
	printf("Fibonacci of %d is %d\n", n, twofib(n).a);
	return 0;
}
