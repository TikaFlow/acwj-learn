#!/bin/bash

exec=$1

for file in test/*.c; do
    if [ -f "$file"  ]; then
        echo "=================== $file ===================" >> test/output.txt
        ./${exec} -vo out "$file"  || exit1
        ./out >> test/output.txt || exit 2
    fi
done

echo
echo "Done."
echo