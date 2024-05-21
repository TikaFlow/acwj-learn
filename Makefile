all: test

comp1: cg.c expr.c gen.c interp.c main.c scan.c tree.c
	cc -o comp1 -g cg.c expr.c gen.c interp.c main.c scan.c tree.c

clean:
	rm -f comp1 compn *.o *.s out

test: comp1
	./comp1 test/02-parser.txt
	cc -o out out.s
	./out