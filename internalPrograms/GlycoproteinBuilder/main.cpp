#include "includes/CodeUtils/directories.hpp"
#include "includes/CentralDataStructure/InternalPrograms/GlycoproteinBuilder/glycoproteinBuilder.hpp"
#include "includes/CentralDataStructure/InternalPrograms/GlycoproteinBuilder/gpInputStructs.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " inputFile\n";
        std::cout << "Exmpl: " << argv[0] << " input.txt\n";
        std::exit(1);
    }
    std::string inputFile = argv[1];
    std::cout << "Input file is " << inputFile << "\n";
    glycoprotein::GlycoproteinBuilderInputs inputStruct = glycoprotein::readGPInputFile(inputFile);
    std::cout << "Reading input file complete, on to construction\n" << std::flush;
    GlycoproteinBuilder glycoproteinBuilder(inputStruct);
    if(glycoproteinBuilder.IsStatusOk())
    {    // Poor pattern, need to throw up to and catch in gems.
        std::cout << "Resolving overlaps" << std::endl;
        glycoproteinBuilder.ResolveOverlaps(); // Default randomize value is true, and output isn't deterministic.
        std::cout << "Writing output files" << std::endl;
        glycoproteinBuilder.WriteOutputFiles();
    }
    if (!glycoproteinBuilder.IsStatusOk()) // Status might be changed by ResolveOverlaps or WriteOuputfiles.
    {
        std::cerr << glycoproteinBuilder.GetStatusMessage() << std::endl;
    }
    std::cout << "Program got to end ok" << std::endl;
    return 0;
}
