#!/bin/bash
# @file      check_equivalence_tests.sh
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     Script to check equivalence tests

# Initialize list of files that are not properly formatted
result=""

# Check source and header files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.c$|.*\.h$|.*\.cpp$|.*\.hpp$)' | grep -vFf .clang-format-ignore`
do
    # This grep matches following formats (which are not allowed):
    # - "variable == CONSTANT" / "variable != CONSTANT"
    # - "function(...) == CONSTANT" / "function(...) != CONSTANT"
    # - "table[...] == CONSTANT" / "table[...] != CONSTANT"
    # Where CONSTANT can be a definition or a number
    # Any number of spaces before and after "==" / "!=" is catched
    lines=$(grep "[])a-z][ ]*[!=]=[ ]*[A-Z0-9]" ${source_file} | wc -l)
    if [[ ! $lines -eq 0 ]]; then
        result="${result}\n${source_file}"
    fi
done

# Check result
if [[ ${result} != "" ]]; then
    echo "The following files are not properly formatted:"
    echo -e $result
    exit 1
fi

exit 0
