#include "includes/CentralDataStructure/Readers/Pdb/pdbResidue.hpp"
#include "includes/CentralDataStructure/Readers/Pdb/pdbAtom.hpp"
#include "includes/CentralDataStructure/Readers/Pdb/pdbResidueId.hpp" // residueId
#include "includes/CentralDataStructure/Measurements/measurements.hpp" // get_cartesian_point_from_internal_coords
#include "includes/CentralDataStructure/Writers/pdbWriter.hpp"
#include "includes/CodeUtils/logging.hpp"
#include "includes/CodeUtils/strings.hpp" //RemoveWhiteSpace
#include <sstream>
#include <string>

using pdb::PdbResidue;
//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
PdbResidue::PdbResidue(std::stringstream &singleResidueSecion, std::string firstLine)
{
    ResidueId resId(firstLine);
    this->setName(resId.getName());
    this->setNumber(std::stoi(resId.getNumber()));
    this->setInsertionCode(resId.getInsertionCode());
    this->setChainId(resId.getChainId());
    std::string firstLineAlternativeLocationIndicator = resId.getAlternativeLocation(); // Normally empty
    std::string line;
    while(getline(singleResidueSecion, line))
    {
        std::string recordName = codeUtils::RemoveWhiteSpace(line.substr(0,6));
        if ( (recordName == "ATOM") || (recordName == "HETATM") )
        {
            ResidueId id(line); //Check alternativeLocation and ignore any that aren't the same as the first one.
            if( firstLineAlternativeLocationIndicator.empty() || id.getAlternativeLocation() == firstLineAlternativeLocationIndicator )
            {
                this->addAtom(std::make_unique<PdbAtom>(line));
            }
        }
    }
    this->SetType( this->determineType( this->getName() ) );
    return;
}

PdbResidue::PdbResidue(const std::string residueName, const PdbResidue *referenceResidue)
: cds::Residue(residueName, referenceResidue)
{ // should instead call copy constructor and then rename with residueName?
//    this->setName(residueName); // handled by cdsResidue cTor
//    this->setNumber(referenceResidue->getNumber()); // handled by cdsResidue cTor
    this->setInsertionCode(referenceResidue->getInsertionCode());
    this->setChainId(referenceResidue->getChainId());
    this->SetType(referenceResidue->GetType());
    return;
}

//PdbResidue::PdbResidue(std::vector<std::unique_ptr<AtomRecord>>& atomRecords)
//{
//    atomRecordss_.swap(atomRecords); // atomRecords will become empty, atoms_ will contain contents of atomRecords.
//    hasTerCard_ = false;
//}
//////////////////////////////////////////////////////////
//                         ACCESSOR                     //
//////////////////////////////////////////////////////////
pdb::ResidueId PdbResidue::getId() const
{
    ResidueId temp(this->getName(), std::to_string(this->getNumber()), this->getInsertionCode(), this->getChainId());
    return temp;
}

const std::string PdbResidue::getNumberAndInsertionCode() const
{
    return std::to_string(this->getNumber()) + this->getInsertionCode();
}


//////////////////////////////////////////////////////////
//                    MUTATOR                           //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                    FUNCTIONS                         //
//////////////////////////////////////////////////////////
void PdbResidue::modifyNTerminal(const std::string& type)
{
    gmml::log(__LINE__,__FILE__,gmml::INF, "Modifying N Terminal of : " + this->printId());
    if (type == "NH3+")
    {
        const cds::Atom* atom = this->FindAtom("H");
        if (atom != nullptr)
        {
            gmml::log(__LINE__,__FILE__,gmml::INF, "Deleting atom with id: " + static_cast<const PdbAtom*>(atom)->GetId());
            this->deleteAtom(atom);
        }
    }
    else
    {
        gmml::log(__LINE__, __FILE__, gmml::WAR, "Cannot handle this type of terminal option: " + type);
    }
    return;
}

void PdbResidue::modifyCTerminal(const std::string& type)
{
    gmml::log(__LINE__,__FILE__,gmml::INF, "Modifying C Terminal of : " + this->printId());
    if (type == "CO2-")
    {
        const cds::Atom* atom = this->FindAtom("OXT");
        if (atom == nullptr)
        {
            // I don't like this, but at least it's somewhat contained:
            const cds::Atom* atomCA = this->FindAtom("CA");
            const cds::Atom* atomC = this->FindAtom("C");
            const cds::Atom* atomO = this->FindAtom("O");
            cds::Coordinate oxtCoord = cds::calculateCoordinateFromInternalCoords(*(atomCA->getCoordinate()), *(atomC->getCoordinate()), *(atomO->getCoordinate()), 120.0, 180.0, 1.25);
            this->addAtom(std::make_unique<PdbAtom>("OXT", oxtCoord));
            gmml::log(__LINE__,__FILE__,gmml::INF, "Created new atom named OXT after " + static_cast<const PdbAtom*>(atomO)->GetId());
        }
    }
    else
    {
        gmml::log(__LINE__, __FILE__, gmml::WAR, "Cannot handle this type of terminal option: " + type);
    }
    return;
}

//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void PdbResidue::Print(std::ostream &out) const
{
    out << "pdb::Residue : " << this->printId() << std::endl;
    for(auto &atom : this->getAtoms())
    {
        out << "    atom : " << static_cast<const PdbAtom*>(atom)->GetId() << " X: "  << atom->getCoordinate()->GetX() << " Y: "  << atom->getCoordinate()->GetY() << " Z: " << atom->getCoordinate()->GetZ() << "\n";
    }
}

void PdbResidue::Write(std::ostream& stream) const
{
    for(auto &atom : this->getAtoms())
    {
        static_cast<const PdbAtom*>(atom)->Write(stream, this->getName(), this->getNumber(), this->getChainId(), this->getInsertionCode());
    }
    if(this->HasTerCard())
    {
        stream << "TER\n";
    }
}
