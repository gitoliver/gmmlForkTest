#!/bin/bash
printf "Testing 019.test.newPDBClass.cpp... ~30 seconds. "
g++ -std=c++17 -I $GEMSHOME/gmml/ -L$GEMSHOME/gmml/bin/ -Wl,-rpath,$GEMSHOME/gmml/bin/ tests/019.test.newPDBClass.cpp -lgmml -pthread -o newPdbClass
for filepath in `ls tests/inputs/019.*.pdb`
do
	file=`basename $filepath`
	./newPdbClass tests/inputs/$file > output.txt
	if ! cmp  output.txt tests/correct_outputs/$file-output.txt > /dev/null 2>&1; then
    	printf "Test FAILED!. output.txt different from tests/correct_outputs/$file-output.txt\n Compare using diff\n"
    	echo "Exit Code: 1" 
    	return 1;
        exit 1;
	elif ! cmp  outputPdbFile.pdb tests/correct_outputs/$file-output.pdb > /dev/null 2>&1; then
		printf "Test FAILED!. outputPdbFile.pdb different from tests/correct_outputs/$file-output.pdb\n Compare using diff or VMD\n"
		echo "Exit Code: 1"
        return 1;
        exit 1;
	fi
done
printf "Test passed.\n"
rm ./newPdbClass outputPdbFile.pdb output.txt
echo "Exit Code: 0"
return 0
