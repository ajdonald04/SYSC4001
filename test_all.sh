#!/bin/bash

# Ensure the script exits if any command fails
set -e

# Define the executable and input file pattern
EXECUTABLE="/Users/jayvenlarsen/gitRepo/sysc4001/sim"
SCHEDULERS=("FCFS" "EP" "RR")
TRACE_FILES=("input_data_1.txt" "input_data_2.txt" "input_data_3.txt" "input_data_4.txt" "input_data_5.txt" 
             "input_data_6.txt" "input_data_7.txt" "input_data_8.txt" "input_data_9.txt" "input_data_10.txt")

# Loop through each trace file and each scheduler
for TRACE in "${TRACE_FILES[@]}"; do
    for SCHEDULER in "${SCHEDULERS[@]}"; do
        echo "Running $SCHEDULER with $TRACE"
        
        # Generate output filenames
        TRACE_NAME="${TRACE%.*}"  # Remove extension (e.g., .txt)
        EXEC_LOG="execution_${SCHEDULER}_${TRACE_NAME}"
        MEM_LOG="memory_status_${SCHEDULER}_${TRACE_NAME}"
        
        # Run the simulator with execution and memory log filenames as arguments
        $EXECUTABLE "$TRACE" "$SCHEDULER" "$EXEC_LOG" "$MEM_LOG"
    done
done

echo "All tests completed!"
