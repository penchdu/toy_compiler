int test(int n)
{
	int a = n + 1;
	int b = n + 2;
	int c = n + 3;
	int d = n + 4;
	int e = n + 5;
	int f = n + 6;
	int g = n + 7;
	int h = n + 8;
	int i = n + 9;
	int j = n + 10;
	int k = n + 11;
	int l = n + 12;
	int m = n + 13;
	int o = n + 14;
	int p = n + 15;
	int q = n + 20;
	int r = n + 16;
	int t = n + 17;
	int u = n + 18;
	int v = n + 19;

	a = b + c;
	b = c + d;
	c = d + e;
	d = e + f;
	e = f + g;
	f = g + h;
	g = h + i;
	h = i + j;
	i = j + k;
	j = k + l;
	k = l + m;
	l = m + o;
	m = o + p;
	o = p + q;
	p = q + r;
	q = r + t;
	r = t + u;
	t = u + v;
	u = v + a;
	v = a + b;
	a = o + p;
	b = q + r;

	return a + b + c + d + e + f + g + h + i + j + k + l + m + o + p + q + r + t + u + v;
}
