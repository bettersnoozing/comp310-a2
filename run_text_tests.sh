#!/bin/bash

make clean && make mysh

if [ ! -f mysh ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful."
echo "----------------------------"

TEST_DIR="test-cases"
shopt -s nullglob
TOTAL=0
PASSED=0

for testfile in "$TEST_DIR"/T_*.txt; do

    # Skip result files
    if [[ $testfile == *_result* ]]; then
        continue
    fi

    base=$(basename "$testfile" .txt)

    echo "Running $base ..."

    # Run test in batch mode
    (cd "$TEST_DIR" && ../mysh < "$base.txt" > ../output.tmp 2>&1)

    result1="$TEST_DIR/${base}_result.txt"
    result2="$TEST_DIR/${base}_result2.txt"

    matched=0

    if [ -f "$result1" ]; then
        if diff -w output.tmp "$result1" > /dev/null; then
            matched=1
        fi
    fi

    if [ $matched -eq 0 ] && [ -f "$result2" ]; then
        if diff -w output.tmp "$result2" > /dev/null; then
            matched=1
        fi
    fi

    if [ $matched -eq 1 ]; then
        echo "$base TEST PASSED"
        PASSED=$((PASSED + 1))
    else
        echo "$base TEST FAILED"
        echo "Expected:"
        if [ -f "$result1" ]; then
            cat "$result1"
        elif [ -f "$result2" ]; then
            cat "$result2"
        fi
        echo "Got:"
        cat output.tmp
        echo "----------------------------"
    fi

    TOTAL=$((TOTAL + 1))
    rm -f output.tmp

done

echo "============================"
echo "Passed $PASSED out of $TOTAL tests."
