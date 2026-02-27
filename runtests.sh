#!/bin/bash

# compile the shell
make clean && make mysh

if [ ! -f mysh ]; then
    echo "Compilation failed!"
    exit 1
fi

TEST_DIR="test-cases"
TOTAL=0
PASSED=0

for test_file in $TEST_DIR/*.txt; do
    # Skip result files
    if [[ $test_file == *_result* ]]; then
        continue
    fi

    base=$(basename "$test_file" .txt)

    # Run the shell from within the test-cases directory
    # so it can find the script files (P_*)
    (cd $TEST_DIR && ../mysh < "$base.txt" > ../output.tmp 2>&1)

    # Find the expected result files
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
        echo "Expected (first result if exists):"
        if [ -f "$result1" ]; then
            cat "$result1"
        elif [ -f "$result2" ]; then
            cat "$result2"
        fi
        echo "Got:"
        cat output.tmp
    fi
    TOTAL=$((TOTAL + 1))
    rm -f output.tmp
done

echo "Passed $PASSED out of $TOTAL tests."