HEADER=defs.h data.h decl.h
SRCS=main.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c type.c
EXEC=tcc

NEW=test/32-struct-access.c
ASM=$(NEW:.c=.s)

.PHONY: all test clean

all: test

$(EXEC): $(HEADER) $(SRCS)
	gcc -o $@ -g -Wall $^

new: $(NEW) $(EXEC)
	./$(EXEC) -S $<
	mv $(ASM) out.s
	as -o out.o out.s
	ld -o out out.o /lib/x86_64-linux-gnu/crt1.o -lc -I /lib64/ld-linux-x86-64.so.2
	./out

test: $(EXEC)
	@echo "Running tests..."
	@test/test-all.sh $(EXEC)
	@echo "Comparing output with expected..."
	@diff test/output.txt test/expected.txt
	@echo "All Tests passed!"

clean:
	rm -f $(EXEC) main out* *.out *.o *.s test/output.txt test/*.s test/*.o