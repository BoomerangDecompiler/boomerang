char gca[5] = {2, 4, 6, 8, 10};

int main() {
	int i, sum = 0;
	for (i=0; i < 5; i++)
		sum += gca[i];
	printf("Sum is %d\n", sum);
	return 0;
}
