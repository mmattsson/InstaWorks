#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

command -v valgrind > /dev/null
HAS_VALGRIND=$?

if [ $HAS_VALGRIND = "0" ]; then
    valgrind -q --error-exitcode=123 --leak-check=full "$DIR/selftest" $*
    _EXIT_CODE=$?
    echo ""
    if [ "$_EXIT_CODE" = "123" ]; then
        echo " THERE WERE MEMORY ERRORS PRESENT!"
    else 
        echo " No memory errors present."
    fi
    echo ""
else
    "$DIR/selftest $*"
fi

