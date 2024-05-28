SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c type.c
UTIL=lib/util.c

all: test

main: $(SRCS)
	cc -o main -g $^

test: main $(UTIL)
	@echo "Running tests..."
	@test/test-all.sh > test/output.txt
	@echo "Comparing output with expected..."
	@diff test/output.txt test/expected.txt
	@echo "All Tests passed!"

clean:
	rm -f main out *.o *.s test/output.txt