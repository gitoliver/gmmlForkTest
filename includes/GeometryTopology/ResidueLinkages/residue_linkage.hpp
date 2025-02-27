#ifndef GMML_INCLUDES_GEOMETRYTOPOLOGY_RESIDUELINKAGES_RESIDUE_LINKAGE_HPP
#define GMML_INCLUDES_GEOMETRYTOPOLOGY_RESIDUELINKAGES_RESIDUE_LINKAGE_HPP
/*
 * This class figures out the rotatable bonds between two residues
 * Starts/ends at the CA atoms in proteins. Looks for cycles (as they aren't rotatable).
 * Stores each rotatable bond as a rotatable_dihedral object.
 */
//#include "gmml.hpp"
#include "includes/MolecularModeling/atom.hpp"
#include "includes/MolecularModeling/residue.hpp"
#include "includes/MolecularModeling/Selections/selections.hpp"
#include "includes/GeometryTopology/ResidueLinkages/rotatable_dihedral.hpp"

using MolecularModeling::Residue;
using MolecularModeling::ResidueVector;

class Residue_linkage;
typedef std::vector<Residue_linkage> ResidueLinkageVector;
class Residue_linkage
{
public:
    //////////////////////////////////////////////////////////
    //                       CONSTRUCTOR                    //
    //////////////////////////////////////////////////////////
    Residue_linkage(Residue *nonReducingResidue1, Residue *reducingResidue2, bool reverseAtomsThatMove = true);
    Residue_linkage(Residue *nonReducingResidue1, Residue *reducingResidue2, AtomVector alsoMovingAtoms, bool reverseAtomsThatMove = true);
    //////////////////////////////////////////////////////////
    //                       ACCESSOR                       //
    //////////////////////////////////////////////////////////
    std::vector<Rotatable_dihedral> GetRotatableDihedrals() const;
    std::vector<Rotatable_dihedral> GetRotatableDihedralsWithMultipleRotamers() const;
    int GetNumberOfRotatableDihedrals() const;
    int GetNumberOfShapes(const bool likelyShapesOnly = false) const;
    Residue* GetFromThisResidue1() const;
    Residue* GetToThisResidue2() const;
    bool CheckIfConformer() const;
    bool GetIfExtraAtoms() const;
    AtomVector GetExtraAtoms();
    unsigned long long GetIndex() const;
    std::string GetName() const;
    //////////////////////////////////////////////////////////
    //                       MUTATOR                        //
    //////////////////////////////////////////////////////////
    void SetRotatableDihedrals(std::vector<Rotatable_dihedral> rotatableDihedrals);
    //////////////////////////////////////////////////////////
    //                       FUNCTIONS                      //
    //////////////////////////////////////////////////////////
    void SetDefaultShapeUsingMetadata();
    void SetRandomShapeUsingMetadata(bool useRanges = true);
    void SetSpecificShapeUsingMetadata(int shapeNumber, bool useRanges = false);
    void SetSpecificShape(std::string dihedralName, std::string selectedRotamer);
    void SetCustomDihedralAngles(std::vector <double> dihedral_angles);
    void SetShapeToPrevious();
    void SetRandomDihedralAngles();
    void DetermineAtomsThatMove();
    // Simple meaning you only check each rotatable_dihedral in series, not every combination.
    void SimpleWiggle(AtomVector& overlapAtomSet1, AtomVector& overlapAtomSet2, const int angleIncrement = 5);
    void SimpleWiggleCurrentRotamers(AtomVector& overlapAtomSet1, AtomVector& overlapAtomSet2, const int angleIncrement = 5);
    void SetIndex(unsigned long long index);
    //////////////////////////////////////////////////////////
    //                       DISPLAY FUNCTION               //
    //////////////////////////////////////////////////////////
    std::string Print() const;
private:
    //////////////////////////////////////////////////////////
    //                    PRIVATE FUNCTIONS                 //
    //////////////////////////////////////////////////////////
    ResidueVector GetResidues() const;
    bool GetIfReversedAtomsThatMove() const;
    Atom* GetFromThisConnectionAtom1() const;
    Atom* GetToThisConnectionAtom2() const;
    void SetIfReversedAtomsThatMove(bool reversedAtomsThatMove);
    void AddExtraAtomsThatMove(AtomVector extraAtoms);
    void InitializeClass(Residue *from_this_residue1, Residue *to_this_residue2, bool reverseAtomsThatMove);
    bool CheckIfViableLinkage() const;
    std::vector<Rotatable_dihedral> FindRotatableDihedralsConnectingResidues(Atom *from_this_connection_atom1, Atom *to_this_connection_atom2);
    //AtomVector DealWithBranchesFromLinkages(AtomVector linearLinkageAtoms, Atom *cycle_point1, Atom *cycle_point2);
    // Previous function generates a list of linearly connected atoms that define the rotatable bonds
    // This function splits that list into groups of 4 and creates rotatable_dihedral objects
    std::vector<Rotatable_dihedral> SplitAtomVectorIntoRotatableDihedrals(AtomVector atoms);
    gmml::MolecularMetadata::GLYCAM::DihedralAngleDataVector FindMetadata() const;
    void AddMetadataToRotatableDihedrals(gmml::MolecularMetadata::GLYCAM::DihedralAngleDataVector metadata);
    void SetResidues(Residue *residue1, Residue *residue2);
    void SetConnectionAtoms(Residue *residue1, Residue *residue2);
    void SetConformerUsingMetadata(bool useRanges = false, int conformerNumber = 0);
    unsigned long long GenerateIndex();
    std::string DetermineLinkageNameFromResidueNames() const;
    void SetName(std::string name);
    //////////////////////////////////////////////////////////
    //                       ATTRIBUTES                     //
    //////////////////////////////////////////////////////////
    Residue* from_this_residue1_;
    Residue* to_this_residue2_;
    Atom* from_this_connection_atom1_;
    Atom* to_this_connection_atom2_;
    std::vector<Rotatable_dihedral> rotatable_dihedrals_;
    bool reverseAtomsThatMove_;
    AtomVector extraAtomsThatMove_;
    bool isExtraAtoms_ = true;
    unsigned long long index_;
    std::string name_; //e.g. "DGalpb1-6DGlcpNAc"
};
#endif // GMML_INCLUDES_GEOMETRYTOPOLOGY_RESIDUELINKAGES_RESIDUE_LINKAGE_HPP
