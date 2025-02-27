#include "../../../../includes/MolecularModeling/assembly.hpp"
#include "../../../../includes/MolecularModeling/Graph/graph.hpp"
#include "includes/CodeUtils/logging.hpp"

#include <regex>

//For an example query with some explaination, see the bottom of this file.  For sparql query information, see https://www.w3.org/TR/rdf-sparql-query/ (It is not the greatest documentation but it helps)

std::string MolecularModeling::Assembly::QueryOntology(std::string searchType, std::string searchTerm, float resolution_min, float resolution_max, float b_factor_min, float b_factor_max, float oligo_b_factor_min, float oligo_b_factor_max, int isError, int isWarning, int isComment, int isLigand, int isGlycomimetic, int isNucleotide, std::string aglycon, std::string count, int page, int resultsPerPage, std::string sortBy, std::string url, std::string output_file_type)
{
    // This function runs a basic query, looking only for ?pdb (PDB_ID), 
    // ?oligo (Oligosaccharides are assigned unique IDs, ie 2dw2_oligo_1), 
    // and ?oligo_sequence (Condensed sequence).
    // These three variables together are displayed initially on GlyFinder.  
    // This function also takes in all of the possible
    // filter variables to return filtered results when updating via ajax
    // This function will also call a function to create a graph from the 
    // search string for searching across branches.

    int local_debug = -1;
    if (local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__, gmml::INF, "Running QueryOntology()");
    }
    std::stringstream query;
    std::stringstream search;
    search << searchType;


    // Create the query string
    query << Ontology::PREFIX << Ontology::SELECT_CLAUSE << "\n";
    // for # results and # pages
    if (count == "TRUE")
    { 
        query << "\t( COUNT( DISTINCT ?oligo) as ?count) \n";
        // query << Ontology::WHERE_CLAUSE << Ontology::SELECT_CLAUSE;
    }
    else
    {
        query << " DISTINCT\n";
        query << "\t?pdb\n";
        query << "\t?oligo_sequence \n";
        query << "\t(STRAFTER(str(?oligo), \"#\") as ?oligo)\n";
    }
    

    if (isComment == 1)
    {
        query << "\t(group_concat(distinct ?comment;separator=\"\\n\") as ?comments)\n";
    }
    if (isWarning == 1)
    {
        query << "\t(group_concat(distinct ?warning;separator=\"\\n\") as ?warnings)\n";
    }
    if (isError == 1)
    {
        query << "\t(group_concat(distinct ?error;separator=\"\\n\") as ?errors)\n";
    }

    query << Ontology::WHERE_CLAUSE;
    query << "?pdb_file\n";
    query << "\t" << Ontology::id                << "\t?pdb;\n";
    query << "\t" << Ontology::hasOligo          << "\t?oligo;\n";
    if ((resolution_max != -1) || (resolution_min != -1))
    {
        query << "\t" << Ontology::hasResolution << "\t?resolution;\n";
    }
    if ((b_factor_max != -1) || (b_factor_min != -1))
    {
        query << "\t" << Ontology::hasBFactor    << "\t?Mean_B_Factor;\n";
    }
    query << "\t" << Ontology::TYPE << "\t" << Ontology::PDB << ".\n";

    //These should be left early in the query, as they filter out a lot of results
    if (search.str() == "PDB")
    {        
        query << "VALUES ?pdb { \"" << searchTerm << "\" }\n";
    }
    if (resolution_max != -1)
    {
        query << "FILTER (" << resolution_max << " > ?resolution)\n";
    }
    if (resolution_min != -1)
    {
        query << "FILTER (" << resolution_min << " < ?resolution)\n";
    }
    if (b_factor_max != -1)
    {
        query << "FILTER (" << b_factor_max << " > ?Mean_B_Factor)\n";
    }
    if (b_factor_min != -1)
    {
        query << "FILTER (" << b_factor_min << " < ?Mean_B_Factor)\n";
    }

    query << "?oligo\n";
    query << "\t" << Ontology::oligo_IUPAC_name  << "\t?oligo_sequence.\n";
    
    if (search.str() == "Condensed_Sequence")
    {
        gmml::FindReplaceString(searchTerm, "[", "\\\\[");
        gmml::FindReplaceString(searchTerm, "]", "\\\\]");
        gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
        query << "VALUES ?oligo_sequence { \"" << searchTerm << "\" }\n";
    }

    // Create query from graph
    if (search.str() == "Oligo_REGEX")
    {
        gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
        gmml::FindReplaceString(searchTerm, ".*", "*");
        while (searchTerm.find("**") != std::string::npos)
        {
            // gmml::FindReplaceString(searchTerm, "**", "*");
            searchTerm.replace(searchTerm.find("**"), 2, "*");
        }
        if (local_debug > 0)
        {
            gmml::log(__LINE__, __FILE__, gmml::INF, searchTerm);
        }
        while (searchTerm.find("1-*") != std::string::npos)
        {
            if (local_debug > 0)
            {
                gmml::log(__LINE__, __FILE__, gmml::INF, "Found 1-* still");
            }
            searchTerm.replace(searchTerm.find("1-*"), 3, "1-0");
        }
        while (searchTerm.find("2-*") != std::string::npos)
        {
            if (local_debug > 0)
            {
                gmml::log(__LINE__, __FILE__, gmml::INF, "Found 2-* still");
            }
            searchTerm.replace(searchTerm.find("2-*"), 3, "2-0");
        }
        while (searchTerm.find("[*") != std::string::npos)
        {
            searchTerm.replace(searchTerm.find("[*"), 2, "[");
        }
        if (local_debug > 0)
        {
            gmml::log(__LINE__, __FILE__, gmml::INF, searchTerm);
        }
        if (searchTerm != "*")
        {
            while (searchTerm[0] == '*')
            {
                // Subgraph match will always find other nodes at the beginning
                searchTerm = searchTerm.substr(1); // char 1 to end (removes first char [0])
            }
            while (searchTerm[searchTerm.size() - 1] == '*')
            {
                // Subgraph match will always find other nodes at the end
                searchTerm = searchTerm.substr(0, searchTerm.size() - 1); // start at the beginning, get 1 less than the # of chars there (remove the last char)
            }
            GraphDS::Graph queryGraph = CreateQueryStringGraph(searchTerm);
            if (local_debug > 0)
            {
                std::stringstream logSS;
                queryGraph.Print(logSS);
                gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
            }
            ConvertGraphToQuery(query, queryGraph);
        }
        else
        {
            // TODO: make query return everything
            // Does it already with no additional code? Looks like it!
        }
    }


    // Filters

    if (isLigand == 1)
    {
        query << "?oligo\t" << Ontology::isAttachedToProtein << "\tfalse.\n";
    }
    else if (isLigand == 0)
    {
        query << "?oligo\t" << Ontology::isAttachedToProtein << "\ttrue.\n";
    }

    if (isGlycomimetic == 1)
    {
        query << "FILTER regex(?oligo_sequence, \"<R\")\n";
    }
    else if (isGlycomimetic == 0)
    {
        query << "FILTER (!regex(?oligo_sequence, \"<R\"))\n";
    }

    if (aglycon.length() > 0)
    {
        // This could probably be done better
        gmml::FindReplaceString(aglycon, "-OH", "-ROH");
        query << "FILTER regex(?oligo_sequence, \"" << aglycon << "$\")\n";
    }

    if ((oligo_b_factor_max != -1) | (oligo_b_factor_min != -1))
    {
        query << "?oligo\t" << Ontology::hasBFactor << "?oligo_mean_B_Factor.\n";
    }
    if (oligo_b_factor_max != -1)
    {
        query << "FILTER (" << oligo_b_factor_max << " > ?oligo_mean_B_Factor)\n";
    }
    if (oligo_b_factor_min != -1)
    {
        query << "FILTER (" << oligo_b_factor_min << " < ?oligo_mean_B_Factor)\n";
    }
    if (isError == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "?errorNote\t" << Ontology::note_type << "\t\"error\";\n";
        query << Ontology::note_description << "\t?error.\n";
    }
    else if (isError == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "FILTER NOT EXISTS { ?errorNote " << Ontology::note_type << " \"error\".}\n}\n";
    }
    if (isWarning == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "?warningNote\t" << Ontology::note_type << "\t\"warning\";\n";
        query << Ontology::note_description << "\t?warning.\n";
    }
    else if (isWarning == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "FILTER NOT EXISTS { ?warningNote " << Ontology::note_type << " \"warning\".}\n}\n";
    }
    if (isComment == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )\t";
        query << " ?commentNote.\n";
        query << "?commentNote\t" << Ontology::note_type << "\t\"comment\";\n";
        query << Ontology::note_description << "\t?comment.\n";
    }
    else if (isComment == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?commentNote.\n";
        query << "FILTER NOT EXISTS { ?commentNote " << Ontology::note_type << " \"comment\".}\n}\n";
    }

    //If we need to filter data instead of cleaning it

    // query << "MINUS\n{\n";
    // query << "?pdb_file     :identifier             ?pdb.\n";
    // query << "?pdb_file     :hasOligo               ?oligo.\n";
    // query << "?oligo        :hasMono                ?mono.\n";
    // query << "?oligo        :oligoIUPACname         ?oligo_sequence.\n";
    // query << "?oligo :hasSequenceResidue ?residue1.\n";
    // query << "?residue1 :monosaccharideShortName ?mono1.\n";
    // query << "FILTER regex(?mono1, \"Ribf.*\")\n";
    // query << "?residue1 :isConnectedTo ?terminal.\n";
    // query << "?oligo :hasTerminal ?terminal.\n";
    // query << "?terminal :identifier ?terminal_name.\n";
    // query << "FILTER regex(?terminal_name, \".*Unknown\")\n}\n";
    // if(searchTerm == "*")
    // {
        //for now need something to filter out false positives
        query << "?oligo\t" << Ontology::hasMono << " / " << Ontology::isSaccharide << " true.\n";
    // }

    query << Ontology::END_WHERE_CLAUSE << "\n";
    // if (count == "TRUE")
    // {
    //     query << Ontology::END_WHERE_CLAUSE << "\n";
    // }
    if (count != "TRUE")
    {
        if(sortBy != "undefined")
        {
            query << "ORDER BY  ?" << sortBy << "\n";
        }
        if (resultsPerPage != -1)
        {
            query << "LIMIT  " << resultsPerPage << "\n";
        }
        query << "OFFSET " << resultsPerPage * (page - 1) << "\n";
        
    }
    if (local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__, gmml::INF, query.str());
        gmml::log(__LINE__, __FILE__, gmml::INF, "Done running QueryOntology()");
    }
    return FormulateCURLGF(output_file_type, query.str(), url);
}

std::string MolecularModeling::Assembly::MoreQuery(std::string pdb_id, std::string oligo_sequence, std::string oligo, std::string url, std::string output_file_type)
{ 
    // This function runs a full query on a single result, which is unique given the pdb_id, oligo_sequence, and oligo 
    int local_debug = -1;
    if(local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__, gmml::INF, "Running MoreQuery()");
    }

    //This is silly, group concats all r's and bind/trim their iri's to know which is which
    int numRgroups = std::count(oligo_sequence.begin(), oligo_sequence.end(), '<');
    std::stringstream query;
    query << Ontology::PREFIX << Ontology::SELECT_DISTINCT << "\n";
    query << "\t" << "?oligo" << "\n";
    query << "\t" << "?residue_links" << "\n";
    query << "\t" << "?title" << "\n";
    query << "\t" << "?resolution" << "\n";
    query << "\t" << "?Mean_B_Factor" << "\n";
    query << "\t" << "?oligo_mean_B_Factor" << "\n";
    query << "\t" << "?authors" << "\n";
    query << "\t" << "?journal" << "\n";
    query << "\t" << "?PMID" << "\n";
    query << "\t" << "?DOI" << "\n";
    query << "\t" << "?pdb_coordinates" << "\n";
    query << "\t" << "?ProteinID" << "\n";


    if(numRgroups > 0)
    {
        for(int i = 0; i < numRgroups; i++)
        {
        query << " ?R" << i + 1;
        }
        query <<  "(group_concat(distinct ?rGroup;separator=\"\\n\") as ?rGroups)\n";
    }
    query << "\t" << "(group_concat(distinct ?comment;separator=\"\\n\") as ?comments)\n";
    query << "\t" << "(group_concat(distinct ?warning;separator=\"\\n\") as ?warnings)\n";
    query << "\t" << "(group_concat(distinct ?error;separator=\"\\n\") as ?errors)\n\n";

    query << Ontology::WHERE_CLAUSE;
    query << "?pdb_file"
          << "\n\t" << Ontology::id << "\t\"" << pdb_id << "\";\n";

    query << "\t" << Ontology::hasOligo << "\t?oligo.\n";
    query << "VALUES ?oligo { " << Ontology::ONT_PREFIX << oligo << " }\n";

    //   gmml::FindReplaceString(oligo_sequence, "-OH", "-ROH");
    //   query << "?oligo        :oligoIUPACname     \"" << oligo_sequence << "\".\n";

    query << "?pdb_file     :hasTitle               ?title;\n";
    query << "              :hasAuthors             ?authors.\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasJournal             ?journal.}\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasProteinID           ?ProteinID.}\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasDOI                 ?DOI.}\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasPMID                ?PMID.}\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasResolution          ?resolution.}\n";
    query << "OPTIONAL {";
    query << "?pdb_file     :hasBFactor             ?Mean_B_Factor.}\n";
    query << "OPTIONAL {";
    query << "?oligo        :oligoResidueLinks      ?residue_links.}\n";
    query << "OPTIONAL {";
    query << "?oligo        :hasBFactor           ?oligo_mean_B_Factor.}\n";
    query << "?oligo        :PDBfile           ?pdb_coordinates.\n";
    if (numRgroups > 0)
    { // There are chemical modifications that need to be returned to the user
        for (int i = 0; i < numRgroups; i++)
        {
        query << "?oligo      :hasR" << i + 1 << "     ?Rgroup" << i + 1 << ".\n";
        query << "?Rgroup" << i + 1 << "   :hasFormula      ?R" << i + 1 << ".\n";
        }
    }
    query << "?oligo        :hasMono            ?mono.\n";
    // query << "OPTIONAL {";
    // query << "?linkage      :hasParent 	            ?oligo;\n";
    // query << "              :glycosidicLinkage      ?glycosidic_linkage.}\n";
    query << "OPTIONAL {";
    // query << "?pdb_file\n\t";
    query << "?oligo\n\t";
    query << "( "  << Ontology::hasNote << " | ";
    // query << Ontology::hasOligo << " | ";
    query << Ontology::hasMono << " )*\t";
    query << " ?errorNote.\n";
    query << "?errorNote\n\t" << Ontology::note_type << "\t\"error\";\n\t";
    query << Ontology::note_description << "\t?error.\n}\n";
    query << "OPTIONAL {";
    // query << "?pdb_file\n\t";
    query << "?oligo\n\t";
    query << "( "  << Ontology::hasNote << " | ";
    // query << Ontology::hasOligo << " | ";
    query << Ontology::hasMono << " )*\t";
    query << " ?warningNote.\n";
    query << "?warningNote\n\t" << Ontology::note_type << "\t\"warning\";\n\t";
    query << Ontology::note_description << "\t?warning.\n}\n";
    query << "OPTIONAL {";
    // query << "?pdb_file\n\t";
    query << "?oligo\n\t";
    query << "( "  << Ontology::hasNote << " | ";
    // query << Ontology::hasOligo << " | ";
    query << Ontology::hasMono << " )*\t";
    query << " ?commentNote.\n";
    query << "?commentNote\n\t" << Ontology::note_type << "\t\"comment\";\n\t";
    query << Ontology::note_description << "\t?comment.\n}\n";



    //add info for coordinates here
    query << Ontology::END_WHERE_CLAUSE << "\n";
    if(local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__, gmml::INF, query.str());
    }
    // std::cout << "\n" << query.str() << "\n";
    return FormulateCURLGF(output_file_type, query.str(), url);

}

std::string MolecularModeling::Assembly::ontologyPDBDownload(std::string searchType, std::string searchTerm, float resolution_min, float resolution_max, float b_factor_min, float b_factor_max, float oligo_b_factor_min, float oligo_b_factor_max, int isError, int isWarning, int isComment, int isLigand, int isGlycomimetic, int isNucleotide, std::string aglycon, std::string count, int page, int resultsPerPage, std::string sortBy, std::string url, std::string output_file_type)
{ // This query creates a list of unique PDB_IDs given all of the user specified filters, and returns a CSV which is downloaded
  int local_debug = -1;
  if(local_debug > 0)
  {
    gmml::log(__LINE__, __FILE__, gmml::INF, "Running ontologyPDBDownload()");
  }
  std::stringstream query;
  std::stringstream search;
  search << searchType;

  query << Ontology::PREFIX << Ontology::SELECT_CLAUSE;
  query << " DISTINCT ?PDB_ID \n";
  query << "(group_concat(distinct ?oligo_sequence;separator=\"\\n\") as ?Oligosaccharides) ";
  if(isComment == 1)
  {
    query << "(group_concat(distinct ?comment;separator=\"\\n\") as ?comments) ";
  }
  if(isWarning == 1)
  {
    query << "(group_concat(distinct ?warning;separator=\"\\n\") as ?warnings) ";
  }
  if(isError == 1)
  {
     query << "(group_concat(distinct ?error;separator=\"\\n\") as ?errors)\n";
  }
  query << Ontology::WHERE_CLAUSE;
  query << "?pdb_file     :identifier             ?PDB_ID.\n";
  if(search.str()=="PDB")
  {
    query << "VALUES ?PDB_ID { \"" << searchTerm << "\" }\n";
  }
  if((resolution_max != -1) | (resolution_min != -1))
  {
    query << "?pdb_file     :hasResolution          ?resolution.\n";
  }
  if(resolution_max != -1)
  {
    query << "FILTER (" << resolution_max << " > ?resolution)\n";
  }
  if(resolution_min != -1)
  {
    query << "FILTER (" << resolution_min << " < ?resolution)\n";
  }
  if((b_factor_max != -1) | (b_factor_min != -1))
  {
    query << "?pdb_file     :hasBFactor             ?Mean_B_Factor.\n";
  }
  if(b_factor_max != -1)
  {
    query << "FILTER (" << b_factor_max << " > ?Mean_B_Factor)\n";
  }
  if(b_factor_min != -1)
  {
    query << "FILTER (" << b_factor_min << " < ?Mean_B_Factor)\n";
  }
  query << "?pdb_file     :hasOligo               ?oligo.\n";
  query << "?oligo        :oligoIUPACname              ?oligo_sequence.\n";
  if(search.str()=="Oligo_REGEX")
  {
    gmml::FindReplaceString(searchTerm, "[", "\\\\[");
    gmml::FindReplaceString(searchTerm, "]", "\\\\]");
    gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
    query << "FILTER regex(?oligo_sequence, \"" << searchTerm << "\")\n";
  }
  if(search.str()=="Condensed_Sequence")
  {
    gmml::FindReplaceString(searchTerm, "[", "\\\\[");
    gmml::FindReplaceString(searchTerm, "]", "\\\\]");
    gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
    query << "VALUES ?oligo_sequence { \"" << searchTerm << "\" }\n";
  }
  if(isLigand == 1)
  {
    query << "FILTER (!regex(?oligo_sequence, \"-ASN$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-THR$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-SER$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-LYZ$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-HYP$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-TYR$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-CYS$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-TRP$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-LYS$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-HIS$\"))\n";
  }
  else if(isLigand == 0)
  {
    query << "FILTER (!regex(?oligo_sequence, \"-ROH$\"))\n";
    query << "FILTER (!regex(?oligo_sequence, \"-OME$\"))\n";
    query << "?oligo    :oligoSequenceName     ?sequenceName.\n";
    query << "FILTER (!regex(?sequenceName, \"-Unknown$\"))\n";
  }
  // if(isNucleotide == 1)
  // {
  //   query << "?mono         :isNucleotide  \"true\"\n";
  // }
  // else if(isNucleotide == 0)
  // {
  //   query << "?mono         :isNucleotide  \"false\"\n";
  // }
  if(isGlycomimetic == 1)
  {
    query << "FILTER regex(?oligo_sequence, \"<R\")\n";
  }
  else if(isGlycomimetic == 0)
  {
    query << "FILTER (!regex(?oligo_sequence, \"<R\"))\n";
  }

  if(aglycon.length() > 0)
  {
    query << "FILTER regex(?oligo_sequence, \"" << aglycon << "$\")\n";
  }
  if((oligo_b_factor_max != -1) | (oligo_b_factor_min != -1))
  {
    query << "?oligo\n\t" << Ontology::hasBFactor << "\t?oligo_mean_B_Factor.\n";
  }
  if(oligo_b_factor_max != -1)
  {
    query << "FILTER (" << oligo_b_factor_max << " > ?oligo_mean_B_Factor)\n";
  }
  if(oligo_b_factor_min != -1)
  {
    query << "FILTER (" << oligo_b_factor_min << " < ?oligo_mean_B_Factor)\n";
  }
  if (isError == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "?errorNote\t" << Ontology::note_type << "\t\"error\";\n";
        query << Ontology::note_description << "\t?error.\n";
    }
    else if (isError == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "FILTER NOT EXISTS { ?errorNote " << Ontology::note_type << " \"error\".}\n}\n";
    }
    if (isWarning == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "?warningNote\t" << Ontology::note_type << "\t\"warning\";\n";
        query << Ontology::note_description << "\t?warning.\n";
    }
    else if (isWarning == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "FILTER NOT EXISTS { ?warningNote " << Ontology::note_type << " \"warning\".}\n}\n";
    }
    if (isComment == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )\t";
        query << " ?commentNote.\n";
        query << "?commentNote\t" << Ontology::note_type << "\t\"comment\";\n";
        query << Ontology::note_description << "\t?comment.\n";
    }
    else if (isComment == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?commentNote.\n";
        query << "FILTER NOT EXISTS { ?commentNote " << Ontology::note_type << " \"comment\".}\n}\n";
    }

  query << Ontology::END_WHERE_CLAUSE << "\n";
  query << "ORDER BY  ?" << sortBy << "\n";

  if(local_debug > 0)
  {
    gmml::log(__LINE__, __FILE__, gmml::INF, query.str());
    gmml::log(__LINE__, __FILE__, gmml::INF, "Done running ontologyPDBDownload()");
  }

  return FormulateCURLGF(output_file_type, query.str(), url);
}

std::string MolecularModeling::Assembly::ontologyDownload(std::string searchType, std::string searchTerm, float resolution_min, float resolution_max, float b_factor_min, float b_factor_max, float oligo_b_factor_min, float oligo_b_factor_max, int isError, int isWarning, int isComment, int isLigand, int isGlycomimetic, int isNucleotide, std::string aglycon, std::string count, int page, int resultsPerPage, std::string sortBy, std::string url, std::string output_file_type)
{ 
    //This is a complete (and therefore slow) query that is a combination of moreQuery() and QueryOntology().  It filters the database by user input, and returns a CSV with all of the data for download.
    int local_debug = -1;
    if(local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__, gmml::INF, "Running ontologyDownload()");
    }

    std::stringstream query;
    std::stringstream search;
    search << searchType;

    query << Ontology::PREFIX;
    query << Ontology::SELECT_CLAUSE << " DISTINCT\n";
    query << "\t?pdb\n";
    query << "\t?title\n";
    query << "\t?DOI\n";
    query << "\t?resolution\n";
    query << "\t?Mean_B_Factor\n";
    query << "\t(STRAFTER(str(?oligo), \"#\") as ?oligo)\n";
    query << "\t?oligo_sequence \n";
    query << "\t?residue_links\n";
    query << "\t?oligo_mean_B_Factor\n";
    query << "(group_concat(distinct ?comment;separator=\"\\n\") as ?comments)\n";
    query << "(group_concat(distinct ?warning;separator=\"\\n\") as ?warnings)\n";
    query << "(group_concat(distinct ?error;separator=\"\\n\") as ?errors)\n\n";
    query << Ontology::WHERE_CLAUSE;

    query << "?pdb_file\n";
    query << "\t" << Ontology::id               << "\t?pdb;\n";
    query << "\t" << Ontology::hasOligo         << "\t?oligo;\n";
    query << "\t" << Ontology::hasResolution    << "\t?resolution;\n";
    query << "\t" << Ontology::hasBFactor       << "\t?Mean_B_Factor;\n";
    query << "\t" << Ontology::hasTitle         << "\t?title;\n";
    query << "\t" << Ontology::hasDOI           << "\t?DOI;\n";
    query << "\t" << Ontology::TYPE << "\t" << Ontology::PDB << ".\n";

  
    if(search.str()=="PDB")
    {
        query << "VALUES ?pdb { \"" << searchTerm << "\" }\n";
    }
    if(resolution_max != -1)
    {
        query << "FILTER (" << resolution_max << " > ?resolution)\n";
    }
    if(resolution_min != -1)
    {
        query << "FILTER (" << resolution_min << " < ?resolution)\n";
    }
    if(b_factor_max != -1)
    {
        query << "FILTER (" << b_factor_max << " > ?Mean_B_Factor)\n";
    }
    if(b_factor_min != -1)
    {
        query << "FILTER (" << b_factor_min << " < ?Mean_B_Factor)\n";
    }
    query << "?oligo\n";
    query << "\t" << Ontology::oligo_IUPAC_name;
    query << "\t?oligo_sequence.\n";


    if(search.str()=="Condensed_Sequence")
    {
        gmml::FindReplaceString(searchTerm, "[", "\\\\[");
        gmml::FindReplaceString(searchTerm, "]", "\\\\]");
        gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
        query << "VALUES ?oligo_sequence { \"" << searchTerm << "\" }\n";
    }
    else if(search.str()=="Oligo_REGEX")
    {
        gmml::FindReplaceString(searchTerm, "-OH", "-ROH");
        gmml::FindReplaceString(searchTerm, ".*", "*");
        while (searchTerm.find("**") != std::string::npos)
        {
            // gmml::FindReplaceString(searchTerm, "**", "*");
            searchTerm.replace(searchTerm.find("**"), 2, "*");
        }
        if (local_debug > 0)
        {
            gmml::log(__LINE__, __FILE__, gmml::INF, searchTerm);
        }
        while (searchTerm.find("1-*") != std::string::npos)
        {
            if (local_debug > 0)
            {
                gmml::log(__LINE__, __FILE__, gmml::INF, "Found 1-* still");
            }
            searchTerm.replace(searchTerm.find("1-*"), 3, "1-0");
        }
        while (searchTerm.find("2-*") != std::string::npos)
        {
            if (local_debug > 0)
            {
                gmml::log(__LINE__, __FILE__, gmml::INF, "Found 2-* still");
            }
            searchTerm.replace(searchTerm.find("2-*"), 3, "2-0");
        }
        while (searchTerm.find("[*") != std::string::npos)
        {
            searchTerm.replace(searchTerm.find("[*"), 2, "[");
        }
        if (local_debug > 0)
        {
            gmml::log(__LINE__, __FILE__, gmml::INF, searchTerm);
        }
        if (searchTerm != "*")
        {
            while (searchTerm[0] == '*')
            {
                // Subgraph match will always find other nodes at the beginning
                searchTerm = searchTerm.substr(1); // char 1 to end (removes first char [0])
            }
            while (searchTerm[searchTerm.size() - 1] == '*')
            {
                // Subgraph match will always find other nodes at the end
                searchTerm = searchTerm.substr(0, searchTerm.size() - 1); // start at the beginning, get 1 less than the # of chars there (remove the last char)
            }
            GraphDS::Graph queryGraph = CreateQueryStringGraph(searchTerm);
            if (local_debug > 0)
            {
                std::stringstream logSS;
                queryGraph.Print(logSS);
                gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
            }
            ConvertGraphToQuery(query, queryGraph);
        }
        else
        {
            // TODO: make query return everything
            // Does it already with no additional code? Looks like it!
        }
    }
  
//   query << "OPTIONAL {";
//   query << "?oligo        :oligoResidueLinks      ?residue_links.}\n";
//   query << "OPTIONAL {";
//   query << "?oligo        :oligoBFactor           ?oligo_mean_B_Factor.\n";
//   if(oligo_b_factor_max != -1)
//   {
//     query << "FILTER (" << oligo_b_factor_max << " > ?oligo_mean_B_Factor)\n";
//   }
//   if(oligo_b_factor_min != -1)
//   {
//     query << "FILTER (" << oligo_b_factor_min << " < ?oligo_mean_B_Factor)\n";
//   }
//   query << "}\n";

    query << "?oligo\n\t";
    query << Ontology::hasBFactor << "\t?oligo_mean_B_Factor;\n\t";
    query << Ontology::oligo_residue_linkages << "\t?residue_links;\n\t";

    if(isLigand == 1)
    {
        query << Ontology::isAttachedToProtein << "\t\"false\";\n\t";
    }
    else if(isLigand == 0)
    {
        query << Ontology::isAttachedToProtein << "\t\"true\";\n\t";
    }

    query << Ontology::TYPE << "\t" << Ontology::Oligosaccharide << ".\n\n";
    

    // Filters

    if(isGlycomimetic == 1)
    {
        query << "FILTER regex(?oligo_sequence, \"<R\")\n";
    }
    else if(isGlycomimetic == 0)
    {
        query << "FILTER (!regex(?oligo_sequence, \"<R\"))\n";
    }

    if(aglycon.length() > 0)
    {
        query << "FILTER regex(?oligo_sequence, \"" << aglycon << "$\")\n";
    }

    if(oligo_b_factor_max != -1)
    {
        query << "FILTER (" << oligo_b_factor_max << " > ?oligo_mean_B_Factor)\n";
    }
    if(oligo_b_factor_min != -1)
    {
        query << "FILTER (" << oligo_b_factor_min << " < ?oligo_mean_B_Factor)\n";
    }

    if (isError == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "?errorNote\t" << Ontology::note_type << "\t\"error\";\n";
        query << Ontology::note_description << "\t?error.\n";
    }
    else if (isError == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?errorNote.\n";
        query << "FILTER NOT EXISTS { ?errorNote " << Ontology::note_type << " \"error\".}\n}\n";
    }
    if (isWarning == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "?warningNote\t" << Ontology::note_type << "\t\"warning\";\n";
        query << Ontology::note_description << "\t?warning.\n";
    }
    else if (isWarning == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?warningNote.\n";
        query << "FILTER NOT EXISTS { ?warningNote " << Ontology::note_type << " \"warning\".}\n}\n";
    }
    if (isComment == 1)
    {
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )\t";
        query << " ?commentNote.\n";
        query << "?commentNote\t" << Ontology::note_type << "\t\"comment\";\n";
        query << Ontology::note_description << "\t?comment.\n";
    }
    else if (isComment == 0)
    {
        query << "OPTIONAL {";
        // query << "?pdb_file\t";
        query << "?oligo\t";
        query << "( "  << Ontology::hasNote << " | ";
        // query << Ontology::hasOligo << " | ";
        query << Ontology::hasMono << " )*\t";
        query << " ?commentNote.\n";
        query << "FILTER NOT EXISTS { ?commentNote " << Ontology::note_type << " \"comment\".}\n}\n";
    }

    
    query << Ontology::END_WHERE_CLAUSE << "\n";
    query << "ORDER BY  ?" << sortBy << "\n";

    if(local_debug > 0)
    {
        gmml::log(__LINE__, __FILE__,  gmml::INF, query.str());
        gmml::log(__LINE__, __FILE__, gmml::INF, "Done running ontologyDownload()");
    }

    return FormulateCURLGF(output_file_type, query.str(), url);
}

void MolecularModeling::Assembly::ConvertGraphToQuery(std::stringstream &queryStream, GraphDS::Graph queryGraph)
{
    int local_debug = -1;
    GraphDS::Graph::NodeVector queryNodes = queryGraph.GetGraphNodeList();
    GraphDS::Graph::EdgeVector queryEdges = queryGraph.GetGraphEdgeList();
    std::string terminalNodeList[] = {"ASN", "SER", "THR", "TRP", "ROH", "OME", "OtBu"};
    // TODO: Make this handle all possible terminal nodes better.  I had to add THR for C-Linked, 
    // there are other residues that need to be added as well, especially for modified terminals.

    std::stringstream oligoStream, monoStream, linkStream, filterStream;
    oligoStream << "?oligo\n";

    for (GraphDS::Graph::NodeVector::iterator it = queryNodes.begin(); it != queryNodes.end(); it++)
    {
        GraphDS::Node *current_node = (*it);
        //if the node isnt in the terminal list
        if (std::find(std::begin(terminalNodeList), std::end(terminalNodeList), current_node->GetNodeId()) == std::end(terminalNodeList))
        {
            // current_node->GetNodeId();
            // this gets the mono shortName (full)
            // which then eventually and ideally needs to be checked
            // to see if it has all of the parts
            // TODO add the split function and query by each part

            // for now just query by the full name
            monoStream << "?mono" << current_node->GetNodeType() << "\n";
            monoStream << "\t" << Ontology::mono_short_name << "\t?monoName" << current_node->GetNodeType() << ";\n";
            monoStream << "\t" << Ontology::isSaccharide << "\ttrue;\n";
            oligoStream << "\t" << Ontology::hasMono << "\t?mono" << current_node->GetNodeType() << ";\n";

            for (GraphDS::Graph::EdgeVector::iterator it1 = queryEdges.begin(); it1 != queryEdges.end(); it1++)
            {
                GraphDS::Edge *current_edge = (*it1);
                GraphDS::Node *destinationNode = current_edge->GetDestinationNode();
                if ((current_edge->GetSourceNode() == current_node) &&
                    (std::find(std::begin(terminalNodeList), std::end(terminalNodeList), destinationNode->GetNodeId()) == std::end(terminalNodeList)))
                {
                    // Name link variables by the node #s they connect
                    linkStream << "?link" << current_node->GetNodeType() << "to" << destinationNode->GetNodeType() <<"\n";

                    // Add nodes to the query
                    linkStream << "\t" << Ontology::hasParentMono << "\t?mono" << current_node->GetNodeType() << ";\n";
                    linkStream << "\t" << Ontology::hasChildMono << "\t?mono" << destinationNode->GetNodeType() << ";\n";

                    // Filter by the linkage type
                    linkStream << "\t" << Ontology::linkageType << "\t?link" << current_node->GetNodeType() << "to";
                    linkStream << destinationNode->GetNodeType() << "Type;\n";

                    // All objects end with their type and a period
                    linkStream << "\t" << Ontology::TYPE << "\t" << Ontology::Linkage << ".\n";
                    
                    linkStream << "VALUES ?link" << current_node->GetNodeType() << "to" << destinationNode->GetNodeType() << "Type { ";

                    if (current_edge->GetEdgeLabels()[0] == "1-0")
                    {    
                        linkStream << "\"1-1\", \"1-2\", \"1-3\", \"1-4\", \"1-5\", \"1-6\" ";
                    }
                    else if (current_edge->GetEdgeLabels()[0] == "2-0")
                    {
                        linkStream << "\"2-1\", \"2-2\", \"2-3\", \"2-4\", \"2-5\", \"2-6\" ";
                    }
                    else if ((current_edge->GetEdgeLabels()[0] == "1-") || 
                              (current_edge->GetEdgeLabels()[0] == "2-") || 
                              (current_edge->GetEdgeLabels()[0] == "-"))
                    { // Terminal linkage

                    }
                    else if (current_edge->GetEdgeLabels()[0] != "*")
                    {
                        linkStream << "\"" << current_edge->GetEdgeLabels()[0] << "\" ";
                    }
                    else
                    {
                        linkStream << "\"1-1\", \"1-2\", \"1-3\", \"1-4\", \"1-5\", \"1-6\", \"2-1\", \"2-2\", \"2-3\", \"2-4\", \"2-5\", \"2-6\" ";
                    }
                    linkStream << "}\n\n";
                }
                else if ((it1 == queryEdges.begin()) && (current_edge->GetSourceNode() == NULL) && 
                         (current_edge->GetDestinationNode() != NULL) && 
                         (std::find(std::begin(terminalNodeList), std::end(terminalNodeList), destinationNode->GetNodeId()) == std::end(terminalNodeList)))
                { // Graph that starts with an edge
                    // Name link variables by the node #s they connect
                    linkStream << "?linkXto" << destinationNode->GetNodeType() <<"\n";
                    
                    // Add nodes to the query
                    linkStream << "\t" << Ontology::hasParentMono << "\t?monoX;\n";
                    linkStream << "\t" << Ontology::hasChildMono << "\t?mono" << destinationNode->GetNodeType() << ";\n";

                    // Filter by the linkage type
                    linkStream << "\t" << Ontology::linkageType << "\t?linkXType;";
                    // All objects end with their type and a period
                    linkStream << "\t" << Ontology::TYPE << "\t" << Ontology::Linkage << ".\n\n";

                    if (current_edge->GetEdgeLabels()[0] != "*")
                    {
                        linkStream << "VALUES ?linkType { ";

                        if (current_edge->GetEdgeLabels()[0] == "1-0")
                        {    
                            linkStream << "\"1-1\", \"1-2\", \"1-3\", \"1-4\", \"1-5\", \"1-6\" ";
                        }
                        else if (current_edge->GetEdgeLabels()[0] == "2-0")
                        {
                            linkStream << "\"2-1\", \"2-2\", \"2-3\", \"2-4\", \"2-5\", \"2-6\" ";
                        }
                        else if ((current_edge->GetEdgeLabels()[0] == "1-") || 
                                (current_edge->GetEdgeLabels()[0] == "2-") || 
                                (current_edge->GetEdgeLabels()[0] == "-"))
                        { // Terminal linkage
                            // shouldn't be possible to get here?
                            // query 1-ASN?
                        }
                        else 
                        {
                            linkStream << "\"" << current_edge->GetEdgeLabels()[0] << "\" ";
                        }
                    }
                    else
                    {
                        linkStream << "\"1-1\", \"1-2\", \"1-3\", \"1-4\", \"1-5\", \"1-6\", \"2-1\", \"2-2\", \"2-3\", \"2-4\", \"2-5\", \"2-6\" ";
                    }
                    linkStream << "}\n\n";
                }
            }
            monoStream << "\t" << Ontology::TYPE << "\t" << Ontology::Monosaccharide << ".\n\n";
            
            // TODO This should split mono name and look at each part

            // If the monoName is one that could commonly have NAc, but doesn't, add a filter
            // The list from the SNFG documentation is
            // GlcNAc, ManNAc, GalNAc, GulNAc, AltNAc, AllNAc, TalNAc, IdoNAc, FucNAc, QuiNAc, RhaNAc
            // Similarly, if the monoName is one that could commonly have N or A, but doesn't, add a filter
            // The list from the SNFG documentation is
            // GlcN, ManN, GalN, GulN, AltN, AllN, TalN, IdoN
            // GlcA, ManA, GalA, GulA, AltA, AllA, TalA, IdoA

            std::vector<std::string> monosCommonlyModified = {"Glc", "Man", "Gal", "Gul", "Alt", "All", "Tal", "Ido", "Fuc", "Qui", "Rha"};
            for (auto mono : monosCommonlyModified)
            {
                if (current_node->GetNodeId().find(mono) != std::string::npos)
                {
                    // If the last character is "a" or "b", we can just filter using the mono name
                    if ((current_node->GetNodeId().back() == 'a') || (current_node->GetNodeId().back() == 'b'))
                    {
                        filterStream << "VALUES ?monoName" << current_node->GetNodeType() << " { \"" << current_node->GetNodeId() << "\" } ";
                    }
                    else
                    {
                        // Otherwise, we need to make sure it doesn't return NAc or N/A versions
                        filterStream << "VALUES ?monoName" << current_node->GetNodeType() << " { \"";
                        filterStream << current_node->GetNodeId() << "a\" \"" << current_node->GetNodeId() << "b\" }\n";
                    }
                }
            }
            
            // monoStream << "FILTER REGEX(?monoName" << current_node->GetNodeType() << ", \"" << current_node->GetNodeId() << "\")\n\n";
        }
        else //if the node is in the terminal list
        {
            oligoStream << "\t" << Ontology::hasTerminal << "\t?terminal;\n";
            filterStream << "?terminal bif:contains \"" << current_node->GetNodeId() << "\".\n\n";
            //TODO handle terminals better
        }   
    }
    oligoStream << "\t" << Ontology::TYPE << "\t" << Ontology::Oligosaccharide << ".\n\n";

    

    queryStream << oligoStream.str() << monoStream.str() << linkStream.str() << filterStream.str();
    
    if (local_debug > 0)
    {
        std::stringstream logSS;
        logSS << "Finished graph creation\n";
        gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
        logSS.str("");
        logSS << "Query:\n" << queryStream.str() << "\n";
        gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
    }
}

GraphDS::Graph MolecularModeling::Assembly::CreateQueryStringGraph(std::string queryString)
{
    int local_debug = -1;
    std::stringstream logSS;
    if (local_debug > 0)
    {
        logSS << "Starting graph creation\n";
        gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
        logSS.str("");
    }

    GraphDS::Graph graph;
    std::vector<parsedString> parsedVector;
    std::vector<std::string> nodeStrings;

    // split string into mono, linkage, and brackets
    // IE DGlcpNAcb1-4[LFucpa1-4]DGlcpNAcb1-ASN is split into:
    //{DGlcpNAcb, 1-4,     [,    LFucpa, 1-4,     ],    DGlcpNAcb, 1-,      ASN} (labels)
    //{Node1,     NULL,    NULL, Node2,  NULL,    NULL, Node3,     NULL,    Node4} (nodes)
    //{NULL,      Edge1-3, NULL, NULL,   Edge2-3, NULL, NULL,      Edge3-4, NULL} (edges)
    std::string labelStr;
    std::size_t linkageStart, linkageEnd, dashLocation;
    dashLocation = queryString.find("-");
    bool noLinkage = false;
    if (dashLocation == std::string::npos)
    {
        noLinkage = true;
    }
    if (isdigit(queryString[dashLocation - 1]))
    {
        linkageStart = dashLocation - 1;
    }
    else
    {
        linkageStart = dashLocation;
    }
    if (isdigit(queryString[dashLocation + 1]))
    {
        linkageEnd = dashLocation + 1;
    }
    else
    {
        linkageEnd = dashLocation;
    }

    // Fill Vector, add nodes & labels
    for (unsigned int i = 0; i < queryString.size(); i++)
    {
        if (local_debug > 0)
        {
            logSS << i << " : " << queryString[i] << "\n";
            gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
            logSS.clear();
        }
        if ((queryString[i] == '[') || (queryString[i] == ']'))
        {
            // To be clear this handles branching, but brackets can also be used
            // for ring modifications of cyclic oligos
            // These shouldn't be in the query string right now but we need a better way to handle them
            if(local_debug > 0)
            {
                logSS << "There's a bracket at: " << i << "\n";
                gmml::log(__LINE__, __FILE__,  gmml::INF, logSS.str());
                logSS.clear();
            }
            labelStr.push_back(queryString[i]);
            parsedVector.push_back(parsedString(labelStr));
            labelStr = "";
            continue;
        }
        else if (queryString[i] == '*')
        {
            labelStr.push_back(queryString[i]);
            GraphDS::Node *newNode = new GraphDS::Node;
            nodeStrings.push_back(labelStr);
            void *ptr = &(nodeStrings[nodeStrings.size() - 1][0]);
            newNode->SetNodeValue(ptr);
            newNode->SetNodeId(labelStr);
            parsedVector.push_back(parsedString(labelStr, newNode));
            labelStr = "";
            continue;
        }
        else if ((i == linkageStart) && (noLinkage == false))
        {
            while ((i + 1 != linkageEnd + 1) && (i < queryString.size()))
            {
                if (local_debug > 0)
                {
                    logSS << i << " : " << linkageEnd + 1 << "\n";
                    gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
                    logSS.clear();
                }
                labelStr.push_back(queryString[i]);
                i++;
            }
            if (i < queryString.size())
            {
                labelStr.push_back(queryString[i]);
                GraphDS::Edge *thisEdge = new GraphDS::Edge;
                thisEdge->AddEdgeLabel(labelStr);
                parsedVector.push_back(parsedString(labelStr, thisEdge));
                labelStr = "";
                dashLocation = queryString.find("-", linkageEnd + 1);
                if (dashLocation != std::string::npos)
                {
                    if (isdigit(queryString[dashLocation - 1]))
                    {
                        linkageStart = dashLocation - 1;
                    }
                    else
                    {
                        linkageStart = dashLocation;
                    }
                    if (isdigit(queryString[dashLocation + 1]))
                    {
                        linkageEnd = dashLocation + 1;
                    }
                    else
                    {
                        linkageEnd = dashLocation;
                    }
                }
                else
                {
                    dashLocation = -1;
                    linkageStart = dashLocation;
                    linkageEnd = dashLocation;
                }
            }
        }
        else
        {
            if (linkageStart != -1)
            {
                while ((i + 1 != linkageStart) && (queryString[i + 1] != '*') && (i < queryString.size()))
                {
                    labelStr.push_back(queryString[i]);
                    i++;
                }
                if (i < queryString.size())
                {
                    labelStr.push_back(queryString[i]);
                    GraphDS::Node *newNode = new GraphDS::Node;
                    nodeStrings.push_back(labelStr);
                    void *ptr = &(nodeStrings[nodeStrings.size() - 1][0]);
                    newNode->SetNodeValue(ptr);
                    newNode->SetNodeId(labelStr);
                    parsedVector.push_back(parsedString(labelStr, newNode));
                    labelStr = "";
                }
            }
            else
            {
                while ((i < queryString.size()) && (queryString[i + 1] != '*'))
                {
                    labelStr.push_back(queryString[i]);
                    i++;
                }
                GraphDS::Node *newNode = new GraphDS::Node;
                nodeStrings.push_back(labelStr);
                void *ptr = &(nodeStrings[nodeStrings.size() - 1][0]);
                newNode->SetNodeValue(ptr);
                newNode->SetNodeId(labelStr);
                parsedVector.push_back(parsedString(labelStr, newNode));
                labelStr = "";
            }
        }
    }

    // Go through vector and add wildcard edges & extra nodes if needed
    for (unsigned int i = 0; i < parsedVector.size(); i++)
    {
        if ((parsedVector[i].label == "*") && (parsedVector[i].node != NULL))
        {
            // Wildcards shouldn't be at the beginning or end so this shouldn't break...

            if ((parsedVector[i - 1].edge == NULL) && (parsedVector[i - 1].node != NULL) &&
                (parsedVector[i - 1].label != "*") && (parsedVector[i + 1].edge == NULL) &&
                (parsedVector[i + 1].node != NULL) && (parsedVector[i + 1].label != "*"))
            { // Node then wildcard then Node; needs to be Node, wildcard edge, wildcard node, and a wildcard edge
                GraphDS::Edge *thisEdge0 = new GraphDS::Edge;
                thisEdge0->AddEdgeLabel("*");
                parsedVector.insert(parsedVector.begin() + i + 1, parsedString("*", thisEdge0));
                GraphDS::Edge *thisEdge01 = new GraphDS::Edge;
                thisEdge01->AddEdgeLabel("*");
                parsedVector.insert(parsedVector.begin() + i, parsedString("*", thisEdge01));

                continue;
            }
            if ((parsedVector[i - 1].edge == NULL) && (parsedVector[i - 1].node != NULL) && (parsedVector[i - 1].label != "*"))
            { // there is a node and then a wildcard; needs an edge to be connected
                GraphDS::Edge *thisEdge = new GraphDS::Edge;
                thisEdge->AddEdgeLabel("*");
                parsedVector.insert(parsedVector.begin() + i, parsedString("*", thisEdge));
                if (local_debug > 0)
                {
                    logSS << "Adding * edge at " << i << "\n";
                    gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
                    logSS.clear();
                }
                continue;
            }
            if ((parsedVector[i + 1].edge == NULL) && (parsedVector[i + 1].node != NULL) && (parsedVector[i + 1].label != "*"))
            { // there is a wildcard and then a node; needs an edge to be connected
                // TODO: insert * edge at i + 1
                GraphDS::Edge *thisEdge1 = new GraphDS::Edge;
                thisEdge1->AddEdgeLabel("*");
                parsedVector.insert(parsedVector.begin() + i + 1, parsedString("*", thisEdge1));
                if (local_debug > 0)
                {
                    logSS << "Adding * edge at " << i + 1 << "\n";
                    gmml::log(__LINE__, __FILE__, gmml::INF, logSS.str());
                    logSS.clear();
                }
                continue;
            }
            // if((parsedVector[i-1].edge == NULL) && (parsedVector[i-1].node == NULL))
            // {//Wildcard is after start/end bracket of branch
            //   if(parsedVector[i-1].label == "[")
            //   {
            //     //This shouldn't matter; sub graph match will automatically match anthing here
            //   }
            //   else if(parsedVector[i-1].label == "]")
            //   {
            //     //Wildcard is after branch, so as long as everything else is good, other nodes should attach to this fine
            //   }
            // }
            if ((parsedVector[i + 1].edge == NULL) && (parsedVector[i + 1].node == NULL))
            { // Wildcard is before end/start bracket of branch
                if (parsedVector[i + 1].label == "[")
                {
                    // TODO: Need an edge at i + 1 added so this will attach to the node after the branch
                    GraphDS::Edge *thisEdge2 = new GraphDS::Edge;
                    thisEdge2->AddEdgeLabel("*");
                    parsedVector.insert(parsedVector.begin() + i + 1, parsedString("*", thisEdge2));
                    // std::cout << __LINE__ << "Adding * edge at " << i + 1 << "\n";
                    continue;
                }
                else if (parsedVector[i + 1].label == "]")
                {
                    // TODO: Wildcard is at the end of a branch so add edge at i + 1 so it will connect to the node after the branch
                    GraphDS::Edge *thisEdge3 = new GraphDS::Edge;
                    thisEdge3->AddEdgeLabel("*");
                    parsedVector.insert(parsedVector.begin() + i + 1, parsedString("*", thisEdge3));
                    // std::cout << __LINE__ << "Adding * edge at " << i + 1 << "\n";
                    continue;
                }
            }
        }
    }
    for (unsigned int i = 0; i < parsedVector.size(); i++)
    { // add a new * node at every * node
        if ((parsedVector[i].label == "*") && (parsedVector[i].node != NULL))
        {
            GraphDS::Node *newNode = new GraphDS::Node;
            nodeStrings.push_back("*");
            void *ptr = &(nodeStrings[nodeStrings.size() - 1][0]);
            newNode->SetNodeValue(ptr);
            newNode->SetNodeId("*");
            parsedVector.insert(parsedVector.begin() + i, parsedString("*", newNode));
            i++;
        }
    }

    // Go through vector, add edges b/w nodes & add nodes & edges to graph
    ConnectNodes(0, static_cast<int>(parsedVector.size()), parsedVector, graph);
    GraphDS::Graph::NodeVector graphNodes = graph.GetGraphNodeList();
    int i = 0;
    for (GraphDS::Graph::NodeVector::iterator it = graphNodes.begin(); it != graphNodes.end(); it++)
    {
        GraphDS::Node *current_node = (*it);
        current_node->SetNodeType(std::to_string(i));
        i++;
    }
    return graph;
}

void MolecularModeling::Assembly::ConnectNodes(int start, int end, std::vector<parsedString>& parsedVector, GraphDS::Graph& graph)
{
  int local_debug = -1;
  if(local_debug > 0)
  {
    std::cout << "Connecting nodes\n";
    std::cout << start << ":" << end << "\n";
    std::cout << parsedVector.size()<< "\n";
  }

  if(end <= parsedVector.size())
  {
    for(int i=start; i<end; i++)
    {
      if(parsedVector[i].node != NULL)
      {
        graph.AddNewNode(parsedVector[i].node);
        if(i < (int)parsedVector.size() - 2)
        {
          if(parsedVector[i+1].edge != NULL)
          {
            if(parsedVector[i+2].node != NULL)
            {
              parsedVector[i+1].edge->SetSourceNode(parsedVector[i].node);
              parsedVector[i+1].edge->SetDestinationNode(parsedVector[i+2].node);
              if(local_debug > 0)
              {
                std::cout << __LINE__ << ": " << parsedVector[i].label << "{" << i <<"}" << " " << parsedVector[i+1].label << "{" << i + 1 <<"}" << " " <<  parsedVector[i + 2].label << "{" << i + 2 <<"}" << "\n";
              }
            }
            else if(parsedVector[i+2].label == "[")
            {//there is a branch.  Find the end of the branch and connect to the next node
              int numOpenBrackets = 1;
              bool onSameBranchPoint = true;
              int numBranchesAtThisNode = 1;
              int endBracketLocation;
              std::vector<int> startBranchLocations;
              startBranchLocations.push_back(i+2);
              std::vector<int> endBranchNodesLocations;
              while (onSameBranchPoint)
              {
                for(int j=i+3; j < end; j++)
                {//this will pass over both branched branches [[]] and multiple branches at the same node [][][]
                  //and hopefully point to the last bracket      ^                                             ^
                  if(parsedVector[j].label == "[")
                  {//branched branches
                    numOpenBrackets++;
                  }
                  if((parsedVector[j].label == "]") && (numOpenBrackets == 1))
                  {
                    if((j < parsedVector.size() - 1) && (parsedVector[j+1].label == "["))
                    {
                      numBranchesAtThisNode++;
                      startBranchLocations.push_back(j+1);
                      if(parsedVector[j-2].node != NULL)
                      {
                        endBranchNodesLocations.push_back(j-2);
                      }
                      numOpenBrackets--;
                      continue;
                    }
                    else
                    {
                      if(parsedVector[j-2].node != NULL)
                      {
                        endBranchNodesLocations.push_back(j-2);
                      }
                      endBracketLocation = j;
                      onSameBranchPoint = false;
                      break;
                    }
                  }
                  else if((parsedVector[j].label == "]") && (numOpenBrackets != 1))
                  {
                    numOpenBrackets--;
                  }
                }
              }
              if((endBracketLocation < parsedVector.size() - 1) && (parsedVector[endBracketLocation + 1].node != NULL))
              {//Deal with the node before all this branching nonesense
                parsedVector[i+1].edge->SetSourceNode(parsedVector[i].node);
                parsedVector[i+1].edge->SetDestinationNode(parsedVector[endBracketLocation + 1].node);
                if(local_debug > 0)
                {
                  std::cout << __LINE__  << ": " << parsedVector[i].label << "{" << i <<"}"<< " " <<  parsedVector[i+1].label << "{" << i +1 <<"}" << " " <<  parsedVector[endBracketLocation + 1].label << "{" << endBracketLocation + 1 <<"}" << "\n";
                }
              }
              for(unsigned int j = 0; j < endBranchNodesLocations.size(); j++)
              {//Attach the end of the branch to the node, recursively call this function to connect nodes in branch and deal with branched branches
                if((parsedVector[endBranchNodesLocations[j]+1].edge != NULL) && (parsedVector[endBracketLocation + 1].node != NULL))
                {
                  parsedVector[endBranchNodesLocations[j]+1].edge->SetSourceNode(parsedVector[endBranchNodesLocations[j]].node);
                  parsedVector[endBranchNodesLocations[j]+1].edge->SetDestinationNode(parsedVector[endBracketLocation + 1].node);
                  if(local_debug > 0)
                  {
                    std::cout << __LINE__  << ": " << parsedVector[endBranchNodesLocations[j]].label << "{" << endBranchNodesLocations[j] <<"}";
                    std::cout << " " <<  parsedVector[endBranchNodesLocations[j]+1].label<< "{" << endBranchNodesLocations[j]+1 <<"}" << " " <<  parsedVector[endBracketLocation + 1].label<< "{" << endBracketLocation + 1 << "}" << "\n";
                  }
                  if(j > 0)
                  {
                    ConnectNodes(startBranchLocations[j], endBranchNodesLocations[j], parsedVector, graph);
                  }
                }
              }
            }
          }
        }
      }
      else if(parsedVector[i].edge != NULL)
      {
        graph.AddEdge(parsedVector[i].edge);
        if(i == 0)
        {//Graph starts with edge, usually you add edge to source node, but you need to add edge to dest node now
          if(parsedVector[i+1].node!=NULL)
          {
            parsedVector[i].edge->SetDestinationNode(parsedVector[i+1].node);
            GraphDS::Node* newNode = NULL;
            parsedVector[i].edge->SetSourceNode(newNode);
          }
        }
      }
    }
  }
}

// To test new queries, you should go to your dev site's online virtuoso query.
// To find the IP address of your Virtuoso database, run docker inspect (YOUR_USERNAME)_gw_virt | grep IPAddress
// Then go to that IP address, followed by :8890/sparql.  For me right now, that's http://172.16.3.8:8890/sparql
// To query our ontology, you need to use the prefixes below.  This tells the ontology what the vocabulary means.
// SELECT is all of the data you want to pull out, and each variable must be present in the WHERE{} part of the query.
// Below is a sample query, and above if you follow the code it generates a couple other queries.
//
// PREFIX : <http://gmmo.uga.edu/#>
// PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
// PREFIX owl: <http://www.w3.org/2002/07/owl#>
// PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
// PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
// SELECT ?pdb ?title ?resolution ?Mean_B_Factor ?oligo_sequence ?oligo_mean_B_Factor ?authors ?journal ?PMID ?DOI
// WHERE
// {
// ?pdb_file :identifier ?pdb.
// ?pdb_file :hasTitle ?title.
// ?pdb_file :hasResolution ?resolution.
// ?pdb_file :hasAuthors ?authors.
// ?pdb_file :hasJournal ?journal.
// ?pdb_file :hasDOI ?DOI.
// ?pdb_file :hasPMID ?PMID.
// ?pdb_file :hasBFactor ?Mean_B_Factor.
// FILTER (!regex(?oligo_IUPAC, "- Unknown$"))
// ?oligo :oligoName ?oligo_sequence.
// ?oligo :oligoBFactor ?oligo_mean_B_Factor.
// }
// ORDER BY DESC(?resolution)


//Here's another that I ran for Rob to get all PDBs with non furanose (!regex line) sugars with unercognized side chains (symbolized as <R)
//
// PREFIX : <http://gmmo.uga.edu/#>
// PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
// PREFIX owl: <http://www.w3.org/2002/07/owl#>
// PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
// PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
// SELECT DISTINCT ?pdb ?oligo (group_concat(distinct ?oligo_sequence;separator="\n") as ?Oligosaccharides)
// WHERE
// {
// ?pdb_file :identifier ?pdb.
// ?pdb_file :hasOligo ?oligo.
// ?oligo :oligoName ?oligo_sequence.
// FILTER regex(?oligo_sequence, ".*<R.*")
// FILTER (!regex(?oligo_sequence, ".*f.*"))
// }

//More filters that I am saving here for copy/paste to the virtuoso sparql endpoint
// ?oligo :oligoIUPACname ?oligo_IUPAC.
// FILTER regex(?oligo_sequence, ".*-Unknown$")
// FILTER (!regex(?oligo_sequence, ".*<R.*"))
// FILTER (!regex(?oligo_IUPAC, "- Unknown$"))
// FILTER (!regex(?oligo_IUPAC, "-$"))
// FILTER (!regex(?oligo_IUPAC, "-ASN$"))
// FILTER (!regex(?oligo_IUPAC, "-THR$"))
// FILTER (!regex(?oligo_IUPAC, "-SER$"))


// PREFIX : <http://gmmo.uga.edu/#>
// PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
// PREFIX owl: <http://www.w3.org/2002/07/owl#>
// PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
// PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
// SELECT ?pdb ?oligo_sequence
// WHERE
// {
// ?pdb_file :identifier ?pdb.
// ?pdb_file :hasOligo ?oligo.
// ?oligo :oligoName ?oligo_sequence.

// SELECT DISTINCT ?residue_links?title ?resolution ?Mean_B_Factor?oligo_mean_B_Factor ?authors ?journal ?PMID ?DOI ?pdb_coordinates ?ProteinID(group_concat(distinct ?comment;separator="\n") as ?comments)(group_concat(distinct ?warning;separator="\n") as ?warnings)(group_concat(distinct ?error;separator="\n") as ?errors)WHERE {
// ?pdb_file     :identifier    "1A14";
//               :hasOligo      ?oligo.
// FILTER regex(?oligo, "oligo1$")
// ?oligo        :oligoIUPACname     "DManpa1-2DManpa1-2DManpa1-3DManpb1-4DGlcpNAcb1-4DGlcpNAcb1-ASN".
// ?pdb_file     :hasTitle               ?title;
//               :hasAuthors             ?authors.
// OPTIONAL {?pdb_file     :hasJournal             ?journal.}
// OPTIONAL {?pdb_file     :hasProteinID           ?ProteinID.}
// OPTIONAL {?pdb_file     :hasDOI                 ?DOI.}
// OPTIONAL {?pdb_file     :hasPMID                ?PMID.}
// OPTIONAL {?pdb_file     :hasResolution          ?resolution.}
// OPTIONAL {?pdb_file     :hasBFactor             ?Mean_B_Factor.}
// OPTIONAL {?oligo        :oligoResidueLinks      ?residue_links.}
// OPTIONAL {?oligo        :oligoBFactor           ?oligo_mean_B_Factor.}
// ?oligo        :PDBfile           ?pdb_coordinates.
// ?oligo        :hasMono            ?mono.
// OPTIONAL {?mono       :hasNote       ?errorNote.
// ?errorNote	    :NoteType      "error".
// ?errorNote      :description   ?error.}
// OPTIONAL {?mono       :hasNote       ?warningNote.
// ?warningNote    :NoteType      "warning".
// ?warningNote    :description   ?warning.}
// OPTIONAL {?mono       :hasNote       ?commentNote.
// ?commentNote    :NoteType      "comment".
// ?commentNote    :description   ?comment.}
// }

// // New query format for branched oligo searching
// // Looking for DGlcpNAcb1-4[LFucpa1-3]DGlcpNAcb
// PREFIX : <http://gmmo.uga.edu/#>
// PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
// PREFIX owl: <http://www.w3.org/2002/07/owl#>
// PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
// PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
// SELECT DISTINCT ?pdb (group_concat(distinct ?Oligosaccharide;separator="\n") as ?Oligosaccharides)
// WHERE {
// ?pdb_file     :identifier    ?pdb;
//               :hasOligo      ?oligo.
// ?pdb_file     :hasTitle               ?title;
//               :hasAuthors             ?authors.
// ?oligo        :oligoIUPACname     ?Oligosaccharide.
// ?oligo        :hasMono            ?mono;
//               :hasSequenceResidue ?residue1.
// ?residue1     :monosaccharideShortName  """LFucpa""".
//
// ?residue1     :is1-3ConnectedTo      ?residue2.
// ?residue2     :monosaccharideShortName """DGlcpNAcb""".
// ?residue3 :is1-4ConnectedTo ?residue2.
// ?residue3 :monosaccharideShortName """DGlcpNAcb""".
// }
//TODO make query work for any permutation of DGalpb1-4DGlcpNAcb1-2DManpa1-3[DGlcpNAcb1-2DManpa1-6]DManpb1-4DGlcpNAcb1-4[LFucpa1-6]DGlcpNAcb1-ASN


//Note to self:
//The following will remove the IRI prefix from gmmo:?PDBID
//BIND(STRAFTER(str(?pdb_file), "#") as ?pdb)

//TODO make this work for r groups
// PREFIX : <http://gmmo.uga.edu/#>
// PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
// PREFIX owl: <http://www.w3.org/2002/07/owl#>
// PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
// PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>
// SELECT DISTINCT ?residue_links ?title ?resolution ?Mean_B_Factor?oligo_mean_B_Factor ?authors ?journal ?PMID ?DOI ?pdb_coordinates ?ProteinID ?rGroup1 ?rGroup2 ?rGroup3 (group_concat(distinct ?comment;separator="\n") as ?comments)(group_concat(distinct ?warning;separator="\n") as ?warnings)(group_concat(distinct ?error;separator="\n") as ?errors)WHERE {
// ?pdb_file     :identifier    "100D";
//               :hasOligo      ?oligo.
// FILTER regex(?oligo, "oligo1$")
// ?oligo        :oligoIUPACname     "DRibf[2D][3<R1>][3<R2>,1<R3>]b1-N1".
// ?oligo  :hasR1    ?R1.
// ?R1 :hasFormula      ?rGroup1.
// ?oligo  :hasR2    ?R2.
// ?R2 :hasFormula      ?rGroup2.
// ?oligo  :hasR3    ?R3.
// ?R3 :hasFormula      ?rGroup3.
// ?pdb_file     :hasTitle               ?title;
//               :hasAuthors             ?authors.
// OPTIONAL {?pdb_file     :hasJournal             ?journal.}
// OPTIONAL {?pdb_file     :hasProteinID           ?ProteinID.}
// OPTIONAL {?pdb_file     :hasDOI                 ?DOI.}
// OPTIONAL {?pdb_file     :hasPMID                ?PMID.}
// OPTIONAL {?pdb_file     :hasResolution          ?resolution.}
// OPTIONAL {?pdb_file     :hasBFactor             ?Mean_B_Factor.}
// OPTIONAL {?oligo        :oligoResidueLinks      ?residue_links.}
// OPTIONAL {?oligo        :oligoBFactor           ?oligo_mean_B_Factor.}
// ?oligo        :PDBfile           ?pdb_coordinates.
// ?oligo        :hasMono            ?mono.
// OPTIONAL {?mono       :hasNote       ?errorNote.
// ?errorNote      :NoteType      "error".
// ?errorNote      :description   ?error.}
// OPTIONAL {?mono       :hasNote       ?warningNote.
// ?warningNote    :NoteType      "warning".
// ?warningNote    :description   ?warning.}
// OPTIONAL {?mono       :hasNote       ?commentNote.
// ?commentNote    :NoteType      "comment".
// ?commentNote    :description   ?comment.}
// }
