
int test(int a, int b, int c)
{
    if (a < b || b < c)
	    return 1;
    return 0;
}

int main()
{
    return test(4, 5, 6);
}
