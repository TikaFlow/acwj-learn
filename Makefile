all: test

comp: main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c
	cc -o comp -g main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c

test: comp
	./comp test/05-statements.txt
	cc -o out out.s
	./out

clean:
	rm -f comp out *.o *.s