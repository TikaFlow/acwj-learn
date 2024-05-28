#!/bin/bash

for file in test/*.c; do
    if [ -f "$file"  ]; then
        echo "=================== $file ==================="
        ./main "$file"
        cc -o out -g -no-pie out.s lib/util.c || exit 1
        ./out || exit 2
    fi
done