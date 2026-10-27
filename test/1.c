int test(int n)
{
	int v0 = n + 1;
	int v1 = n + 2;
	int v2 = n + 3;
	int v3 = n + 4;
	int v4 = n + 5;

	v0 = v1 * v2 + v3 * v0 + v4 + n;
	v1 = v2 * v3 + v4 * v1 - v0 + n;
	v2 = v3 * v4 + v0 * v2 - v1 + n;
	v3 = v4 * v0 + v1 * v3 - v2 + n;
	v4 = v0 * v1 + v2 * v4 + v3 + n;
	return v0 + v1 + v2 + v3 + v4;
}
