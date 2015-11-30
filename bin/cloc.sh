#!/bin/sh

H_FILES=`find . -type f -name *.h`
#H_FILES=`find . -type f -name *.h | grep -v ./external`
C_FILES=`find . -type f -name *.c`
#C_FILES=`find . -type f -name *.c | grep -v ./external`

NUM_H_FILES=`echo $H_FILES | wc | awk '{ print $2 }'`
NUM_C_FILES=`echo $C_FILES | wc | awk '{ print $2 }'`

H_FILE_LINES=`wc -l $H_FILES | grep " \{1,\}[0-9]\{1,\} total" | awk '{ print $1 }'`
C_FILE_LINES=`wc -l $C_FILES | grep " \{1,\}[0-9]\{1,\} total" | awk '{ print $1 }'`

H_FILE_EMPTY=`cat $H_FILES | grep "^[[:space:]]*$" | wc -l`
C_FILE_EMPTY=`cat $C_FILES | grep "^[[:space:]]*$" | wc -l`

H_FILE_COMMENT=`cat $H_FILES | grep "^[[:space:]]*//" | wc -l`
C_FILE_COMMENT=`cat $C_FILES | grep "^[[:space:]]*//" | wc -l`

H_FILE_CODE=$((H_FILE_LINES - H_FILE_EMPTY - H_FILE_COMMENT))
C_FILE_CODE=$((C_FILE_LINES - C_FILE_EMPTY - C_FILE_COMMENT))

TOT_FILES=$((NUM_H_FILES + NUM_C_FILES))
TOT_FILE_EMPTY=$((H_FILE_EMPTY + C_FILE_EMPTY))
TOT_FILE_COMMENT=$((H_FILE_COMMENT + C_FILE_COMMENT))
TOT_FILE_CODE=$((H_FILE_CODE + C_FILE_CODE))
TOT_FILE_LINES=$((H_FILE_LINES + C_FILE_LINES))

echo "----------------------------------------------------------------------"
echo "                     Files      Empty   Comments      Code      Total"
echo "----------------------------------------------------------------------"
printf " Header Files        %5s      %5s      %5s     %5s      %5s\n" \
    $NUM_H_FILES  $H_FILE_EMPTY  $H_FILE_COMMENT  $H_FILE_CODE  $H_FILE_LINES
printf " C Source Files      %5s      %5s      %5s     %5s      %5s\n" \
    $NUM_C_FILES  $C_FILE_EMPTY  $C_FILE_COMMENT  $C_FILE_CODE  $C_FILE_LINES
echo "----------------------------------------------------------------------"
printf " Sum                 %5s      %5s      %5s     %5s      %5s\n" \
    $TOT_FILES $TOT_FILE_EMPTY $TOT_FILE_COMMENT $TOT_FILE_CODE $TOT_FILE_LINES
echo "----------------------------------------------------------------------"

