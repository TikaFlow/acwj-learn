SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c types.c
TEST=test/13-func2.txt

all: test

comp: $(SRCS)
	cc -o comp -g $^

test: $(TEST) comp lib/util.c
	./comp $<
	cc -o out out.s lib/util.c
	./out

clean:
	rm -f comp out *.o *.s