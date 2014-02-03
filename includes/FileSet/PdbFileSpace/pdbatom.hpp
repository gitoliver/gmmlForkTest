// Created by: Delaram Rahbarinia
// Modified by: Alireza Khatamian, Delaram Rahbarinia

#ifndef PDBATOM_HPP
#define PDBATOM_HPP

#include <string>
#include "../../../includes/Geometry/coordinate.hpp"

namespace PdbFileSpace
{
    class PdbAtom
    {
        public:

            //////////////////////////////////////////////////////////
            //                       Constructor                    //
            //////////////////////////////////////////////////////////            
            /*! \fn
              * Default constructor
              */
            PdbAtom();
            /*! \fn
              * Constructor with required parameters
              * @param line A single line in a pdb file that represents pdb atom in model card
              */
            PdbAtom(std::string& line);

            //////////////////////////////////////////////////////////
            //                       ACCESSOR                       //
            //////////////////////////////////////////////////////////
            /*! \fn
              * An accessor function in order to access to the atom serial number in a pdb atom
              * @return atom_serial_number_ attribute of the current object of this class
              */
            int GetAtomSerialNumber();
            /*! \fn
              * An accessor function in order to access to the atom name in a pdb atom
              * @return atom_name_ attribute of the current object of this class
              */
            std::string GetAtomName();
            /*! \fn
              * An accessor function in order to access to the atom alternate location in a pdb atom
              * @return atom_alternate_location_ attribute of the current object of this class
              */
            char GetAtomAlternateLocation();
            /*! \fn
              * An accessor function in order to access to the atom residue name in a pdb atom
              * @return atom_residue_name_ attribute of the current object of this class
              */
            std::string GetAtomResidueName();
            /*! \fn
              * An accessor function in order to access to the atom chain id in a pdb atom
              * @return atom_chain_id_ attribute of the current object of this class
              */
            char GetAtomChainId();
            /*! \fn
              * An accessor function in order to access to the atom residue sequence number in a pdb atom
              * @return atom_residue_sequence_number_ attribute of the current object of this class
              */
            int GetAtomResidueSequenceNumber();
            /*! \fn
              * An accessor function in order to access to the atom insertion code in a pdb atom
              * @return atom_insertion_code_ attribute of the current object of this class
              */
            char GetAtomInsertionCode();
            /*! \fn
              * An accessor function in order to access to the atom orthogonal coordinate in a pdb atom
              * @return atom_orthogonal_coordinate_ attribute of the current object of this class
              */
            Geometry::Coordinate GetAtomOrthogonalCoordinate();
            /*! \fn
              * An accessor function in order to access to the atom occupancy in a pdb atom
              * @return atom_occupancy_ attribute of the current object of this class
              */
            double GetAtomOccupancy();
            /*! \fn
              * An accessor function in order to access to the atom tempreture factor in a pdb atom
              * @return atom_tempreture_factor_ attribute of the current object of this class
              */
            double GetAtomTempretureFactor();
            /*! \fn
              * An accessor function in order to access to the atom element symbol in a pdb atom
              * @return atom_element_symbol_ attribute of the current object of this class
              */
            std::string GetAtomElementSymbol();
            /*! \fn
              * An accessor function in order to access to the atom charge in a pdb atom
              * @return atom_charge_ attribute of the current object of this class
              */
            std::string GetAtomCharge();

            //////////////////////////////////////////////////////////
            //                       MUTATOR                        //
            //////////////////////////////////////////////////////////
            /*! \fn
              * A mutator function in order to set the atom serial number of the current object
              * Set the atom_serial_number_ attribute of the current pdb atom
              * @param atom_serial_number The atom serial number of the current object
              */
            void SetAtomSerialNumber(int atom_serial_number);
            /*! \fn
              * A mutator function in order to set the record name of the current object
              * Set the atom_name_ attribute of the current pdb atom
              * @param atom_name The atom name of the current object
              */
            void SetAtomName(const std::string atom_name);
            /*! \fn
              * A mutator function in order to set the atom alternate location of the current object
              * Set the atom_alternate_location_ attribute of the current pdb atom
              * @param atom_alternate_location The atom alternate location of the current object
              */
            void SetAtomAlternateLocation(char atom_alternate_location);
            /*! \fn
              * A mutator function in order to set the atom residue name of the current object
              * Set the atom_residue_name_ attribute of the current pdb atom
              * @param atom_residue_name The atom residue name of the current object
              */
            void SetAtomResidueName(const std::string atom_residue_name);
            /*! \fn
              * A mutator function in order to set the atom chain id of the current object
              * Set the atom_chain_id_ attribute of the current pdb atom
              * @param atom_chain_id The atom chain id of the current object
              */
            void SetAtomChainId(char atom_chain_id);
            /*! \fn
              * A mutator function in order to set the atom residue sequence number of the current object
              * Set the atom_residue_sequence_number_ attribute of the current pdb atom
              * @param atom_residue_sequence_number The atom residue sequence number of the current object
              */
            void SetAtomResidueSequenceNumber(int atom_residue_sequence_number);
            /*! \fn
              * A mutator function in order to set the atom insertion code of the current object
              * Set the atom_insertion_code_ attribute of the current pdb atom
              * @param atom_insertion_code The atom insertion code of the current object
              */
            void SetAtomInsertionCode(char atom_insertion_code);
            /*! \fn
              * A mutator function in order to set the atom orthogonal coordinate of the current object
              * Set the atom_orthogonal_coordinate_ attribute of the current pdb atom
              * @param atom_orthogonal_coordinate The atom orthogonal coordinate of the current object
              */
            void SetAtomOrthogonalCoordinate(Geometry::Coordinate atom_orthogonal_coordinate);
            /*! \fn
              * A mutator function in order to set the atom occupancy of the current object
              * Set the atom_occupancy_ attribute of the current pdb atom
              * @param atom_occupancy The atom occupancy of the current object
              */
            void SetAtomOccupancy(double atom_occupancy);
            /*! \fn
              * A mutator function in order to set the atom tempreture factor of the current object
              * Set the atom_tempreture_factor_ attribute of the current pdb atom
              * @param atom_tempreture_factor The atom tempreture factor of the current object
              */
            void SetAtomTempretureFactor(double atom_tempreture_factor);
            /*! \fn
              * A mutator function in order to set the atom element symbol of the current object
              * Set the atom_element_symbol_ attribute of the current pdb atom
              * @param atom_element_symbol The atom element symbol of the current object
              */
            void SetAtomElementSymbol(const std::string atom_element_symbol);
            /*! \fn
              * A mutator function in order to set the atom charge of the current object
              * Set the atom_charge_ attribute of the current pdb atom
              * @param atom_charge The atom charge of the current object
              */
            void SetAtomCharge(const std::string atom_charge);

            //////////////////////////////////////////////////////////
            //                       DISPLAY FUNCTION               //
            //////////////////////////////////////////////////////////



        private:
            //////////////////////////////////////////////////////////
            //                       ATTRIBUTES                     //
            //////////////////////////////////////////////////////////
            int atom_serial_number_;
            std::string atom_name_;
            char atom_alternate_location_;
            std::string atom_residue_name_;
            char atom_chain_id_;
            int atom_residue_sequence_number_;
            char atom_insertion_code_;
            Geometry::Coordinate atom_orthogonal_coordinate_;
            double atom_occupancy_;
            double atom_tempreture_factor_;
            std::string atom_element_symbol_;
            std::string atom_charge_;

    };
}

#endif // PDBATOM_HPP
