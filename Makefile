HEADER=defs.h data.h decl.h
INC_DIR=/tmp/include
SRCS=tcc.c scan.c expr.c cg.c gen.c tree.c stmt.c misc.c decl.c sym.c type.c opt.c
EXEC=./tcc

NEW=test/f03-member-offset.c
CPP=$(NEW:.c=.i)
ASM=$(CPP:.i=.s)

STARTER=/lib/x86_64-linux-gnu/crt1.o
LOADER=/lib64/ld-linux-x86-64.so.2

.PHONY: all test clean inc

all: test

inc:
	mkdir -p $(INC_DIR)
	rsync -a include/ $(INC_DIR)/

$(EXEC): $(HEADER) $(SRCS)
	gcc -o $@ -g -Wall -DDEBUG $^

new: $(NEW) $(EXEC) inc
	cpp -o $(CPP) -nostdinc -isystem $(INC_DIR) $(NEW)
	$(EXEC) $(CPP)
	mv $(ASM) out.s
	as -o out.o out.s
	ld -o out out.o $(STARTER) -lc -I $(LOADER)
	@echo "=================== $(NEW) ==================="
	@./out

test: clean $(EXEC) inc
	@echo "Running tests..."
	@test/test-all.sh $(EXEC) $(STARTER) $(LOADER)
	@echo "Comparing output with expected..."
	@diff test/output.txt test/expected.txt
	@echo "All Tests passed!"

clean:
	rm -f $(EXEC) out* *.out *.o *.s test/output.txt test/*.i test/*.s test/*.o