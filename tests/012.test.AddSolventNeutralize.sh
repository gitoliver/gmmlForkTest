#!/bin/bash

GMML_ROOT_DIR=$(git rev-parse --show-toplevel)

if [ "$(git config --get remote.origin.url)" != "https://github.com/GLYCAM-Web/gmml.git" ]; then
            exit 1
fi

## Note: Oliver was checking the functionality. It does not yet work as required, but it took a while to figure out how to run the code
## So this test is just a snapshot of how it's currently working and how I managed to get output.
printf "Testing 012.AddSolventNeutralize... "
g++ -std=c++17 -I "${GMML_ROOT_DIR}"/ -L"${GMML_ROOT_DIR}"/bin/ -Wl,-rpath,"${GMML_ROOT_DIR}"/bin/ tests/012.addSolventNeutralize.cc -lgmml -pthread -o addSolventNeutralize
./addSolventNeutralize > 012.output_addSolventNeutralize.txt
if [ -f 012.addSolventNeutralize.pdb ] ; then
    if ! cmp 012.addSolventNeutralize.pdb tests/correct_outputs/012.addSolventNeutralize.pdb > /dev/null 2>&1; then
        printf "Test FAILED! 012.addSolventNeutralize.pdb different from tests/correct_outputs/012.addSolventNeutralize.pdb\n"
        echo "Exit Code: 1"
        return 1
    #elif ! cmp structure.off tests/correct_outputs/010.buildBySequenceRotamer.off > /dev/null 2>&1; then
    #    printf "Test FAILED!. Off file different.\n"
    #    return 1;
    elif ! cmp  012.output_addSolventNeutralize.txt tests/correct_outputs/012.output_addSolventNeutralize.txt > /dev/null 2>&1; then
        printf "Test FAILED! Output file different\n"
        echo "Exit Code: 1"
        return 1
    else
        printf "Test passed.\n"
        rm 012.addSolventNeutralize.pdb addSolventNeutralize 012.output_addSolventNeutralize.txt
        echo "Exit Code: 0"
        return 0
    fi
else
    printf "Test FAILED!\n 012.addSolventNeutralize.pdb was not created.\n"
    echo "Exit Code: 1"
    return 1
fi

