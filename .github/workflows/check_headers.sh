#!/bin/bash
# @file      check_headers.sh
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     Script to check the header format

# Function used to check headers
# Arg1: Source file path
# Arg2: First line, "" if it is not present
# Arg3: String added in front of each new line except the first and last lines
# Arg4: Last line, "" if it is not present
check_header() {
    source_file=$1
    first_line=$2
    new_line=$3
    last_line=$4
    filename=$(echo $(basename ${source_file}) | sed -r 's/\+/\\+/g')
    pcregrep -Me "${first_line}${first_line:+\n}${new_line} @file      ${filename}\n${new_line} @author    [[:print:]]*\n${new_line}\n${new_line} @note      This file is part of Witekio's Zephyr Demo project\n${new_line} @brief     [[:print:]]*${last_line:+\n}${last_line}\n" ${source_file} > /dev/null 2>&1
    return $?
}

# Initialize list of files that are not properly formatted
result=""

# Check source, header, ASM and linker files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.c$|.*\.h$|.*\.cpp$|.*\.hpp$|.*\.s$|.*\.ld$|.*\.dtsi|.*\.overlay)' | grep -vFf .clang-format-ignore`
do
    check_header ${source_file} "/\*\*" " \*" " \*/"
    if [[ ! $? -eq 0 ]]; then
        result="${result}\n${source_file}"
    fi
done

# Check YAML, Python, CMake and configuration files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.yml$|.*\.py$|CMakeLists.*\.txt$|.*\.cmake$|.*\.cfg$|.*\.conf$|.clang-format$|Kconfig$)' | grep -vFf .clang-format-ignore`
do
    check_header ${source_file} "" "#" ""
    if [[ ! $? -eq 0 ]]; then
        result="${result}\n${source_file}"
    fi
done

# Check bash files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.sh$)' | grep -vFf .clang-format-ignore`
do
    check_header ${source_file} "#!/bin/bash" "#" ""
    if [[ ! $? -eq 0 ]]; then
        result="${result}\n${source_file}"
    fi
done

# Check result
if [[ ${result} != "" ]]; then
    echo "The following files have wrong header format:"
    echo -e $result
    exit 1
fi

exit 0
