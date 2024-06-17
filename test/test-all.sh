#!/bin/bash

exec=$1
shift 1
starter=$1
shift 1
loader=$1

# clean file
echo -n "" > test/expected.txt
echo -n "" > test/output.txt

for file in test/*.c; do
    if [ -f ${file}  ]; then
        cpp=${file%.c}.i
        asm=${file%.c}.s
        obj=${file%.c}.o

        # read args if there are any
        args=""
        line=$(head -n 1 ${file})
        if [[ ${line} == "// arg:"* ]]; then
            args=${line#// arg:}
        fi

        echo "=================== ${file} ===================" >> test/expected.txt

        # get expected output
        gcc -S -o out.s ${file}
        as -o out.o out.s
        ld -o out out.o ${starter} -lc -I ${loader}
        ./out ${args} >> test/expected.txt
        rm -f out*

        echo "=================== ${file} ===================" >> test/output.txt

        # pre-process
        (set -x; cpp -o ${cpp} -nostdinc -isystem /tmp/include ${file}) || exit 1
        # compile
        (set -x; ${exec} ${cpp}) || exit 1
        # assemble
        (set -x; as -o ${obj} ${asm}) || exit 1
        # link
        (set -x; ld -o out ${obj} ${starter} -lc -I ${loader}) || exit 1

        # run with args
        (set -x; ./out ${args} >> test/output.txt) || exit 1
        echo
    fi
done

echo "Done."
echo