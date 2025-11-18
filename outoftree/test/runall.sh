#!/bin/bash

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=========================================${NC}"
echo -e "${BLUE}      Running All Test Suites            ${NC}"
echo -e "${BLUE}=========================================${NC}"

# Find all 'run.sh' files in subdirectories, sort them so they run in order
find . -name "run.sh" | sort | while read script_path; do
    
    # Get the directory and the filename
    dir_name=$(dirname "$script_path")
    base_name=$(basename "$script_path")

    # Skip the current script if it happens to be named run.sh (prevents recursion)
    if [ "$script_path" == "./run.sh" ]; then
        continue
    fi

    echo -e "\n${BLUE}[INFO] Entering directory: ${dir_name}${NC}"
    
    # Pushd: Save current location and change to the test directory
    pushd "$dir_name" > /dev/null

    # Ensure the script is executable
    if [ ! -x "$base_name" ]; then
        echo -e "${BLUE}[INFO] Making $base_name executable...${NC}"
        chmod +x "$base_name"
    fi

    # Execute the script
    echo -e "${GREEN}>>> Executing $base_name...${NC}"
    ./"$base_name"
    exit_code=$?

    # Check status
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}[SUCCESS] Finished $script_path${NC}"
    else
        echo -e "${RED}[FAILURE] Script $script_path failed with exit code $exit_code${NC}"
    fi

    # Popd: Return to the previous directory
    popd > /dev/null

done

echo -e "\n${BLUE}=========================================${NC}"
echo -e "${BLUE}      All Test Suites Completed          ${NC}"
echo -e "${BLUE}=========================================${NC}"