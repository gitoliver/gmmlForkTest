#ifndef INCLUDES_PARAMETERSET_PREPFILE_PREPFILEATOM_HPP
#define INCLUDES_PARAMETERSET_PREPFILE_PREPFILEATOM_HPP

#include "includes/CodeUtils/constants.hpp"
#include "includes/CentralDataStructure/atom.hpp"
#include <string>
#include <iostream>

namespace prep
{ // repeated from common or utils as they should be here, not in gmml scope. Left over there as the old class needs them until I delete it.
enum TopologicalType
{
	kTopTypeE,
	kTopTypeS,
	kTopTypeB,
	kTopType3,
	kTopType4,
	kTopTypeM
};
class PrepAtom : public cds::Atom
{
public:
	//////////////////////////////////////////////////////////
	//                       Constructor                    //
	//////////////////////////////////////////////////////////
	PrepAtom(const std::string& line);
	~PrepAtom() {}//{std::cout << "PrepAtom dtor for " << this->getName() << ", ";}
	//////////////////////////////////////////////////////////
	//                         FUNCTIONS                    //
	//////////////////////////////////////////////////////////
	void Determine3dCoordinate();
	void FindDihedralAtoms(std::vector<PrepAtom*>& foundAtoms, int currentDepth = 0, const int& targetDepth = 3);
	inline void visit() {++visitCount_;}
	//////////////////////////////////////////////////////////
	//                     DISPLAY FUNCTIONS                //
	//////////////////////////////////////////////////////////
	void Print(std::ostream& out = std::cerr) const;
	void Write(std::ostream &stream) const;
	//////////////////////////////////////////////////////////
	//                           ACCESSOR                   //
	//////////////////////////////////////////////////////////
	inline const int& GetVisits() const {return visitCount_;}
	TopologicalType GetTopologicalType() const;
	int GetBondIndex() const;
	int GetAngleIndex() const;
	int GetDihedralIndex() const;
	double GetBondLength() const;
	double GetAngle() const;
	double GetDihedral() const;
	//////////////////////////////////////////////////////////
	//                           MUTATOR                    //
	//////////////////////////////////////////////////////////
	void SetTopologicalType(TopologicalType topological_type);
	void SetBondIndex(int bond_index);
	void SetAngleIndex(int angle_index);
	void SetDihedralIndex(int dihedral_index);
	void SetBondLength(double bond_length);
	void SetAngle(double angle);
	void SetDihedral(double dihedral);
private:
	//////////////////////////////////////////////////////////
	//                         FUNCTIONS                    //
	//////////////////////////////////////////////////////////
	TopologicalType ExtractAtomTopologicalType(std::istream& ss);
	std::string GetStringFormatOfTopologicalType() const;
	TopologicalType GetTopologicalTypeFromString(std::string topological_type) const;
	//////////////////////////////////////////////////////////
	//                         ATTRIBUTES                   //
	//////////////////////////////////////////////////////////
	std::string type_ = "";                          /*!< Atom type; fill by the third column of the residue section of the file */
	TopologicalType topological_type_ = kTopTypeM;          /*!< Topological type (for chain extraction of the residue); fill by th 4th column of the residue section of the file */
	int bond_index_ = 0;;                            /*!< Bond index; fill by the 5th column of the residue section of the file */
	int angle_index_ = 0;;                           /*!< Angle index; fill by the 6th column of the residue section of the file */
	int dihedral_index_ = 0;                        /*!< Dihedral index; fill by the 7th column of the residue section of the file */
	double bond_length_ = constants::dNotSet;                        /*!< Bond length; fill by the 8th column of the residue section of the file */
	double angle_ = constants::dNotSet;                              /*!< Angle; fill by the 9th column of the residue section of the file */
	double dihedral_ = constants::dNotSet;                           /*!< Dihedral; fill by the 10th column of the residue section of the file */
	int visitCount_ = 0;
	/*!< Sample line of the atom section of a prep file: 4 H1   H1  M  3  2  1  1.000    90.0     180.0     0.0 */
};
} // namespace
#endif // INCLUDES_PARAMETERSET_PREPFILE_PREPFILEATOM_HPP
