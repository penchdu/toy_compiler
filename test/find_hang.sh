#!/bin/bash
cd /mnt/d/workspace/vsc/test
for f in case1/*.c case4/*.c case2/*.c case3_vsl_alloc_cases/*.c ../vsc_test/time_compare/case/*.c ../vsc_test/time_compare/case2/*.c; do
	[ -e "$f" ] || continue
	timeout 5 ../build/vsc "$f" > /dev/null 2>&1
	code=$?
	if [ $code -eq 124 ]; then echo "TIMEOUT: $f"
	elif [ $code -ne 0 ]; then echo "CRASH($code): $f"; fi
done
echo done