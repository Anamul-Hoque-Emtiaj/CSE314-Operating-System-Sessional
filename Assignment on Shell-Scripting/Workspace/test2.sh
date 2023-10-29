#!/bin/bash

# Declare an array
fruits=("Apple" "Banana" "Orange")

# Access individual elements
echo "First fruit: ${fruits[0]}"
echo "Second fruit: ${fruits[1]}"
echo "Third fruit: ${fruits[2]}"

# Loop through the array
for fruit in "${fruits[@]}"
do
    echo "Fruit: $fruit"
done


name="Alice"
greeting="Hello, ${name^^}!"

echo $greeting   # Output: "Hello, ALICE!"

# Search for lines containing "example" in a file
grep "example" myfile.txt

# Get the type of a file
file myfile.txt


# Recursive function to calculate factorial
factorial() {
    if [ $1 -eq 0 ]; then
        echo 1
    else
        local prev=$(($1-1))
        local result=$(factorial $prev)
        echo $(($1*$result))
    fi
}

# Calculate factorial of 5
result=$(factorial 5)
echo "Factorial: $result"  # Output: "Factorial: 120"



# Replace "apple" with "orange" in a file
sed 's/apple/orange/' myfile.txt



# View the contents of a file using more
more myfile.txt


# View the contents of a file using less
less myfile.txt

# Display the first 5 lines of a file
head -n 5 myfile.txt

# Display the last 5 lines of a file
tail -n 5 myfile.txt

wc myfile.txt

ls /bin /usr/bin | sort | uniq | less

# Generate a range of numbers
echo {1..5}   # Output: 1 2 3 4 5

# Generate a sequence with a step value
echo {1..10..2}   # Output: 1 3 5 7 9

# Generate a combination of characters
echo {a,b,c}{1,2}   # Output: a1 a2 b1 b2 c1 c2



# Access variable value
name="John"
echo "Hello, $name!"   # Output: Hello, John!

# Set default value if variable is empty
unset age
echo "Age: ${age:-18}"   # Output: Age: 18 (default value)

# Perform string substitution
message="Hello, World!"
echo ${message/World/Universe}   # Output: Hello, Universe!

# Replace a word in a file
sed 's/apple/orange/' myfile.txt

# Print lines matching a pattern
sed -n '/pattern/p' myfile.txt

# Delete lines containing a pattern
sed '/pattern/d' myfile.txt

# Insert text after a specific line
sed '/pattern/a\Insert this line' myfile.txt




input_dir="$1"
output_dir="$2"

# Create the output directory if it doesn't exist
mkdir -p "$output_dir"

# Iterate over the files in the input directory
for file in "$input_dir"/*; do
  # Get the file name without extension
  filename=$(basename "$file")
  filename_no_ext="${filename%.*}"

  # Get the length of the filename
  length=${#filename_no_ext}

  # Create a subdirectory based on the length
  subdir="$output_dir/$length"
  mkdir -p "$subdir"

  # Copy the file to the subdirectory and rename it
  new_filename="$subdir/${#subdir}_$filename"
  cp "$file" "$new_filename"
done







