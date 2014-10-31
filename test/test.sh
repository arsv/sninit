#!/bin/sh

# Usage: checkrun.sh file.out
#
# If there is file.rig around, compare output to file.rig
# Otherwise, look for "FAIL" in the output.

out=$1
rig=${out/.out/.rig}

if [ -f "$rig" ]; then
	diff -q "$out" "$rig" || exit 1
else
	grep -q FAIL "$out" && cat "$out" && exit 1
fi

exit 0
