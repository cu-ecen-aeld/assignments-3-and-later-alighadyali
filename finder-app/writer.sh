# !bin/bash

if [ $# -ne 2 ]; then
    echo "Error: Total number of arguments should be 2."
    echo "The order of the arguments should be:"
    echo "  1) Path to file inc. filename."
    echo "  2) String"
    exit 1
fi

writefile=$1
writestr=$2

mkdir -p "$(dirname "$writefile")"

echo "$writestr" > "$writefile"

if [ $? -ne 0 ]; then
    echo "Error: Failed to create file $writefile."
    exit 1
fi

echo "File $writefile created successfully with content:"
echo "$writestr"