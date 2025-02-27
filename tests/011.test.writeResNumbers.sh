#!/bin/bash

GMML_ROOT_DIR=$(git rev-parse --show-toplevel)

if [ "$(git config --get remote.origin.url)" != "https://github.com/GLYCAM-Web/gmml.git" ]; then
            exit 1
fi

printf "Testing 011.writeResNumbers.cc (write original and new residue numbers into a PDB file)... "
g++ -std=c++17 -I "${GMML_ROOT_DIR}"/ -L"${GMML_ROOT_DIR}"/bin/ -Wl,-rpath,"${GMML_ROOT_DIR}"/bin/ tests/011.writeResNumbers.cc -lgmml -pthread -o writeResNumbers
./writeResNumbers tests/inputs/008.convertPdbToGlycam_4YG0.pdb > 011.output_writeResidueNumbers.txt
if [ -f 011.output.newNumbers.pdb ] && [ -f 011.output.original.pdb ]; then
    if ! cmp 011.output.newNumbers.pdb tests/correct_outputs/011.output.newNumbers.pdb > /dev/null 2>&1; then
        printf "Test FAILED! PDB file with new numbers is different\n"
        echo "Exit Code: 1"
        return 1
    elif ! cmp 011.output.original.pdb tests/correct_outputs/011.output.original.pdb > /dev/null 2>&1; then
        printf "Test FAILED! PDB file with original numbers is different\n"
        echo "Exit Code: 1"
        return 1
    else
        printf "Test passed.\n"
        rm 011.output.newNumbers.pdb 011.output.original.pdb writeResNumbers 011.output_writeResidueNumbers.txt
        echo "Exit Code: 0"
        return 0
    fi
else
    printf "Test FAILED!\n"
    echo "Exit Code: 1"
    return 1
fi

