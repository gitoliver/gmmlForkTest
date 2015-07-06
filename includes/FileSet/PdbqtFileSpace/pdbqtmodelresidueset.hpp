#ifndef PDBQTMODELRESIDUESET_HPP
#define PDBQTMODELRESIDUESET_HPP

#include <string>
#include <vector>
#include <iostream>

namespace PdbqtFileSpace
{
    class PdbqtRootCard;
    class PdbqtBranchCard;
    class PdbqtModelResidueSet
    {
        public:
            //////////////////////////////////////////////////////////
            //                       TYPE DEFINITION                //
            //////////////////////////////////////////////////////////
            /*! \typedef
              * List of atom cards in a pdbqt model residue set
              */
            typedef std::vector<PdbqtBranchCard*> BranchCardVector;

            //////////////////////////////////////////////////////////
            //                       CONSTRUCTOR                    //
            //////////////////////////////////////////////////////////
            /*! \fn
              * Default constructor
              */
            PdbqtModelResidueSet();

            //////////////////////////////////////////////////////////
            //                       ACCESSOR                       //
            //////////////////////////////////////////////////////////
            /*! \fn
              * An accessor function in order to access to the roots in a pdbqt model residue set
              * @return roots_ attribute of the current object of this class
              */
            PdbqtRootCard* GetRoots();
            /*! \fn
              * An accessor function in order to access to the branches in a pdbqt model residue set
              * @return branches_ attribute of the current object of this class
              */
            BranchCardVector GetBranches();

            //////////////////////////////////////////////////////////
            //                       MUTATOR                        //
            //////////////////////////////////////////////////////////
            /*! \fn
              * A mutator function in order to set the roots of the current object
              * Set the roots_ attribute of the current pdbqt model residue set
              * @param rots The roots attribute of the current object
              */
            void SetRoots(PdbqtRootCard* roots);
            /*! \fn
              * A mutator function in order to set the branches of the current object
              * Set the branches_ attribute of the current pdbqt model residue set
              * @param branches The branches of the current object
              */
            void SetBranches(BranchCardVector branches);
            /*! \fn
              * A function in order to add the brnach to the current object
              * Set the branches_ attribute of the current pdbqt model residue set
              * @param branch The branch of the current object
              */
            void AddBranch(PdbqtBranchCard* branch);

            //////////////////////////////////////////////////////////
            //                        FUNCTIONS                     //
            //////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////
            //                       DISPLAY FUNCTION               //
            //////////////////////////////////////////////////////////
            /*! \fn
              * A function to print out the pdbqt model residue set contents in a structural format
              * Print out the information in a defined structure
              * @param out An output stream, the print result will be written in the given output stream
              */
            void Print(std::ostream& out = std::cout);

        private:
            //////////////////////////////////////////////////////////
            //                       ATTRIBUTES                     //
            //////////////////////////////////////////////////////////
            PdbqtRootCard* roots_;                          /*!< List of roots that are in a pdbqt model >*/
            BranchCardVector branches_;       /*!< List of branch cards that are in a pdbqt model >*/

    };
}

#endif // PDBQTMODELRESIDUESET_HPP
