#include <stdio.h>

char *chomp(char *s, int size, FILE *f)
{
	char *res = fgets(s, size, f);
	if (res) {
		char *p = strchr(res, '\n');
		if (p) {
			*p = 0;
		}
	}
	return res;
}

int main(int argc, char **argv)
{
	FILE *f;
	char line[1024];

	if (argc < 2)
		return 1;
	f = fopen(argv[1], "r");
	if (f == NULL)
		return 1;
	if (chomp(line, sizeof(line), f)) {
		printf("%s\n", line);
	}
	fclose(f);
}
