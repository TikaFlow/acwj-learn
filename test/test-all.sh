#!/bin/bash

exec=$1
shift 1
macros=$@

for file in test/*.c; do
    if [ -f "$file"  ]; then
        echo "=================== $file ===================" >> test/output.txt
        # compile
        (set -x; ./${exec} -vo out ${macros} "$file") || exit 1

        # read args if there are any
        args=""
        line=$(head -n 1 "$file")
        if [[ ${line} == "// arg:"* ]]; then
            args=${line#// arg:}
        fi

        # run with args
        (set -x; ./out ${args} >> test/output.txt) || exit 2
        echo
    fi
done

echo "Done."
echo