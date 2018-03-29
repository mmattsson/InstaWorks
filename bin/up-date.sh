#!/bin/sh

# Make sure we don't alter files without any user input, print out explanation of the program

echo "up-date.sh - Should be run in the root directory of the git project. This will"
echo "find all files with the string 'Copyright' and 'Mattias Mattsson' and update"
echo "the copyright year."
echo ""
echo "Usage:"
echo "# up-date.sh update"
echo ""

if [ "$1" != "update" ]; then
    echo "No command provided, exiting."
    echo ""
    exit 0
fi

YEAR=`date '+%Y'`

SINGLE_DATE_FILES=`grep -r Copyright | grep "Mattias Mattsson" | grep " [0-9]\+ " | awk -F ":" '{ print $1 }'`

for F in $SINGLE_DATE_FILES
do
    echo "Adding end year copyright date in file '$F'"
    sed -i "s/\(Copyright.*\)\([0-9]\+\)\(.*Mattias Mattsson\)/\1\2-$YEAR\3/" $F
done

DATE_RANGE_FILES=`grep -r Copyright | grep "Mattias Mattsson" | grep "[0-9]\+-[0-9]\+" | awk -F ":" '{ print $1 }'`

for F in $DATE_RANGE_FILES
do
    P=`basename $F`
    N=`basename $0`
    if [ "$P" != "$N" ]; then
        echo "Updating end year copyright date in file '$F'"
        sed -i "/Copyright.*Mattias Mattsson/ s=-[0-9]\+=-$YEAR=" $F
    fi
done

