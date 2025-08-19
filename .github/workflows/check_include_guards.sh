#!/bin/bash
# @file      check_include_guards.sh
# @author    Theodore Bardy
#
# @note      This file is part of Witekio's Zephyr Demo project
# @brief     Check include guards in header files

# Initialize list of files that are not properly formatted
result=""

# Check header files
for source_file in `git ls-tree -r HEAD --name-only | grep -E '(.*\.h$|.*\.hpp$)' | grep -vFf .clang-format-ignore`
do
    uppercase=$(echo $(basename ${source_file^^}) | tr '.' '_' | tr '-' '_')
    pcregrep -Me "#ifndef ${uppercase}\n#define ${uppercase}\n\n#ifdef __cplusplus\nextern \"C\"\n{\n#endif // __cplusplus" ${source_file} > /dev/null 2>&1
    if [[ ! $? -eq 0 ]]; then
        result="${result}\n${source_file}"
    else
        pcregrep -Me "#ifdef __cplusplus\n}\n#endif // __cplusplus\n\n#endif // ${uppercase}" ${source_file} > /dev/null 2>&1
        if [[ ! $? -eq 0 ]]; then
            result="${result}\n${source_file}"
        fi
   fi
done

# Check result
if [[ ${result} != "" ]]; then
    echo "The following files have wrong or missing include guards:"
    echo -e $result
    exit 1
fi

exit 0
