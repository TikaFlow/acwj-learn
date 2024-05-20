all: test test2

test: parser
	./parser test/02-parser.txt

test2: parser2
	./parser2 test/02-parser.txt

parser: expr.c interp.c main.c scan.c tree.c
	cc -o parser -g expr.c interp.c main.c scan.c tree.c

parser2: expr2.c interp.c main.c scan.c tree.c
	cc -o parser2 -g expr2.c interp.c main.c scan.c tree.c
clean:
	rm -f parser parser2 *.o