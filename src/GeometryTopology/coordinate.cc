#include <limits>
#include "includes/GeometryTopology/coordinate.hpp"
#include "includes/common.hpp"
#include "includes/utils.hpp"
#include "includes/CodeUtils/logging.hpp"

using GeometryTopology::Coordinate;

//////////////////////////////////////////////////////////
//                       Constructor                    //
//////////////////////////////////////////////////////////
Coordinate::Coordinate() : x_(0.0), y_(0.0), z_(0.0) {}

Coordinate::Coordinate(double x, double y, double z) : x_(x), y_(y), z_(z) {}
Coordinate::Coordinate(const std::string x, const std::string y, const std::string z)
{
    try
    {
        x_ = std::stod(x);
        y_ = std::stod(y);
        z_ = std::stod(z);
    }
    catch (...)
    {
        gmml::log(__LINE__, __FILE__, gmml::ERR, "Could not convert these strings to doubles: " + x + ", " + y + ", " + z + ", ");
        throw;
    }
}
Coordinate::Coordinate(const Coordinate &coordinate) : x_(coordinate.x_), y_(coordinate.y_), z_(coordinate.z_) {}

Coordinate::Coordinate(Coordinate* coordinate) : x_(coordinate->x_), y_(coordinate->y_), z_(coordinate->z_) {}

//////////////////////////////////////////////////////////
//                           ACCESSOR                   //
//////////////////////////////////////////////////////////
double Coordinate::GetX() const
{
    return x_;
}

double Coordinate::GetY() const
{
    return y_;
}

double Coordinate::GetZ() const
{
    return z_;
}

//////////////////////////////////////////////////////////
//                           MUTATOR                    //
//////////////////////////////////////////////////////////
void Coordinate::SetX(double x)
{
    x_ = x;
}

void Coordinate::SetY(double y)
{
    y_ = y;
}

void Coordinate::SetZ(double z)
{
    z_ = z;
}

//////////////////////////////////////////////////////////
//                         FUNCTIONS                    //
//////////////////////////////////////////////////////////
void Coordinate::Translate(double x, double y, double z)
{
    x_ += x;
    y_ += y;
    z_ += z;
}

bool Coordinate::CompareTo(Coordinate coordinate) const
{
    if(x_ == coordinate.x_ && y_ == coordinate.y_ && z_ == coordinate.z_)
        return true;
    else
        return false;
}

// This should be the definitive place for this calc.
bool Coordinate::withinDistance(const Coordinate *coordinate, const double distance) const
{ // Vast majority of calls to this function will be able to return false after first if.
    if (this->GetX() - coordinate->GetX() < distance)
    {
        if (this->GetY() - coordinate->GetY() < distance)
        {
            if (this->Distance(coordinate) < distance) // Need to test if also checking Z is faster.
            {
                return true;
            }
        }
    }
    return false;
}

double Coordinate::Distance(const Coordinate &coordinate) const
{
    double dist = (x_ - coordinate.x_) * (x_ - coordinate.x_) + (y_ - coordinate.y_) * (y_ - coordinate.y_) + (z_ - coordinate.z_) * (z_ - coordinate.z_);
    if(dist > 0.00000001) // can sometimes measure distance to self, in which case get sqrt(0), which should be fine but zero is funky and somtimes is actually slightly negative.
    {
        return sqrt(dist);
    }
    return 0.0;
}

double Coordinate::Distance(const Coordinate *coordinate) const
{
    return this->Distance(*coordinate);
}

double Coordinate::length() const
{
    return sqrt( (this->GetX() * this->GetX()) + (this->GetY() * this->GetY()) + (this->GetZ() * this->GetZ()) );
}

void Coordinate::Normalize()
{
    double length = this->length();
    if(length != 0.0)
    {
        x_ = x_ / length;
        y_ = y_ / length;
        z_ = z_ / length;
    }
}
double Coordinate::DotProduct(Coordinate coordinate)
{
    return ((x_ * coordinate.x_) + (y_ * coordinate.y_) + (z_ * coordinate.z_));
}
void Coordinate::CrossProduct(Coordinate coordinate)
{
    double x = x_;
    double y = y_;
    double z = z_;
    x_ = (y * coordinate.z_) - (coordinate.y_ * z);
    y_ = (z * coordinate.x_) - (coordinate.z_ * x);
    z_ = (x * coordinate.y_) - (coordinate.x_ * y);
}
void Coordinate::operator+(Coordinate coordinate)
{
    x_ += coordinate.x_;
    y_ += coordinate.y_;
    z_ += coordinate.z_;
}
void Coordinate::operator +(double addition)
{
    x_ += addition;
    y_ += addition;
    z_ += addition;
}
void Coordinate::operator-(Coordinate coordinate)
{
    x_ -= coordinate.x_;
    y_ -= coordinate.y_;
    z_ -= coordinate.z_;
}
void Coordinate::operator /(Coordinate coordinate)
{
    x_ /= coordinate.x_;
    y_ /= coordinate.y_;
    z_ /= coordinate.z_;
}
void Coordinate::operator/(double divisor)
{
    x_ /= divisor;
    y_ /= divisor;
    z_ /= divisor;
}
void Coordinate::operator *(double multiplier)
{
    x_ *= multiplier;
    y_ *= multiplier;
    z_ *= multiplier;
}
void Coordinate::TranslateAll(CoordinateVector coordinate_set, double margin, int pos)
{
    if(coordinate_set.size() == 0)
        return;
    Coordinate* direction=NULL;
    if(pos == 1)
        direction  = new Coordinate(coordinate_set.at(0));
    if(pos == -1)
        direction  = new Coordinate(coordinate_set.at(coordinate_set.size()-1));
    direction->operator -(this);
    Coordinate* offset = new Coordinate(direction);
    offset->Normalize();
    offset->operator *(margin);
    offset->operator -(direction);
    for(CoordinateVector::iterator it = coordinate_set.begin(); it != coordinate_set.end(); it++)
        (*it)->Translate(offset->GetX(), offset->GetY(), offset->GetZ());
}

void Coordinate::RotateAngularAll(CoordinateVector coordinate_set, double angle, int pos)
{
    if(coordinate_set.size() < 2)
        return;
    double current_angle = 0.0;
    Coordinate* a1 = this;
    Coordinate* a2 = NULL;
    Coordinate* a3 = NULL;
    if(pos == 1){
        a2 = new Coordinate(coordinate_set.at(0));
        a3 = new Coordinate(coordinate_set.at(1));
    }
    if(pos == -1){
        a2 = new Coordinate(coordinate_set.at(coordinate_set.size()-1));
        a3 = new Coordinate(coordinate_set.at(coordinate_set.size()-2));
    }

    Coordinate* b1 = new Coordinate(*a1);
    b1->operator -(*a2);
    Coordinate* b2 = new Coordinate(*a3);
    b2->operator -(*a2);

    current_angle = acos((b1->DotProduct(*b2)) / (b1->length() * b2->length() + gmml::DIST_EPSILON));
    double rotation_angle = gmml::ConvertDegree2Radian(angle) - current_angle;

    Coordinate* direction = new Coordinate(*b1);
    direction->CrossProduct(*b2);
    direction->Normalize();
    double** rotation_matrix = gmml::GenerateRotationMatrix(direction, a2, rotation_angle);

    if(pos == 1)
    {
        for(CoordinateVector::iterator it = coordinate_set.begin() + 1; it != coordinate_set.end(); it++)
        {
            Coordinate* coordinate = (*it);
            Coordinate* result = new Coordinate();
            result->SetX(rotation_matrix[0][0] * coordinate->GetX() + rotation_matrix[0][1] * coordinate->GetY() +
                    rotation_matrix[0][2] * coordinate->GetZ() + rotation_matrix[0][3]);
            result->SetY(rotation_matrix[1][0] * coordinate->GetX() + rotation_matrix[1][1] * coordinate->GetY() +
                    rotation_matrix[1][2] * coordinate->GetZ() + rotation_matrix[1][3]);
            result->SetZ(rotation_matrix[2][0] * coordinate->GetX() + rotation_matrix[2][1] * coordinate->GetY() +
                    rotation_matrix[2][2] * coordinate->GetZ() + rotation_matrix[2][3]);

            (*it)->SetX(result->GetX());
            (*it)->SetY(result->GetY());
            (*it)->SetZ(result->GetZ());
        }
    }
    if(pos == -1)
    {
        for(CoordinateVector::iterator it = coordinate_set.begin(); it != coordinate_set.end() - 1; it++)
        {
            Coordinate* coordinate = (*it);
            Coordinate* result = new Coordinate();
            result->SetX(rotation_matrix[0][0] * coordinate->GetX() + rotation_matrix[0][1] * coordinate->GetY() +
                    rotation_matrix[0][2] * coordinate->GetZ() + rotation_matrix[0][3]);
            result->SetY(rotation_matrix[1][0] * coordinate->GetX() + rotation_matrix[1][1] * coordinate->GetY() +
                    rotation_matrix[1][2] * coordinate->GetZ() + rotation_matrix[1][3]);
            result->SetZ(rotation_matrix[2][0] * coordinate->GetX() + rotation_matrix[2][1] * coordinate->GetY() +
                    rotation_matrix[2][2] * coordinate->GetZ() + rotation_matrix[2][3]);

            (*it)->SetX(result->GetX());
            (*it)->SetY(result->GetY());
            (*it)->SetZ(result->GetZ());
        }
    }
}

void Coordinate::RotateTorsionalAll(CoordinateVector coordinate_set, double torsion, int pos)
{
    if(coordinate_set.size() < 3)
        return;
    double current_dihedral = 0.0;
    Coordinate* a1 = this;
    Coordinate* a2 = NULL;
    Coordinate* a3 = NULL;
    Coordinate* a4 = NULL;
    if(pos == 1)
    {
        a2 = new Coordinate(coordinate_set.at(0));
        a3 = new Coordinate(coordinate_set.at(1));
        a4 = new Coordinate(coordinate_set.at(2));
    }
    if(pos == -1)
    {
        a2 = new Coordinate(coordinate_set.at(coordinate_set.size()-1));
        a3 = new Coordinate(coordinate_set.at(coordinate_set.size()-2));
        a4 = new Coordinate(coordinate_set.at(coordinate_set.size()-3));
    }

    Coordinate* b1 = new Coordinate(*a2);
    b1->operator -(*a1);
    Coordinate* b2 = new Coordinate(*a3);
    b2->operator -(*a2);
    Coordinate* b3 = new Coordinate(*a4);
    b3->operator -(*a3);
    Coordinate* b4 = new Coordinate(*b2);
    b4->operator *(-1);

    Coordinate* b2xb3 = new Coordinate(*b2);
    b2xb3->CrossProduct(*b3);

    Coordinate* b1_m_b2n = new Coordinate(*b1);
    b1_m_b2n->operator *(b2->length());

    Coordinate* b1xb2 = new Coordinate(*b1);
    b1xb2->CrossProduct(*b2);

    current_dihedral = atan2(b1_m_b2n->DotProduct(*b2xb3), b1xb2->DotProduct(*b2xb3));

    double** torsion_matrix = gmml::GenerateRotationMatrix(b4, a2, current_dihedral - gmml::ConvertDegree2Radian(torsion));


    if(pos == 1)
    {
        for(CoordinateVector::iterator it = coordinate_set.begin() + 2; it != coordinate_set.end(); it++)
        {
            Coordinate* coordinate = (*it);
            Coordinate* result = new Coordinate();
            result->SetX(torsion_matrix[0][0] * coordinate->GetX() + torsion_matrix[0][1] * coordinate->GetY() +
                    torsion_matrix[0][2] * coordinate->GetZ() + torsion_matrix[0][3]);
            result->SetY(torsion_matrix[1][0] * coordinate->GetX() + torsion_matrix[1][1] * coordinate->GetY() +
                    torsion_matrix[1][2] * coordinate->GetZ() + torsion_matrix[1][3]);
            result->SetZ(torsion_matrix[2][0] * coordinate->GetX() + torsion_matrix[2][1] * coordinate->GetY() +
                    torsion_matrix[2][2] * coordinate->GetZ() + torsion_matrix[2][3]);

            (*it)->SetX(result->GetX());
            (*it)->SetY(result->GetY());
            (*it)->SetZ(result->GetZ());
        }
    }
    if(pos == -1)
    {
        for(CoordinateVector::iterator it = coordinate_set.begin(); it != coordinate_set.end() - 2; it++)
        {
            Coordinate* coordinate = (*it);
            Coordinate* result = new Coordinate();
            result->SetX(torsion_matrix[0][0] * coordinate->GetX() + torsion_matrix[0][1] * coordinate->GetY() +
                    torsion_matrix[0][2] * coordinate->GetZ() + torsion_matrix[0][3]);
            result->SetY(torsion_matrix[1][0] * coordinate->GetX() + torsion_matrix[1][1] * coordinate->GetY() +
                    torsion_matrix[1][2] * coordinate->GetZ() + torsion_matrix[1][3]);
            result->SetZ(torsion_matrix[2][0] * coordinate->GetX() + torsion_matrix[2][1] * coordinate->GetY() +
                    torsion_matrix[2][2] * coordinate->GetZ() + torsion_matrix[2][3]);

            (*it)->SetX(result->GetX());
            (*it)->SetY(result->GetY());
            (*it)->SetZ(result->GetZ());
        }
    }
}


Coordinate* Coordinate::ConvertInternalCoordinate2CartesianCoordinate(
            CoordinateVector coordinate_list, double distance, double angle, double torsion)
{
    //std::stringstream logss;
        if(coordinate_list.size() == 0)
        {
            Coordinate* coordinate = new Coordinate();
            return coordinate;
        }
        if(coordinate_list.size() == 1)
        {
            Coordinate* coordinate = new Coordinate(coordinate_list.at(0)->GetX() + distance, 0.0, 0.0);
            return coordinate;
        }
        if(coordinate_list.size() == 2)
        {
            Coordinate* coordinate = new Coordinate(
                coordinate_list.at(1)->GetX() - cos(gmml::ConvertDegree2Radian(angle) * distance),
                sin(gmml::ConvertDegree2Radian(angle)) * distance, 0.0);
            return coordinate;
        }
        else
        {
//! \todo Add these cout statements to the debugging mechanism once the DebugLevel class (or whatever) is implemented. 
            //logss << "Internal to Cartesian.  distance, angle and torsion are:  " << distance << " " << angle << " " << torsion << "\n";
            if (torsion < 0.0 ) torsion += 360.0 ;
            torsion = gmml::PI_DEGREE - torsion;
//logss << "      torsion is now:  " << torsion << "\n";
            Coordinate great_grandparent_vector = Coordinate(coordinate_list.at(0)->GetX(), coordinate_list.at(0)->GetY(), coordinate_list.at(0)->GetZ());
            Coordinate grandparent_vector = Coordinate(coordinate_list.at(1)->GetX(), coordinate_list.at(1)->GetY(), coordinate_list.at(1)->GetZ());
            Coordinate parent_vector = Coordinate(coordinate_list.at(2)->GetX(), coordinate_list.at(2)->GetY(), coordinate_list.at(2)->GetZ());
//logss << "   The three coords are:  " << "\n";
//logss << "      Parent:  " << "\n";
//logss << "         X  :  " << parent_vector.GetX() << "\n";
//logss << "         Y  :  " << parent_vector.GetY() << "\n";
//logss << "         Z  :  " << parent_vector.GetZ() << "\n";
//logss << "      G Parent:  " << "\n";
//logss << "         X  :  " << grandparent_vector.GetX() << "\n";
//logss << "         Y  :  " << grandparent_vector.GetY() << "\n";
//logss << "         Z  :  " << grandparent_vector.GetZ() << "\n";
//logss << "      G G Parent:  " << "\n";
//logss << "         X  :  " << great_grandparent_vector.GetX() << "\n";
//logss << "         Y  :  " << great_grandparent_vector.GetY() << "\n";
//logss << "         Z  :  " << great_grandparent_vector.GetZ() << "\n";
            Coordinate v1 = Coordinate(great_grandparent_vector);
            Coordinate v2 = Coordinate(grandparent_vector);
//logss << "   v1 and v2 start as:  " << "\n";
//logss << "      v1:  " << "\n";
//logss << "         X  :  " << v1.GetX() << "\n";
//logss << "         Y  :  " << v1.GetY() << "\n";
//logss << "         Z  :  " << v1.GetZ() << "\n";
//logss << "      v2:  " << "\n";
//logss << "         X  :  " << v2.GetX() << "\n";
//logss << "         Y  :  " << v2.GetY() << "\n";
//logss << "         Z  :  " << v2.GetZ() << "\n";
            v1.operator-(grandparent_vector);
            v2.operator-(parent_vector);
//logss << "   v1 and v2 after subtraction:  " << "\n";
//logss << "      v1:  " << "\n";
//logss << "         X  :  " << v1.GetX() << "\n";
//logss << "         Y  :  " << v1.GetY() << "\n";
//logss << "         Z  :  " << v1.GetZ() << "\n";
//logss << "      v2:  " << "\n";
//logss << "         X  :  " << v2.GetX() << "\n";
//logss << "         Y  :  " << v2.GetY() << "\n";
//logss << "         Z  :  " << v2.GetZ() << "\n";

            v1.Normalize();
            v2.Normalize();
//logss << "   v1 and v2 after normalization:  " << "\n";
//logss << "      v1:  " << "\n";
//logss << "         X  :  " << v1.GetX() << "\n";
//logss << "         Y  :  " << v1.GetY() << "\n";
//logss << "         Z  :  " << v1.GetZ() << "\n";
//logss << "      v2:  " << "\n";
//logss << "         X  :  " << v2.GetX() << "\n";
//logss << "         Y  :  " << v2.GetY() << "\n";
//logss << "         Z  :  " << v2.GetZ() << "\n";
            double localEpsilon = std::numeric_limits<double>::epsilon( );
            if(   ( fabs(v1.GetX() - v2.GetX()) < localEpsilon ) &&
                  ( fabs(v1.GetY() - v2.GetY()) < localEpsilon ) &&
                  ( fabs(v1.GetZ() - v2.GetZ()) < localEpsilon )  )
            {
//logss << "   localEpsilon is  :  " << localEpsilon << "\n";
//logss << "      v1 - v2 :  " << "\n";
//logss << "         v1.GetX() - v2.GetX()  :  " << v1.GetX() - v2.GetX() << "\n";
//logss << "         v1.GetY() - v2.GetY()  :  " << v1.GetY() - v2.GetY() << "\n";
//logss << "         v1.GetZ() - v2.GetZ()  :  " << v1.GetZ() - v2.GetZ() << "\n";
//logss << "     fabs ( v1 - v2 ) :  " << "\n";
//logss << "         fabs v1.GetX() - v2.GetX()  :  " << fabs ( v1.GetX() - v2.GetX() ) << "\n";
//logss << "         fabs v1.GetY() - v2.GetY()  :  " << fabs ( v1.GetY() - v2.GetY() ) << "\n";
//logss << "         fabs v1.GetZ() - v2.GetZ()  :  " << fabs ( v1.GetZ() - v2.GetZ() ) << "\n";
            
            //! \todo  Add these cout statmentes and associated error messaging to the errors/logging functions when created
            //  These cout statements are actual error messages for any users who use input with these issues.
//                std::cout << "You are trying to set a dihedral based on three colinear atoms." << "\n";
//                std::cout << "   This is an undefined situation, so I am going to bail now. " << "\n";
//                std::cout << "   You probably need to reset how your internal coordinates are defined." << "\n";
//                std::cout << "   Here are the Cartesian coordinates of the three atoms: " << "\n";
//                std::cout << "      Parent:  " << "\n";
//                std::cout << "         X  :  " << parent_vector.GetX() << "\n";
//                std::cout << "         Y  :  " << parent_vector.GetY() << "\n";
//                std::cout << "         Z  :  " << parent_vector.GetZ() << "\n";
//                std::cout << "      G Parent:  " << "\n";
//                std::cout << "         X  :  " << grandparent_vector.GetX() << "\n";
//                std::cout << "         Y  :  " << grandparent_vector.GetY() << "\n";
//                std::cout << "         Z  :  " << grandparent_vector.GetZ() << "\n";
//                std::cout << "      G G Parent:  " << "\n";
//                std::cout << "         X  :  " << great_grandparent_vector.GetX() << "\n";
//                std::cout << "         Y  :  " << great_grandparent_vector.GetY() << "\n";
//                std::cout << "         Z  :  " << great_grandparent_vector.GetZ() << "\n";
//                std::cout << "    Exiting. " << "\n";
                int localErrorCode = 1; // massive confusion or cannot execute order
                exit(localErrorCode);
            }

            Coordinate v1_cross_v2 = Coordinate(v1);
            v1_cross_v2.CrossProduct(v2);
            v1_cross_v2.Normalize();
            double matrix[3][4];
            matrix[0][1] = v1_cross_v2.GetX();
            matrix[1][1] = v1_cross_v2.GetY();
            matrix[2][1] = v1_cross_v2.GetZ();

            matrix[0][2] = v2.GetX();
            matrix[1][2] = v2.GetY();
            matrix[2][2] = v2.GetZ();

            Coordinate v1_cross_v2_cross_v2 = Coordinate(v1_cross_v2);
            v1_cross_v2_cross_v2.CrossProduct(v2);

            matrix[0][0] = v1_cross_v2_cross_v2.GetX();
            matrix[1][0] = v1_cross_v2_cross_v2.GetY();
            matrix[2][0] = v1_cross_v2_cross_v2.GetZ();

            matrix[0][3] = parent_vector.GetX();
            matrix[1][3] = parent_vector.GetY();
            matrix[2][3] = parent_vector.GetZ();

 
            double vVx = distance * sin(gmml::ConvertDegree2Radian(angle)) * cos(gmml::ConvertDegree2Radian(torsion));
            double vVy = distance * sin(gmml::ConvertDegree2Radian(angle)) * sin(gmml::ConvertDegree2Radian(torsion));
            double vVz = distance * cos(gmml::ConvertDegree2Radian(angle));
            Coordinate v = Coordinate(vVx,vVy,vVz);
//! \todo Add these to the debugging mechanism once the DebugLevel class (or whatever) is implemented. 
//logss << "vVx is " << vVx << "\n";
//logss << "vVy is " << vVy << "\n";
//logss << "vVz is " << vVz << "\n";
            double coordx = matrix[0][0] * v.GetX() + matrix[0][1] * v.GetY() + matrix[0][2] * v.GetZ() + matrix[0][3];
            double coordy = matrix[1][0] * v.GetX() + matrix[1][1] * v.GetY() + matrix[1][2] * v.GetZ() + matrix[1][3];
            double coordz = matrix[2][0] * v.GetX() + matrix[2][1] * v.GetY() + matrix[2][2] * v.GetZ() + matrix[2][3];
            Coordinate* coordinate = new Coordinate(coordx,coordy,coordz);
            //logss << "Coords: " << coordx << "  " << coordy << "  " << coordz;
            //gmml::log(__LINE__,__FILE__,gmml::INF,logss.str());
            return coordinate;
        }
}
Coordinate* Coordinate::ConvertCartesianCoordinate2InternalCoordinate(Coordinate* coordinate, CoordinateVector coordinate_list)
{
        if(coordinate_list.size() == 0)
            return new Coordinate();
        if(coordinate_list.size() == 1)
        {
            Coordinate parent_vector = Coordinate(*coordinate_list.at(0));
            double distance = coordinate->Distance(parent_vector);
            return new Coordinate(distance, 0.0, 0.0);
        }
        if(coordinate_list.size() == 2)
        {
            Coordinate grandparent_vector = Coordinate(*coordinate_list.at(0));
            Coordinate parent_vector = Coordinate(*coordinate_list.at(1));
            double distance = coordinate->Distance(parent_vector);

            Coordinate dist_current_parent_vector = Coordinate(*coordinate);
            Coordinate dist_grandparent_parent_vector = Coordinate(grandparent_vector);
            dist_current_parent_vector.operator -(parent_vector);
            dist_grandparent_parent_vector.operator -(parent_vector);
            double dist_current_parent = dist_current_parent_vector.length();
            double dist_grandparent_parent = dist_grandparent_parent_vector.length();
            double dist_current_parent_dot_grandparent_parent = dist_current_parent_vector.DotProduct(dist_grandparent_parent_vector);
            double angle = gmml::ConvertRadian2Degree(acos(
                dist_current_parent_dot_grandparent_parent/(dist_current_parent * dist_grandparent_parent)));

            return new Coordinate(distance, angle, 0.0);
        }
        else
        {
            Coordinate greatgrandparent_vector = Coordinate(*coordinate_list.at(0));
            Coordinate grandparent_vector = Coordinate(*coordinate_list.at(1));
            Coordinate parent_vector = Coordinate(*coordinate_list.at(2));
            double distance = coordinate->Distance(parent_vector);

            Coordinate dist_current_parent_vector = Coordinate(*coordinate);
            Coordinate dist_grandparent_parent_vector = Coordinate(grandparent_vector);
            dist_current_parent_vector.operator -(parent_vector);
            dist_grandparent_parent_vector.operator -(parent_vector);
            double dist_current_parent = dist_current_parent_vector.length();
            double dist_grandparent_parent = dist_grandparent_parent_vector.length();
            double dist_current_parent_dot_grandparent_parent = dist_current_parent_vector.DotProduct(dist_grandparent_parent_vector);
            double angle = gmml::ConvertRadian2Degree(acos(
                dist_current_parent_dot_grandparent_parent/(dist_current_parent * dist_grandparent_parent)));

            Coordinate dist_parent_current_vector = Coordinate(parent_vector);
            dist_parent_current_vector.operator -(*coordinate);
            Coordinate dist_grandparent_parent_vector_1 = Coordinate(grandparent_vector);
            dist_grandparent_parent_vector_1.operator -(parent_vector);
            Coordinate dist_greatgrandparent_grandparent_vector = Coordinate(greatgrandparent_vector);
            dist_greatgrandparent_grandparent_vector.operator -(grandparent_vector);
            Coordinate dist_grandparent_parent_cross_dist_greatgrandparent_grandparent_vector =
                    Coordinate(dist_grandparent_parent_vector);
            dist_grandparent_parent_cross_dist_greatgrandparent_grandparent_vector.CrossProduct(
                dist_greatgrandparent_grandparent_vector);
            Coordinate dist_parent_current_cross_dist_grandparent_parent_vector = Coordinate(dist_parent_current_vector);
            dist_parent_current_cross_dist_grandparent_parent_vector.CrossProduct(dist_grandparent_parent_vector_1);
            Coordinate dist_parent_current_multiply_dist_grandparent_parent_vector = Coordinate(dist_parent_current_vector);
            dist_parent_current_multiply_dist_grandparent_parent_vector.operator *(dist_grandparent_parent_vector.length());

            double torsion = gmml::ConvertRadian2Degree(
                        atan2(dist_parent_current_multiply_dist_grandparent_parent_vector.DotProduct(
                                  dist_grandparent_parent_cross_dist_greatgrandparent_grandparent_vector),
                              dist_parent_current_cross_dist_grandparent_parent_vector.DotProduct(
                                  dist_grandparent_parent_cross_dist_greatgrandparent_grandparent_vector)));

            return new Coordinate(distance, angle, torsion);
        }
}

//////////////////////////////////////////////////////////
//                     DISPLAY FUNCTIONS                //
//////////////////////////////////////////////////////////
void Coordinate::Print(std::ostream& out) const
{
    if(this->CompareTo(Coordinate(gmml::dNotSet, gmml::dNotSet, gmml::dNotSet)) == true)
        out << std::setw(10) << " " << ", " << std::setw(10) << " " << ", " << std::setw(10) << " ";
    else
        out << std::setw(10) << x_ << ", " << std::setw(10) << y_ << ", " << std::setw(10) << z_;
}

std::string Coordinate::ToString() const
{
    std::stringstream ss;
    this->Print(ss);
    return ss.str();
}

