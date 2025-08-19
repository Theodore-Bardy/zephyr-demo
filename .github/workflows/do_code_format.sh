#!/bin/bash
# @file      do_code_format.sh
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     Format the code using clang-format

# Update source and header files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.c$|.*\.h$|.*\.cpp$|.*\.hpp$)' | grep -vFf .clang-format-ignore`
do
    clang-format -i ${source_file} || exit 1
done

exit 0
