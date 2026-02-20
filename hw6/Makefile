CC=gcc
CFLAGS=-g

foobar.so: foobar.c
	$(CC) $(CFLAGS) -shared -fPIC $^ -o $@

foo: foo.c foobar.so
	$(CC) $(CFLAGS) $^ -o $@

bar: bar.c foobar.so
	$(CC) $(CFLAGS) $^ -o $@