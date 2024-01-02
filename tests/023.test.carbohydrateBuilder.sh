#!/bin/bash
printf "Testing 023.carbohydrateBuilder... "

GMML_ROOT_DIR=$(git rev-parse --show-toplevel)

if [[ "${GMML_ROOT_DIR}" != *"gmmlForkTest" ]]; then
    echo -e "Test 023 failed, we think our GMML root directory is:\t${GMML_ROOT_DIR}\n"
    exit 1
fi

g++ -std=c++17 -I "${GMML_ROOT_DIR}"/ -L"${GMML_ROOT_DIR}"/bin/ -Wl,-rpath,"${GMML_ROOT_DIR}"/bin/ ../internalPrograms/CarbohydrateBuilder/main.cpp -lgmml -pthread -o carbohydrateBuilder
rm -r 023.outputs/ >/dev/null 2>&1
mkdir 023.outputs/
./carbohydrateBuilder tests/inputs/023.smallLibrary.txt _ 023.outputs >023.output_carbohydrateBuilder.txt 2>&1

for i in $(cut -d _ -f1 tests/inputs/023.smallLibrary.txt); do
    if [ -f 023.outputs/"${i}".pdb ]; then
        echo "${i}.pdb succesfully created." >>023.output_carbohydrateBuilder.txt
        if ! cmp 023.outputs/"${i}".pdb tests/correct_outputs/023.outputs/"${i}".pdb >/dev/null 2>&1; then
            echo "Test FAILED! Created pdb file 023.outputs/${i}.pdb is different from tests/correct_outputs/023.outputs/${i}.pdb"
            echo "Exit Code: 1"
            return 1
        fi
    else
        echo "${i}.pdb not created." >>023.output_carbohydrateBuilder.txt
        if [ -f tests/correct_outputs/023.outputs/"${i}".pdb ]; then
            echo "Test FAILED! Did not create ${i}.pdb, yet it exists in tests/correct_outputs/023.outputs/${i}.pdb"
            echo "Exit Code: 1"
            return 1
        fi
    fi
done
for i in $(cut -d _ -f1 tests/inputs/023.smallLibrary.txt); do
    if [ -f 023.outputs/"${i}".off ]; then
        echo "${i}.off succesfully created." >>023.output_carbohydrateBuilder.txt
        if ! cmp 023.outputs/"${i}".off tests/correct_outputs/023.outputs/"${i}".off >/dev/null 2>&1; then
            echo "Test FAILED! Created off file 023.outputs/${i}.off is different from tests/correct_outputs/023.outputs/${i}.off"
            echo "Exit Code: 1"
            return 1
        fi
    else
        echo "${i}.off not created." >>023.output_carbohydrateBuilder.txt
        if [ -f tests/correct_outputs/023.outputs/"${i}".off ]; then
            echo "Test FAILED! Did not create ${i}.off, yet it exists in tests/correct_outputs/023.outputs/${i}.off"
            echo "Exit Code: 1"
            return 1
        fi
    fi
done
if ! cmp 023.output_carbohydrateBuilder.txt tests/correct_outputs/023.output_carbohydrateBuilder.txt >/dev/null 2>&1; then
    printf "Test FAILED! Output file %s different from %s \n" 023.output_carbohydrateBuilder.txt tests/correct_outputs/023.output_carbohydrateBuilder.txt
    echo "Exit Code: 1"
    return 1
else
    printf "Test passed.\n"
    rm -r 023.outputs/ carbohydrateBuilder 023.output_carbohydrateBuilder.txt
    echo "Exit Code: 0"
    return 0
fi
