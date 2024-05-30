HEADER=defs.h data.h decl.h
SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c type.c
EXEC=tcc

.PHONY: all test clean

all: test

$(EXEC): $(HEADER) $(SRCS)
	gcc -o $@ -g -Wall $^

test: $(EXEC)
	@echo "Running tests..."
	@test/test-all.sh $(EXEC)
	@echo "Comparing output with expected..."
	@diff test/output.txt test/expected.txt
	@echo "All Tests passed!"

clean:
	rm -f $(EXEC) out* *.out *.o *.s test/output.txt test/*.s test/*.o