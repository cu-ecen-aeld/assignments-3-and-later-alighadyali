#!/bin/sh

# Check if both arguments are provided
if [ $# -ne 2 ]; then
    echo "Error: Total number of arguments should be 2."
    echo "The order of the arguments should be:"
    echo "  1) Path to dir."
    echo "  2) String"
    exit 1
fi

filesdir=$1
searchstr=$2

# Check if filesdir exists and is a directory
if [ ! -d "$filesdir" ]; then
    echo "Error: $filesdir is not a directory."
    exit 1
fi

num_files=$(find "$filesdir" -type f | wc -l)

num_matching_lines=$(grep -r "$searchstr" "$filesdir" | wc -l)

echo "The number of files are $num_files and the number of matching lines are $num_matching_lines"

exit 0
