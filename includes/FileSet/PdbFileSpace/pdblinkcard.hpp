// Created by: Delaram Rahbarinia
// Modified by: Alireza Khatamian, Delaram Rahbarinia

#ifndef PDBLINKCARD_HPP
#define PDBLINKCARD_HPP

#include <string>
#include <vector>
#include <sstream>

namespace PdbFileSpace
{
    class PdbLink;

    class PdbLinkCard
    {
        public:
            //////////////////////////////////////////////////////////
            //                       TYPE DEFINITION                //
            //////////////////////////////////////////////////////////
            typedef std::vector< PdbLink* > LinkVector;

            //////////////////////////////////////////////////////////
            //                       CONSTRUCTOR                    //
            //////////////////////////////////////////////////////////
            /*! \fn
              * Default constructor
              */
            PdbLinkCard();
            PdbLinkCard(std::stringstream& stream_block);

            //////////////////////////////////////////////////////////
            //                       ACCESSOR                       //
            //////////////////////////////////////////////////////////
            /*! \fn
              * An accessor function in order to access to the record name in a link card
              * @return record_name_ attribute of the current object of this class
              */
            std::string GetRecordName();
            /*! \fn
              * An accessor function in order to access to the resdiue links in a link card
              * @return resdiue_links_ attribute of the current object of this class
              */
            LinkVector GetResidueLinks();

            //////////////////////////////////////////////////////////
            //                       MUTATOR                        //
            //////////////////////////////////////////////////////////
            /*! \fn
              * A mutator function in order to set the record name of the current object
              * Set the record_name_ attribute of the current link card
              * @param record_name The record name of the current object
              */
            void SetRecordName(std::string record_name);
            /*! \fn
              * A mutator function in order to set the residue links of the current object
              * Set the residue_links_ attribute of the current link card
              * @param residue_links The residue links of the current object
              */
            void SetResidueLinks(const LinkVector residue_links);
            /*! \fn
              * A function in order to add the residue link to the current object
              * Set the residue_link_ attribute of the current link card
              * @param residue_link The residue link of the current object
              */
            void AddResidueLink(PdbLink* residue_link);

            //////////////////////////////////////////////////////////
            //                        FUNCTIONS                     //
            //////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////
            //                       DISPLAY FUNCTION               //
            //////////////////////////////////////////////////////////

        private:
            //////////////////////////////////////////////////////////
            //                       ATTRIBUTES                     //
            //////////////////////////////////////////////////////////
            std::string record_name_;
            LinkVector residue_links_;

    };
}

#endif // PDBLINKCARD_HPP
