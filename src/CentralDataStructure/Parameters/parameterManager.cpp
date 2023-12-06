#include "includes/CentralDataStructure/Parameters/parameterManager.hpp"
#include "includes/CodeUtils/directories.hpp"

// How exactly this happens can be improved, but the information should only ever be loaded into gmml in one place.
namespace cdsParameters
{
    static const std::vector<std::string> prepFilesToLoad = {"/dat/prep/GLYCAM_06j-1_GAGS_KDN.prep"};

    static const std::vector<std::string> libFilesToLoad = {
        "/dat/CurrentParams/leaprc_GLYCAM_06j-1_2014-03-14/GLYCAM_amino_06j_12SB.lib",
        "/dat/CurrentParams/leaprc_GLYCAM_06j-1_2014-03-14/GLYCAM_aminont_06j_12SB.lib",
        "/dat/CurrentParams/leaprc.ff12SB_2014-04-24/nucleic12.lib",
        "/dat/CurrentParams/leaprc.ff12SB_2014-04-24/nucleic12.lib",
        "/dat/CurrentParams/other/solvents.lib",
        "/dat/CurrentParams/leaprc.ff12SB_2014-04-24/amino12.lib",
        "/dat/CurrentParams/leaprc.ff12SB_2014-04-24/aminoct12.lib",
        "/dat/CurrentParams/leaprc.ff12SB_2014-04-24/aminont12.lib",
    };
} // namespace cdsParameters

using cdsParameters::ParameterManager;

ParameterManager::ParameterManager()
{ // Library files of 3D structures with parameters for simulations.
    const std::string gmmlhome = codeUtils::getGmmlHomeDir();
    gmml::log(__LINE__, __FILE__, gmml::INF, "gmmlhome is: " + gmmlhome);
    for (auto& prepFilePath : cdsParameters::prepFilesToLoad)
    {
        auto& file = prepFiles_.emplace_back(gmmlhome + prepFilePath);
        this->InitializeResidueMap(file.getResidues());
    }
    for (auto& libFilePath : cdsParameters::libFilesToLoad)
    {
        auto& file = libFiles_.emplace_back(gmmlhome + libFilePath);
        this->InitializeResidueMap(file.getResidues());
    }
    gmml::log(__LINE__, __FILE__, gmml::INF, "Finished construction of ParameterManager.");
}

ParameterManager::ParameterManager(const std::vector<std::string> queryNames)
{}

cds::Residue* ParameterManager::findParameterResidue(const std::string name) const
{
    if (auto search = parameterResidueMap_.find(name); search != parameterResidueMap_.end())
    {
        return search->second;
    }
    gmml::log(__LINE__, __FILE__, gmml::WAR, "Did not find parameters for residue named: " + name);
    return nullptr;
}

cds::Residue ParameterManager::copyParameterResidue(const std::string name) const
{
    cds::Residue* reference = this->findParameterResidue(name);
    if (reference == nullptr)
    {
        std::string message = "Did not find and therefore cannot copy a parameter residue with this name: " + name;
        gmml::log(__LINE__, __FILE__, gmml::ERR, message);
        throw std::runtime_error(message);
    }
    return *reference; // copy
}

void ParameterManager::setAtomChargesForResidues(std::vector<cds::Residue*> queryResidues) const
{
    for (auto& residue : queryResidues)
    {
        this->setAtomChargesForResidue(residue);
    }
}

void ParameterManager::createAtomsForResidue(cds::Residue* queryResidue, const std::string glycamNameForResidue) const
{
    cds::Residue parameterResidue = this->copyParameterResidue(glycamNameForResidue);
    // Need parsedResidue as e.g. 0MA and not DManpa1-4. Maybe a "GlycamName" variable?
    queryResidue->setName(parameterResidue.getName());
    // extractAtoms moves the atoms parameterResidue will go out of scope
    queryResidue->setAtoms(parameterResidue.extractAtoms());
    // std::cout << "Finished moving atoms from parameterResidue to parsed Residue. Adventure awaits! Huzzah!" <<
    // std::endl;
    return;
}

// PRIVATE FUNCTIONS

void ParameterManager::InitializeResidueMap(std::vector<cds::Residue*> incomingResidues)
{
    for (auto& residue : incomingResidues)
    {
        parameterResidueMap_[residue->getName()] = residue;
    }
}

bool ParameterManager::setAtomChargesForResidue(cds::Residue* queryResidue) const
{
    bool allAtomsPresent           = true;
    cds::Residue* parameterResidue = this->findParameterResidue(queryResidue->GetParmName());
    if (parameterResidue == nullptr)
    {
        gmml::log(__LINE__, __FILE__, gmml::WAR,
                  "Did not find parameters and so cannot set charges for residue named: " +
                      queryResidue->GetParmName());
        return false;
    }
    std::vector<cds::Atom*> parameterAtoms = parameterResidue->getAtoms();
    for (auto& queryAtom : queryResidue->getAtoms())
    {
        if (!cdsParameters::setChargeForAtom(queryAtom, parameterAtoms))
        {
            allAtomsPresent = false;
        }
    }
    return allAtomsPresent;
}

bool cdsParameters::setChargeForAtom(cds::Atom* queryAtom, std::vector<cds::Atom*> referenceAtoms)
{
    for (auto& refAtom : referenceAtoms)
    {
        if (queryAtom->getName() == refAtom->getName())
        {
            queryAtom->setCharge(refAtom->getCharge());
            queryAtom->setType(refAtom->getType());
            return true;
        }
    }
    gmml::log(__LINE__, __FILE__, gmml::WAR, "No charges found for atom named " + queryAtom->getName());
    return false;
}
