#include <random>
#include "includes/GeometryTopology/ResidueLinkages/rotatable_dihedral.hpp"
#include "includes/MolecularModeling/atomnode.hpp" // For UpdateAtomsIfPsi
#include "includes/utils.hpp"
#include "includes/External_Libraries/PCG/pcg_random.h"
#include "includes/GeometryTopology/geometrytopology.hpp"
#include "includes/CodeUtils/logging.hpp"
#include "includes/MolecularModeling/overlaps.hpp"

// Seed with a real random value, if available
static pcg_extras::seed_seq_from<std::random_device> seed_source;
// Make a random number engine
static pcg32 rng(seed_source);
using GeometryTopology::Coordinate;

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
Rotatable_dihedral::Rotatable_dihedral(Atom *atom1, Atom *atom2, Atom *atom3, Atom *atom4, bool reverseAtomsThatMove)
{
    AtomVector atoms {atom1, atom2, atom3, atom4};
    this->Initialize(atoms, reverseAtomsThatMove);
}
Rotatable_dihedral::Rotatable_dihedral(Atom *atom1, Atom *atom2, Atom *atom3, Atom *atom4, AtomVector extraAtomsThatMove, bool reverseAtomsThatMove)
{
    AtomVector atoms {atom1, atom2, atom3, atom4};
    this->Initialize(atoms, reverseAtomsThatMove);
    this->AddExtraAtomsThatMove(extraAtomsThatMove);
}
//////////////////////////////////////////////////////////
//                       ACCESSOR                       //
//////////////////////////////////////////////////////////
double Rotatable_dihedral::CalculateDihedralAngle(const std::string type) const
{
    if (type == "glycamReport")
    {
        if ( (this->GetName() == "Phi") && (atom1_->GetName() == "C1") )
        {
            Coordinate* o5Coord = atom1_->GetResidue()->GetAtom("O5")->GetCoordinate();
            return GeometryTopology::CalculateDihedralAngle(o5Coord, atom2_->GetCoordinate(), atom3_->GetCoordinate(), atom4_->GetCoordinate());        
        }
    }
    return GeometryTopology::CalculateDihedralAngle(atom1_->GetCoordinate(), atom2_->GetCoordinate(), atom3_->GetCoordinate(), atom4_->GetCoordinate());
}

AtomVector Rotatable_dihedral::GetAtoms() const
{
    AtomVector atoms = {atom1_, atom2_, atom3_, atom4_};
    return atoms;
}

AtomVector Rotatable_dihedral::GetAtomsThatMove() const
{
    return atoms_that_move_;
}

bool Rotatable_dihedral::GetIsAtomsThatMoveReversed() const
{
    return isAtomsThatMoveReversed_;
}

double Rotatable_dihedral::GetPreviousDihedralAngle() const
{
    return previous_dihedral_angle_;
}

const DihedralAngleDataVector& Rotatable_dihedral::GetMetadata() const
{
    return assigned_metadata_;
}

DihedralAngleDataVector Rotatable_dihedral::GetLikelyMetadata() const
{
    DihedralAngleDataVector returningMetadata;
    for (auto &entry : this->GetMetadata())
    {
        if(entry.weight_ >= 0.01 ) // HARDCODE EVERYTHING.
        {
            //std::cout << "Likely entry for: " << entry.dihedral_angle_name_ << " weigh: " << entry.weight_ << "\n";
            returningMetadata.push_back(entry);
        }
        //else {std::cout << "UnLikely entry for: " << entry.dihedral_angle_name_ << " weigh: " << entry.weight_ << "\n";}
    }
    return returningMetadata;
}

int Rotatable_dihedral::GetNumberOfRotamers(bool likelyShapesOnly) const
{
    if (this->GetMetadata().empty())
    {
        gmml::log(__LINE__,__FILE__,gmml::ERR, "Error in Rotatable_dihedral::GetNumberOfRotamers; no metadata has been set.\n");
        return 0;
    }
    else
    {
        int count = 0;
        for (auto & entry: this->GetMetadata())
        {
            if ( (entry.weight_ < 0.01) && (likelyShapesOnly) ) // I'm hardcoding it, I don't care.
                {}// Do nought.
            else
                count++;
        }
       return count;
    }
}

std::vector<double> Rotatable_dihedral::GetAllPossibleAngleValues(const int interval) const
{
    std::vector<double> allPossibleAngleValues;
    if (assigned_metadata_.empty())
    {
        std::stringstream ss;
        ss << "Error in Rotatable_dihedral::GetAllPossibleAngleValues; no metadata has been set for:\n"
                << atom1_->GetId() << " " << atom2_->GetId() << " " << atom3_->GetId() << " " << atom4_->GetId() << "\n";
        gmml::log(__LINE__,__FILE__,gmml::ERR, ss.str());
        throw std::runtime_error(ss.str());
    }
    else
    {
        gmml::MolecularMetadata::GLYCAM::DihedralAngleDataVector metadata_entries = this->GetMetadata();
        //std::cout << metadata_entries.at(0).dihedral_angle_name_;
        for(auto &metadata : metadata_entries)
        {
            double lower_bound = (metadata.default_angle_value_ - metadata.lower_deviation_);
            double upper_bound = (metadata.default_angle_value_ + metadata.upper_deviation_);
            double current_dihedral = lower_bound;
            while(current_dihedral <= upper_bound)
            {
                allPossibleAngleValues.push_back(current_dihedral);
                current_dihedral += interval; // increment
            }
        }
    }
    return allPossibleAngleValues;
}

std::string Rotatable_dihedral::GetName() const
{
    if (this->GetLikelyMetadata().empty())
        return "Boo";
    else
        return this->GetLikelyMetadata().at(0).dihedral_angle_name_;
}   

//////////////////////////////////////////////////////////
//                       MUTATOR                        //
//////////////////////////////////////////////////////////
void Rotatable_dihedral::DetermineAtomsThatMove()
{
    // In keeping with giving residues as GlcNAc1-4Gal, and wanting the moving atoms to be in the opposite direction (defaults):
    AtomVector atoms_that_move;
    if (this->GetIsAtomsThatMoveReversed())
    {
        atoms_that_move.push_back(atom3_);
        atom2_->FindConnectedAtoms(atoms_that_move);
    }
    else
    {
        atoms_that_move.push_back(atom2_);
        atom3_->FindConnectedAtoms(atoms_that_move);
    }
    this->SetAtomsThatMove(atoms_that_move);
}

void Rotatable_dihedral::AddExtraAtomsThatMove(AtomVector extraAtoms)
{
    extra_atoms_that_move_.insert(extra_atoms_that_move_.end(), extraAtoms.begin(), extraAtoms.end());
    return;
}

// Only this function uses radians. Everything else, in and out, should be degrees.
void Rotatable_dihedral::SetDihedralAngle(const double dihedral_angle)
{
    if(!this->wasEverRotated_)
    {
        this->SetWasEverRotated(true);
        this->DetermineAtomsThatMove();
    }
    Coordinate* a1 = atom1_->GetCoordinate();
    Coordinate* a2 = atom2_->GetCoordinate();
    Coordinate* a3 = atom3_->GetCoordinate();
    Coordinate* a4 = atom4_->GetCoordinate();
    if(this->GetIsAtomsThatMoveReversed())
    { // A function that was agnostic of this would be great.
        a1 = atom4_->GetCoordinate();
        a2 = atom3_->GetCoordinate();
        a3 = atom2_->GetCoordinate();
        a4 = atom1_->GetCoordinate();
    }
    this->RecordPreviousDihedralAngle(this->CalculateDihedralAngle());
    std::stringstream ss;
    ss << "Setting dihedral for " << atom1_->GetId() << ":"  << atom2_->GetId() << ":"  << atom3_->GetId() << ":"  << atom4_->GetId() <<  ": " << dihedral_angle << "\n";
    gmml::log(__LINE__,__FILE__,gmml::INF, ss.str());
    std::vector<Coordinate*> coordinatesToMove;
    for(auto &atom : this->GetAtomsThatMove())
    {
    	coordinatesToMove.push_back(atom->GetCoordinate());
    }
    for(auto &atom : extra_atoms_that_move_)
    {
    	coordinatesToMove.push_back(atom->GetCoordinate());
    }
    GeometryTopology::SetDihedralAngle(a1, a2, a3, a4, dihedral_angle, coordinatesToMove);
    return;
}

void Rotatable_dihedral::SetDihedralAngleToPrevious()
{
    this->SetDihedralAngle(this->GetPreviousDihedralAngle());
}

double Rotatable_dihedral::RandomizeDihedralAngle()
{
    return Rotatable_dihedral::RandomizeDihedralAngleWithinRange(0.0, 360.0);
}

double Rotatable_dihedral::RandomizeDihedralAngleWithinRange(double min, double max)
{
    std::uniform_real_distribution<> angle_distribution(min, max); // define the range
    double random_angle = angle_distribution(rng);
    /*******************************************/
    /*               IMPORTANT                 */
    /*******************************************/

    this->SetDihedralAngle(random_angle); // THIS IS IMPORTANT!!! THIS SHOULD BE SEPARATED?!?! The two other functions call this one. Seems fragile.

    /*******************************************/
    /*               IMPORTANT                 */
    /*******************************************/
    return random_angle;
}

// OG for the CB, I need this class to know which metadata index it's currently set to. So this won't work for that.
double Rotatable_dihedral::RandomizeDihedralAngleWithinRanges(std::vector<std::pair<double,double>> ranges)
{
    std::uniform_int_distribution<> distr(0, (ranges.size() - 1)); // define the range
    // Select one of the ranges
    int range_selection = distr(rng);
    // create an angle within the selected range
    return this->RandomizeDihedralAngleWithinRange(ranges.at(range_selection).first, ranges.at(range_selection).second);
}

void Rotatable_dihedral::SetMetadata(DihedralAngleDataVector metadataVector)
{
    assigned_metadata_ = metadataVector;
    this->UpdateAtomsIfPsi();
    return;
}

void Rotatable_dihedral::AddMetadata(DihedralAngleData metadata)
{
    assigned_metadata_.push_back(metadata);
    this->UpdateAtomsIfPsi();
    return;
}

void Rotatable_dihedral::ClearMetadata()
{
    assigned_metadata_.clear();
    return;
}

void Rotatable_dihedral::SetRandomAngleEntryUsingMetadata(bool useRanges)
{
    if (assigned_metadata_.empty())
    {
        std::stringstream ss;
        ss << "Error in Rotatable_dihedral::SetRandomAngleEntryUsingMetadata; no metadata has been set for:.\n"
                << atom1_->GetId() << atom2_->GetId() << atom3_->GetId() << atom4_->GetId();
        gmml::log(__LINE__,__FILE__,gmml::ERR, ss.str());
        throw std::runtime_error(ss.str());
    }
    else
    {   // first randomly pick one of the meta data entries. If there is only one entry, it will randomly always be it.
        std::uniform_int_distribution<> distr(0, (assigned_metadata_.size() - 1)); // define the range
        DihedralAngleData& entry = assigned_metadata_.at(distr(rng));
        this->SetCurrentMetaData(entry);
        if (useRanges)
        {
            double lower = (entry.default_angle_value_ - entry.lower_deviation_) ;
            double upper = (entry.default_angle_value_ + entry.upper_deviation_) ;
            this->RandomizeDihedralAngleWithinRange(lower, upper);
        }
        else
        { // Just set it to the entries default value
            this->RandomizeDihedralAngleWithinRange(entry.default_angle_value_, entry.default_angle_value_);
        }
    }
    return;
//    else if(assigned_metadata_.size() == 1)
//    {
//        DihedralAngleData& entry = assigned_metadata_.at(0);
//        this->SetCurrentMetaData(entry);
//        if (useRanges)
//        {
//            double lower = (entry.default_angle_value_ - entry.lower_deviation_) ;
//            double upper = (entry.default_angle_value_ + entry.upper_deviation_) ;
//            this->RandomizeDihedralAngleWithinRange(lower, upper);
//        }
//        else
//        {
//            this->SetDihedralAngle(entry.default_angle_value_);
//        }
//    } // I need different behaviour here than previously. If use_ranges is false, just take the first of multiple rotamers, don't randomize.
//    else if(assigned_metadata_.size() >= 2) // Some dihedral angles have multiple rotamers, thus mulitple ranges to select from. e.g -60, 60, 180
//    {
//        // OG update for carb builder (CB). This class needs to know what metadata entry it's currently set to.
//        if (useRanges)
//        {
//            // first randomly pick one of the meta data entries
//            std::uniform_int_distribution<> distr(0, (assigned_metadata_.size() - 1)); // define the range
//            DihedralAngleData& randomEntry = assigned_metadata_.at(distr(rng));
//            double lower = (randomEntry.default_angle_value_ - randomEntry.lower_deviation_) ;
//            double upper = (randomEntry.default_angle_value_ + randomEntry.upper_deviation_) ;
//            this->RandomizeDihedralAngleWithinRange(lower, upper);
//            this->SetCurrentMetaData(randomEntry);
//        }
//        else
//        { // Just set it to the first entries default value
//            DihedralAngleData& entry = assigned_metadata_.at(0);
//            this->RandomizeDihedralAngleWithinRange(entry.default_angle_value_, entry.default_angle_value_);
//            this->SetCurrentMetaData(entry);
//        }
//    }
//    return;
}

void Rotatable_dihedral::SetSpecificAngleEntryUsingMetadata(bool useRanges, int angleEntryNumber)
{
    if (assigned_metadata_.empty())
    {
        std::stringstream ss;
        ss << "Error in Rotatable_dihedral::SetSpecificAngleUsingMetadata; no metadata has been set for:\n"
                << atom1_->GetId() << " " << atom2_->GetId() << " " << atom3_->GetId() << " " << atom4_->GetId() << "\n";
        gmml::log(__LINE__,__FILE__,gmml::ERR, ss.str());
        throw std::runtime_error(ss.str());
    }
    else if (assigned_metadata_.size() <= angleEntryNumber)
    {
        std::stringstream ss;
        ss << "Error in Rotatable_dihedral::SetSpecificAngleUsingMetadata; angleEntryNumber of " << angleEntryNumber << " is too large as metadata.size() is " << assigned_metadata_.size() << ".\n";
        gmml::log(__LINE__,__FILE__,gmml::ERR, ss.str());
        throw std::runtime_error(ss.str());
    }
    else
    {
        DihedralAngleData& entry = assigned_metadata_.at(angleEntryNumber);
        this->SetCurrentMetaData(entry);
        if (useRanges)
        {
            double lower = (entry.default_angle_value_ - entry.lower_deviation_) ;
            double upper = (entry.default_angle_value_ + entry.upper_deviation_) ;
            this->RandomizeDihedralAngleWithinRange(lower, upper);
            std::stringstream ss;
            ss << entry.dihedral_angle_name_ << " was set to " <<  entry.default_angle_value_ << "+/-" << entry.lower_deviation_ << ". Atoms: "
                    << atom1_->GetId() << " " << atom2_->GetId() << " " << atom3_->GetId() << " " << atom4_->GetId() << "\n";
            gmml::log(__LINE__,__FILE__,gmml::INF, ss.str());
        }
        else
        {
            this->SetDihedralAngle(entry.default_angle_value_);
            std::stringstream ss;
            ss << entry.dihedral_angle_name_ << " was set to " <<  entry.default_angle_value_ << ". Atoms: "
                    << atom1_->GetId() << " " << atom2_->GetId() << " " << atom3_->GetId() << " " << atom4_->GetId() << "\n";
            gmml::log(__LINE__,__FILE__,gmml::INF, ss.str());
        }
    }
    return;
}

bool Rotatable_dihedral::SetSpecificShape(std::string dihedralName, std::string selectedRotamer)
{
    if (assigned_metadata_.empty())
    {
        gmml::log(__LINE__,__FILE__,gmml::ERR, "No metadata set");
    }
    else
    {
        gmml::log(__LINE__,__FILE__,gmml::INF, "Made it here with " + dihedralName + " and " + selectedRotamer);
        if (dihedralName == this->GetMetadata().at(0).dihedral_angle_name_)
        {
            for(auto &metadata : this->GetMetadata())
            {
                std::stringstream ss;
                ss << dihedralName << ": " <<  selectedRotamer <<  " vs metadata " <<  metadata.dihedral_angle_name_ << ": " << metadata.rotamer_name_;
                gmml::log(__LINE__,__FILE__,gmml::INF, ss.str());
                if (metadata.rotamer_name_ == selectedRotamer)
                {
                    this->SetDihedralAngle(metadata.default_angle_value_);
                    this->SetCurrentMetaData(metadata);
                    ss << "Bingo! Setting " << dihedralName << " to " << metadata.default_angle_value_ << " for " << atom1_->GetId() << " " << atom2_->GetId() << " " << atom3_->GetId() << " " << atom4_->GetId() << "\n";
                    gmml::log(__LINE__,__FILE__,gmml::INF, ss.str());
                    return true;
                }
            }
        }
    }
    return false;
}

void Rotatable_dihedral::WiggleUsingAllRotamers(MolecularModeling::AtomVector& overlapAtomSet1, MolecularModeling::AtomVector &overlapAtomSet2, const int &angleIncrement)
{
    double bestDihedral = this->CalculateDihedralAngle();
    double lowestOverlap = gmml::CalculateAtomicOverlapsBetweenNonBondedAtoms(overlapAtomSet1, overlapAtomSet2);;
    for(auto &metadata : this->GetMetadata())
    {
        double lowerBound = (metadata.default_angle_value_ - metadata.lower_deviation_);
        double upperBound = (metadata.default_angle_value_ + metadata.upper_deviation_);
        double newOverlap = this->WiggleWithinRanges(overlapAtomSet1, overlapAtomSet2, angleIncrement, lowerBound, upperBound);
        if (lowestOverlap >= (newOverlap + 0.01))
        {
            lowestOverlap = newOverlap;
            bestDihedral = this->CalculateDihedralAngle();
            this->SetCurrentMetaData(metadata);
        }
        else
        {
            this->SetDihedralAngle(bestDihedral);
        }
    }
}

void Rotatable_dihedral::WiggleWithinCurrentRotamer(MolecularModeling::AtomVector& overlapAtomSet1, MolecularModeling::AtomVector &overlapAtomSet2, const int &angleIncrement)
{
    double lowerBound = (this->GetCurrentMetaData()->default_angle_value_ - this->GetCurrentMetaData()->lower_deviation_);
    double upperBound = (this->GetCurrentMetaData()->default_angle_value_ + this->GetCurrentMetaData()->upper_deviation_);
    // Set to lowest deviation, work through to highest. Set best value and return it for reference.
    this->WiggleWithinRanges(overlapAtomSet1, overlapAtomSet2, angleIncrement, lowerBound, upperBound);
}

double Rotatable_dihedral::WiggleWithinRanges(MolecularModeling::AtomVector& overlapAtomSet1, MolecularModeling::AtomVector &overlapAtomSet2, const int &angleIncrement, const double& lowerBound, const double& upperBound)
{
    this->SetDihedralAngle(lowerBound);
    double currentDihedral = lowerBound;
    double bestDihedral = lowerBound;
    double currentOverlap = gmml::CalculateAtomicOverlapsBetweenNonBondedAtoms(overlapAtomSet1, overlapAtomSet2);
    double lowestOverlap = currentOverlap;
//    std::cout << "Starting wiggle with overlap : " << currentOverlap << " . Current Angle: " << currentDihedral << " lowerBound: " << lowerBound << " upperBound: " << upperBound << "\n";
    while(currentDihedral < upperBound)
    {
        currentDihedral += angleIncrement; // increment
        this->SetDihedralAngle(currentDihedral);
        currentOverlap = gmml::CalculateAtomicOverlapsBetweenNonBondedAtoms(overlapAtomSet1, overlapAtomSet2);
        if (lowestOverlap >= (currentOverlap + 0.01)) // rounding errors
        {
            lowestOverlap = currentOverlap;
            bestDihedral = currentDihedral;
        }
        // Prefer angles closer to default if overlap is the same.
        else if ( (lowestOverlap == currentOverlap) && (std::abs(this->GetCurrentMetaData()->default_angle_value_ - bestDihedral)
                > std::abs(this->GetCurrentMetaData()->default_angle_value_ - currentDihedral)) )
        {
            bestDihedral = currentDihedral;
        }
    }
    this->SetDihedralAngle(bestDihedral);
    return lowestOverlap;
}

//////////////////////////////////////////////////////////
//                  PRIVATE FUNCTIONS                   //
//////////////////////////////////////////////////////////
void Rotatable_dihedral::Initialize(AtomVector atoms, bool reverseAtomsThatMove)
{
    this->SetWasEverRotated(false);
    this->SetAtoms(atoms);
    this->SetIsAtomsThatMoveReversed(reverseAtomsThatMove);
    currentMetadata_ = nullptr;
    // this->DetermineAtomsThatMove(); // Will be done the first time SetDihedralAngle is called.
    return;
}

void Rotatable_dihedral::SetAtoms(AtomVector atoms)
{
    atom1_ = atoms.at(0);
    atom2_ = atoms.at(1);
    atom3_ = atoms.at(2);
    atom4_ = atoms.at(3);
    return;
}

void Rotatable_dihedral::SetAtomsThatMove(AtomVector atoms)
{
    atoms_that_move_ = atoms;
    return;
}

void Rotatable_dihedral::SetIsAtomsThatMoveReversed(bool isAtomsThatMoveReversed)
{
    isAtomsThatMoveReversed_ = isAtomsThatMoveReversed;
    return;
}

void Rotatable_dihedral::RecordPreviousDihedralAngle(double dihedral_angle)
{
    previous_dihedral_angle_ = dihedral_angle;
}

void Rotatable_dihedral::UpdateAtomsIfPsi()
{
    for(auto &entry : assigned_metadata_)
    {
        // If it's a psi angle and is supposed to be defined by a H...
        if ((entry.dihedral_angle_name_.compare("Psi")==0) && (entry.atom4_.at(0)=='H'))
        {// Find the neighbor of current atom3 which is a hydrogen, and set it to be the new atom4;
            bool foundHydrogen = false;
            for(auto &neighbor : atom3_->GetNode()->GetNodeNeighbors())
            {
                if(neighbor->GetName().at(0)=='H')
                {
//                    std::cout << "In ";
//                    this->Print();
//                    std::cout << "Replaced atom4_ with " << neighbor->GetId() << "\n";
                    atom4_ = neighbor;
                    foundHydrogen = true;
                }
            }
            int numberOfNeighbors = atom3_->GetNode()->GetNodeNeighbors().size();
            if (!foundHydrogen && numberOfNeighbors <= 3)
            {
                //std::cout << "Creating HHH atom for " << atom3_->GetId() << " as it has " << numberOfNeighbors << " neighbors\n";
                atom4_ = Rotatable_dihedral::CreateHydrogenAtomForPsi(atom3_);
            }
        }
    }
    return;
}

Atom* Rotatable_dihedral::CreateHydrogenAtomForPsi(Atom *centralAtom)
{
    Coordinate newCoord = GeometryTopology::CreateMissingCoordinateForTetrahedralAtom(centralAtom);
    Atom *newAtom = new Atom(centralAtom->GetResidue(), "HHH", newCoord);
    centralAtom->GetResidue()->AddAtom(newAtom);
    centralAtom->GetNode()->AddNodeNeighbor(newAtom);
    // Have to create a new node for this new atom.
    MolecularModeling::AtomNode *leakyNode = new MolecularModeling::AtomNode();
    newAtom->SetNode(leakyNode); // A jest!
    newAtom->GetNode()->AddNodeNeighbor(centralAtom);
    return newAtom;
}

void Rotatable_dihedral::SetWasEverRotated(bool wasEverRotated)
{
    wasEverRotated_ = wasEverRotated;
}

bool Rotatable_dihedral::CheckIfEverRotated() const
{
    return wasEverRotated_;
}
//////////////////////////////////////////////////////////
//                       DISPLAY FUNCTION               //
//////////////////////////////////////////////////////////
std::string Rotatable_dihedral::Print() const
{
   std::stringstream ss;
   ss << atom1_->GetName() << ", " << atom2_->GetName() << ", " << atom3_->GetName() << ", " << atom4_->GetName() << ": " << this->CalculateDihedralAngle()  << ".\n";
   gmml::log(__LINE__, __FILE__, gmml::INF, ss.str());
   return ss.str();
}
