#!/bin/sh
# Script to compare production code to files stored in directory $1
# Modifications by Lotherius to produce individual diff files.

cd ../src

for file in *.[ch]
do
    if [ -s diff_$file ]
    then
       rm diff_$file
    fi
	echo '################' >> diff_$file
	echo $file >> diff_$file
	diff -b -B -p -c "$file" "/home/aelfwyne/sunder/src/$file" >> diff_$file
	echo $file >> diff_$file
	echo '################' >> diff_$file
	echo  >> diff_$file
    mv diff_$file /home/aelfwyne/sunder2.1a/bin/diffs/
done


