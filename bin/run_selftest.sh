#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if valgrind is present
! command -v valgrind > /dev/null
HAS_VALGRIND=$?

# If we're just querying the selftest binary, then don't run the test
if [ "$#" = "0" -o "$1" = "show" ]; then
    $DIR/selftest $*
    exit 0
fi

if [ $HAS_VALGRIND = "1" ]; then
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
    $DIR/selftest $*
fi

