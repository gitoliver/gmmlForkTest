
#include "../../../includes/FileSet/TopologyFileSpace/topologydihedraltype.hpp"

using namespace std;
using namespace TopologyFileSpace;

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
TopologyDihedralType::TopologyDihedralType() {}

//////////////////////////////////////////////////////////
//                         ACCESSOR                     //
//////////////////////////////////////////////////////////
int TopologyDihedralType::GetIndex()
{
    return index_;
}
double TopologyDihedralType::GetPeriodicity()
{
    return periodicity_;
}
double TopologyDihedralType::GetPhase()
{
    return phase_;
}


//////////////////////////////////////////////////////////
//                          MUTATOR                     //
//////////////////////////////////////////////////////////
void TopologyDihedralType::SetIndex(int index)
{
    index_ = index;
}
void TopologyDihedralType::SetPeriodicity(double periodicity)
{
    periodicity_ = periodicity;
}
void TopologyDihedralType::SetPhase(double phase)
{
    phase_ = phase;
}
void TopologyDihedralType::SetScee(double scee)
{
    scee_ = scee;
}
void TopologyDihedralType::SetScnb(double scnb)
{
    scnb_ = scnb;
}
//////////////////////////////////////////////////////////
//                        FUNCTIONS                     //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void TopologyDihedralType::Print(ostream &out)
{
}



