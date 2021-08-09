#!/bin/bash

# Formats the committed C/C++ files using clang-format

RETURN=0
COMMAND=$(which clang-format)
if [ $? -ne 0 ]; then
       echo "[!] clang-format not installed. Unable to check source file format policy." >&2
       exit 1
fi

FILES=`git diff --cached --name-only --diff-filter=ACMR | grep -E "\.(c|cc|cpp|h|hh)$"`
for FILE in $FILES; do
       $COMMAND $FILE | cmp -s $FILE -
       if [ $? -ne 0 ]; then
               echo "[!] $FILE does not respect the agreed coding style." >&2
               echo "Formatting: $FILE ..." >&2
               $COMMAND -i $FILE
               RETURN=1
       fi
done

exit $RETURN
