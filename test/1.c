int test(int n)
{
	int v0 = n + 1;
	int v1 = n + 2;
	int v2 = n + 3;
	int v3 = n + 4;
	int v4 = n + 5;
	int v5 = n + 6;

	v0 = v1 + v2 + v3 * v5 + v4;
//	v1 = v2 * v3 + v4 * v0 - v5;
//	v2 = v3 * v4 + v5 * v1 + v0;
//	v3 = v4 * v5 + v0 * v2 - v1;
//	v4 = v5 * v0 + v1 * v3 - v2;
//	v5 = v0 * v1 + v2 * v4 + v3;

	//return v1 + v2 + v3 + v4 + v5 + v0;
	return v0 + v1 + v2 + v3 + v4 + v5;
}




//int test(int n)
//{
//	int a = n + 1;
//	int b = n + 2;
//	int c = n + 3;
//	int d = n + 4;
//	int e = n + 5;
//	int f = n + 6;
//	int g = n + 7;
//	int h = n + 8;
//	int i = n + 9;
//	int j = n + 10;
//	int k = n + 11;
//	int l = n + 12;
//	int m = n + 13;
//	int o = n + 14;
//	int p = n + 15;
//	int q = n + 16;
//	int r = n + 17;
//	int t = n + 18;
//	int u = n + 19;
//	int v = n + 20;
//
//	a = b + c;
//	b = c + d;
//	c = d + e;
//	d = e + f;
//	e = f + g;
//	f = g + h;
//	g = h + i;
//	h = i + j;
//	i = j + k;
//	j = k + l;
//	k = l + m;
//	l = m + o;
//	m = o + p;
//	o = p + q;
//	p = q + r;
//	q = r + t;
//	r = t + u;
//	t = u + v;
//	u = v + a;
//	v = a + b;
//	a = o - p;
//	b = q - r;
//
//	return a + b + c + d + e + f + g + h + i + j - k + l - m + o * p + q + r + t + u + v;
//}
