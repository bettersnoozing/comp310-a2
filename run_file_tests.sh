#!/bin/bash

make clean && make mysh

if [ ! -f mysh ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful."
echo "----------------------------"

PROGRAM_DIR="test-cases" 
TOTAL=0

# Loop over every P_*.file
for prog in $PROGRAM_DIR/P_*; do
    base=$(basename "$prog")

    echo "Running $base ..."
    
    # Create temporary input file for mysh
    echo "exec $base FCFS" > temp_input.txt
    echo "quit" >> temp_input.txt

    # Run shell from inside program directory
    (cd $PROGRAM_DIR && ../mysh < ../temp_input.txt > ../output.txt 2>&1)

    echo "Output for $base:"
    echo "----------------------------"
    cat output.txt
    echo "============================"
    echo ""

    TOTAL=$((TOTAL + 1))
done

rm -f temp_input.txt output.txt

echo "Finished running $TOTAL programs."
