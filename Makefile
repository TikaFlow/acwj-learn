SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c
TEST=test/06-variables.txt

all: test

comp: $(SRCS)
	cc -o comp -g $^

test: $(TEST) comp
	./comp $<
	cc -o out out.s
	./out

clean:
	rm -f comp out *.o *.s