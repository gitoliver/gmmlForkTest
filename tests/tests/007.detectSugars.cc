// This is a C++ version of the GEMS test, detect_sugars.
// By: Davis Templeton
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "includes/gmml.hpp"

// This looks horrible, but it works. Someone make it
// look better.
const std::string GEMSHOME_ERROR = "\n"
"Must set GEMSHOME environment variable.\n\n"
"    BASH:   export GEMSHOME=/path/to/gems\n"
"    SH:     setenv GEMSHOME /path/to/gems\n";

const std::string USAGE = "\n"
"Usage:\n\n"
"    detect_sugars PDB_file.pdb\n\n"
"The output goes to standard out (your terminal window, ususally).\n"
"So, alternately:\n\n"
"    detect_sugars PDB_file.pdb > output_file_name\n";

int main(int argc, char *argv[])
{
  const std::string GEMSHOME = std::filesystem::current_path().parent_path().parent_path();

  // Check if the environment variable exists.
  if(GEMSHOME == "") {
      std::cout << GEMSHOME_ERROR << std::endl;
      return EXIT_FAILURE;
  }

  // Check to make sure we have enough command line arguments.
  if(argc < 2) {
      std::cout << USAGE << std::endl;
      return EXIT_FAILURE;
  }

  // Get the Amino Lib file from GMML.
  std::vector<std::string> aminolibs;
  aminolibs.push_back(GEMSHOME + "/gmml/dat/CurrentParams/leaprc.ff12SB_2014-04-24/amino12.lib");


  // Initialize an Assembly from the PDB file.
  // Get command line argument, which should be an PDB file.
  std::string pdb_file(argv[1]);
  // Initialize an Assembly from the PDB file.
  MolecularModeling::Assembly assembly(pdb_file, gmml::PDB);

  // Remove Hydrgens & Build by Distance
  assembly.RemoveAllHydrogenAtoms();
  assembly.BuildStructureByDistance(3);
  
  // Find the Sugars.
  bool glyprobity_report = false;
  bool populate_ontology = true;
  bool individualOntologies = false;
  assembly.ExtractSugars(aminolibs, glyprobity_report, populate_ontology, individualOntologies);
  
  // YAY! We made it!
  return EXIT_SUCCESS;
}
