SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c type.c
TEST=test/19-array1.txt

all: test

main: $(SRCS)
	cc -o main -g $^

test: $(TEST) main lib/util.c
	./main $<
	cc -o out out.s lib/util.c
	./out

clean:
	rm -f main out *.o *.s