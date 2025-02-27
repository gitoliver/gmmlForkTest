#include <algorithm>    // std::any_of
#include "includes/MolecularModeling/residue.hpp"
#include "includes/MolecularModeling/assembly.hpp"
#include "includes/MolecularModeling/atom.hpp"
#include "includes/MolecularModeling/atomnode.hpp"
#include "includes/MolecularModeling/residueproperties.hpp"
#include "includes/MolecularModeling/residuenode.hpp"
#include "includes/MolecularModeling/overlaps.hpp"
#include "includes/CodeUtils/logging.hpp"
#include "includes/ParameterSet/LibraryFileSpace/libraryfileresidue.hpp"
#include "includes/ParameterSet/LibraryFileSpace/libraryfileatom.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileresidue.hpp"
#include "includes/ParameterSet/PrepFileSpace/prepfileatom.hpp"
#include "includes/MolecularModeling/moleculardynamicatom.hpp"

using MolecularModeling::Residue;

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
Residue::Residue() : Node()
{
    index_ = this->generateIndex();
    this->SetId(this->CreateID());
}

Residue::Residue(MolecularModeling::Assembly *assembly, std::string name) : Node(name)
{
    assembly_ = assembly;
    name_ = name;
    atoms_ = AtomVector();
    head_atoms_ = AtomVector();
    tail_atoms_ = AtomVector();
    chemical_type_ = "";
    description_ = "";
    index_ = this->generateIndex();
    this->SetId(this->CreateID());
}

Residue::Residue(PrepFileSpace::PrepFileResidue *prep_residue) : Node()
{
    this->SetIndex(this->generateIndex());
    this->SetName(prep_residue->GetName());
    this->SetId(this->CreateID());
    this->BuildResidueFromPrepFileResidue(prep_residue);
 }

Residue::Residue(PrepFileSpace::PrepFileResidue *prep_residue, Abstract::ResidueType type) : Node()
{
    this->SetIndex(this->generateIndex());
    this->SetName(prep_residue->GetName());
    this->SetId(this->CreateID());
    this->SetType(type);
    this->BuildResidueFromPrepFileResidue(prep_residue);
}

std::string Residue::CreateID(std::string name, std::string chain, std::string number)
{ // OG Apr 2021: I have no idea what should make up the ID. Can't find any info on what the last three things are.
    //std::cout << name << ", " << chain << ", " << number << std::endl;
    if (name == "default")
    {
        name = this->GetName();
    }
    if (chain == "default")
    {
        chain = "A"; // Leaving this commented out as it was the old default. Maybe changing it messes something up and we need to go back.
        //chain = gmml::BLANK_SPACE;
    }
    if (number == "default")
    {
        std::stringstream ss;
        ss << this->GetIndex();
        number = ss.str();
    }
    std::stringstream id_stream;
    id_stream << name << "_" << chain << "_" << number << "_" << gmml::BLANK_SPACE << "_" << gmml::BLANK_SPACE << "_" << gmml::BLANK_SPACE; 
    //std::cout << id_stream.str() << std::endl;
    return id_stream.str();
}

void Residue::SetChainID(std::string chain)
{
    std::string newID = this->CreateID(this->GetName(), chain, this->GetNumber());
    this->SetId(newID);
    return;
}

void Residue::SetResidueNumber(std::string number)
{
    std::string newID = this->CreateID(this->GetName(), this->GetChainID(), number);
    this->SetId(newID);
    return;
}

void Residue::SetResidueNumber(unsigned int number)
{
    std::stringstream ss;
    ss << number;
    std::string newID = this->CreateID(this->GetName(), this->GetChainID(), ss.str());
    this->SetId(newID);
    return;
}

// Residue::Residue(Residue *residue)
// {
//     assembly_ = new Assembly(residue->GetAssembly());
//     name_ = residue->GetName();
//     atoms_ = AtomVector();
//     AtomVector atoms = residue->GetAtoms();
//     for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
//         atoms_.push_back(new MolecularModeling::Atom(*it));
//     head_atoms_ = AtomVector();
//     AtomVector head_atoms = residue->GetHeadAtoms();
//     for(AtomVector::iterator it = head_atoms.begin(); it != head_atoms.end(); it++)
//         head_atoms_.push_back(new MolecularModeling::Atom(*it));
//     tail_atoms_ = AtomVector();
//     AtomVector tail_atoms = residue->GetTailAtoms();
//     for(AtomVector::iterator it = tail_atoms.begin(); it != tail_atoms.end(); it++)
//         tail_atoms_.push_back(new MolecularModeling::Atom(*it));
//     chemical_type_ = residue->GetChemicalType();
//     description_ = residue->GetDescription();
//     id_ = residue->GetId();
//     index_ = this->generateIndex();
// }

// Residue::Residue(Residue& residue)
// {
//     Assembly *tempAssembly=residue.GetAssembly();
//     this->assembly_=tempAssembly;

//     this->name_=residue.GetName();
//     this->atoms_ = AtomVector();

//     AtomVector atoms = residue.GetAtoms();
//     for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
//         atoms_.push_back(new MolecularModeling::Atom(*it));

//     this->head_atoms_ = AtomVector();
//     AtomVector head_atoms = residue.GetHeadAtoms();
//     for(AtomVector::iterator it = head_atoms.begin(); it != head_atoms.end(); it++)
//         head_atoms_.push_back(new MolecularModeling::Atom(*it));

//     this->tail_atoms_ = AtomVector();
//     AtomVector tail_atoms = residue.GetTailAtoms();
//     for(AtomVector::iterator it = tail_atoms.begin(); it != tail_atoms.end(); it++)
//         tail_atoms_.push_back(new MolecularModeling::Atom(*it));

//     this->chemical_type_ = residue.GetChemicalType();
//     this->description_ = residue.GetDescription();
//     this->id_ = residue.GetId();
//     index_ = this->generateIndex();
// }
//////////////////////////////////////////////////////////
//                         ACCESSOR                     //
//////////////////////////////////////////////////////////
MolecularModeling::Assembly* Residue::GetAssembly()
{
    return assembly_;
}
std::string Residue::GetName()
{
    return name_;
}
std::string Residue::GetNumber()
{
    StringVector id = gmml::Split(id_, "_");
    return id.at(2); // This is silly, why not add residue number to class? OG: I know right? OG: Single point of truth?
}
std::string Residue::GetChainID()
{
    StringVector id = gmml::Split(id_, "_");
    return id.at(1);
}
std::string Residue::GetInsertionCode()
{
    StringVector id = gmml::Split(id_, "_");
    return id.at(3);
}
MolecularModeling::AtomVector Residue::GetAtoms()
{
    return atoms_;
}
MolecularModeling::AtomVector Residue::GetHeadAtoms()
{
    return head_atoms_;
}
MolecularModeling::AtomVector Residue::GetTailAtoms()
{
    return tail_atoms_;
}
std::string Residue::GetChemicalType() const
{
    return chemical_type_;
}
std::string Residue::GetDescription() const
{
    return description_;
}
std::string Residue::GetId() const
{
    return id_;
}
//Added by Yao on 06/13/2018
bool Residue::GetIsSugarDerivative()
{
    return is_sugar_derivative_;
}
//Added by Yao on 06/30/2018
bool Residue::GetIsAglycon()
{
    return is_aglycon_;
}
 //Added by ayush on 11/20/17 for residuenode in assembly
MolecularModeling::ResidueNode* Residue::GetNode()
{
        return residuenode_;
}
//Added by Dave 2/1/19
bool Residue::GetIsSugar()
{
    return is_sugar_;
}

//////////////////////////////////////////////////////////
//                          MUTATOR                     //
//////////////////////////////////////////////////////////
void Residue::SetAssembly(MolecularModeling::Assembly *assembly)
{
    assembly_ = assembly;
}
void Residue::SetName(std::string name)
{
    name_ = name;
}
void Residue::SetAtoms(AtomVector atoms)
{
    atoms_.clear();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); it++)
    {
        atoms_.push_back(*it);
    }
}
void Residue::AddAtom(MolecularModeling::Atom *atom)
{
    atoms_.push_back(atom);
}
void Residue::RemoveAtom(MolecularModeling::Atom *atom, bool remove_bonds)
{
    AtomVector newAtoms = AtomVector();
    std::vector<MolecularModeling::AtomNode*> leaving_atom_nodes = atom->GetNodes();

    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* a = *it;
        if(a != atom)
        {
	    if (remove_bonds){
		std::vector<MolecularModeling::AtomNode*> nodes = a->GetNodes();
		for (std::vector<MolecularModeling::AtomNode*>::iterator it2 = nodes.begin(); it2 != nodes.end(); it2++){
                    if((*it2) != NULL){  
                        (*it2)->RemoveNodeNeighbor(atom);
	            }
	        }
		for (std::vector<MolecularModeling::AtomNode*>::iterator it2 = leaving_atom_nodes.begin(); it2 != leaving_atom_nodes.end(); it2++){
		    if ((*it2) != NULL){
		        (*it2)->RemoveNodeNeighbor(a);
		    } 
		}
	    }
            newAtoms.push_back(a);
        }
    }
    this->SetAtoms(newAtoms);
}

void Residue::SetHeadAtoms(AtomVector head_atoms)
{
    head_atoms_.clear();
    for(AtomVector::iterator it = head_atoms.begin(); it != head_atoms.end(); it++)
    {
        head_atoms_.push_back(*it);
    }
}
void Residue::AddHeadAtom(MolecularModeling::Atom *head_atom)
{
    head_atoms_.push_back(head_atom);
}
void Residue::RemoveHeadAtom(MolecularModeling::Atom* head_atom)
{
    AtomVector::iterator position = std::find(head_atoms_.begin(), head_atoms_.end(), head_atom);
    if (position != head_atoms_.end()){
        head_atoms_.erase(position);
    }
}
void Residue::SetTailAtoms(AtomVector tail_atoms)
{
    tail_atoms_.clear();
    for(AtomVector::iterator it = tail_atoms.begin(); it != tail_atoms.end(); it++)
    {
        tail_atoms_.push_back(*it);
    }
}
void Residue::AddTailAtom(MolecularModeling::Atom *tail_atom)
{
    tail_atoms_.push_back(tail_atom);
}
void Residue::RemoveTailAtom(MolecularModeling::Atom* tail_atom)
{
    AtomVector::iterator position = std::find(tail_atoms_.begin(), tail_atoms_.end(), tail_atom);
    if (position != tail_atoms_.end()){
        tail_atoms_.erase(position);
    }
}
void Residue::SetChemicalType(std::string chemical_type)
{
    chemical_type_ = chemical_type;
}
void Residue::SetDescription(std::string description)
{
    description_ = description;
}
void Residue::SetId(std::string id)
{
    id_ = id;
}
void Residue::SetIndex(unsigned long long index)
{
    index_ = index;
}
void Residue::ReplaceAtomCoordinates(AtomVector *newAtoms)
{
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); ++it)
    {
        MolecularModeling::Atom *atom = (*it);
        for(AtomVector::iterator itt = newAtoms->begin(); itt != newAtoms->end(); ++itt)
        {
            MolecularModeling::Atom *atom1 = (*itt);
            //std::std::cout << "Comparing with " << atom1->GetName() << std::std::endl;
            if (atom->GetName() == atom1->GetName() )
            {
                //std::std::cout << "Replacing " << atom1->GetName() << " with " << atom->GetName() << std::std::endl;
                //std::std::cout << "Before X=" << atom->GetCoordinates().at(0)->GetX() << std::std::endl;
                atom->GetCoordinates().at(0)->SetX( atom1->GetCoordinates().at(0)->GetX() );
                atom->GetCoordinates().at(0)->SetY( atom1->GetCoordinates().at(0)->GetY() );
                atom->GetCoordinates().at(0)->SetZ( atom1->GetCoordinates().at(0)->GetZ() );
                //std::std::cout << "After X=" << atom->GetCoordinates().at(0)->GetX() << std::std::endl;
            }
        }
    }
}

 //Added by ayush on 11/20/17 for residuenode in assembly
void Residue::SetNode(MolecularModeling::ResidueNode* residuenode)
{
    residuenode_ = residuenode;
}
//Added by Yao 06/13/2018
void Residue::SetIsSugarDerivative(bool is_derivative)
{   
    is_sugar_derivative_ = is_derivative;
}
void Residue::SetIsAglycon(bool is_aglycon)
{
    is_aglycon_ = is_aglycon;
}
void Residue::SetIsSugar(bool is_sugar)
{
  is_sugar_ = is_sugar;
}

void Residue::AddChargesTypesToAtoms(LibraryFileSpace::LibraryFileResidue &libResidue)
{
    gmml::log(__LINE__, __FILE__, gmml::INF, "Adjusting charges and types for residue: " + this->GetId());
    for (auto &assemblyAtom : this->GetAtoms())
    {
        LibraryFileSpace::LibraryFileAtom *atomTemplate = libResidue.GetLibraryAtomByAtomName(assemblyAtom->GetName());
        assemblyAtom->MolecularDynamicAtom::SetAtomType(atomTemplate->GetType());
        assemblyAtom->MolecularDynamicAtom::SetCharge(atomTemplate->GetCharge());
        // Is element needed for off files?
        //            gmml::MolecularMetadata::GLYCAM::AmberAtomTypeInfoContainer AtomTypeMetaData;
        //            std::string element = AtomTypeMetaData.GetElementForAtomType(atom_type);
        //            assembly_atom->SetElementSymbol(element);
    }
    return;
}

// This is duct-tape for now. The PDBPreprocessor should write out an (e.g.) off file that contains all the info I'm trying to get here. And then GMML should read it in.
std::string Residue::GetTerminalCode()
{
    if ( (this->GetName() == "NME") || (this->GetName() == "ACE") )
    {
        return ""; // Despite being on the ends, these caps don't get the CNME or NACE treatment like regular amino acids. Fun.
    }
    if ( selection::FindNeighborResidueConnectedViaSpecificAtom(this, "N")  == nullptr )
    {
        return "N"; // No residues connected to N atom, ergo I am the N terminus.
    }
    else if ( selection::FindNeighborResidueConnectedViaSpecificAtom(this, "C")  == nullptr )
    {
        return "C"; // No residues connected to N atom, ergo I am the C terminus.
    }
    return "";
}

void Residue::AddChargesTypesToAtoms(PrepFileSpace::PrepFileResidue &prepResidue)
{
    gmml::log(__LINE__, __FILE__, gmml::INF, "Adjusting charges and types for residue: " + this->GetId());
    for (auto &assemblyAtom : this->GetAtoms())
    {
        PrepFileSpace::PrepFileAtom *atomTemplate = prepResidue.GetPrepAtomByAtomName(assemblyAtom->GetName());
        assemblyAtom->MolecularDynamicAtom::SetAtomType(atomTemplate->GetType());
        assemblyAtom->MolecularDynamicAtom::SetCharge(atomTemplate->GetCharge());
    }
    return;
}

//////////////////////////////////////////////////////////
//                       FUNCTIONS                      //
//////////////////////////////////////////////////////////
bool Residue::CheckSymbolBasedElementLabeling()
{
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        if(atom->GetElementSymbol().compare("") == 0)
            return false;
    }
    return true;
}

bool Residue::CheckParameterBasedElementLabeling()
{
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        if(atom->GetAtomType().compare("UNK") == 0 || atom->GetAtomType().compare("") == 0)
            return false;
    }
    return true;
}

bool Residue::GraphElementLabeling()
{
    if(this->CheckSymbolBasedElementLabeling())
        return this->GraphSymbolBasedElementLabeling();
    else if(this->CheckParameterBasedElementLabeling())
        return this->GraphParameterBasedElementLabeling();
    return GraphPredictionBasedElementLabeling();
}

bool Residue::GraphSymbolBasedElementLabeling()
{
//    std::cout << "Labeling residue nodes based on elements' symbol ... " << std::endl;
    bool flag = true;
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        MolecularModeling::AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
            atom_node->SetElementLabel(atom->GetElementSymbol());
        else
        {
            atom_node->SetElementLabel("UNK");
            flag = false;
        }
    }
    return flag;
}

bool Residue::GraphParameterBasedElementLabeling()
{
//    std::cout << "Labeling residue nodes based on atom type and parameter file" << std::endl;
    bool flag = true;
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        MolecularModeling::AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
        {
            std::string element_symbol = gmml::AtomTypesLookup(atom->GetAtomType()).element_symbol_;
            if(element_symbol != "")
                atom_node->SetElementLabel(element_symbol);
            else
            {
                atom_node->SetElementLabel("UNK");
                flag = false;
            }
        }
        else
        {
            atom_node->SetElementLabel("UNK");
            flag = false;
        }
    }
    return flag;
}

bool Residue::GraphPredictionBasedElementLabeling()
{
//    std::cout << "Labeling residue nodes based on first letter prediction ... " << std::endl;
    bool flag = true;
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        MolecularModeling::AtomNode* atom_node = atom->GetNode();
        if(atom_node != NULL)
            atom_node->SetElementLabel(isdigit(atom->GetName().at(0)) ? atom->GetName().at(1) + "" : atom->GetName().at(0) + "");
        else
        {
            atom_node->SetElementLabel("UNK");
            flag = false;
        }
    }
    return flag;
}

MolecularModeling::AtomVector Residue::GetAtomsWithLowestIntraDegree()
{
    int degree = (int) INFINITY;
    AtomVector lowest_degree_atoms = AtomVector();
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        int current = atom->GetNode()->GetIntraEdgeDegree();
        if(current < degree)
            current = degree;
    }
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        if(atom->GetNode()->GetIntraEdgeDegree() == degree)
            lowest_degree_atoms.push_back(atom);
    }

    return lowest_degree_atoms;
}

double Residue::CalculateAtomicOverlaps(MolecularModeling::Assembly *assemblyB)
{
    AtomVector assemblyBAtoms = assemblyB->GetAllAtomsOfAssembly();
    AtomVector residueAtoms = this->GetAtoms();
    return gmml::CalculateAtomicOverlaps(residueAtoms, assemblyBAtoms);
}
double Residue::CalculateAtomicOverlaps(AtomVector assemblyBAtoms)
{
    AtomVector residueAtoms = this->GetAtoms();
    return gmml::CalculateAtomicOverlaps(residueAtoms, assemblyBAtoms);
}

bool Residue::CheckIfProtein()
{
  int local_debug = -1;
  if( std::find( gmml::PROTEINS, ( gmml::PROTEINS + gmml::PROTEINSSIZE ), this->GetName() ) != ( gmml::PROTEINS + gmml::PROTEINSSIZE ) )
  {
    if (local_debug > 0)
    {
      gmml::log(__LINE__, __FILE__, gmml::INF, "Protein Found");
    }
    return true;
  }
  return false;
}

bool Residue::CheckIfNucleicAcid()
{
  int local_debug = -1;
  if( std::find( gmml::NUCLEICACIDS, ( gmml::NUCLEICACIDS + gmml::NUCLEICACIDSSIZE ), this->GetName() ) != ( gmml::NUCLEICACIDS + gmml::NUCLEICACIDSSIZE ) )
  {
    if (local_debug > 0)
    {
      gmml::log(__LINE__, __FILE__, gmml::INF, "Nucleic Acid Found");
    }
    return true;
  }
  return false;
}

bool Residue::CheckIfSaccharide()
{
  int local_debug = -1;
  if( std::find( gmml::SACCHARIDES, ( gmml::SACCHARIDES + gmml::SACCHARIDESSIZE ), this->GetName() ) != ( gmml::SACCHARIDES + gmml::SACCHARIDESSIZE ) )
  {
    if (local_debug > 0)
    {
      gmml::log(__LINE__, __FILE__, gmml::INF, "Saccharide Found");
    }
    return true;
  }
  return false;
}

bool Residue::CheckIfWater() {
	if( this->GetName().compare( "HOH" ) == 0 ) {
		return true;
	}
	return false;
}

// bool Residue::CheckIfProtein()
// {
//     std::string resname = this->GetName();
//     if(resname.compare("ALA")==0)
//         return true;
//     else if (resname.compare("ASP")==0)
//         return true;
//     else if (resname.compare("ASN")==0)
//         return true;
//     else if (resname.compare("ASP")==0)
//         return true;
//     else if (resname.compare("ARG")==0)
//         return true;
//     else if (resname.compare("GLY")==0)
//         return true;
//     else if (resname.compare("GLU")==0)
//         return true;
//     else if (resname.compare("GLN")==0)
//         return true;
//     else if (resname.compare("PRO")==0)
//         return true;
//     else if (resname.compare("HIS")==0)
//         return true;
//     else if (resname.compare("ASP")==0)
//         return true;
//     else if (resname.compare("VAL")==0)
//         return true;
//     else if (resname.compare("LEU")==0)
//         return true;
//     else if (resname.compare("THR")==0)
//         return true;
//     else if (resname.compare("SER")==0)
//         return true;
//     else if (resname.compare("LYS")==0)
//         return true;
//     else if (resname.compare("MET")==0)
//         return true;
//     else if (resname.compare("TYR")==0)
//         return true;
//     else if (resname.compare("TRP")==0)
//         return true;
//     else if (resname.compare("PHE")==0)
//         return true;
//     else if (resname.compare("SEC")==0)
//         return true;
//     else if (resname.compare("ILE")==0)
//         return true;
//     else if (resname.compare("CYX")==0)
//         return true;
//     else if (resname.compare("HID")==0)
//         return true;
//     else if (resname.compare("HIE")==0)
//         return true;
//     else if (resname.compare("NLN")==0)
//         return true;
//     else if (resname.compare("OLT")==0)
//         return true;
//     else if (resname.compare("OLS")==0)
//         return true;
//     else if (resname.compare("OLY")==0)
//         return true;
//     else
//         return false;
// }


/*GeometryTopology::Coordinate Residue::GetRingCenter() // Disabled by OG; GetIsRing returns true for all atoms even when IsRing wasn't set.
{
    double sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    int numberOfRingAtoms = 0;
    AtomVector atoms = this->GetAtoms();

    for(MolecularModeling::MolecularModeling::AtomVector::iterator atom = atoms.begin(); atom != atoms.end(); atom++)
    {
        if ( (*atom)->GetIsRing() )
        {
            numberOfRingAtoms++;
            //std::std::cout << "Atom is ring: " << (*atom)->GetName() << std::std::endl;
            sumX += (*atom)->GetCoordinates().at(0)->GetX();
            sumY += (*atom)->GetCoordinates().at(0)->GetY();
            sumZ += (*atom)->GetCoordinates().at(0)->GetZ();
        }
    }
    GeometryTopology::Coordinate center;
    center.SetX( sumX / numberOfRingAtoms  );
    center.SetY( sumY / numberOfRingAtoms  );
    center.SetZ( sumZ / numberOfRingAtoms  );
    return center;
}*/

GeometryTopology::Coordinate Residue::GetGeometricCenter()
{
    AtomVector atoms = this->GetAtoms();
    if(atoms.size() == 0)
    {
//        std::cout << "Problem in Residue::GetGeometricCenter(), the residue " << this->GetId() << " contains no atoms." << std::endl;
    }

    double sumX = 0.0, sumY = 0.0, sumZ = 0.0;
    for(MolecularModeling::AtomVector::iterator atom = atoms.begin(); atom != atoms.end(); ++atom)
    {
       // std::cout << "atoms size is " << atoms.size() << " for " << (*atom)->GetId() << std::endl;
        sumX += (*atom)->GetCoordinates().at(0)->GetX();
        sumY += (*atom)->GetCoordinates().at(0)->GetY();
        sumZ += (*atom)->GetCoordinates().at(0)->GetZ();
    }
    GeometryTopology::Coordinate center;
    center.SetX( sumX / atoms.size()  );
    center.SetY( sumY / atoms.size()  );
    center.SetZ( sumZ / atoms.size()  );
    return center;
}

MolecularModeling::Atom* Residue::GetAtom(std::string query_name)
{
    MolecularModeling::Atom* return_atom = nullptr;
    AtomVector atoms = this->GetAtoms();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); ++it)
    {
        if ((*it)->GetName() == query_name)
        {
            return_atom = (*it);
            return return_atom;
            //std::cout << "From Residue::GetAtom(), returning: " << return_atom->GetId() << " as a match for " << query_name << std::endl;
        }
    }
    return return_atom; // may be unset
}

MolecularModeling::Atom* Residue::GetAtom(unsigned long long query_index)
{
    MolecularModeling::Atom* return_atom = nullptr;
    AtomVector atoms = this->GetAtoms();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); ++it)
    {
        if ((*it)->GetIndex() == query_index)
        {
            return_atom = (*it);
        }
    }
    return return_atom; // may be unset
}

MolecularModeling::Atom* Residue::GetAtomWithId(std::string query_id)
{
    MolecularModeling::Atom* return_atom = nullptr;
    AtomVector atoms = this->GetAtoms();
    for(AtomVector::iterator it = atoms.begin(); it != atoms.end(); ++it)
    {
        if ((*it)->GetId() == query_id)
        {
            return_atom = (*it);
        }
    }
    return return_atom; // may be unset
}

void Residue::MakeDeoxy(std::string oxygenNumber)
{ // if oxygenNumber is 6, then C6-O6-H6O becomes C6-Hd 
    MolecularModeling::Atom* hydrogenAtom = this->GetAtom("H" + oxygenNumber + "O");
    MolecularModeling::Atom* oxygenAtom = this->GetAtom("O" + oxygenNumber);
    MolecularModeling::Atom* carbonAtom = this->GetAtom("C" + oxygenNumber);
    // Add O and H charge to the C atom.
    carbonAtom->SetCharge(carbonAtom->GetCharge() + oxygenAtom->GetCharge() + hydrogenAtom->GetCharge());
    // Delete the H of O-H
    this->RemoveAtom(hydrogenAtom);
    // Now transform the Oxygen to a Hd. Easier than deleting O and creating H.
    std::string newID = oxygenAtom->GetId();
    newID.replace(0,oxygenAtom->GetName().size(),"Hd");
    oxygenAtom->SetId(newID);
    oxygenAtom->SetName("Hd");
    oxygenAtom->MolecularDynamicAtom::SetAtomType("H1");
    oxygenAtom->SetCharge(0.0000);
    oxygenAtom->SetElementSymbol("H");
    gmml::log(__LINE__, __FILE__, gmml::INF, "Completed MakeDeoxy\n");
}

double Residue::CalculateCharge()
{
    double charge = 0.0;
    for (auto &atom : this->GetAtoms())
    {
        charge += atom->GetCharge();
    }
    return charge;
}

unsigned long long Residue::GetIndex() const
{
    return this->index_;
} // end GetIndex


//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void Residue::Print(std::ostream &out)
{
    out << "------------------------ " << name_ << " --------------------------" << std::endl;
    out << "Head atoms: ";
    for(AtomVector::iterator it = head_atoms_.begin(); it != head_atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
//        out << atom->GetResidue()->GetName() << ":" << atom->GetName() << "; ";
        out << atom->GetId() << "; ";
    }
    out << std::endl;
    out << "Tail atoms: ";
    for(AtomVector::iterator it = tail_atoms_.begin(); it != tail_atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
//        out << atom->GetResidue()->GetName() << ":" << atom->GetName() << "; ";
        out << atom->GetId() << "; ";
    }
    out << std::endl;
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        atom->Print(out);
    }
}

void Residue::PrettyPrintHet(std::ostream &out)
{
    out << "------------------------ " << "Residue " << " --------------------------" << std::endl;
    out << " ID: " << id_ << std::endl;
    out << " Name: " << name_ << std::endl;
//    out << " Chemical type: " << chemical_type_ << std::endl;
//    out << " Description: " << description_ << std::endl;
    out << " ATOMS: ";

    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        out << atom->GetId() << ", ";
    }

    out << std::endl;
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        out << "------------------------ " << "Atom" << " --------------------------" << std::endl;
        out << " ID: " << atom->GetId() << std::endl;
        out << " Name: " << atom->GetName() << std::endl;
        out << " Atom type: " << atom->GetAtomType() << std::endl;
        out << " Charge: " << atom->GetCharge() << std::endl;
//        out << " Chemical Type: " << atom->GetChemicalType() << std::endl;
//        out << " Description: " << atom->GetDescription() << std::endl;
        out << " Mass: " << atom->GetMass() << std::endl;
        GeometryTopology::Coordinate* coords = atom->GetCoordinates().at(0);
        out << " Coordinates" <<  " X: " << coords->GetX() << ", Y: " << coords->GetY() << ", Z: " << coords->GetZ() << std::endl;
        out << " Neighbors: ";
        MolecularModeling::AtomNode* node = atom->GetNode();
        AtomVector neighbors = node->GetNodeNeighbors();
        for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
        {
            MolecularModeling::Atom* neighbor = *it1;
            out << neighbor->GetId() << ", ";
        }
        out << std::endl;
        atom->PrintHet(out);
    }
}

void Residue::PrintHetResidues(std::ostream &out)
{
    out << id_ << ";" << name_ << ";";
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        out << atom->GetId() << ",";
    }
    out << std::endl;
}

void Residue::PrintHetAtoms(std::ostream &out)
{
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        out << atom->GetId() << ";" << atom->GetName() << ";" << atom->GetAtomType() << ";" << atom->GetCharge() << ";" << atom->GetMass() << ";";
        GeometryTopology::Coordinate* coords = atom->GetCoordinates().at(0);
        out << coords->GetX() << "," << coords->GetY() << "," << coords->GetZ() << ";";

        MolecularModeling::AtomNode* node = atom->GetNode();
        AtomVector neighbors = node->GetNodeNeighbors();
        for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
        {
            MolecularModeling::Atom* neighbor = *it1;
            out << neighbor->GetId() << ",";
        }
        out << std::endl;
    }
}

void Residue::WriteHetResidues(std::ofstream& out)
{
    out << id_ << ";" << name_ << ";";
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        if (it == (atoms_.end() - 1) )
            out << atom->GetId();
        else
            out << atom->GetId() << ",";
    }
    out << std::endl;
}

void Residue::WriteHetAtoms(std::ofstream& out)
{
    for(AtomVector::iterator it = atoms_.begin(); it != atoms_.end(); it++)
    {
        MolecularModeling::Atom* atom = *it;
        out << atom->GetId() << ";" << atom->GetName() << ";" << atom->GetAtomType() << ";" << atom->GetCharge() << ";" << atom->GetMass() << ";";
        GeometryTopology::Coordinate* coords = atom->GetCoordinates().at(0);
        out << coords->GetX() << "," << coords->GetY() << "," << coords->GetZ() << ";";

        MolecularModeling::AtomNode* node = atom->GetNode();
        AtomVector neighbors = node->GetNodeNeighbors();
        for(AtomVector::iterator it1 = neighbors.begin(); it1 != neighbors.end(); it1++)
        {
            MolecularModeling::Atom* neighbor = *it1;
            if(it1 == (neighbors.end() - 1))
                out << neighbor->GetId();
            else
                out << neighbor->GetId() << ",";
        }
        out << std::endl;
    }
}


//////////////////////////////////////////////////////////
//                   OVERLOADED OPERATORS               //
//////////////////////////////////////////////////////////
bool Residue::operator== (const Residue &otherResidue)
{
    return (this->GetIndex() == otherResidue.GetIndex());
}

bool Residue::operator!= (const Residue &otherResidue)
{
    return (this->GetIndex() != otherResidue.GetIndex());
}

unsigned long long Residue::generateIndex()
{
    static unsigned long long s_ResidueIndex = 0; // static keyword means it is created only once and persists beyond scope of code block.
    return s_ResidueIndex++; // makes copy of s_AtomIndex, increments the real s_AtomIndex, then returns the value in the copy
} // end generateAtomIndex
