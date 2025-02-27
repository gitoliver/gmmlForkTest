#include <math.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <queue>
#include <stack>

#include "includes/MolecularModeling/assembly.hpp"
#include "includes/MolecularModeling/residue.hpp"
#include "includes/MolecularModeling/atom.hpp"
#include "includes/MolecularModeling/atomnode.hpp"
#include "includes/InputSet/CondensedSequenceSpace/condensedsequence.hpp"
#include "includes/InputSet/CondensedSequenceSpace/condensedsequenceresidue.hpp"
#include "includes/InputSet/CondensedSequenceSpace/condensedsequenceglycam06residue.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyfile.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyassembly.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyresidue.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyatom.hpp"
#include "includes/InputSet/TopologyFileSpace/topologybond.hpp"
#include "includes/InputSet/TopologyFileSpace/topologybondtype.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyangle.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyangletype.hpp"
#include "includes/InputSet/TopologyFileSpace/topologydihedral.hpp"
#include "includes/InputSet/TopologyFileSpace/topologydihedraltype.hpp"
#include "includes/InputSet/TopologyFileSpace/topologyatompair.hpp"
#include "includes/InputSet/CoordinateFileSpace/coordinatefile.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfile.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileresidue.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileatom.hpp"
#include "includes/InputSet/PdbFileSpace/pdbfile.hpp"
#include "includes/InputSet/PdbFileSpace/pdbtitlesection.hpp"
#include "includes/InputSet/PdbFileSpace/pdbmodelcard.hpp"
#include "includes/InputSet/PdbFileSpace/pdbmodelsection.hpp"
#include "includes/InputSet/PdbFileSpace/pdbmodelresidueset.hpp"
#include "includes/InputSet/PdbFileSpace/pdbatomcard.hpp"
#include "includes/InputSet/PdbFileSpace/pdbheterogenatomsection.hpp"
#include "includes/InputSet/PdbFileSpace/pdbatomsection.hpp"
#include "includes/InputSet/PdbFileSpace/pdbconnectsection.hpp"
#include "includes/InputSet/PdbFileSpace/pdblinkcard.hpp"
#include "includes/InputSet/PdbFileSpace/pdblinksection.hpp"
#include "includes/InputSet/PdbFileSpace/pdblinkcardresidue.hpp"
#include "includes/InputSet/PdbFileSpace/pdbfileprocessingexception.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtfile.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtatom.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtmodel.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtmodelcard.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtatomcard.hpp"
#include "includes/InputSet/PdbqtFileSpace/pdbqtmodelresidueset.hpp"
#include "includes/ParameterSet/LibraryFileSpace/libraryfile.hpp"
#include "includes/ParameterSet/LibraryFileSpace/libraryfileatom.hpp"
#include "includes/ParameterSet/LibraryFileSpace/libraryfileresidue.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfile.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfilebond.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfileangle.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfiledihedral.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfiledihedralterm.hpp"
#include "includes/ParameterSet/ParameterFileSpace/parameterfileatom.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfile.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileresidue.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileatom.hpp"
#include "includes/GeometryTopology/grid.hpp"
#include "includes/GeometryTopology/cell.hpp"
#include "includes/CodeUtils/logging.hpp"

#include <unistd.h>
#include <errno.h>
#include <string.h>

using MolecularModeling::Assembly;

//////////////////////////////////////////////////////////
//                       FUNCTIONS                      //
//////////////////////////////////////////////////////////
TopologyFileSpace::TopologyFile* Assembly::BuildTopologyFileStructureFromAssembly(std::string parameter_file_path, std::string ion_parameter_file_path)
{
    gmml::log(__LINE__, __FILE__, gmml::INF, "Creating topology file ...");
    TopologyFileSpace::TopologyFile* topology_file = new TopologyFileSpace::TopologyFile();
    TopologyFileSpace::TopologyAssembly* topology_assembly = new TopologyFileSpace::TopologyAssembly();
    ResidueVector assembly_residues = this->GetAllResiduesOfAssembly();
    int number_of_excluded_atoms = 0;
    int residue_counter = 0;
    int atom_counter = 1;
    std::stringstream ss;
    int bond_type_counter = 0;
    std::vector<std::vector<std::string> > inserted_bond_types = std::vector<std::vector<std::string> >();
    std::vector<std::vector<std::string> > inserted_bonds = std::vector<std::vector<std::string> >();
    int angle_type_counter = 0;
    std::vector<std::vector<std::string> > inserted_angle_types = std::vector<std::vector<std::string> >();
    std::vector<std::vector<std::string> > inserted_angles = std::vector<std::vector<std::string> >();
    int dihedral_type_counter = 0;
    std::vector<std::string>  inserted_dihedral_types = std::vector<std::string>();
    std::vector<std::vector<std::string> > inserted_dihedrals = std::vector<std::vector<std::string> >();
    TopologyFileSpace::TopologyFile::TopologyAtomPairMap pairs = TopologyFileSpace::TopologyFile::TopologyAtomPairMap();
    int pair_count = 1;
    std::vector<std::string> inserted_pairs = std::vector<std::string>();
    std::vector<std::string> excluded_atom_list = std::vector<std::string>();
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::BondMap bonds = parameter_file->GetBonds();
    ParameterFileSpace::ParameterFile::AngleMap angles = parameter_file->GetAngles();
    ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
    ParameterFileSpace::ParameterFile::AtomTypeMap atom_types_map = parameter_file->GetAtomTypes();
    ParameterFileSpace::ParameterFile* ion_parameter_file = NULL;
    ParameterFileSpace::ParameterFile::AtomTypeMap ion_atom_types_map = ParameterFileSpace::ParameterFile::AtomTypeMap();
    if(ion_parameter_file_path.compare("") != 0)
    {
        ion_parameter_file = new ParameterFileSpace::ParameterFile(ion_parameter_file_path, gmml::IONICMOD);
        ion_atom_types_map = ion_parameter_file->GetAtomTypes();
    }
    for(ResidueVector::iterator it = assembly_residues.begin(); it != assembly_residues.end(); it++)
    {
        Residue* assembly_residue = *it;

        TopologyFileSpace::TopologyResidue* topology_residue = new TopologyFileSpace::TopologyResidue();
        residue_counter++;
        topology_residue->SetIndex(residue_counter);
        std::string residue_name = assembly_residue->GetName();
        if(residue_name.compare("TIP3") == 0 || residue_name.compare("TIP3PBOX") == 0)
            residue_name = "TP3";
        if(residue_name.compare("TIP5") == 0 || residue_name.compare("TIP5PBOX") == 0)
            residue_name = "TP5";
        topology_residue->SetResidueName(residue_name);
        if(distance(assembly_residues.begin(), it) == (int)assembly_residues.size()-1)
            ss << assembly_residue->GetName();
        else
            ss << assembly_residue->GetName() << "-";

       // --Assinging assembly residue which is solvent to topology residue.---edited by Ayush on 07/11/17
        bool is_residue_solvent=assembly_residue->GetIsResidueSolvent();
        topology_residue->SetIsResidueSolvent(is_residue_solvent);


        topology_residue->SetStartingAtomIndex(atom_counter);
        AtomVector assembly_atoms = assembly_residue->GetAtoms();
        PrepFileSpace::PrepFileResidue::Loop loops = PrepFileSpace::PrepFileResidue::Loop();
        std::vector<int> bond_index = std::vector<int>();
        int atom_index = 1;

        std::vector<gmml::TopologicalType> residue_topological_types = GetAllTopologicalTypesOfAtomsOfResidue(assembly_atoms, loops, bond_index, 0);

        for(AtomVector::iterator it1 = assembly_atoms.begin(); it1 != assembly_atoms.end(); it1++)
        {
            Atom* assembly_atom = (*it1);
            std::stringstream key1;
            key1 << assembly_atom->GetId();
            TopologyFileSpace::TopologyAtom* topology_atom = new TopologyFileSpace::TopologyAtom();
            topology_atom->SetAtomName(assembly_atom->GetName());
            topology_atom->SetAtomCharge(assembly_atom->MolecularDynamicAtom::GetCharge() * gmml::CHARGE_DIVIDER);
            topology_atom->SetAtomicNumber(gmml::iNotSet);
            topology_atom->SetAtomMass(assembly_atom->GetMass());
            topology_atom->SetResidueName(assembly_residue->GetName());
            topology_atom->SetType(assembly_atom->GetAtomType());
//            if(atom_types_map.find(assembly_atom->GetAtomType()) == atom_types_map.end())
//                std::cout << assembly_atom->GetAtomType() << " atom type is not found in parameter file" << std::endl;
            topology_atom->SetTreeChainClasification("0");
            topology_atom->SetRadii(gmml::dNotSet);
            topology_atom->SetScreen(gmml::dNotSet);
            topology_atom->SetIndex(atom_counter);
            topology_atom->SetTreeChainClasification(gmml::ConvertTopologicalType2String(residue_topological_types.at(atom_index - 1)));
            topology_residue->AddAtom(topology_atom);
            atom_counter++;
            atom_index++;
            ///Bond Types, Bonds
            AtomNode* atom_node = assembly_atom->GetNode();
            if(atom_node != NULL)
            {
                AtomVector neighbors = atom_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors.begin(); it2 != neighbors.end(); it2++)
                {
                    Atom* neighbor = (*it2);
                    std::stringstream key2;
                    key2 << neighbor->GetId();
                    ExtractTopologyBondTypesFromAssembly(inserted_bond_types, assembly_atom, neighbor, bonds, bond_type_counter, topology_file);
                    ExtractTopologyBondsFromAssembly(inserted_bonds, inserted_bond_types, assembly_atom, neighbor, topology_file);

                    ///Excluded Atoms
                    std::stringstream first_order_interaction;
                    std::stringstream reverse_first_order_interaction;
                    first_order_interaction << key1.str() << "-" << key2.str();
                    reverse_first_order_interaction << key2.str() << "-" << key1.str();
                    if(find(excluded_atom_list.begin(), excluded_atom_list.end(), first_order_interaction.str()) == excluded_atom_list.end() &&
                            find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_first_order_interaction.str()) == excluded_atom_list.end())
                    {
                        excluded_atom_list.push_back(first_order_interaction.str());
                        std::vector<std::string> key_tokens = gmml::Split(key2.str(),"_");
                        topology_atom->AddExcludedAtom(key_tokens.at(2) + "(" + key_tokens.at(4) + "):" + key_tokens.at(0) + "(" + key_tokens.at(1) +")");
                    }

                    ///Angle Types, Angle
                    AtomNode* neighbor_node = neighbor->GetNode();
                    AtomVector neighbors_of_neighbor = neighbor_node->GetNodeNeighbors();

                    for(AtomVector::iterator it3 = neighbors_of_neighbor.begin(); it3 != neighbors_of_neighbor.end(); it3++)
                    {
                        Atom* neighbor_of_neighbor = (*it3);
                        std::stringstream key3;
                        key3 << neighbor_of_neighbor->GetId();
                        if(key1.str().compare(key3.str()) != 0)
                        {
                            ExtractTopologyAngleTypesFromAssembly(assembly_atom, neighbor, neighbor_of_neighbor, inserted_angle_types, angle_type_counter,
                                                                  topology_file, angles);
                            ExtractTopologyAnglesFromAssembly(assembly_atom, neighbor, neighbor_of_neighbor, inserted_angles, inserted_angle_types, topology_file);

                            ///Excluded Atoms
                            std::stringstream second_order_interaction;
                            std::stringstream reverse_second_order_interaction;
                            second_order_interaction << key1.str() << "-" << key3.str();
                            reverse_second_order_interaction << key3.str() << "-" << key1.str();
                            if(find(excluded_atom_list.begin(), excluded_atom_list.end(), second_order_interaction.str()) == excluded_atom_list.end() &&
                                    find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_second_order_interaction.str()) == excluded_atom_list.end())
                            {
                                excluded_atom_list.push_back(second_order_interaction.str());
                                std::vector<std::string> key_tokens = gmml::Split(key3.str(),"_");
                                topology_atom->AddExcludedAtom(key_tokens.at(2) + "(" + key_tokens.at(4) + "):" + key_tokens.at(0) + "(" + key_tokens.at(1) +")");
                            }


                            //Dihedral Types, Dihedrals
                            AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                            AtomVector neighbors_of_neighbor_neighbor = neighbor_of_neighbor_node->GetNodeNeighbors();
                            for(AtomVector::iterator it4 =  neighbors_of_neighbor_neighbor.begin(); it4 != neighbors_of_neighbor_neighbor.end(); it4++)
                            {
                                Atom* neighbor_of_neighbor_of_neighbor = (*it4);
                                std::stringstream key4;
                                key4 << neighbor_of_neighbor_of_neighbor->GetId();
                                if(key2.str().compare(key4.str()) != 0)
                                {
                                    ExtractTopologyDihedralTypesFromAssembly(assembly_atom, neighbor, neighbor_of_neighbor, neighbor_of_neighbor_of_neighbor,
                                                                             inserted_dihedral_types, dihedral_type_counter, topology_file, dihedrals);
                                    ExtractTopologyDihedralsFromAssembly(assembly_atom, neighbor, neighbor_of_neighbor, neighbor_of_neighbor_of_neighbor,
                                                                         inserted_dihedrals, inserted_dihedral_types, dihedrals, topology_file);

                                    ///Excluded Atoms
                                    std::stringstream third_order_interaction;
                                    std::stringstream reverse_third_order_interaction;
                                    third_order_interaction << key1.str() << "-" << key4.str();
                                    reverse_third_order_interaction << key4.str() << "-" << key1.str();
                                    if(find(excluded_atom_list.begin(), excluded_atom_list.end(), third_order_interaction.str()) == excluded_atom_list.end() &&
                                            find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_third_order_interaction.str()) == excluded_atom_list.end())
                                    {
                                        excluded_atom_list.push_back(third_order_interaction.str());
                                        std::vector<std::string> key_tokens = gmml::Split(key4.str(),"_");
                                        topology_atom->AddExcludedAtom(key_tokens.at(2) + "(" + key_tokens.at(4) + "):" + key_tokens.at(0) + "(" + key_tokens.at(1) +")");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            topology_atom->GetExcludedAtoms().size() == 0 ? number_of_excluded_atoms++ :
                                                            number_of_excluded_atoms += topology_atom->GetExcludedAtoms().size();
        }
        topology_assembly->AddResidue(topology_residue);
    }

    AtomVector all_atoms = this->GetAllAtomsOfAssembly();
    for(AtomVector::iterator it = all_atoms.begin(); it != all_atoms.end(); it++)
    {
        Atom* assembly_atom = *it;
        ///Pairs
        for(AtomVector::iterator it2 = all_atoms.begin(); it2 != all_atoms.end(); it2++)
        {
            Atom* pair_assembly_atom = (*it2);
            std::string atom_type1 = assembly_atom->GetAtomType();
            std::string atom_type2 = pair_assembly_atom->GetAtomType();
            std::vector<std::string> pair_vector = std::vector<std::string>();
            pair_vector.push_back(atom_type1);
            pair_vector.push_back(atom_type2);
            std::stringstream sss;
            sss << atom_type1 << "-" << atom_type2;
            std::stringstream reverse_sss;
            reverse_sss << atom_type2 << "-" << atom_type1;
            if(find(inserted_pairs.begin(), inserted_pairs.end(), sss.str()) == inserted_pairs.end() &&
                    find(inserted_pairs.begin(), inserted_pairs.end(), reverse_sss.str()) == inserted_pairs.end())
            {
                TopologyFileSpace::TopologyAtomPair* topology_atom_pair = new TopologyFileSpace::TopologyAtomPair();
                if(atom_types_map.find(atom_type1) != atom_types_map.end() && atom_types_map.find(atom_type2) != atom_types_map.end())
                {
                    ParameterFileSpace::ParameterFileAtom* parameter_atom1 = atom_types_map[atom_type1];
                    ParameterFileSpace::ParameterFileAtom* parameter_atom2 = atom_types_map[atom_type2];
                    double epsilon = sqrt(parameter_atom1->GetWellDepth() * parameter_atom2->GetWellDepth());
                    double sigma = pow(parameter_atom1->GetRadius() + parameter_atom2->GetRadius(), 6);
                    double coefficient_a = epsilon * sigma * sigma;
                    double coefficient_b = 2 * epsilon * sigma;
                    topology_atom_pair->SetCoefficientA(coefficient_a);
                    topology_atom_pair->SetCoefficientB(coefficient_b);
                    topology_atom_pair->SetPairType(sss.str());
                    topology_atom_pair->SetIndex(pair_count);
                    pair_count++;
                    inserted_pairs.push_back(sss.str());
                    pairs[sss.str()] = topology_atom_pair;
                }
                else if(!ion_atom_types_map.empty() && ion_atom_types_map.find(atom_type1) != ion_atom_types_map.end() &&
                        ion_atom_types_map.find(atom_type2) != ion_atom_types_map.end())
                {
                    ParameterFileSpace::ParameterFileAtom* parameter_atom1 = ion_atom_types_map[atom_type1];
                    ParameterFileSpace::ParameterFileAtom* parameter_atom2 = ion_atom_types_map[atom_type2];
                    double epsilon = sqrt(parameter_atom1->GetWellDepth() * parameter_atom2->GetWellDepth());
                    double sigma = pow(parameter_atom1->GetRadius() + parameter_atom2->GetRadius(), 6);
                    double coefficient_a = epsilon * sigma * sigma;
                    double coefficient_b = 2 * epsilon * sigma;
                    topology_atom_pair->SetCoefficientA(coefficient_a);
                    topology_atom_pair->SetCoefficientB(coefficient_b);
                    topology_atom_pair->SetPairType(sss.str());
                    topology_atom_pair->SetIndex(pair_count);
                    pair_count++;
                    inserted_pairs.push_back(sss.str());
                    pairs[sss.str()] = topology_atom_pair;
                }
                else if(atom_types_map.find(atom_type1) != atom_types_map.end() && !ion_atom_types_map.empty() &&
                        ion_atom_types_map.find(atom_type2) != ion_atom_types_map.end())
                {
                    ParameterFileSpace::ParameterFileAtom* parameter_atom1 = atom_types_map[atom_type1];
                    ParameterFileSpace::ParameterFileAtom* parameter_atom2 = ion_atom_types_map[atom_type2];
                    double epsilon = sqrt(parameter_atom1->GetWellDepth() * parameter_atom2->GetWellDepth());
                    double sigma = pow(parameter_atom1->GetRadius() + parameter_atom2->GetRadius(), 6);
                    double coefficient_a = epsilon * sigma * sigma;
                    double coefficient_b = 2 * epsilon * sigma;
                    topology_atom_pair->SetCoefficientA(coefficient_a);
                    topology_atom_pair->SetCoefficientB(coefficient_b);
                    topology_atom_pair->SetPairType(sss.str());
                    topology_atom_pair->SetIndex(pair_count);
                    pair_count++;
                    inserted_pairs.push_back(sss.str());
                    pairs[sss.str()] = topology_atom_pair;
                }
                else if(!ion_atom_types_map.empty() && ion_atom_types_map.find(atom_type1) != ion_atom_types_map.end() &&
                        atom_types_map.find(atom_type2) != atom_types_map.end())
                {
                    ParameterFileSpace::ParameterFileAtom* parameter_atom1 = ion_atom_types_map[atom_type1];
                    ParameterFileSpace::ParameterFileAtom* parameter_atom2 = atom_types_map[atom_type2];
                    double epsilon = sqrt(parameter_atom1->GetWellDepth() * parameter_atom2->GetWellDepth());
                    double sigma = pow(parameter_atom1->GetRadius() + parameter_atom2->GetRadius(), 6);
                    double coefficient_a = epsilon * sigma * sigma;
                    double coefficient_b = 2 * epsilon * sigma;
                    topology_atom_pair->SetCoefficientA(coefficient_a);
                    topology_atom_pair->SetCoefficientB(coefficient_b);
                    topology_atom_pair->SetPairType(sss.str());
                    topology_atom_pair->SetIndex(pair_count);
                    pair_count++;
                    inserted_pairs.push_back(sss.str());
                    pairs[sss.str()] = topology_atom_pair;
                }
            }
        }
    }

    topology_assembly->SetAssemblyName(ss.str());
    topology_file->SetAtomPairs(pairs);
    topology_file->SetAssembly(topology_assembly);

    // Set headers
    topology_file->SetNumberOfAtoms(this->CountNumberOfAtoms());
    topology_file->SetNumberOfTypes(this->CountNumberOfAtomTypes());
    topology_file->SetNumberOfBondsIncludingHydrogen(this->CountNumberOfBondsIncludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfBondsExcludingHydrogen(this->CountNumberOfBondsExcludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfAnglesIncludingHydrogen(this->CountNumberOfAnglesIncludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfAnglesExcludingHydrogen(this->CountNumberOfAnglesExcludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfDihedralsIncludingHydrogen(this->CountNumberOfDihedralsIncludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfDihedralsExcludingHydrogen(this->CountNumberOfDihedralsExcludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfExcludedAtoms(number_of_excluded_atoms);
    topology_file->SetNumberOfResidues(this->CountNumberOfResidues());
    topology_file->SetTotalNumberOfBonds(this->CountNumberOfBondsExcludingHydrogen(parameter_file_path));
    topology_file->SetTotalNumberOfAngles(this->CountNumberOfAnglesExcludingHydrogen(parameter_file_path));
    topology_file->SetTotalNumberOfDihedrals(this->CountNumberOfDihedralsExcludingHydrogen(parameter_file_path));
    topology_file->SetNumberOfBondTypes(this->CountNumberOfBondTypes(parameter_file_path));
    topology_file->SetNumberOfAngleTypes(this->CountNumberOfAngleTypes(parameter_file_path));
    topology_file->SetNumberOfDihedralTypes(this->CountNumberOfDihedralTypes(parameter_file_path));
    topology_file->SetNumberOfAtomsInLargestResidue(this->CountMaxNumberOfAtomsInLargestResidue());

    return topology_file;
}

void Assembly::ExtractTopologyBondTypesFromAssembly(std::vector<std::vector<std::string> > &inserted_bond_types, Atom* assembly_atom, Atom* neighbor, ParameterFileSpace::ParameterFile::BondMap &bonds,
                                                    int &bond_type_counter, TopologyFileSpace::TopologyFile* topology_file)
{
    std::vector<std::string> atom_pair_type = std::vector<std::string>();
    std::vector<std::string> reverse_atom_pair_type = std::vector<std::string>();
    std::stringstream key2;
    key2 << neighbor->GetId();
    atom_pair_type.push_back(assembly_atom->GetAtomType());
    atom_pair_type.push_back(neighbor->GetAtomType());
    reverse_atom_pair_type.push_back(neighbor->GetAtomType());
    reverse_atom_pair_type.push_back(assembly_atom->GetAtomType());

    if(find(inserted_bond_types.begin(), inserted_bond_types.end(), atom_pair_type) == inserted_bond_types.end() &&
            find(inserted_bond_types.begin(), inserted_bond_types.end(), reverse_atom_pair_type) == inserted_bond_types.end())
    {
        ParameterFileSpace::ParameterFileBond* parameter_file_bond;
        if(bonds.find(atom_pair_type) != bonds.end())
        {
            parameter_file_bond = bonds[atom_pair_type];
            inserted_bond_types.push_back(atom_pair_type);
        }
        else if(bonds.find(reverse_atom_pair_type) != bonds.end())
        {
            parameter_file_bond = bonds[reverse_atom_pair_type];
            inserted_bond_types.push_back(reverse_atom_pair_type);
        }
        else
        {
            //            std::stringstream ss;
            //            ss << atom_pair_type.at(0) << "-" << atom_pair_type.at(1) << " bond type does not exist in the parameter files";
            //            std::cout << ss.str() << std::endl;
            //            gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
            return;
        }
        TopologyFileSpace::TopologyBondType* topology_bond_type = new TopologyFileSpace::TopologyBondType();
        topology_bond_type->SetForceConstant(parameter_file_bond->GetForceConstant());
        topology_bond_type->SetEquilibriumValue(parameter_file_bond->GetLength());
        topology_bond_type->SetIndex(bond_type_counter);
        bond_type_counter++;
        topology_file->AddBondType(topology_bond_type);
    }
}

void Assembly::ExtractTopologyBondsFromAssembly(std::vector<std::vector<std::string> > &inserted_bonds, std::vector<std::vector<std::string> > &inserted_bond_types,
                                                Atom *assembly_atom, Atom *neighbor, TopologyFileSpace::TopologyFile* topology_file)
{
    std::vector<std::string> atom_pair_type = std::vector<std::string>();
    std::vector<std::string> reverse_atom_pair_type = std::vector<std::string>();
    std::stringstream key2;
    key2 << neighbor->GetId();
    atom_pair_type.push_back(assembly_atom->GetAtomType());
    atom_pair_type.push_back(neighbor->GetAtomType());
    reverse_atom_pair_type.push_back(neighbor->GetAtomType());
    reverse_atom_pair_type.push_back(assembly_atom->GetAtomType());

    std::vector<std::string> atom_pair_name = std::vector<std::string>();;
    std::vector<std::string> reverse_atom_pair_name = std::vector<std::string>();;
    atom_pair_name.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
    atom_pair_name.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
    reverse_atom_pair_name.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
    reverse_atom_pair_name.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
    std::vector<std::string> residue_names = std::vector<std::string>();;
    std::vector<std::string> reverse_residue_names = std::vector<std::string>();;
    residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
    residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
    reverse_residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
    reverse_residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
    std::vector<std::string> bond = std::vector<std::string>();
    std::vector<std::string> reverse_bond = std::vector<std::string>();
    std::stringstream ss;
    ss << residue_names.at(0) << ":" << atom_pair_name.at(0);
    std::stringstream ss1;
    ss1 << residue_names.at(1) << ":" << atom_pair_name.at(1);
    bond.push_back(ss.str());
    bond.push_back(ss1.str());
    reverse_bond.push_back(ss1.str());
    reverse_bond.push_back(ss.str());


    if(find(inserted_bonds.begin(), inserted_bonds.end(), bond) == inserted_bonds.end() &&
            find(inserted_bonds.begin(), inserted_bonds.end(), reverse_bond) == inserted_bonds.end())
    {
        TopologyFileSpace::TopologyBond* topology_bond=NULL;
        if(find(inserted_bonds.begin(), inserted_bonds.end(), bond) == inserted_bonds.end())
        {
            topology_bond = new TopologyFileSpace::TopologyBond(atom_pair_name, residue_names);
            inserted_bonds.push_back(bond);
        }
        else if (find(inserted_bonds.begin(), inserted_bonds.end(), reverse_bond) == inserted_bonds.end())
        {
            topology_bond = new TopologyFileSpace::TopologyBond(reverse_atom_pair_name, reverse_residue_names);
            inserted_bonds.push_back(reverse_bond);
        }

        if((assembly_atom->GetName().substr(0,1).compare("H") == 0 ||
            (assembly_atom->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(assembly_atom->GetName().substr(0,1)))))
                || (neighbor->GetName().substr(0,1).compare("H") == 0 ||
                    (neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor->GetName().substr(0,1))))))
            topology_bond->SetIncludingHydrogen(true);
        else
            topology_bond->SetIncludingHydrogen(false);

        int index = 0;
        if(find(inserted_bond_types.begin(), inserted_bond_types.end(), atom_pair_type) != inserted_bond_types.end())
            index = distance(inserted_bond_types.begin(), find(inserted_bond_types.begin(), inserted_bond_types.end(), atom_pair_type));
        else if(find(inserted_bond_types.begin(), inserted_bond_types.end(), reverse_atom_pair_type) != inserted_bond_types.end())
            index = distance(inserted_bond_types.begin(), find(inserted_bond_types.begin(), inserted_bond_types.end(), reverse_atom_pair_type));
        else
        {
            //            std::stringstream ss;
            //            ss << atom_pair_type.at(0) << "-" << atom_pair_type.at(1) << " bond type does not exist in the parameter files";
            //            std::cout << ss.str() << std::endl;
            //            gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
            return;
        }
        topology_bond->SetBondType(topology_file->GetBondTypeByIndex(index));
        topology_file->AddBond(topology_bond);
    }
}



void Assembly::ExtractTopologyAngleTypesFromAssembly(Atom* assembly_atom, Atom* neighbor, Atom* neighbor_of_neighbor, std::vector<std::vector<std::string> > &inserted_angle_types,
                                                     int &angle_type_counter, TopologyFileSpace::TopologyFile* topology_file, ParameterFileSpace::ParameterFile::AngleMap &angles)
{
    std::vector<std::string> angle_type = std::vector<std::string>();
    std::vector<std::string> reverse_angle_type = std::vector<std::string>();
    angle_type.push_back(assembly_atom->GetAtomType());
    angle_type.push_back(neighbor->GetAtomType());
    angle_type.push_back(neighbor_of_neighbor->GetAtomType());
    reverse_angle_type.push_back(neighbor_of_neighbor->GetAtomType());
    reverse_angle_type.push_back(neighbor->GetAtomType());
    reverse_angle_type.push_back(assembly_atom->GetAtomType());

    if(find(inserted_angle_types.begin(), inserted_angle_types.end(), angle_type) == inserted_angle_types.end() &&
            find(inserted_angle_types.begin(), inserted_angle_types.end(), reverse_angle_type) == inserted_angle_types.end())
    {
        ParameterFileSpace::ParameterFileAngle* parameter_file_angle;
        if(angles.find(angle_type) != angles.end())
        {
            parameter_file_angle = angles[angle_type];
            inserted_angle_types.push_back(angle_type);
        }
        else if(angles.find(reverse_angle_type) != angles.end())
        {
            parameter_file_angle = angles[reverse_angle_type];
            inserted_angle_types.push_back(reverse_angle_type);
        }
        else
        {
            //            std::stringstream ss;
            //            ss << angle_type.at(0) << "-" << angle_type.at(1) << "-" << angle_type.at(2) << " angle type does not exist in the parameter files";
            //            std::cout << ss.str() << std::endl;
            //            gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
            return;
        }
        TopologyFileSpace::TopologyAngleType* topology_angle_type = new TopologyFileSpace::TopologyAngleType();
        topology_angle_type->SetForceConstant(parameter_file_angle->GetForceConstant());
        topology_angle_type->SetEquilibriumValue(parameter_file_angle->GetAngle());
        topology_angle_type->SetIndex(angle_type_counter);
        angle_type_counter++;
        topology_file->AddAngleType(topology_angle_type);
    }
}

void Assembly::ExtractTopologyAnglesFromAssembly(Atom* assembly_atom, Atom* neighbor, Atom* neighbor_of_neighbor, std::vector<std::vector<std::string> > &inserted_angles,
                                                 std::vector<std::vector<std::string> > &inserted_angle_types, TopologyFileSpace::TopologyFile* topology_file)
{
    std::vector<std::string> angle_type = std::vector<std::string>();
    std::vector<std::string> reverse_angle_type = std::vector<std::string>();
    angle_type.push_back(assembly_atom->GetAtomType());
    angle_type.push_back(neighbor->GetAtomType());
    angle_type.push_back(neighbor_of_neighbor->GetAtomType());
    reverse_angle_type.push_back(neighbor_of_neighbor->GetAtomType());
    reverse_angle_type.push_back(neighbor->GetAtomType());
    reverse_angle_type.push_back(assembly_atom->GetAtomType());

    std::vector<std::string> angle_atom_names = std::vector<std::string>();
    std::vector<std::string> reverse_angle_atom_names = std::vector<std::string>();
    angle_atom_names.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
    angle_atom_names.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
    angle_atom_names.push_back(neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor->GetId(),"_").at(1) + ")");
    reverse_angle_atom_names.push_back(neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor->GetId(),"_").at(1) + ")");
    reverse_angle_atom_names.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
    reverse_angle_atom_names.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");

    std::vector<std::string> residue_names = std::vector<std::string>();
    std::vector<std::string> reverse_residue_names = std::vector<std::string>();
    residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
    residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
    residue_names.push_back(neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
    reverse_residue_names.push_back(neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
    reverse_residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
    reverse_residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
    std::vector<std::string> angle = std::vector<std::string>();
    std::vector<std::string> reverse_angle = std::vector<std::string>();
    std::stringstream ss;
    ss << residue_names.at(0) << ":" << angle_atom_names.at(0);
    std::stringstream ss1;
    ss1 << residue_names.at(1) << ":" << angle_atom_names.at(1);
    std::stringstream ss2;
    ss2 << residue_names.at(2) << ":" << angle_atom_names.at(2);
    angle.push_back(ss.str());
    angle.push_back(ss1.str());
    angle.push_back(ss2.str());
    reverse_angle.push_back(ss2.str());
    reverse_angle.push_back(ss1.str());
    reverse_angle.push_back(ss.str());

    if(find(inserted_angles.begin(), inserted_angles.end(), angle) == inserted_angles.end() &&
            find(inserted_angles.begin(), inserted_angles.end(), reverse_angle) == inserted_angles.end())
    {
        TopologyFileSpace::TopologyAngle* topology_angle=NULL;
        if(find(inserted_angles.begin(), inserted_angles.end(), angle) == inserted_angles.end())
        {
            topology_angle = new TopologyFileSpace::TopologyAngle(angle_atom_names, residue_names);
            inserted_angles.push_back(angle);
        }
        else if (find(inserted_angles.begin(), inserted_angles.end(), reverse_angle) == inserted_angles.end())
        {
            topology_angle = new TopologyFileSpace::TopologyAngle(reverse_angle_atom_names, reverse_residue_names);
            inserted_angles.push_back(reverse_angle);
        }
        if((assembly_atom->GetName().substr(0,1).compare("H") == 0 ||
            (assembly_atom->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(assembly_atom->GetName().substr(0,1)))))
                || (neighbor->GetName().substr(0,1).compare("H") == 0 ||
                    (neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor->GetName().substr(0,1)))))
                ||(neighbor_of_neighbor->GetName().substr(0,1).compare("H") == 0 ||
                   (neighbor_of_neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor->GetName().substr(0,1))))))
            topology_angle->SetIncludingHydrogen(true);
        else
            topology_angle->SetIncludingHydrogen(false);

        int index = 0;
        if(find(inserted_angle_types.begin(), inserted_angle_types.end(), angle_type) != inserted_angle_types.end())
            index = distance(inserted_angle_types.begin(), find(inserted_angle_types.begin(), inserted_angle_types.end(), angle_type));
        else if(find(inserted_angle_types.begin(), inserted_angle_types.end(), reverse_angle_type) != inserted_angle_types.end())
            index = distance(inserted_angle_types.begin(), find(inserted_angle_types.begin(), inserted_angle_types.end(), reverse_angle_type));
        else
        {
            //            std::stringstream ss;
            //            ss << angle_type.at(0) << "-" << angle_type.at(1) << "-" << angle_type.at(2) << " angle type does not exist in the parameter files";
            //            std::cout << ss.str() << std::endl;
            //            gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
            return;
        }
        topology_angle->SetAnlgeType(topology_file->GetAngleTypeByIndex(index));
        topology_file->AddAngle(topology_angle);
    }
}

void Assembly::ExtractTopologyDihedralTypesFromAssembly(Atom *assembly_atom, Atom *neighbor, Atom *neighbor_of_neighbor, Atom *neighbor_of_neighbor_of_neighbor,
                                                        std::vector<std::string>& inserted_dihedral_types, int &dihedral_type_counter, TopologyFileSpace::TopologyFile *topology_file, ParameterFileSpace::ParameterFile::DihedralMap& dihedrals)
{
    std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(assembly_atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                      neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
    bool is_found = false;
    for(std::vector<std::vector<std::string> >::iterator it = all_atom_type_permutations.begin(); it != all_atom_type_permutations.end(); it++)
    {
        std::vector<std::string> atom_types = (*it);
        if(dihedrals[atom_types] != NULL)
        {
            std::stringstream ss;
            ss << atom_types.at(0) << "_" << atom_types.at(1) << "_" << atom_types.at(2) << "_" << atom_types.at(3);
            if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), ss.str()) == inserted_dihedral_types.end())
            {
                ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedral = dihedrals[atom_types];
                std::vector<ParameterFileSpace::ParameterFileDihedralTerm> dihedral_terms = parameter_file_dihedral->GetTerms();
                for(std::vector<ParameterFileSpace::ParameterFileDihedralTerm>::iterator it1 = dihedral_terms.begin(); it1 != dihedral_terms.end(); it1++)
                {
                    inserted_dihedral_types.push_back(ss.str());
                    ParameterFileSpace::ParameterFileDihedralTerm parameter_file_dihedral_term = (*it1);
                    TopologyFileSpace::TopologyDihedralType* topology_dihedral_type = new TopologyFileSpace::TopologyDihedralType();
                    topology_dihedral_type->SetIndex(dihedral_type_counter);
                    dihedral_type_counter++;
                    topology_dihedral_type->SetForceConstant(parameter_file_dihedral_term.GetForceConstant());
                    topology_dihedral_type->SetPeriodicity(fabs(parameter_file_dihedral_term.GetPeriodicity()));
                    topology_dihedral_type->SetPhase(parameter_file_dihedral_term.GetPhase());
                    topology_dihedral_type->SetScee(parameter_file_dihedral->GetScee());
                    topology_dihedral_type->SetScnb(parameter_file_dihedral->GetScnb());
                    topology_file->AddDihedralType(topology_dihedral_type);
                }
                is_found = true;
                break;
            }
        }
    }
    if(!is_found)
    {
        //        std::stringstream ss;
        //        ss << all_atom_type_permutations.at(0).at(0) << "-" << all_atom_type_permutations.at(0).at(1) << "-" << all_atom_type_permutations.at(0).at(2) << "-"
        //           << all_atom_type_permutations.at(0).at(3) << " dihedral type (or any other permutation of it) does not exist in the parameter files";
        //        std::cout << ss.str() << std::endl;
        //        gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
    }

    ///Improper Dihedrals
    AtomNode* atom_node = assembly_atom->GetNode();
    AtomVector neighbors = atom_node->GetNodeNeighbors();
    if(neighbors.size() == 3)
    {
        Atom* neighbor1 = neighbors.at(0);
        Atom* neighbor2 = neighbors.at(1);
        Atom* neighbor3 = neighbors.at(2);
        std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                     neighbor3->GetAtomType(), assembly_atom->GetAtomType());
        bool is_improper_found = false;
        for(std::vector<std::vector<std::string> >::iterator it = all_improper_dihedrals_atom_type_permutations.begin(); it != all_improper_dihedrals_atom_type_permutations.end(); it++)
        {
            std::vector<std::string> improper_dihedral_permutation = (*it);
            if(dihedrals[improper_dihedral_permutation] != NULL)
            {
                std::stringstream ss;
                ss << improper_dihedral_permutation.at(0) << "_" << improper_dihedral_permutation.at(1) << "_" << improper_dihedral_permutation.at(2) << "_" << improper_dihedral_permutation.at(3);
                if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), ss.str()) == inserted_dihedral_types.end())
                {
                    ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedral = dihedrals[improper_dihedral_permutation];
                    std::vector<ParameterFileSpace::ParameterFileDihedralTerm> dihedral_terms = parameter_file_dihedral->GetTerms();
                    for(std::vector<ParameterFileSpace::ParameterFileDihedralTerm>::iterator it1 = dihedral_terms.begin(); it1 != dihedral_terms.end(); it1++)
                    {
                        inserted_dihedral_types.push_back(ss.str());
                        ParameterFileSpace::ParameterFileDihedralTerm parameter_file_dihedral_term = (*it1);
                        TopologyFileSpace::TopologyDihedralType* topology_dihedral_type = new TopologyFileSpace::TopologyDihedralType();
                        topology_dihedral_type->SetIndex(dihedral_type_counter);
                        dihedral_type_counter++;
                        topology_dihedral_type->SetForceConstant(parameter_file_dihedral_term.GetForceConstant());
                        topology_dihedral_type->SetPeriodicity(fabs(parameter_file_dihedral_term.GetPeriodicity()));
                        topology_dihedral_type->SetPhase(parameter_file_dihedral_term.GetPhase());
                        topology_dihedral_type->SetScee(parameter_file_dihedral->GetScee());
                        topology_dihedral_type->SetScnb(parameter_file_dihedral->GetScnb());
                        topology_file->AddDihedralType(topology_dihedral_type);
                    }
                    is_improper_found = true;
                    break;
                }
            }
        }
        if(!is_improper_found)
        {
            //            std::stringstream ss;
            //            ss << all_improper_dihedrals_atom_type_permutations.at(0).at(0) << "-" << all_improper_dihedrals_atom_type_permutations.at(0).at(1) << "-"
            //               << all_improper_dihedrals_atom_type_permutations.at(0).at(2) << "-" << all_improper_dihedrals_atom_type_permutations.at(0).at(3)
            //               << " improer dihedral type (or any other permutation of it) does not exist in the parameter files";
            //            std::cout << ss.str() << std::endl;
            //            gmml::log(__LINE__, __FILE__, gmml::ERR, ss.str());
        }
    }
}

void Assembly::ExtractTopologyDihedralsFromAssembly(Atom *assembly_atom, Atom *neighbor, Atom *neighbor_of_neighbor, Atom *neighbor_of_neighbor_of_neighbor,
                                                    std::vector<std::vector<std::string> >& inserted_dihedrals, std::vector<std::string>& inserted_dihedral_types,
                                                    ParameterFileSpace::ParameterFile::DihedralMap &dihedrals, TopologyFileSpace::TopologyFile *topology_file)
{
    std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(assembly_atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                      neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
    for(std::vector<std::vector<std::string> >::iterator it = all_atom_type_permutations.begin(); it != all_atom_type_permutations.end(); it++)
    {
        std::vector<std::string> atom_types = (*it);
        std::stringstream sss;
        sss << atom_types.at(0) << "_" << atom_types.at(1) << "_" << atom_types.at(2) << "_" << atom_types.at(3);
        if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str()) != inserted_dihedral_types.end())
        {
            std::vector<std::string> dihedral_atom_names = std::vector<std::string>();
            std::vector<std::string> reverse_dihedral_atom_names = std::vector<std::string>();
            dihedral_atom_names.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
            dihedral_atom_names.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
            dihedral_atom_names.push_back(neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor->GetId(),"_").at(1) + ")");
            dihedral_atom_names.push_back(neighbor_of_neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor_of_neighbor->GetId(),"_").at(1) + ")");
            reverse_dihedral_atom_names.push_back(neighbor_of_neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor_of_neighbor->GetId(),"_").at(1) + ")");
            reverse_dihedral_atom_names.push_back(neighbor_of_neighbor->GetName() + "(" + gmml::Split(neighbor_of_neighbor->GetId(),"_").at(1) + ")");
            reverse_dihedral_atom_names.push_back(neighbor->GetName() + "(" + gmml::Split(neighbor->GetId(),"_").at(1) + ")");
            reverse_dihedral_atom_names.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");

            std::vector<std::string> residue_names = std::vector<std::string>();
            std::vector<std::string> reverse_residue_names = std::vector<std::string>();
            residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
            residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
            residue_names.push_back(neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
            residue_names.push_back(neighbor_of_neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
            reverse_residue_names.push_back(neighbor_of_neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
            reverse_residue_names.push_back(neighbor_of_neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor_of_neighbor->GetResidue()->GetId(),"_").at(2)+")");
            reverse_residue_names.push_back(neighbor->GetResidue()->GetName()+"("+gmml::Split(neighbor->GetResidue()->GetId(),"_").at(2)+")");
            reverse_residue_names.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");

            std::vector<std::string> dihedral = std::vector<std::string>();
            std::vector<std::string> reverse_dihedral = std::vector<std::string>();
            std::stringstream ss;
            ss << residue_names.at(0) << ":" << dihedral_atom_names.at(0);
            std::stringstream ss1;
            ss1 << residue_names.at(1) << ":" << dihedral_atom_names.at(1);
            std::stringstream ss2;
            ss2 << residue_names.at(2) << ":" << dihedral_atom_names.at(2);
            std::stringstream ss3;
            ss3 << residue_names.at(3) << ":" << dihedral_atom_names.at(3);
            dihedral.push_back(ss.str());
            dihedral.push_back(ss1.str());
            dihedral.push_back(ss2.str());
            dihedral.push_back(ss3.str());
            reverse_dihedral.push_back(ss3.str());
            reverse_dihedral.push_back(ss2.str());
            reverse_dihedral.push_back(ss1.str());
            reverse_dihedral.push_back(ss.str());

            if(find(inserted_dihedrals.begin(), inserted_dihedrals.end(), dihedral) == inserted_dihedrals.end() &&
                    find(inserted_dihedrals.begin(), inserted_dihedrals.end(), reverse_dihedral) == inserted_dihedrals.end())
            {
                ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedral = dihedrals[atom_types];
                std::vector<ParameterFileSpace::ParameterFileDihedralTerm> dihedral_terms = parameter_file_dihedral->GetTerms();
                for(std::vector<ParameterFileSpace::ParameterFileDihedralTerm>::iterator it1 = dihedral_terms.begin(); it1 != dihedral_terms.end(); it1++)
                {
                    TopologyFileSpace::TopologyDihedral* topology_dihedral = new TopologyFileSpace::TopologyDihedral();
                    topology_dihedral->SetIsImproper(false);
                    topology_dihedral->SetIgnoredGroupInteraction(false);///not sure
                    if((assembly_atom->GetName().substr(0,1).compare("H") == 0 ||
                        (assembly_atom->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(assembly_atom->GetName().substr(0,1)))))
                            || (neighbor->GetName().substr(0,1).compare("H") == 0 ||
                                (neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor->GetName().substr(0,1)))))
                            ||(neighbor_of_neighbor->GetName().substr(0,1).compare("H") == 0 ||
                               (neighbor_of_neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor->GetName().substr(0,1)))))
                            ||(neighbor_of_neighbor_of_neighbor->GetName().substr(0,1).compare("H") == 0 ||
                               (neighbor_of_neighbor_of_neighbor->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_of_neighbor->GetName().substr(0,1))))))
                        topology_dihedral->SetIncludingHydrogen(true);
                    else
                        topology_dihedral->SetIncludingHydrogen(false);

                    if(     (atom_types.at(0).compare(assembly_atom->GetAtomType()) == 0) ||
                            ((atom_types.at(0).compare("X") == 0) && (atom_types.at(3).compare(neighbor_of_neighbor_of_neighbor->GetAtomType()) == 0)) ||
                            ((atom_types.at(0).compare("X") == 0 && atom_types.at(3).compare("X") == 0) && (atom_types.at(1).compare(neighbor->GetAtomType()) == 0)) ||
                            ((atom_types.at(0).compare("X") == 0 && atom_types.at(3).compare("X") == 0) && (atom_types.at(2).compare(neighbor_of_neighbor->GetAtomType())) == 0) )
                    {
                        topology_dihedral->SetResidueNames(residue_names);
                        topology_dihedral->SetDihedrals(dihedral_atom_names);
                    }
                    else
                    {
                        topology_dihedral->SetResidueNames(reverse_residue_names);
                        topology_dihedral->SetDihedrals(reverse_dihedral_atom_names);
                    }

                    int index = 0;
                    if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str()) != inserted_dihedral_types.end())
                        index = distance(inserted_dihedral_types.begin(), find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str())) +
                                distance(dihedral_terms.begin(), it1);
                    topology_dihedral->SetDihedralType(topology_file->GetDihedralTypeByIndex(index));
                    topology_file->AddDihedral(topology_dihedral);
                }
                if(atom_types.at(0).compare(assembly_atom->GetAtomType()) == 0 ||
                        (atom_types.at(0).compare("X") == 0 && atom_types.at(3).compare(neighbor_of_neighbor_of_neighbor->GetAtomType()) == 0) ||
                        (atom_types.at(0).compare("X") == 0 && atom_types.at(3).compare("X") == 0 && atom_types.at(1).compare(neighbor->GetAtomType()) == 0) ||
                        (atom_types.at(0).compare("X") == 0 && atom_types.at(3).compare("X") == 0 && atom_types.at(2).compare(neighbor_of_neighbor->GetAtomType()) == 0) )
                    inserted_dihedrals.push_back(dihedral);
                else
                    inserted_dihedrals.push_back(reverse_dihedral);

                break;
            }
        }
    }
    /**/
    ///Improper Dihedrals
    AtomNode* atom_node = assembly_atom->GetNode();
    AtomVector neighbors = atom_node->GetNodeNeighbors();
    if(neighbors.size() == 3)
    {
        Atom* neighbor1 = neighbors.at(0);
        Atom* neighbor2 = neighbors.at(1);
        Atom* neighbor3 = neighbors.at(2);
        std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                     neighbor3->GetAtomType(), assembly_atom->GetAtomType());
        for(std::vector<std::vector<std::string> >::iterator it = all_improper_dihedrals_atom_type_permutations.begin(); it != all_improper_dihedrals_atom_type_permutations.end(); it++)
        {
            std::vector<std::string> improper_dihedral_permutation = (*it);
            std::stringstream sss;
            sss << improper_dihedral_permutation.at(0) << "_" << improper_dihedral_permutation.at(1) << "_" << improper_dihedral_permutation.at(2) << "_" << improper_dihedral_permutation.at(3);
            if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str()) != inserted_dihedral_types.end())
            {
                std::vector<std::string> dihedral_atom_names1 = std::vector<std::string>();
                dihedral_atom_names1.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");
                dihedral_atom_names1.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");
                dihedral_atom_names1.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                dihedral_atom_names1.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                //                std::vector<std::string> dihedral_atom_names2 = std::vector<std::string>();
                //                dihedral_atom_names2.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");
                //                dihedral_atom_names2.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                //                dihedral_atom_names2.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                //                dihedral_atom_names2.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");
                std::vector<std::string> dihedral_atom_names3 = std::vector<std::string>();
                dihedral_atom_names3.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");
                dihedral_atom_names3.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                dihedral_atom_names3.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                dihedral_atom_names3.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");

                //                std::vector<std::string> reverse_dihedral_atom_names1 = std::vector<std::string>();
                //                reverse_dihedral_atom_names1.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names1.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names1.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names1.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");
                std::vector<std::string> reverse_dihedral_atom_names2 = std::vector<std::string>();
                reverse_dihedral_atom_names2.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");
                reverse_dihedral_atom_names2.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                reverse_dihedral_atom_names2.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                reverse_dihedral_atom_names2.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");
                //                std::vector<std::string> reverse_dihedral_atom_names3 = std::vector<std::string>();
                //                reverse_dihedral_atom_names3.push_back(neighbor2->GetName() + "(" + gmml::Split(neighbor2->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names3.push_back(assembly_atom->GetName() + "(" + gmml::Split(assembly_atom->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names3.push_back(neighbor3->GetName() + "(" + gmml::Split(neighbor3->GetId(),"_").at(1) + ")");
                //                reverse_dihedral_atom_names3.push_back(neighbor1->GetName() + "(" + gmml::Split(neighbor1->GetId(),"_").at(1) + ")");

                std::vector<std::string> residue_names1 = std::vector<std::string>();
                residue_names1.push_back(neighbor1->GetResidue()->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");
                residue_names1.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");
                residue_names1.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                residue_names1.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                //                std::vector<std::string> residue_names2 = std::vector<std::string>();
                //                residue_names2.push_back(neighbor1->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");
                //                residue_names2.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                //                residue_names2.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                //                residue_names2.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");
                std::vector<std::string> residue_names3 = std::vector<std::string>();
                residue_names3.push_back(neighbor1->GetResidue()->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");
                residue_names3.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                residue_names3.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                residue_names3.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");

                //                std::vector<std::string> reverse_residue_names1 = std::vector<std::string>();
                //                reverse_residue_names1.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names1.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names1.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names1.push_back(neighbor1->GetResidue()->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");
                std::vector<std::string> reverse_residue_names2 = std::vector<std::string>();
                reverse_residue_names2.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");
                reverse_residue_names2.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                reverse_residue_names2.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                reverse_residue_names2.push_back(neighbor1->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");
                //                std::vector<std::string> reverse_residue_names3 = std::vector<std::string>();
                //                reverse_residue_names3.push_back(neighbor2->GetResidue()->GetName()+"("+gmml::Split(neighbor2->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names3.push_back(assembly_atom->GetResidue()->GetName()+"("+gmml::Split(assembly_atom->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names3.push_back(neighbor3->GetResidue()->GetName()+"("+gmml::Split(neighbor3->GetResidue()->GetId(),"_").at(2)+")");
                //                reverse_residue_names3.push_back(neighbor1->GetResidue()->GetName()+"("+gmml::Split(neighbor1->GetResidue()->GetId(),"_").at(2)+")");

                std::vector<std::string> dihedral1 = std::vector<std::string>();
                std::vector<std::string> dihedral2 = std::vector<std::string>();
                std::vector<std::string> dihedral3 = std::vector<std::string>();
                std::vector<std::string> reverse_dihedral1 = std::vector<std::string>();
                std::vector<std::string> reverse_dihedral2 = std::vector<std::string>();
                std::vector<std::string> reverse_dihedral3 = std::vector<std::string>();
                std::stringstream ss;
                ss << residue_names1.at(2) << ":" << dihedral_atom_names1.at(2);
                std::stringstream ss1;
                ss1 << residue_names1.at(0) << ":" << dihedral_atom_names1.at(0);
                std::stringstream ss2;
                ss2 << residue_names1.at(1) << ":" << dihedral_atom_names1.at(1);
                std::stringstream ss3;
                ss3 << residue_names1.at(3) << ":" << dihedral_atom_names1.at(3);

                dihedral1.push_back(ss1.str());
                dihedral1.push_back(ss2.str());
                dihedral1.push_back(ss.str());
                dihedral1.push_back(ss3.str());
                reverse_dihedral1.push_back(ss3.str());
                reverse_dihedral1.push_back(ss.str());
                reverse_dihedral1.push_back(ss2.str());
                reverse_dihedral1.push_back(ss1.str());

                dihedral2.push_back(ss1.str());
                dihedral2.push_back(ss.str());
                dihedral2.push_back(ss3.str());
                dihedral2.push_back(ss2.str());
                reverse_dihedral2.push_back(ss2.str());
                reverse_dihedral2.push_back(ss3.str());
                reverse_dihedral2.push_back(ss.str());
                reverse_dihedral2.push_back(ss1.str());

                dihedral3.push_back(ss1.str());
                dihedral3.push_back(ss3.str());
                dihedral3.push_back(ss.str());
                dihedral3.push_back(ss2.str());
                reverse_dihedral3.push_back(ss2.str());
                reverse_dihedral3.push_back(ss.str());
                reverse_dihedral3.push_back(ss3.str());
                reverse_dihedral3.push_back(ss1.str());
                if(find(inserted_dihedrals.begin(), inserted_dihedrals.end(), dihedral1) == inserted_dihedrals.end() &&
                        find(inserted_dihedrals.begin(), inserted_dihedrals.end(), dihedral2) == inserted_dihedrals.end() &&
                        find(inserted_dihedrals.begin(), inserted_dihedrals.end(), dihedral3) == inserted_dihedrals.end() &&
                        find(inserted_dihedrals.begin(), inserted_dihedrals.end(), reverse_dihedral1) == inserted_dihedrals.end() &&
                        find(inserted_dihedrals.begin(), inserted_dihedrals.end(), reverse_dihedral2) == inserted_dihedrals.end() &&
                        find(inserted_dihedrals.begin(), inserted_dihedrals.end(), reverse_dihedral3) == inserted_dihedrals.end())
                {
                    int permutation_index = distance(all_improper_dihedrals_atom_type_permutations.begin(), it);
                    ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedral = dihedrals[improper_dihedral_permutation];
                    std::vector<ParameterFileSpace::ParameterFileDihedralTerm> dihedral_terms = parameter_file_dihedral->GetTerms();
                    for(std::vector<ParameterFileSpace::ParameterFileDihedralTerm>::iterator it1 = dihedral_terms.begin(); it1 != dihedral_terms.end(); it1++)
                    {
                        TopologyFileSpace::TopologyDihedral* topology_dihedral = new TopologyFileSpace::TopologyDihedral();
                        topology_dihedral->SetIsImproper(true);
                        topology_dihedral->SetIgnoredGroupInteraction(false);///not sure
                        if((assembly_atom->GetName().substr(0,1).compare("H") == 0 ||
                            (assembly_atom->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(assembly_atom->GetName().substr(0,1)))))
                                || (neighbor1->GetName().substr(0,1).compare("H") == 0 ||
                                    (neighbor1->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor1->GetName().substr(0,1)))))
                                ||(neighbor2->GetName().substr(0,1).compare("H") == 0 ||
                                   (neighbor2->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor2->GetName().substr(0,1)))))
                                ||(neighbor3->GetName().substr(0,1).compare("H") == 0 ||
                                   (neighbor3->GetName().substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor3->GetName().substr(0,1))))))
                            topology_dihedral->SetIncludingHydrogen(true);
                        else
                            topology_dihedral->SetIncludingHydrogen(false);
                        if(permutation_index % 6 == 0 || permutation_index % 6 == 2)
                        {
                            topology_dihedral->SetResidueNames(residue_names1);
                            topology_dihedral->SetDihedrals(dihedral_atom_names1);
                        }
                        //                        if(permutation_index % 6 == 2)
                        //                        {
                        //                            topology_dihedral->SetResidueNames(residue_names2);
                        //                            topology_dihedral->SetDihedrals(dihedral_atom_names2);
                        //                        }
                        if(permutation_index % 6 == 4 || permutation_index % 6 == 1)
                        {
                            topology_dihedral->SetResidueNames(residue_names3);
                            topology_dihedral->SetDihedrals(dihedral_atom_names3);
                        }
                        //                        if(permutation_index % 6 == 1)
                        //                        {
                        //                            topology_dihedral->SetResidueNames(reverse_residue_names1);
                        //                            topology_dihedral->SetDihedrals(reverse_dihedral_atom_names1);
                        //                        }
                        if(permutation_index % 6 == 3 || permutation_index % 6 == 5)
                        {
                            topology_dihedral->SetResidueNames(reverse_residue_names2);
                            topology_dihedral->SetDihedrals(reverse_dihedral_atom_names2);
                        }
                        //                        if(permutation_index % 6 == 5)
                        //                        {
                        //                            topology_dihedral->SetResidueNames(reverse_residue_names3);
                        //                            topology_dihedral->SetDihedrals(reverse_dihedral_atom_names3);
                        //                        }

                        int index = 0;
                        if(find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str()) != inserted_dihedral_types.end())
                            index = distance(inserted_dihedral_types.begin(), find(inserted_dihedral_types.begin(), inserted_dihedral_types.end(), sss.str())) +
                                    distance(dihedral_terms.begin(), it1);
                        topology_dihedral->SetDihedralType(topology_file->GetDihedralTypeByIndex(index));
                        topology_file->AddDihedral(topology_dihedral);
                    }
                    if(permutation_index % 6 == 0)
                    {
                        inserted_dihedrals.push_back(dihedral1);
                    }
                    if(permutation_index % 6 == 2)
                    {
                        inserted_dihedrals.push_back(dihedral2);
                    }
                    if(permutation_index % 6 == 4)
                    {
                        inserted_dihedrals.push_back(dihedral3);
                    }
                    if(permutation_index % 6 == 1)
                    {
                        inserted_dihedrals.push_back(reverse_dihedral1);
                    }
                    if(permutation_index % 6 == 3)
                    {
                        inserted_dihedrals.push_back(reverse_dihedral2);
                    }
                    if(permutation_index % 6 == 5)
                    {
                        inserted_dihedrals.push_back(reverse_dihedral3);
                    }
                    break;
                }
            }
        }
    }
    /**/
}

int Assembly::CountNumberOfAtoms()
{
    int counter = 0;
    for(AssemblyVector::iterator it = assemblies_.begin(); it != assemblies_.end(); it++)
    {
        Assembly* assembly = (*it);
        counter += assembly->CountNumberOfAtoms();
    }
    for(ResidueVector:: iterator it1 = residues_.begin(); it1 != residues_.end(); it1++)
    {
        Residue* residue = (*it1);
        MolecularModeling::AtomVector atoms = residue->GetAtoms();
        counter += atoms.size();
    }
    return counter;
}

int Assembly::CountNumberOfAtomTypes()
{
    std::vector<std::string> type_list = std::vector<std::string>();
    AtomVector atoms = GetAllAtomsOfAssembly();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::string atom_type = atom->GetAtomType();
        if(find(type_list.begin(), type_list.end(), atom_type) != type_list.end())
        {}
        else
        {
            type_list.push_back(atom_type);
        }
    }
    return type_list.size();
}

int Assembly::CountNumberOfResidues()
{
    int counter = 0;
    for(AssemblyVector::iterator it = assemblies_.begin(); it != assemblies_.end(); it++)
    {
        Assembly* assembly = (*it);
        counter += assembly->CountNumberOfResidues();
    }
    counter += residues_.size();
    return counter;
}

int Assembly::CountNumberOfBondsIncludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::BondMap bonds = parameter_file->GetBonds();
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector node_neighbors = atom_node->GetNodeNeighbors();
            //            if((atom_name.substr(0,1).compare("H") == 0 ||
            //                (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))))
            //            {
            //                counter += node_neighbors.size();
            //            }
            //            else
            //            {
            for(AtomVector::iterator it1 = node_neighbors.begin(); it1 != node_neighbors.end(); it1++)
            {
                Atom* node_neighbor = (*it1);
                std::string node_neighbor_name = node_neighbor->GetName();
                if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                        (node_neighbor_name.substr(0,1).compare("H") == 0 || (node_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(node_neighbor_name.substr(0,1))))))
                {
                    std::vector<std::string> atom_pair_type = std::vector<std::string>();
                    std::vector<std::string> reverse_atom_pair_type = std::vector<std::string>();
                    atom_pair_type.push_back(atom->GetAtomType());
                    atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(atom->GetAtomType());

                    if(bonds.find(atom_pair_type) != bonds.end() || bonds.find(reverse_atom_pair_type) != bonds.end())
                        counter++;
                }
            }
            //            }
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfBondsExcludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::BondMap bonds = parameter_file->GetBonds();
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector node_neighbors = atom_node->GetNodeNeighbors();

            //            if((atom_name.substr(0,1).compare("H") == 0 ||
            //                (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))))
            //            {}
            //            else
            //            {
            for(AtomVector::iterator it1 = node_neighbors.begin(); it1 != node_neighbors.end(); it1++)
            {
                Atom* node_neighbor = (*it1);
                std::string node_neighbor_name = node_neighbor->GetName();
                if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                        (node_neighbor_name.substr(0,1).compare("H") == 0 || (node_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(node_neighbor_name.substr(0,1))))))
                {}
                else
                {
                    std::vector<std::string> atom_pair_type = std::vector<std::string>();
                    std::vector<std::string> reverse_atom_pair_type = std::vector<std::string>();
                    atom_pair_type.push_back(atom->GetAtomType());
                    atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(atom->GetAtomType());

                    if(bonds.find(atom_pair_type) != bonds.end() || bonds.find(reverse_atom_pair_type) != bonds.end())
                        counter++;
                }
            }
            //            }
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfBonds()
{
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector node_neighbors = atom_node->GetNodeNeighbors();
            counter += node_neighbors.size();
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfBondTypes(std::string parameter_file_path)
{
    AtomVector atoms = GetAllAtomsOfAssembly();
    std::vector<std::string> type_list = std::vector<std::string>();
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::BondMap bonds = parameter_file->GetBonds();

    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::string atom_bond_type = atom->GetAtomType();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector node_neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = node_neighbors.begin(); it1 != node_neighbors.end(); it1++)
            {
                Atom* node_neighbor = (*it1);
                std::string node_neighbor_bond_type = node_neighbor->GetAtomType();
                std::stringstream ss;
                ss << atom_bond_type << "_" << node_neighbor_bond_type;
                std::string key = ss.str();
                std::stringstream ss1;
                ss1 << node_neighbor_bond_type << "_" << atom_bond_type;
                std::string key1 = ss1.str();
                if(find(type_list.begin(), type_list.end(), key ) != type_list.end() ||
                        find(type_list.begin(), type_list.end(), key1 ) != type_list.end())
                {}
                else
                {
                    std::vector<std::string> atom_pair_type = std::vector<std::string>();
                    std::vector<std::string> reverse_atom_pair_type = std::vector<std::string>();
                    atom_pair_type.push_back(atom->GetAtomType());
                    atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(node_neighbor->GetAtomType());
                    reverse_atom_pair_type.push_back(atom->GetAtomType());

                    if(bonds.find(atom_pair_type) != bonds.end() || bonds.find(reverse_atom_pair_type) != bonds.end())
                        type_list.push_back(key);
                }
            }
        }
    }
    return type_list.size();
}

int Assembly::CountNumberOfAnglesIncludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::AngleMap angles = parameter_file->GetAngles();
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::string neighbor_name = neighbor->GetName();
                AtomNode* neighbor_atom_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_atom_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss1;
                    ss1 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss1.str()) != 0)
                    {
                        std::string neighbor_of_neighbor_name = neighbor_of_neighbor->GetName();
                        if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                                (neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_name.substr(0,1))))) ||
                                (neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_name.substr(0,1))))))
                        {
                            std::vector<std::string> angle_type = std::vector<std::string>();
                            std::vector<std::string> reverse_angle_type = std::vector<std::string>();
                            angle_type.push_back(atom->GetAtomType());
                            angle_type.push_back(neighbor->GetAtomType());
                            angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor->GetAtomType());
                            reverse_angle_type.push_back(atom->GetAtomType());
                            if(angles.find(angle_type) != angles.end() || angles.find(reverse_angle_type) != angles.end())
                                counter++;
                        }
                    }
                }
            }
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfAnglesExcludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::AngleMap angles = parameter_file->GetAngles();
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::string neighbor_name = neighbor->GetName();
                AtomNode* neighbor_atom_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_atom_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss1;
                    ss1 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss1.str()) != 0)
                    {
                        std::string neighbor_of_neighbor_name = neighbor_of_neighbor->GetName();
                        if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                                (neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_name.substr(0,1))))) ||
                                (neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_name.substr(0,1))))))
                        {}
                        else
                        {
                            std::vector<std::string> angle_type = std::vector<std::string>();
                            std::vector<std::string> reverse_angle_type = std::vector<std::string>();
                            angle_type.push_back(atom->GetAtomType());
                            angle_type.push_back(neighbor->GetAtomType());
                            angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor->GetAtomType());
                            reverse_angle_type.push_back(atom->GetAtomType());
                            if(angles.find(angle_type) != angles.end() || angles.find(reverse_angle_type) != angles.end())
                                counter++;
                        }
                    }
                }
            }
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfAngles()
{
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                AtomNode* neighbor_atom_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_atom_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss1;
                    ss1 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss1.str()) != 0)
                    {
                        counter++;
                    }
                }
            }
        }
    }
    return counter/2;
}

int Assembly::CountNumberOfAngleTypes(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    ParameterFileSpace::ParameterFile::AngleMap angles = parameter_file->GetAngles();
    AtomVector atoms = GetAllAtomsOfAssembly();

    std::vector<std::string> type_list = std::vector<std::string>();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::string type1 = atom->GetAtomType();
        std::stringstream ss;
        ss << atom->GetId();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::string type2 = neighbor->GetAtomType();
                AtomNode* neighbor_atom_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_atom_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::string type3 = neighbor_of_neighbor->GetAtomType();
                    std::stringstream ss1;
                    ss1 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss1.str()) != 0)
                    {
                        std::stringstream ss2;
                        ss2 << type1 << "_" << type2 << "_" << type3;
                        std::stringstream ss3;
                        ss3 << type3 << "_" << type2 << "_" << type1;
                        if(find(type_list.begin(), type_list.end(), ss2.str()) == type_list.end() &&
                                find(type_list.begin(), type_list.end(), ss3.str()) == type_list.end())
                        {
                            std::vector<std::string> angle_type = std::vector<std::string>();
                            std::vector<std::string> reverse_angle_type = std::vector<std::string>();
                            angle_type.push_back(atom->GetAtomType());
                            angle_type.push_back(neighbor->GetAtomType());
                            angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor_of_neighbor->GetAtomType());
                            reverse_angle_type.push_back(neighbor->GetAtomType());
                            reverse_angle_type.push_back(atom->GetAtomType());
                            if(angles.find(angle_type) != angles.end() || angles.find(reverse_angle_type) != angles.end())
                            {
                                type_list.push_back(ss2.str());
                            }
                        }
                    }
                }
            }
        }
    }
    return type_list.size();
}

int Assembly::CountNumberOfDihedralsIncludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    int improper_counter = 0;
    //    int not_found_counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::stringstream ss1;
                ss1 << neighbor->GetId();
                std::string neighbor_name = neighbor->GetName();
                AtomNode* neighbor_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss2;
                    ss2 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss2.str()) != 0)
                    {
                        std::string neighbor_of_neighbor_name = neighbor_of_neighbor->GetName();
                        AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                        AtomVector neighbors_of_neighbor_of_neighbor = neighbor_of_neighbor_node->GetNodeNeighbors();
                        for(AtomVector::iterator it3 = neighbors_of_neighbor_of_neighbor.begin(); it3 != neighbors_of_neighbor_of_neighbor.end(); it3++)
                        {
                            Atom* neighbor_of_neighbor_of_neighbor = (*it3);
                            std::stringstream ss3;
                            ss3 << neighbor_of_neighbor_of_neighbor->GetId();
                            if(ss1.str().compare(ss3.str()) != 0)
                            {
                                std::string neighbor_of_neighbor_of_neighbor_name = neighbor_of_neighbor_of_neighbor->GetName();
                                if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                                        (neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_name.substr(0,1))))) ||
                                        (neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_name.substr(0,1))))) ||
                                        (neighbor_of_neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_of_neighbor_name.substr(0,1))))))
                                {

                                    std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                                                      neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
                                    ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                                    for(std::vector<std::vector<std::string> >::iterator it4 = all_atom_type_permutations.begin(); it4 != all_atom_type_permutations.end(); it4++)
                                    {
                                        std::vector<std::string> atom_types = (*it4);
                                        if(dihedrals[atom_types] != NULL)
                                        {
                                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[atom_types];
                                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                                            counter += terms_count;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ///Improper Dihedrals
            if(neighbors.size() == 3)
            {
                Atom* neighbor1 = neighbors.at(0);
                Atom* neighbor2 = neighbors.at(1);
                Atom* neighbor3 = neighbors.at(2);
                std::string neighbor1_name = neighbor1->GetName();
                std::string neighbor2_name = neighbor2->GetName();
                std::string neighbor3_name = neighbor3->GetName();
                std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                             neighbor3->GetAtomType(), atom->GetAtomType());

                ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                for(std::vector<std::vector<std::string> >::iterator it1 = all_improper_dihedrals_atom_type_permutations.begin(); it1 != all_improper_dihedrals_atom_type_permutations.end(); it1++)
                {
                    std::vector<std::string> improper_dihedral_permutation = (*it1);
                    if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                            (neighbor1_name.substr(0,1).compare("H") == 0 || (neighbor1_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor1_name.substr(0,1))))) ||
                            (neighbor2_name.substr(0,1).compare("H") == 0 || (neighbor2_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor2_name.substr(0,1))))) ||
                            (neighbor3_name.substr(0,1).compare("H") == 0 || (neighbor3_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor3_name.substr(0,1))))))
                    {
                        if(dihedrals[improper_dihedral_permutation] != NULL)
                        {
                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[improper_dihedral_permutation];
                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                            improper_counter += terms_count;
                            break;
                        }
                    }
                }
            }
        }
    }
    //    std::cout << not_found_counter/2 << " dihedrals not found in parameter file" << std::endl;
    return counter/2 + improper_counter;
}

int Assembly::CountNumberOfDihedralsExcludingHydrogen(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    int improper_counter = 0;
    //    int not_found_counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        std::string atom_name = atom->GetName();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::stringstream ss1;
                ss1 << neighbor->GetId();
                std::string neighbor_name = neighbor->GetName();
                AtomNode* neighbor_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss2;
                    ss2 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss2.str()) != 0)
                    {
                        std::string neighbor_of_neighbor_name = neighbor_of_neighbor->GetName();
                        AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                        AtomVector neighbors_of_neighbor_of_neighbor = neighbor_of_neighbor_node->GetNodeNeighbors();
                        for(AtomVector::iterator it3 = neighbors_of_neighbor_of_neighbor.begin(); it3 != neighbors_of_neighbor_of_neighbor.end(); it3++)
                        {
                            Atom* neighbor_of_neighbor_of_neighbor = (*it3);
                            std::stringstream ss3;
                            ss3 << neighbor_of_neighbor_of_neighbor->GetId();
                            if(ss1.str().compare(ss3.str()) != 0)
                            {


                                std::string neighbor_of_neighbor_of_neighbor_name = neighbor_of_neighbor_of_neighbor->GetName();
                                if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                                        (neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_name.substr(0,1))))) ||
                                        (neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_name.substr(0,1))))) ||
                                        (neighbor_of_neighbor_of_neighbor_name.substr(0,1).compare("H") == 0 || (neighbor_of_neighbor_of_neighbor_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor_of_neighbor_of_neighbor_name.substr(0,1))))))
                                {}
                                else
                                {

                                    std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                                                      neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
                                    ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                                    for(std::vector<std::vector<std::string> >::iterator it4 = all_atom_type_permutations.begin(); it4 != all_atom_type_permutations.end(); it4++)
                                    {
                                        std::vector<std::string> atom_types = (*it4);
                                        if(dihedrals[atom_types] != NULL)
                                        {
                                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[atom_types];
                                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                                            counter += terms_count;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ///Improper Dihedrals
            if(neighbors.size() == 3)
            {
                Atom* neighbor1 = neighbors.at(0);
                Atom* neighbor2 = neighbors.at(1);
                Atom* neighbor3 = neighbors.at(2);
                std::string neighbor1_name = neighbor1->GetName();
                std::string neighbor2_name = neighbor2->GetName();
                std::string neighbor3_name = neighbor3->GetName();
                std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                             neighbor3->GetAtomType(), atom->GetAtomType());

                ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                for(std::vector<std::vector<std::string> >::iterator it1 = all_improper_dihedrals_atom_type_permutations.begin(); it1 != all_improper_dihedrals_atom_type_permutations.end(); it1++)
                {
                    std::vector<std::string> improper_dihedral_permutation = (*it1);
                    if((atom_name.substr(0,1).compare("H") == 0 || (atom_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(atom_name.substr(0,1))))) ||
                            (neighbor1_name.substr(0,1).compare("H") == 0 || (neighbor1_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor1_name.substr(0,1))))) ||
                            (neighbor2_name.substr(0,1).compare("H") == 0 || (neighbor2_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor2_name.substr(0,1))))) ||
                            (neighbor3_name.substr(0,1).compare("H") == 0 || (neighbor3_name.substr(1,1).compare("H") == 0 && isdigit(gmml::ConvertString<char>(neighbor3_name.substr(0,1))))))
                    {}
                    else
                    {
                        if(dihedrals[improper_dihedral_permutation] != NULL)
                        {
                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[improper_dihedral_permutation];
                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                            improper_counter += terms_count;
                            break;
                        }
                    }
                }
            }
        }
    }
    //    std::cout << not_found_counter/2 << " dihedrals not found in parameter file" << std::endl;
    return counter/2 + improper_counter;
}

int Assembly::CountNumberOfDihedrals(std::string parameter_file_path)
{
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    int improper_counter = 0;
    //    int not_found_counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::stringstream ss1;
                ss1 << neighbor->GetId();
                AtomNode* neighbor_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss2;
                    ss2 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss2.str()) != 0)
                    {
                        AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                        AtomVector neighbors_of_neighbor_of_neighbor = neighbor_of_neighbor_node->GetNodeNeighbors();
                        for(AtomVector::iterator it3 = neighbors_of_neighbor_of_neighbor.begin(); it3 != neighbors_of_neighbor_of_neighbor.end(); it3++)
                        {
                            Atom* neighbor_of_neighbor_of_neighbor = (*it3);
                            std::stringstream ss3;
                            ss3 << neighbor_of_neighbor_of_neighbor->GetId();
                            if(ss1.str().compare(ss3.str()) != 0)
                            {
                                std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                                                  neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
                                ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                                for(std::vector<std::vector<std::string> >::iterator it4 = all_atom_type_permutations.begin(); it4 != all_atom_type_permutations.end(); it4++)
                                {
                                    std::vector<std::string> atom_types = (*it4);
                                    //                                std::cout << atom_types.at(0) << atom_types.at(1) << atom_types.at(2) << atom_types.at(3) << std::endl;
                                    if(dihedrals[atom_types] != NULL)
                                    {
                                        ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[atom_types];
                                        int terms_count = parameter_file_dihedrals->GetTerms().size();
                                        counter += terms_count;
                                        break;
                                    }

                                }
                            }
                        }
                    }
                }
            }
            ///Improper Dihedrals
            if(neighbors.size() == 3)
            {

                Atom* neighbor1 = neighbors.at(0);
                Atom* neighbor2 = neighbors.at(1);
                Atom* neighbor3 = neighbors.at(2);

                std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                             neighbor3->GetAtomType(), atom->GetAtomType());
                ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                for(std::vector<std::vector<std::string> >::iterator it1 = all_improper_dihedrals_atom_type_permutations.begin(); it1 != all_improper_dihedrals_atom_type_permutations.end(); it1++)
                {
                    std::vector<std::string> improper_dihedral_permutation = (*it1);
                    if(dihedrals[improper_dihedral_permutation] != NULL)
                    {
                        ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[improper_dihedral_permutation];
                        int terms_count = parameter_file_dihedrals->GetTerms().size();
                        improper_counter += terms_count;
                        break;
                    }
                }
            }
        }
    }
    //    std::cout << not_found_counter/2 << " dihedrals not found in parameter file" << std::endl;
    return counter/2 + improper_counter;
}

int Assembly::CountNumberOfDihedralTypes(std::string parameter_file_path)
{
    std::vector<std::string> type_list = std::vector<std::string>();
    ParameterFileSpace::ParameterFile* parameter_file = new ParameterFileSpace::ParameterFile(parameter_file_path);
    AtomVector atoms = GetAllAtomsOfAssembly();
    int counter = 0;
    //    int not_found_counter = 0;
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            AtomVector neighbors = atom_node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::stringstream ss1;
                ss1 << neighbor->GetId();
                AtomNode* neighbor_node = neighbor->GetNode();
                AtomVector neighbors_of_neighbor = neighbor_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbors_of_neighbor.begin(); it2 != neighbors_of_neighbor.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss2;
                    ss2 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss2.str()) != 0)
                    {
                        AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                        AtomVector neighbors_of_neighbor_of_neighbor = neighbor_of_neighbor_node->GetNodeNeighbors();
                        for(AtomVector::iterator it3 = neighbors_of_neighbor_of_neighbor.begin(); it3 != neighbors_of_neighbor_of_neighbor.end(); it3++)
                        {
                            Atom* neighbor_of_neighbor_of_neighbor = (*it3);
                            std::stringstream ss3;
                            ss3 << neighbor_of_neighbor_of_neighbor->GetId();
                            if(ss1.str().compare(ss3.str()) != 0)
                            {

                                std::vector<std::vector<std::string> > all_atom_type_permutations = CreateAllAtomTypePermutationsforDihedralType(atom->GetAtomType(), neighbor->GetAtomType(),
                                                                                                                                  neighbor_of_neighbor->GetAtomType(), neighbor_of_neighbor_of_neighbor->GetAtomType());
                                ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                                for(std::vector<std::vector<std::string> >::iterator it4 = all_atom_type_permutations.begin(); it4 != all_atom_type_permutations.end(); it4++)
                                {
                                    std::vector<std::string> atom_types = (*it4);


                                    //                            std::vector<std::string> atom_types = std::vector<std::string>();
                                    //                            atom_types.push_back(atom->GetAtomType());
                                    //                            atom_types.push_back(neighbor->GetAtomType());
                                    //                            atom_types.push_back(neighbor_of_neighbor->GetAtomType());
                                    //                            atom_types.push_back(neighbor_of_neighbor_of_neighbor->GetAtomType());
                                    if(dihedrals[atom_types] != NULL)
                                    {
                                        std::stringstream ss4;
                                        ss4 << atom_types.at(0) << "_" << atom_types.at(1) << "_" << atom_types.at(2) << "_" << atom_types.at(3);
                                        std::stringstream ss5;
                                        ss5 << atom_types.at(3) << "_" << atom_types.at(2) << "_" << atom_types.at(1) << "_" << atom_types.at(0);
                                        if(find(type_list.begin(), type_list.end(), ss4.str()) == type_list.end() &&
                                                find(type_list.begin(), type_list.end(), ss5.str()) == type_list.end())
                                        {
                                            type_list.push_back(ss4.str());
                                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[atom_types];
                                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                                            counter += terms_count;
                                            break;
                                        }
                                    }
                                    //                                else
                                    //                                {
                                    //                                    atom_types[0] = neighbor_of_neighbor_of_neighbor->GetAtomType();
                                    //                                    atom_types[1] = neighbor_of_neighbor->GetAtomType();
                                    //                                    atom_types[2] = neighbor->GetAtomType();
                                    //                                    atom_types[3] = atom->GetAtomType();
                                    //                                    if(dihedrals[atom_types] != NULL)
                                    //                                    {
                                    //                                        std::stringstream ss6;
                                    //                                        ss6 << atom->GetAtomType() << "_" << neighbor->GetAtomType() << "_" << neighbor_of_neighbor->GetAtomType() << "_" << neighbor_of_neighbor_of_neighbor->GetAtomType();
                                    //                                        std::stringstream ss7;
                                    //                                        ss7 << neighbor_of_neighbor_of_neighbor->GetAtomType() << "_" << neighbor_of_neighbor->GetAtomType() << "_" << neighbor->GetAtomType() << "_" << atom->GetAtomType();
                                    //                                        if(find(type_list.begin(), type_list.end(), ss6.str()) == type_list.end() &&
                                    //                                                find(type_list.begin(), type_list.end(), ss7.str()) == type_list.end() )
                                    //                                        {
                                    //                                            type_list.push_back(ss7.str());
                                    //                                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[atom_types];
                                    //                                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                                    //                                            counter += terms_count;
                                    //                                        }
                                    //                                    }
                                    //                                    else
                                    //                                    {
                                    //                                        not_found_counter++;
                                    //                                    }
                                    //                                }
                                }
                            }
                        }
                    }
                }
            }
            ///Improper Dihedrals
            if(neighbors.size() == 3)
            {
                Atom* neighbor1 = neighbors.at(0);
                Atom* neighbor2 = neighbors.at(1);
                Atom* neighbor3 = neighbors.at(2);
                std::vector<std::vector<std::string> > all_improper_dihedrals_atom_type_permutations = CreateAllAtomTypePermutationsforImproperDihedralType(neighbor1->GetAtomType(), neighbor2->GetAtomType(),
                                                                                                                                             neighbor3->GetAtomType(), atom->GetAtomType());

                for(std::vector<std::vector<std::string> >::iterator it1 = all_improper_dihedrals_atom_type_permutations.begin(); it1 != all_improper_dihedrals_atom_type_permutations.end(); it1++)
                {
                    std::vector<std::string> improper_dihedral_permutation = (*it1);
                    ParameterFileSpace::ParameterFile::DihedralMap dihedrals = parameter_file->GetDihedrals();
                    if(dihedrals[improper_dihedral_permutation] != NULL)
                    {
                        std::stringstream ss;
                        ss << improper_dihedral_permutation.at(0) << "_" << improper_dihedral_permutation.at(1) << "_" << improper_dihedral_permutation.at(2) << "_" << improper_dihedral_permutation.at(3);
                        std::stringstream ss1;
                        ss1 << improper_dihedral_permutation.at(3) << "_" << improper_dihedral_permutation.at(2) << "_" << improper_dihedral_permutation.at(1) << "_" << improper_dihedral_permutation.at(0);
                        if(find(type_list.begin(), type_list.end(), ss.str()) == type_list.end() &&
                                find(type_list.begin(), type_list.end(), ss1.str()) == type_list.end())
                        {
                            type_list.push_back(ss.str());
                            ParameterFileSpace::ParameterFileDihedral* parameter_file_dihedrals = dihedrals[improper_dihedral_permutation];
                            int terms_count = parameter_file_dihedrals->GetTerms().size();
                            counter += terms_count;
                            break;
                        }
                    }
                }
            }
        }
    }
    //    std::cout << not_found_counter << " dihedrals not found in parameter file" << std::endl;
    //    std::cout << type_list.size() << std::endl;
    return counter;
}

std::vector<std::vector<std::string> > Assembly::CreateAllAtomTypePermutationsforDihedralType(std::string atom_type1, std::string atom_type2, std::string atom_type3, std::string atom_type4)
{
    std::vector<std::vector<std::string> > all_permutations = std::vector<std::vector<std::string> >();
    std::vector<std::string> normal_order = std::vector<std::string>();
    std::vector<std::string> reverse_order = std::vector<std::string>();
    normal_order.push_back(atom_type1);
    normal_order.push_back(atom_type2);
    normal_order.push_back(atom_type3);
    normal_order.push_back(atom_type4);
    reverse_order.push_back(atom_type4);
    reverse_order.push_back(atom_type3);
    reverse_order.push_back(atom_type2);
    reverse_order.push_back(atom_type1);
    all_permutations.push_back(normal_order);
    all_permutations.push_back(reverse_order);

    ///Permutations w/ one X
    for(int i = 0; i < 4; i++)
    {
        normal_order.clear();
        normal_order.push_back(atom_type1);
        normal_order.push_back(atom_type2);
        normal_order.push_back(atom_type3);
        normal_order.push_back(atom_type4);
        normal_order.at(i) = "X";
        reverse_order.clear();
        reverse_order.push_back(atom_type4);
        reverse_order.push_back(atom_type3);
        reverse_order.push_back(atom_type2);
        reverse_order.push_back(atom_type1);
        reverse_order.at(i) = "X";
        all_permutations.push_back(normal_order);
        all_permutations.push_back(reverse_order);
    }
    ///Permutations w/ two X
    for(int i = 0; i < 3; i++)
        for(int j = i+1; j < 4; j++)
        {
            normal_order.clear();
            normal_order.push_back(atom_type1);
            normal_order.push_back(atom_type2);
            normal_order.push_back(atom_type3);
            normal_order.push_back(atom_type4);
            normal_order.at(i) = "X";
            normal_order.at(j) = "X";
            reverse_order.clear();
            reverse_order.push_back(atom_type4);
            reverse_order.push_back(atom_type3);
            reverse_order.push_back(atom_type2);
            reverse_order.push_back(atom_type1);
            reverse_order.at(i) = "X";
            reverse_order.at(j) = "X";
            all_permutations.push_back(normal_order);
            all_permutations.push_back(reverse_order);
        }
    ///Permutations w/ three X
    for(int i = 0; i < 2; i++)
        for(int j = i+1; j < 3; j++)
            for(int k = j+1; k < 4; k++)
            {
                normal_order.clear();
                normal_order.push_back(atom_type1);
                normal_order.push_back(atom_type2);
                normal_order.push_back(atom_type3);
                normal_order.push_back(atom_type4);
                normal_order.at(i) = "X";
                normal_order.at(j) = "X";
                normal_order.at(k) = "X";
                reverse_order.clear();
                reverse_order.push_back(atom_type4);
                reverse_order.push_back(atom_type3);
                reverse_order.push_back(atom_type2);
                reverse_order.push_back(atom_type1);
                reverse_order.at(i) = "X";
                reverse_order.at(j) = "X";
                reverse_order.at(k) = "X";
                all_permutations.push_back(normal_order);
                all_permutations.push_back(reverse_order);
            }
    return all_permutations;
}

std::vector<std::vector<std::string> > Assembly::CreateAllAtomTypePermutationsforImproperDihedralType(std::string neighbor1_type, std::string neighbor2_type, std::string neighbor3_type, std::string atom_type)
{
    std::vector<std::vector<std::string> > all_permutations = std::vector<std::vector<std::string> >();
    std::vector<std::string> order1 = std::vector<std::string>();
    std::vector<std::string> order2 = std::vector<std::string>();
    std::vector<std::string> order3 = std::vector<std::string>();
    std::vector<std::string> reverse_order1 = std::vector<std::string>();
    std::vector<std::string> reverse_order2 = std::vector<std::string>();
    std::vector<std::string> reverse_order3 = std::vector<std::string>();

    order1.push_back(neighbor1_type);
    order1.push_back(neighbor2_type);
    order1.push_back(atom_type);
    order1.push_back(neighbor3_type);
    reverse_order1.push_back(neighbor3_type);
    reverse_order1.push_back(atom_type);
    reverse_order1.push_back(neighbor2_type);
    reverse_order1.push_back(neighbor1_type);

    order2.push_back(neighbor1_type);
    order2.push_back(atom_type);
    order2.push_back(neighbor3_type);
    order2.push_back(neighbor2_type);
    reverse_order2.push_back(neighbor2_type);
    reverse_order2.push_back(neighbor3_type);
    reverse_order2.push_back(atom_type);
    reverse_order2.push_back(neighbor1_type);

    order3.push_back(neighbor1_type);
    order3.push_back(neighbor3_type);
    order3.push_back(atom_type);
    order3.push_back(neighbor2_type);
    reverse_order3.push_back(neighbor2_type);
    reverse_order3.push_back(atom_type);
    reverse_order3.push_back(neighbor3_type);
    reverse_order3.push_back(neighbor1_type);

    all_permutations.push_back(order1);
    all_permutations.push_back(order2);
    all_permutations.push_back(order3);
    all_permutations.push_back(reverse_order1);
    all_permutations.push_back(reverse_order2);
    all_permutations.push_back(reverse_order3);

    ///Permutations w/ one X
    for(int i = 0; i < 4; i++)
    {
        order1.clear();
        order2.clear();
        order3.clear();
        reverse_order1.clear();
        reverse_order2.clear();
        reverse_order3.clear();

        order1.push_back(neighbor1_type);
        order1.push_back(neighbor2_type);
        order1.push_back(atom_type);
        order1.push_back(neighbor3_type);
        order1.at(i) = "X";
        reverse_order1.push_back(neighbor3_type);
        reverse_order1.push_back(atom_type);
        reverse_order1.push_back(neighbor2_type);
        reverse_order1.push_back(neighbor1_type);
        reverse_order1.at(i) = "X";
        order2.push_back(neighbor1_type);
        order2.push_back(atom_type);
        order2.push_back(neighbor3_type);
        order2.push_back(neighbor2_type);
        order2.at(i) = "X";
        reverse_order2.push_back(neighbor2_type);
        reverse_order2.push_back(neighbor3_type);
        reverse_order2.push_back(atom_type);
        reverse_order2.push_back(neighbor1_type);
        reverse_order2.at(i) = "X";
        order3.push_back(neighbor1_type);
        order3.push_back(neighbor3_type);
        order3.push_back(atom_type);
        order3.push_back(neighbor2_type);
        order3.at(i) = "X";
        reverse_order3.push_back(neighbor2_type);
        reverse_order3.push_back(atom_type);
        reverse_order3.push_back(neighbor3_type);
        reverse_order3.push_back(neighbor1_type);
        reverse_order3.at(i) = "X";

        all_permutations.push_back(order1);
        all_permutations.push_back(reverse_order1);
        all_permutations.push_back(order2);
        all_permutations.push_back(reverse_order2);
        all_permutations.push_back(order3);
        all_permutations.push_back(reverse_order3);
    }
    ///Permutations w/ two X
    for(int i = 0; i < 3; i++)
        for(int j = i+1; j < 4; j++)
        {
            order1.clear();
            order2.clear();
            order3.clear();
            reverse_order1.clear();
            reverse_order2.clear();
            reverse_order3.clear();

            order1.push_back(neighbor1_type);
            order1.push_back(neighbor2_type);
            order1.push_back(atom_type);
            order1.push_back(neighbor3_type);
            order1.at(i) = "X";
            order1.at(j) = "X";
            reverse_order1.push_back(neighbor3_type);
            reverse_order1.push_back(atom_type);
            reverse_order1.push_back(neighbor2_type);
            reverse_order1.push_back(neighbor1_type);
            reverse_order1.at(i) = "X";
            order2.push_back(neighbor1_type);
            order2.push_back(atom_type);
            order2.push_back(neighbor3_type);
            order2.push_back(neighbor2_type);
            order2.at(i) = "X";
            order2.at(j) = "X";
            reverse_order2.push_back(neighbor2_type);
            reverse_order2.push_back(neighbor3_type);
            reverse_order2.push_back(atom_type);
            reverse_order2.push_back(neighbor1_type);
            reverse_order2.at(i) = "X";
            reverse_order2.at(j) = "X";
            order3.push_back(neighbor1_type);
            order3.push_back(neighbor3_type);
            order3.push_back(atom_type);
            order3.push_back(neighbor2_type);
            order3.at(i) = "X";
            order3.at(j) = "X";
            reverse_order3.push_back(neighbor2_type);
            reverse_order3.push_back(atom_type);
            reverse_order3.push_back(neighbor3_type);
            reverse_order3.push_back(neighbor1_type);
            reverse_order3.at(i) = "X";
            reverse_order3.at(j) = "X";

            all_permutations.push_back(order1);
            all_permutations.push_back(reverse_order1);
            all_permutations.push_back(order2);
            all_permutations.push_back(reverse_order2);
            all_permutations.push_back(order3);
            all_permutations.push_back(reverse_order3);;
        }
    ///Permutations w/ three X
    for(int i = 0; i < 2; i++)
        for(int j = i+1; j < 3; j++)
            for(int k = j+1; k < 4; k++)
            {
                order1.clear();
                order2.clear();
                order3.clear();
                reverse_order1.clear();
                reverse_order2.clear();
                reverse_order3.clear();

                order1.push_back(neighbor1_type);
                order1.push_back(neighbor2_type);
                order1.push_back(atom_type);
                order1.push_back(neighbor3_type);
                order1.at(i) = "X";
                order1.at(j) = "X";
                order1.at(k) = "X";
                reverse_order1.push_back(neighbor3_type);
                reverse_order1.push_back(atom_type);
                reverse_order1.push_back(neighbor2_type);
                reverse_order1.push_back(neighbor1_type);
                reverse_order1.at(i) = "X";
                order2.push_back(neighbor1_type);
                order2.push_back(atom_type);
                order2.push_back(neighbor3_type);
                order2.push_back(neighbor2_type);
                order2.at(i) = "X";
                order2.at(j) = "X";
                order2.at(k) = "X";
                reverse_order2.push_back(neighbor2_type);
                reverse_order2.push_back(neighbor3_type);
                reverse_order2.push_back(atom_type);
                reverse_order2.push_back(neighbor1_type);
                reverse_order2.at(i) = "X";
                reverse_order2.at(j) = "X";
                reverse_order2.at(k) = "X";
                order3.push_back(neighbor1_type);
                order3.push_back(neighbor3_type);
                order3.push_back(atom_type);
                order3.push_back(neighbor2_type);
                order3.at(i) = "X";
                order3.at(j) = "X";
                order3.at(k) = "X";
                reverse_order3.push_back(neighbor2_type);
                reverse_order3.push_back(atom_type);
                reverse_order3.push_back(neighbor3_type);
                reverse_order3.push_back(neighbor1_type);
                reverse_order3.at(i) = "X";
                reverse_order3.at(j) = "X";
                reverse_order3.at(k) = "X";

                all_permutations.push_back(order1);
                all_permutations.push_back(reverse_order1);
                all_permutations.push_back(order2);
                all_permutations.push_back(reverse_order2);
                all_permutations.push_back(order3);
                all_permutations.push_back(reverse_order3);;
            }
    return all_permutations;
}

MolecularModeling::AtomVector Assembly::GetAllAtomsOfAssemblyWithAtLeastThreeNeighbors()
{
    AtomVector all_atoms = GetAllAtomsOfAssembly();
    AtomVector atoms_with_at_least_three_neighbors = AtomVector();
    for(AtomVector::iterator it = all_atoms.begin(); it != all_atoms.end(); it++)
    {
        Atom* atom = (*it);
        AtomNode* node = atom->GetNode();
        AtomVector neighbors = node->GetNodeNeighbors();
        if(neighbors.size() > 2)
            atoms_with_at_least_three_neighbors.push_back(atom);
    }
    return atoms_with_at_least_three_neighbors;
}

int Assembly::CountNumberOfExcludedAtoms()
{
    AtomVector atoms = GetAllAtomsOfAssembly();
    std::vector<std::string> excluded_atom_list = std::vector<std::string>();
    std::map<std::string, std::vector<std::string> > excluded_atom_list_map = std::map<std::string, std::vector<std::string> >();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        Atom* atom = (*it);
        std::stringstream ss;
        ss << atom->GetId();
        AtomNode* node = atom->GetNode();
        excluded_atom_list_map[atom->GetId()] = std::vector<std::string>();
        if(node != NULL)
        {
            AtomVector neighbors = node->GetNodeNeighbors();
            for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
            {
                Atom* neighbor = (*it1);
                std::stringstream ss1;
                ss1 << neighbor->GetId();
                std::stringstream first_order_interaction;
                std::stringstream reverse_first_order_interaction;
                first_order_interaction << ss.str() << "-" << ss1.str();
                reverse_first_order_interaction << ss1.str() << "-" << ss.str();
                if(find(excluded_atom_list.begin(), excluded_atom_list.end(), first_order_interaction.str()) == excluded_atom_list.end() &&
                        find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_first_order_interaction.str()) == excluded_atom_list.end())
                {
                    excluded_atom_list.push_back(first_order_interaction.str());
                    excluded_atom_list_map[atom->GetId()].push_back(neighbor->GetId());
                }
                AtomNode* neighbor_node = neighbor->GetNode();
                AtomVector neighbor_of_neighbors = neighbor_node->GetNodeNeighbors();
                for(AtomVector::iterator it2 = neighbor_of_neighbors.begin(); it2 != neighbor_of_neighbors.end(); it2++)
                {
                    Atom* neighbor_of_neighbor = (*it2);
                    std::stringstream ss2;
                    ss2 << neighbor_of_neighbor->GetId();
                    if(ss.str().compare(ss2.str()) != 0)
                    {
                        std::stringstream second_order_interaction;
                        std::stringstream reverse_second_order_interaction;
                        second_order_interaction << ss.str() << "-" << ss2.str();
                        reverse_second_order_interaction << ss2.str() << "-" << ss.str();
                        if(find(excluded_atom_list.begin(), excluded_atom_list.end(), second_order_interaction.str()) == excluded_atom_list.end() &&
                                find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_second_order_interaction.str()) == excluded_atom_list.end())
                        {
                            excluded_atom_list.push_back(second_order_interaction.str());
                            excluded_atom_list_map[atom->GetId()].push_back(neighbor_of_neighbor->GetId());
                        }
                        AtomNode* neighbor_of_neighbor_node = neighbor_of_neighbor->GetNode();
                        AtomVector neighbor_of_neighbor_of_neighbors = neighbor_of_neighbor_node->GetNodeNeighbors();
                        for(AtomVector::iterator it3 = neighbor_of_neighbor_of_neighbors.begin(); it3 != neighbor_of_neighbor_of_neighbors.end(); it3++)
                        {
                            Atom* neighbor_of_neighbor_of_neighbor = (*it3);
                            std::stringstream ss3;
                            ss3 << neighbor_of_neighbor_of_neighbor->GetId();
                            if(ss1.str().compare(ss3.str()) != 0)
                            {
                                std::stringstream third_order_interaction;
                                std::stringstream reverse_third_order_interaction;
                                third_order_interaction << ss.str() << "-" << ss3.str();
                                reverse_third_order_interaction << ss3.str() << "-" << ss.str();
                                if(find(excluded_atom_list.begin(), excluded_atom_list.end(), third_order_interaction.str()) == excluded_atom_list.end() &&
                                        find(excluded_atom_list.begin(), excluded_atom_list.end(), reverse_third_order_interaction.str()) == excluded_atom_list.end())
                                {
                                    excluded_atom_list.push_back(third_order_interaction.str());
                                    excluded_atom_list_map[atom->GetId()].push_back(neighbor_of_neighbor_of_neighbor->GetId());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    int number_of_excluded_atoms = 0;
    for(std::map<std::string, std::vector<std::string> >::iterator it = excluded_atom_list_map.begin(); it != excluded_atom_list_map.end(); it++)
        (*it).second.size() == 0 ? number_of_excluded_atoms++ : number_of_excluded_atoms += (*it).second.size();
    return number_of_excluded_atoms;
}

int Assembly::CountMaxNumberOfAtomsInLargestResidue()
{
    unsigned int max = 0;
    for(AssemblyVector::iterator it = assemblies_.begin(); it != assemblies_.end(); it++)
    {
        Assembly* assembly = (*it);
        if(max <= (unsigned int)assembly->CountNumberOfAtoms())
            max = assembly->CountNumberOfAtoms();
    }
    for(ResidueVector:: iterator it1 = residues_.begin(); it1 != residues_.end(); it1++)
    {
        Residue* residue = (*it1);
        MolecularModeling::AtomVector atoms = residue->GetAtoms();
        if(max <= atoms.size())
            max = atoms.size();
    }
    return max;
}
