#include "includes/CentralDataStructure/Readers/Pdb/SectionClasses/titleRecord.hpp"
#include "includes/CodeUtils/strings.hpp" //Trim removeMultipleSpaces
#include <iostream>
#include <sstream>
#include <iomanip> //setw
#include <cmath> //ceil

using pdb::TitleRecord;
//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////
TitleRecord::TitleRecord(std::string record_name, std::string title)
{
    name_ = record_name;
    title_ = title;
}

TitleRecord::TitleRecord(std::stringstream& stream_block)
{
    std::string line;
    bool is_name_set = false;
    std::stringstream ss;
    getline(stream_block, line);
    std::string temp = line;
    while (!codeUtils::Trim(temp).empty())
    {
        if(!is_name_set){
            name_ = line.substr(0,6);
            codeUtils::Trim(name_);
            is_name_set=true;
        }
        ss << line.substr(10,70);

        getline(stream_block, line);
        temp = line;
    }
    this->SetTitle( ss.str() );
}

//////////////////////////////////////////////////////////
//                       ACCESSOR                       //
//////////////////////////////////////////////////////////
const std::string& TitleRecord::GetRecordName() const
{
    return name_;
}
const std::string& TitleRecord::GetTitle() const
{
    return title_;
}

//////////////////////////////////////////////////////////
//                       MUTATOR                        //
//////////////////////////////////////////////////////////
void TitleRecord::SetName(std::string record_name)
{
    name_ = record_name;
}

void TitleRecord::SetTitle(const std::string title)
{
    this->title_ = title;
	codeUtils::Trim( this->title_ );
	codeUtils::removeMultipleSpaces( this->title_ );
}

//////////////////////////////////////////////////////////
//                        FUNCTIONS                     //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
//                      DISPLAY FUNCTION                //
//////////////////////////////////////////////////////////
void TitleRecord::Print(std::ostream &out) const
{
    out << "Record Name: " << name_ << ", Title: " << title_ << std::endl << std::endl;
}

void TitleRecord::Write(std::ostream &stream) const
{ // OG: I just copied this...
    const int MAX_TITLE_LENGTH_IN_LINE = 70;
    stream << std::left << std::setw(6) << this->GetRecordName()
                   << std::left << std::setw(2) << " ";
    if((int)this->GetTitle().length() > MAX_TITLE_LENGTH_IN_LINE)
    {
        stream << std::right << std::setw(2) << " "
                << std::left << std::setw(70) << this->GetTitle().substr(0,MAX_TITLE_LENGTH_IN_LINE)
                << std::endl;

        int counter = ceil((double)(this->GetTitle().length()) / MAX_TITLE_LENGTH_IN_LINE);
        for(int i = 2; i <= counter; i++)
        {
            if(i != counter)
            {
                stream << std::left << std::setw(6) << this->GetRecordName()
                               << std::left << std::setw(2) << " "
                               << std::right << std::setw(2) << i
                               << std::left << std::setw(70) << this->GetTitle().substr(MAX_TITLE_LENGTH_IN_LINE*(i-1), MAX_TITLE_LENGTH_IN_LINE)
                               << std::endl;
            }
            else
            {
                stream << std::left << std::setw(6) << this->GetRecordName()
                               << std::left << std::setw(2) << " "
                               << std::right << std::setw(2) << i
                               << std::left << std::setw(70) << this->GetTitle().substr(MAX_TITLE_LENGTH_IN_LINE*(i-1), this->GetTitle().length()-MAX_TITLE_LENGTH_IN_LINE*(i-1))
                               << std::endl;
            }
        }
    }
    else
    {
        stream << std::right << std::setw(2) << " "
                << std::left << std::setw(70) << this->GetTitle()
                << std::endl;
    }
}
