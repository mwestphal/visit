// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: sw4imgPluginInfo.C
// ****************************************************************************

#include <sw4imgPluginInfo.h>

#include <visit-config.h>
VISIT_PLUGIN_VERSION(sw4img,DBP_EXPORT)

VISIT_DATABASE_PLUGIN_ENTRY(sw4img,General)

// ****************************************************************************
//  Method: sw4imgGeneralPluginInfo::GetName
//
//  Purpose:
//    Return the name of the database plugin.
//
//  Returns:    A pointer to the name of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
sw4imgGeneralPluginInfo::GetName() const
{
    return "sw4img";
}

// ****************************************************************************
//  Method: sw4imgGeneralPluginInfo::GetVersion
//
//  Purpose:
//    Return the version of the database plugin.
//
//  Returns:    A pointer to the version of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
sw4imgGeneralPluginInfo::GetVersion() const
{
    return "1.0";
}

// ****************************************************************************
//  Method: sw4imgGeneralPluginInfo::GetID
//
//  Purpose:
//    Return the id of the database plugin.
//
//  Returns:    A pointer to the id of the database plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

const char *
sw4imgGeneralPluginInfo::GetID() const
{
    return "sw4img_1.0";
}
// ****************************************************************************
//  Method: sw4imgGeneralPluginInfo::EnabledByDefault
//
//  Purpose:
//    Return true if this plugin should be enabled by default; false otherwise.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
sw4imgGeneralPluginInfo::EnabledByDefault() const
{
    return true;
}
// ****************************************************************************
//  Method: sw4imgGeneralPluginInfo::HasWriter
//
//  Purpose:
//    Return true if this plugin has a database writer.
//
//  Returns:    true/false
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

bool
sw4imgGeneralPluginInfo::HasWriter() const
{
    return false;
}
// ****************************************************************************
//  Method:  sw4imgGeneralPluginInfo::GetDefaultFilePatterns
//
//  Purpose:
//    Returns the default patterns for a sw4img database.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
std::vector<std::string>
sw4imgGeneralPluginInfo::GetDefaultFilePatterns() const
{
    std::vector<std::string> defaultPatterns;
    defaultPatterns.push_back("*.sw4img");
    defaultPatterns.push_back("*.3Dimg");

    return defaultPatterns;
}

// ****************************************************************************
//  Method:  sw4imgGeneralPluginInfo::AreDefaultFilePatternsStrict
//
//  Purpose:
//    Returns if the file patterns for a sw4img database are
//    intended to be interpreted strictly by default.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
sw4imgGeneralPluginInfo::AreDefaultFilePatternsStrict() const
{
    return false;
}

// ****************************************************************************
//  Method:  sw4imgGeneralPluginInfo::OpensWholeDirectory
//
//  Purpose:
//    Returns if the sw4img plugin opens a whole directory name
//    instead of a single file.
//
//  Programmer:  generated by xml2info
//  Creation:    omitted
//
// ****************************************************************************
bool
sw4imgGeneralPluginInfo::OpensWholeDirectory() const
{
    return false;
}
