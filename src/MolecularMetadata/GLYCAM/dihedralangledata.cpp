#include "../../../../includes/MolecularMetadata/GLYCAM/dihedralangledata.hpp"

//////////////////////////////////////////////////////////
//                       CONSTRUCTOR                    //
//////////////////////////////////////////////////////////

using gmml::MolecularMetadata::GLYCAM::DihedralAngleDataContainer;

// Struct is copied here for reference.
//struct DihedralAngleData
//{
//    std::string linking_atom1_ ;
//    std::string linking_atom2_ ;
//    std::string dihedral_angle_name_ ;
//    double default_angle_value_ ;
//    double lower_deviation_ ;
//    double upper_deviation_ ;
//    double weight_;
//    std::string rotamer_type_ ; // permutation or conformer
//    std::string rotamer_name_ ;
//    int number_of_bonds_from_anomeric_carbon_;
//    int index_ ; // Used to indicate whether multiple entries are meant to overwrite each other or generate an additional angle
//    StringVector residue1_conditions_ ;
//    StringVector residue2_conditions_ ;
//    std::string atom1_ ;
//    std::string atom2_ ;
//    std::string atom3_ ;
//    std::string atom4_ ;
//} ;

/*
 * A note on index_.
 * number_of_bonds_from_anomeric_carbon_. So phi is 1, psi is 2, Omg is 3.
 * The index refers the rotamer number. If there are two phi rotamers, they will have an index of 1 and 2.
 * Chi angle index numbering varies depending on side chain length, so in ASN the Chi1 is 4 bonds away from the sugar, so would be 4
 * If two entries have matching regex and have the same index_ number (e.g. 1), the first will be overwritten. Note I haven't used this feature as of 13 Feb 2019
 * If two entries have matching regex and different index_ numbers (e.g. 1,2,3) they will all be used to create multiple rotamers/conformers
 * If two entries have different regex, but apply to the same dihedral angle (e.g. phi), give them the same index_ number (e.g. 1).
 */

DihedralAngleDataContainer::DihedralAngleDataContainer()
{   // const AmberAtomTypeInfo Glycam06j1AtomTypes[] =
    dihedralAngleDataVector_ =
    {    // Regex:       , name                                                                                 // Atom names this applies to
        { "C1", "O[1-9]" , "phi"  , 180.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , ""   , 1 , 1 , {"aldose"}     , {"none"}                  ,  "C2" , "C1" , "O." , "C."  }, // phi should be C2-C1(ano)-Ox-Cx, or C1-C2(ano)-Ox-Cx
        { "C2", "O[1-9]" , "phi"  , 180.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , ""   , 1 , 1 , {"none"}       , {"none"}                  ,  "C3" , "C2" , "O." , "C."  }, // phi should be C2-C1(ano)-Ox-Cx, or C1-C2(ano)-Ox-Cx
        { "C2", "O[1-9]" , "phi"  , -60.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , ""   , 1 , 2 , {"ulosonate", "alpha"}  , {"none"}         ,  "C3" , "C2" , "O." , "C."  },

        { "C.", "O[1-5]" , "psi"  ,   0.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , ""   , 2 , 1 , {"none"}       , {"none"}                  ,  "C." , "O." , "C." , "H."  }, // psi should be C(ano)-Ox-Cx-Hx, if Cx is ring, otherwise, C(ano)-Ox-Cx-C(x-1)
        { "C.", "O[6-9]" , "psi"  , 180.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , ""   , 2 , 1 , {"none"}       , {"none"}                  ,  "C." , "O." , "C." , "C."  },

        { "C.", "O6"     , "omg"  , -60.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , "gg" , 3 , 1 , {"none"}       , {"none"}                  ,  "O6" , "C6" , "C5" , "O5"  }, // omg is O6-C5-C5-O5
        { "C.", "O6"     , "omg"  ,  60.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , "gt" , 3 , 2 , {"none"}       , {"none"}                  ,  "O6" , "C6" , "C5" , "O5"  },
        { "C.", "O6"     , "omg"  , 180.0  ,  20.0  ,  20.0  , 1.0   , "permutation" , "tg" , 3 , 3 , {"none"}       , {"gauche-effect=galacto"} ,  "O6" , "C6" , "C5" , "O5"  },

         // 2-8 linkages
      //{ "C2", "O8"   , "omg7" ,    };

        // Protein linkages
        // ASN // Values are from Petrescu et al 2004.
        { "C.", "ND2"    , "chi1" , 191.6  ,  14.4  ,  14.4  , 0.497 , "conformer"   , ""   , 4 , 1 , {"none"}       , {"amino-acid"}            ,  "CG" , "CB" , "CA" , "N"   },
        { "C.", "ND2"    , "chi2" , 177.6  ,  43.0  ,  43.0  , 0.497 , "conformer"   , ""   , 3 , 1 , {"none"}       , {"amino-acid"}            ,  "ND2", "CG" , "CB" , "CA"  },
        { "C.", "ND2"    , "psi"  , 177.3  ,  12.3  ,  12.3  , 0.497 , "conformer"   , ""   , 2 , 1 , {"none"}       , {"amino-acid"}            ,  "C." , "ND2", "CG" , "CB"  },
        { "C1", "ND2"    , "phi"  , 261.0  ,  21.3  ,  21.3  , 0.497 , "conformer"   , ""   , 1 , 1 , {"none"}       , {"amino-acid"}            ,  "C." , "C." , "ND2", "CG"  },

        { "C.", "ND2"    , "chi1" ,  63.6  ,   8.9  ,   8.9  , 0.178 , "conformer"   , ""   , 4 , 2 , {"none"}       , {"amino-acid"}            ,  "CG" , "CB" , "CA" , "N"   },
        { "C.", "ND2"    , "chi2" , 191.1  ,  31.6  ,  31.6  , 0.178 , "conformer"   , ""   , 3 , 2 , {"none"}       , {"amino-acid"}            ,  "ND2", "CG" , "CB" , "CA"  },
        { "C.", "ND2"    , "psi"  , 178.5  ,  13.9  ,  13.9  , 0.178 , "conformer"   , ""   , 2 , 2 , {"none"}       , {"amino-acid"}            ,  "C." , "ND2", "CG" , "CB"  },
        { "C1", "ND2"    , "phi"  , 253.7  ,  21.5  ,  21.5  , 0.178 , "conformer"   , ""   , 1 , 2 , {"none"}       , {"amino-acid"}            ,  "C." , "C." , "ND2", "CG"  },

        { "C.", "ND2"    , "chi1" , 290.6  ,  12.7  ,  12.7  , 0.235 , "conformer"   , ""   , 4 , 3 , {"none"}       , {"amino-acid"}            ,  "CG" , "CB" , "CA" , "N"   },
        { "C.", "ND2"    , "chi2" , 152.9  ,  23.9  ,  23.9  , 0.235 , "conformer"   , ""   , 3 , 3 , {"none"}       , {"amino-acid"}            ,  "ND2", "CG" , "CB" , "CA"  },
        { "C.", "ND2"    , "psi"  , 173.1  ,  12.2  ,  12.2  , 0.235 , "conformer"   , ""   , 2 , 3 , {"none"}       , {"amino-acid"}            ,  "C." , "ND2", "CG" , "CB"  },
        { "C1", "ND2"    , "phi"  , 268.0  ,  20.3  ,  20.3  , 0.235 , "conformer"   , ""   , 1 , 3 , {"none"}       , {"amino-acid"}            ,  "C." , "C." , "ND2", "CG"  },

        { "C.", "ND2"    , "chi1" , 302.3  ,  11.5  ,  11.5  , 0.090 , "conformer"   , ""   , 4 , 4 , {"none"}       , {"amino-acid"}            ,  "CG" , "CB" , "CA" , "N"   },
        { "C.", "ND2"    , "chi2" , 255.0  ,  28.8  ,  28.8  , 0.090 , "conformer"   , ""   , 3 , 4 , {"none"}       , {"amino-acid"}            ,  "ND2", "CG" , "CB" , "CA"  },
        { "C.", "ND2"    , "psi"  , 178.1  ,  11.5  ,  11.5  , 0.090 , "conformer"   , ""   , 2 , 4 , {"none"}       , {"amino-acid"}            ,  "C." , "ND2", "CG" , "CB"  },
        { "C1", "ND2"    , "phi"  , 267.5  ,  23.9  ,  23.9  , 0.090 , "conformer"   , ""   , 1 , 4 , {"none"}       , {"amino-acid"}            ,  "C." , "C." , "ND2", "CG"  },


//        // THR // Values are from Lovell et al "PENULTIMATE ROTAMER LIBRARY"
//        { "C.", "OG1"    , "chi1" ,  59.0  ,  10.0  ,  10.0  , 0.49  , ""   ,  3.1 , "none"   , "amino-acid"            ,  "OG1", "CB" , "CA" , "N"   },
//        { "C.", "OG1"    , "chi1" ,-171.0  ,   6.0  ,   6.0  , 0.07  , ""   ,  3.2 , "none"   , "amino-acid"            ,  "OG1", "CB" , "CA" , "N"   },
//        { "C.", "OG1"    , "chi1" , -61.0  ,   7.0  ,   7.0  , 0.43  , ""   ,  3.3 , "none"   , "amino-acid"            ,  "OG1", "CB" , "CA" , "N"   },

//        { "C.", "OG1"    , "psi"  , -60.0  ,  60.0  ,  60.0  , 1.000 , ""   ,  2.1 , "none"   , "amino-acid"            ,  "C." , "OG1", "CB" , "CA"  },
//        { "C.", "OG1"    , "phi"  , 180.0  ,  20.0  ,  20.0  , 1.000 , ""   ,  1.1 , "none"   , "amino-acid"            ,  "C." , "C." , "OG1", "CB"  },

//         // SER // Values not checked
//        { "C.", "OG"     , "chi1" ,  64.0  ,  10.0  ,  10.0  , 0.48  ,  ""   ,  3.1 , "none"   , "amino-acid"            ,  "OG" , "CB" , "CA" , "N"   },
//        { "C.", "OG"     , "chi1" , 178.0  ,  11.0  ,  11.0  , 0.22  ,  ""   ,  3.2 , "none"   , "amino-acid"            ,  "OG" , "CB" , "CA" , "N"   },
//        { "C.", "OG"     , "chi1" , -65.0  ,   9.0  ,   9.0  , 0.29  ,  ""   ,  3.3 , "none"   , "amino-acid"            ,  "OG" , "CB" , "CA" , "N"   },

//         { "C.", "OG"     , "psi"  , -60.0  ,  20.0  ,  20.0  , ""   ,  2.1 , "none"   , "amino-acid"            ,  "C." , "OG" , "CB" , "CA"  },
//        { "C.", "OG"     , "phi"  , 180.0  ,  20.0  ,  20.0  , "t"  ,  1.1 , "none"   , "amino-acid"            ,  "C." , "C." , "OG1", "CB"  },

//         // TYR // Values not checked
//        { "C.", "OH"     , "chi1" , -60.0  ,  20.0  ,  20.0  , ""   ,  7.1 , "none"   , "amino-acid"            ,  "CG" , "CB" , "CA" , "N"   },
//        { "C.", "OH"     , "chi1" ,  60.0  ,  20.0  ,  20.0  , ""   ,  7.2 , "none"   , "amino-acid"            ,  "CG" , "CB" , "CA" , "N"   },
//        { "C.", "OH"     , "chi1" , 180.0  ,  20.0  ,  20.0  , ""   ,  7.3 , "none"   , "amino-acid"            ,  "CG" , "CB" , "CA" , "N"   },
//        { "C.", "OH"     , "chi2" , -60.0  ,  20.0  ,  20.0  , ""   ,  6.1 , "none"   , "amino-acid"            ,  "CD1", "CG" , "CB" , "CA"  },
//        { "C.", "OH"     , "psi"  , -60.0  ,  20.0  ,  20.0  , ""   ,  2.1 , "none"   , "amino-acid"            ,  "C." , "OH" , "CZ" , "CE1" },
//        { "C.", "OH"     , "phi"  , 180.0  ,  20.0  ,  20.0  , "t"  ,  1.1 , "none"   , "amino-acid"            ,  "C." , "C." , "OH ", "CZ"  },

    };
}

//    Statistical analysis of the protein environment of N-glycosylation sites: implications for occupancy, structure, and folding
//    Andrei-J. Petrescu  Adina-L. Milac  Stefana M. Petrescu  Raymond A. Dwek Mark R. Wormald
//    Glycobiology, Volume 14, Issue 2, 1 February 2004, Pages 103–114,
