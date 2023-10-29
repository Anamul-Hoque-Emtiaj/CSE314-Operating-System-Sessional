#!/bin/bash

print_usage() {
    echo "Usage: "
    echo -e "./organize.sh <submission folder> <target folder> <test folder> <answer folder> [-v] [-noexecute]\n"
    echo "-v: verbose"
    echo -e "-noexecute: do not execute code files\n"
}

if [ $# -lt 4 ]; then
    print_usage
    exit 1
fi

submissionFolder=$1
targetFolder=$2
testFolder=$3
answerFolder=$4
printInfo=false
noExecute=false

print_info() 
{
    if [ "$printInfo" = true ]; then
        echo "$1"
    fi
}

for arg in "${@:5}"
do
    case "$arg" in
        -v)
            printInfo=true
            ;;
        -noexecute)
            noExecute=true
            ;;
        *)
            echo "Invalid Argument: $arg"
            exit 1
            ;;
    esac

done

count=$(ls -1 "$testFolder" | wc -l)
print_info "Found "$count" test files"

if [ -d "$targetFolder" ]; then
    rm -rf "$targetFolder"
fi
mkdir -p "$targetFolder"
mkdir -p "$targetFolder"/C
mkdir -p "$targetFolder"/Python
mkdir -p "$targetFolder"/Java
if [ "$noExecute" = false ]; then
    result="$targetFolder/result.csv"
    chmod +w "$targetFolder"
    echo "student_id,type,matched,not_matched" > "$result"
fi

for zip_file in "$submissionFolder"/*.zip
do
    tmp=${zip_file: -11}
    sid=${tmp%.zip}
    mkdir -p "$sid"
    unzip -qq "$zip_file" -d "$sid"
    cd "$sid"
    print_info "Organizing files of $sid"
    fileName=""
    type=""
    # Save the current IFS value
    OLDIFS="$IFS"
    # Set IFS to newline
    IFS=$'\n'
    for file in $(find . -type f)
    do

        if [[ "$file" == *".c" ]]; then
            fileName=$(realpath "$file")
            type="C"
            break
        elif [[ "$file" == *".py" ]]; then
            fileName=$(realpath "$file")
            type="Python"
            break
        elif [[ "$file" == *".java" ]]; then
            fileName=$(realpath "$file")
            type="Java"
            break
        fi
    done
    # Restore the original IFS value
    IFS="$OLDIFS"
    cd ../
    studentDir="$targetFolder"/"$type"/"$sid"
    mkdir -p "$studentDir"
    if [ "$type" = "C" ]; then
        cp "$fileName" "$studentDir"/main.c
    elif [ "$type" = "Python" ]; then
        cp "$fileName" "$studentDir"/main.py
    elif [ "$type" = "Java" ]; then
        cp "$fileName" "$studentDir"/Main.java
    fi

    if [ "$noExecute" = false ]; then
        print_info "Executing files of $sid"
        if [ "$type" = "C" ]; then
            gcc "$studentDir"/main.c -o "$studentDir"/main.out
        elif [ "$type" = "Java" ]; then
            javac "$studentDir"/Main.java
        fi

        matched=0
        notMatched=0
        

        for testFile in "$testFolder"/test*.txt
        do
            tmp=${testFile%.txt}
            testNum=${tmp:10}
            outputFile="$studentDir"/out"$testNum".txt
            if [ "$type" = "C" ]; then
                "$studentDir"/main.out < "$testFile" > "$outputFile"
            elif [ "$type" = "Python" ]; then
                python3 "$studentDir"/main.py < "$testFile" > "$outputFile"
            elif [ "$type" = "Java" ]; then
                java -cp "$studentDir" Main < "$testFile" > "$outputFile"
            fi

            answerFile="$answerFolder"/ans"$testNum".txt

            if diff_result=$(diff -q "$outputFile" "$answerFile"); then
                matched=$((matched + 1))
            else
                notMatched=$((notMatched + 1))
                
            fi
            
        done
        echo "$sid","$type","$matched","$notMatched" >> "$result"
    fi
    
    rm -rf "$sid"
    
done

