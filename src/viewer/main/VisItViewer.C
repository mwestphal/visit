// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItViewer.h>

#include <qdir.h>
#include <avtCallback.h>

#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerProperties.h>
#include <ViewerSubject.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerMethods.h>

#ifdef HAVE_DDT
#include <DDTManager.h>
#endif

#include <DebugStream.h>
#include <VisItInit.h>
#include <InitVTK.h>
#include <InitVTKRendering.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <RuntimeSetting.h>
#include <InstallationFunctions.h>

#include <FileFunctions.h>
#include <StringHelpers.h>

#include <visit-config.h>

#include <QtVisWindow.h>
#include <vtkQtRenderWindow.h>

// Error handling callback functions.
static void ViewerErrorCallback(void *, const char *);
static void ViewerWarningCallback(void *, const char *);
// Log GLX and display information.
static void LogGlxAndXdpyInfo();

// ****************************************************************************
// Method: VisItViewer::Initialize
//
// Purpose:
//   Initializes the viewer library, etc.
//
// Arguments:
//   argc : The number of command line args.
//   argv : The command line args.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:31:28 PDT 2008
//
// Modifications:
//   Brad Whitlock, Fri Nov 13 13:57:36 PST 2009
//   I added RuntimeSetting initialization.
//
//   Jonathan Byrd (Allinea Software), Sun 18 Dec, 2011
//   Auto-connect to DDT if the DDT_LAUNCHED_VISIT env. var. exists
//
//   Kathleen Biagas, Wed Aug 28 14:34:55 PDT 2013
//   Don't call LogGlxAndXdpyInfo if in nowin mode.
//
//   Alok Hota, Tue Feb 23 19:10:32 PST 2016
//   Add OSPRay support.
//
//   Kathleen Biagas, Wed Aug 17, 2022
//   Incorporate ARSanderson's OSPRAY 2.8.0 work for VTK 9.
//
// ****************************************************************************

void
VisItViewer::Initialize(int *argc, char ***argv)
{
    VisItInit::SetComponentName("viewer");
    VisItInit::Initialize(*argc, *argv, 0, 1, false);
    bool nowin = false;
    for (int i = 0 ; i < *argc ; i++)
    {
        if (strcmp((*argv)[i], "-nowin") == 0)
        {
            nowin = true;
        }
#if defined(HAVE_OSPRAY) // ospray 3.0, vtk 9
        else if (strcmp((*argv)[i], "-ospray") == 0)
        {
            debug5 << "Viewer launching with OSPRay" << endl;
            avtCallback::SetUseOSPRay(true);
        }
#endif
    }
    RuntimeSetting::parse_command_line(*argc, const_cast<const char**>(*argv));
    if (!nowin)
        LogGlxAndXdpyInfo();

#ifdef HAVE_DDT
    // If VisIt has been launched by DDT, try to connect to DDT once the event
    // loop is started (socket communication needs the event loop to be active)
    if (getenv("DDT_LAUNCHED_VISIT")!=NULL)
        QMetaObject::invokeMethod(DDTManager::getInstance(),"makeConnection",
                Qt::QueuedConnection);
#endif
}

// ****************************************************************************
// Method: VisItViewer::Finalize
//
// Purpose:
//   Finalizes the viewer library.
//
// Note:       No viewer calls should be made after calling this function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:32:17 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::Finalize()
{
    // Unload plugins.
    delete ViewerBase::GetPlotPluginManager();
    delete ViewerBase::GetOperatorPluginManager();

    VisItInit::Finalize();
}

// ****************************************************************************
// Method: VisItViewer::VisItViewer
//
// Purpose:
//   Constructor for the VisItViewer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:32:59 PDT 2008
//
// Modifications:
//
// ****************************************************************************

VisItViewer::VisItViewer() : visitHome()
{
    visitHomeMethod = FromEnvironment;
    viewer = new ViewerSubject;

    //
    // Initialize the error logging.
    //
    VisItInit::ComponentRegisterErrorFunction(ViewerErrorCallback, (void*)viewer);
    InitVTK::Initialize();
    InitVTKRendering::Initialize();
    avtCallback::RegisterWarningCallback(ViewerWarningCallback,
                                         (void*)viewer);
}

// ****************************************************************************
// Method: VisItViewer::~VisItViewer
//
// Purpose:
//   Destructor for the VisItViewer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:33:31 PDT 2008
//
// Modifications:
//
// ****************************************************************************

VisItViewer::~VisItViewer()
{
    delete viewer;
}

// ****************************************************************************
// Method: VisItViewer::SetVISITHOMEMethod
//
// Purpose:
//   Set the method used for determining VISITHOME.
//
// Arguments:
//   m : The method to use for determining VISITHOME.
//
// Returns:
//
// Note:       Must be called before ProcessCommandLine.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:23:29 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::SetVISITHOMEMethod(VISITHOME_METHOD m)
{
    visitHomeMethod = m;
    if(visitHomeMethod != UserSpecified)
        visitHome = "";
}

// ****************************************************************************
// Method: VisItViewer::SetVISITHOME
//
// Purpose:
//   Set the path to VISITHOME. Implicitly selects UserDefined method for
//   for determining VISITHOME.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:24:19 PDT 2008
//
// Modifications:
//
// ****************************************************************************
void
VisItViewer::SetVISITHOME(const std::string &path)
{
    visitHomeMethod = UserSpecified;
    visitHome = path;
}

// ****************************************************************************
// Method: VisItViewer::GetVisItHome
//
// Purpose:
//   Return VisItHome based on the method that we've selected.
//
// Arguments:
//
// Returns:    The value that we'll use for VISITHOME
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:34:23 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Aug 26 13:41:04 PDT 2008
//   Qt 4.
//
//   Kathleen Bonnell, Fri Oct 8 08:57:19 PDT 2010
//   Remove extra leading underscore from _WIN32.
//
// ****************************************************************************

std::string
VisItViewer::GetVisItHome() const
{
    std::string home;
    if(visitHomeMethod == FromEnvironment)
    {
        if(getenv("VISITHOME") != NULL)
            home = getenv("VISITHOME");
#ifdef _WIN32
        else
            home = GetVisItInstallationDirectory();
#endif
    }
    else if(visitHomeMethod == FromArgv0)
    {
        QString appDir(QDir::current().path());
#if defined(_WIN32)
        int index = appDir.lastIndexOf("\\");
#else
        int index = appDir.lastIndexOf("/");
#endif
        if(index != -1)
            home = appDir.left(index).toStdString();
        else
            home = appDir.toStdString();
    }
    else // UserSpecified
        home = visitHome;

    return home;
}

// ****************************************************************************
// Method: VisItViewer::ProcessCommandLine
//
// Purpose:
//   Sets various viewer options by examining the command line.
//
// Arguments:
//   argc   : The number of command line args.
//   argv   : The command line args.
//   addDir : Whether the application's startup dir should be added.
//
// Returns:
//
// Note:       Calling this method is optional but it is encouraged so you
//             can pass options to the viewer. In addition, if you are not
//             setting VISITHOME from the environment then you really need to
//             call this function to ensure that the appropriate -dir options
//             are passed when launching other local VisIt components.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:33:49 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Oct 16 19:29:12 EDT 2008
//    Added a flag for whether or not ProcessCommandLine should force
//    the identical version.  It's necessary for new viewer-apps but
//    might confuse VisIt-proper's smart versioning.
//
// ****************************************************************************

void
VisItViewer::ProcessCommandLine(int argc, char **argv, bool addForceVersion)
{
    if(visitHomeMethod == FromArgv0 || visitHomeMethod == UserSpecified)
    {
        char dirName[1024];
        strcpy(dirName, GetVisItHome().c_str());

        // Call ProcessCommandLine with 4 extra arguments so the right
        // mdserver and engine get called if we're launching VisIt components
        // from an embedded application where we can't guarantee that various
        // VisIt environment variables will be set.
        int argc2 = argc + 4;
        char **argv2 = new char*[argc2 + 1];
        char dir[] = "-dir";
        char forceversion[] = "-forceversion";
        char visitversion[] = VISIT_VERSION;
        for(int i = 0; i < argc; ++i)
            argv2[i] = argv[i];
        argv2[argc    ] = dir;
        argv2[argc + 1] = dirName;
        argv2[argc + 2] = forceversion;
        argv2[argc + 3] = visitversion;
        argv2[argc + 4] = NULL;
        if (!addForceVersion)
        {
            argv2[argc + 2] = NULL;
            argc2 -= 2;
        }
        viewer->ProcessCommandLine(argc2, argv2);
        delete [] argv2;
    }
    else
    {
        // Call ProcessCommandLine with 2 extra arguments so the right
        // mdserver and engine get called if we're launching VisIt components
        // from an embedded application where we can't guarantee that various
        // VisIt environment variables will be set.
        int argc2 = argc + 2;
        char **argv2 = new char*[argc2 + 1];
        char forceversion[] = "-forceversion";
        char visitversion[] = VISIT_VERSION;
        for(int i = 0; i < argc; ++i)
            argv2[i] = argv[i];
        argv2[argc    ] = forceversion;
        argv2[argc + 1] = visitversion;
        argv2[argc + 2] = NULL;
        if (!addForceVersion)
        {
            argv2[argc] = NULL;
            argc2 -= 2;
        }
        viewer->ProcessCommandLine(argc2, argv2);
        delete [] argv2;
    }
}

// ****************************************************************************
// Method: VisItViewer::Connect
//
// Purpose:
//   Connects the viewer to the VisIt client that launched it.
//
// Arguments:
//   argc : The number of command line args.
//   argv : The command line args.
//
// Returns:
//
// Note:       Calling this method is optional unless you want your viewer
//             application to be driven by a client application.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:34:48 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::Connect(int *argc, char ***argv)
{
    viewer->Connect(argc, argv);
}

// ****************************************************************************
// Method: VisItViewer::SetWindowCreationCallback
//
// Purpose:
//   This method installs a window creation function so that you have more
//   control over how your vtkQtRenderWindow objects are created.
//
// Arguments:
//   wcc : The window creation callback function.
//   wccdata : Data to be passed to the window creation callback function.
//
// Returns:
//
// Note:       This method must be called before Setup() to have any effect
//             on the first vis window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:37:03 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr 14 13:45:06 PDT 2009
//   Use ViewerProperties.
//
// ****************************************************************************

void
VisItViewer::SetWindowCreationCallback(vtkQtRenderWindow* (*wcc)(void *),
                                       void *wccdata)
{
    // Turn off using the window metrics because we're going to embed our
    // windows in another application's widgets.
    viewer->GetViewerProperties()->SetUseWindowMetrics(false);
    // Set the window creation callback.
    QtVisWindow::SetWindowCreationCallback(wcc, wccdata);
//    ViewerWindowManager::Instance()->DisableWindowDeletion();
}

// ****************************************************************************
// Method: VisItViewer::Setup
//
// Purpose:
//   Sets up most of the viewer objects.
//
// Arguments:
//
// Returns:
//
// Note:       This method should be called after Connect, ProcessCommandLine
//             but before any
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:36:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::Setup()
{
    // If the plugin dir has not been set then let's set it in the plugin
    // managers based on the current directory.
    if(getenv("VISITPLUGINDIR") == NULL)
    {

        std::string pluginDir(GetVisItHome());
#ifndef _WIN32
        pluginDir += "/plugins";
#endif

        viewer->GetOperatorPluginManager()->SetPluginDir(pluginDir.c_str());
        viewer->GetPlotPluginManager()->SetPluginDir(pluginDir.c_str());
    }

    viewer->Initialize();
}

// ****************************************************************************
// Method: VisItViewer::RemoveCrashRecoveryFile
//
// Purpose:
//   Removes the crash recovery file.
//
// Note:       This method should be called on exit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:38:34 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::RemoveCrashRecoveryFile() const
{
    viewer->RemoveCrashRecoveryFile();
}

// ****************************************************************************
// Method: VisItViewer::GetNowinMode
//
// Purpose:
//   Returns whether the viewer has been told to run -nowin.
//
// Returns:    True if the viewer is running -nowin, false otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:38:59 PDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
VisItViewer::GetNowinMode() const
{
    return viewer->GetNowinMode();
}

// ****************************************************************************
// Method: VisItViewer::Methods
//
// Purpose:
//   Returns the methods object that lets you control the viewer.
//
// Arguments:
//
// Returns:    The methods object for controlling the viewer.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:39:33 PDT 2008
//
// Modifications:
//
// ****************************************************************************

ViewerMethods *
VisItViewer::Methods() const
{
    return viewer->GetViewerMethods();
}

ViewerMethods *
VisItViewer::DelayedMethods() const
{
    return viewer->GetViewerDelayedMethods();
}

// ****************************************************************************
// Method: VisItViewer::State
//
// Purpose:
//   Returns the viewer state so you can access various state objects.
//
// Arguments:
//
// Returns:    The viewer state.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:03 PDT 2008
//
// Modifications:
//
// ****************************************************************************

ViewerState *
VisItViewer::State() const
{
    return viewer->GetViewerState();
}

ViewerState *
VisItViewer::DelayedState() const
{
    return viewer->GetViewerDelayedState();
}

// ****************************************************************************
// Method: VisItViewer::Properties
//
// Purpose:
//   Return the viewer properties.
//
// Returns:    The viewer properties.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 14:25:26 PDT 2009
//
// Modifications:
//
// ****************************************************************************

ViewerProperties *
VisItViewer::Properties() const
{
    return viewer->GetViewerProperties();
}

// ****************************************************************************
// Method: VisItViewer::GetMetaData
//
// Purpose:
//   Returns a file's metadata.
//
// Arguments:
//   hostDB : The host:db filename.
//   ts       : The time state [optional].
//
// Returns:    The metadata for the file.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 16:37:55 PDT 2008
//
// Modifications:
//
// ****************************************************************************

const avtDatabaseMetaData *
VisItViewer::GetMetaData(const std::string &hostDB, int ts)
{
    std::string host, db;
    FileFunctions::SplitHostDatabase(hostDB, host, db);
    const avtDatabaseMetaData *md = 0;
    if(ts == -1)
        md = ViewerBase::GetViewerFileServer()->GetMetaData(host, db);
    else
        md = ViewerBase::GetViewerFileServer()->GetMetaDataForState(host, db, ts);
    return md;
}

// ****************************************************************************
// Method: VisItViewer::Error
//
// Purpose:
//   Lets you issue an error message to VisIt clients.
//
// Arguments:
//   msg : The message to issue.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:26 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::Error(const QString &msg)
{
    viewer->GetViewerMessaging()->Error(msg.toStdString());
}

// ****************************************************************************
// Method: VisItViewer::Warning
//
// Purpose:
//   Lets you issue a warning message to VisIt clients.
//
// Arguments:
//   msg : The message to issue.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:26 PDT 2008
//
// Modifications:
//
// ****************************************************************************

void
VisItViewer::Warning(const QString &msg)
{
    viewer->GetViewerMessaging()->Warning(msg.toStdString());
}

// ****************************************************************************
// Method: VisItViewer::GetNumPlotPlugins
//
// Purpose:
//   Return the number of plot plugins.
//
// Arguments:
//
// Returns:    The number of plot plugins.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:42:39 PDT 2008
//
// Modifications:
//
// ****************************************************************************

int
VisItViewer::GetNumPlotPlugins() const
{
    return viewer->GetPlotPluginManager()->GetNEnabledPlugins();
}

// ****************************************************************************
// Method: VisItViewer::GetPlotName
//
// Purpose:
//   Returns the name of the i'th plot plugin.
//
// Arguments:
//   index : The index of the plot plugin whose name we want.
//
// Returns:    The name of the index'th plot plugin.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:42:59 PDT 2008
//
// Modifications:
//
// ****************************************************************************

std::string
VisItViewer::GetPlotName(int index) const
{
    std::string name;
    if(index >= 0 && index < GetNumPlotPlugins())
    {
        std::string id = viewer->GetPlotPluginManager()->GetEnabledID(index);
        name = std::string(viewer->GetPlotPluginManager()->
                               GetViewerPluginInfo(id)->GetName());
    }
    return name;
}

// ****************************************************************************
// Method: VisItViewer::GetPlotIndex
//
// Purpose:
//   Returns the index of the plot having the specified name.
//
// Arguments:
//   plotName : The name of the plot for which we want an index.
//
// Returns:    The index of the plot or -1 on failure.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:43:41 PDT 2008
//
// Modifications:
//
// ****************************************************************************

int
VisItViewer::GetPlotIndex(const std::string &plotName) const
{
    for(int i = 0; i < GetNumPlotPlugins(); ++i)
    {
        if(GetPlotName(i) == plotName)
            return i;
    }
    return -1;
}

// ****************************************************************************
// Method: VisItViewer::GetNumOperatorPlugins
//
// Purpose:
//   Get the number of operator plugins.
//
// Arguments:
//
// Returns:    The number of operator plugins.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:44:25 PDT 2008
//
// Modifications:
//
// ****************************************************************************

int
VisItViewer::GetNumOperatorPlugins() const
{
    return viewer->GetOperatorPluginManager()->GetNEnabledPlugins();
}

// ****************************************************************************
// Method: VisItViewer::GetOperatorName
//
// Purpose:
//   Get the name of the i'th operator.
//
// Arguments:
//   index : The index of the operator whose name we want.
//
// Returns:    The name of the operator.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:45:04 PDT 2008
//
// Modifications:
//
// ****************************************************************************

std::string
VisItViewer::GetOperatorName(int index) const
{
    std::string name;
    if(index >= 0 && index < GetNumOperatorPlugins())
    {
        std::string id = viewer->GetOperatorPluginManager()->GetEnabledID(index);
        name = std::string(viewer->GetOperatorPluginManager()->
                               GetViewerPluginInfo(id)->GetName());
    }
    return name;
}

// ****************************************************************************
// Method: VisItViewer::GetOperatorIndex
//
// Purpose:
//   Gets the index of the operator, given its name.
//
// Arguments:
//   operatorName : The operator name whose index we want.
//
// Returns:    The index of the operator or -1 on failure.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:45:52 PDT 2008
//
// Modifications:
//
// ****************************************************************************

int
VisItViewer::GetOperatorIndex(const std::string &operatorName) const
{
    for(int i = 0; i < GetNumOperatorPlugins(); ++i)
    {
        if(GetOperatorName(i) == operatorName)
            return i;
    }
    return -1;
}

// ****************************************************************************
// Method: VisItViewer::GetVisItCommand
//
// Purpose:
//   Returns the VisIt command used by VISITHOME.
//
// Arguments:
//
// Returns:    The VisIt command used by VISITHOME.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 15:20:06 PDT 2008
//
// Modifications:
//   Kathleen Bonnell, Fri Oct 8 08:57:19 PDT 2010
//   Remove extra leading underscore from _WIN32.
//
// ****************************************************************************

std::string
VisItViewer::GetVisItCommand() const
{
#if defined(_WIN32)
    return GetVisItHome() + "\\visit.exe";
#else
    return GetVisItHome() + "/bin/visit";
#endif
}

// ****************************************************************************
//  Function: ViewerErrorCallback
//
//  Purpose:
//      A callback routine that can issue error messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ViewerErrorCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->GetViewerMessaging()->Error(msg);
}


// ****************************************************************************
//  Function: ViewerWarningCallback
//
//  Purpose:
//      A callback routine that can issue warning messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2005
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ViewerWarningCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->GetViewerMessaging()->Warning(msg);
}

#if !defined(_WIN32) && !defined(Q_OS_MAC)
// ****************************************************************************
//  Function: Log output from a piped system command to debug logs
//
//  Programmer: Mark C. Miller
//  Created:    April 9, 2008
// ****************************************************************************

static void
LogCommand(const char *cmd, const char *truncate_at_pattern)
{
#if !defined(_WIN32)
    char buf[256];
    buf[sizeof(buf)-1] = '\0';
    FILE *pfile = popen(cmd,"r");

    if (pfile)
    {
        debug5 << endl;
        debug5 << "Begin output from \"" << cmd << "\"..." << endl;
        debug5 << "-------------------------------------------------------------" << endl;
        while (fgets(buf, sizeof(buf)-1, pfile) != 0)
        {
            if (truncate_at_pattern && (StringHelpers::FindRE(buf, truncate_at_pattern) != StringHelpers::FindNone))
            {
                debug5 << "############### TRUNCATED #################" << endl;
                break;
            }
            debug5 << buf;
        }
        debug5 << "End output from \"" << cmd << "\"..." << endl;
        debug5 << "-------------------------------------------------------------" << endl;
        debug5 << endl;
        pclose(pfile);
    }
#endif
}
#endif
// ****************************************************************************
//  Function: Log output from xdpyinfo and glxinfo commands w/truncation
//
//  Programmer: Mark C. Miller
//  Created:    April 9, 2008
//
//  Modifications:
//    Mark C. Miller, Thu Apr 10 08:16:51 PDT 2008
//    Truncated glxinfo output
//
//    Sean Ahern, Thu Apr 24 18:17:33 EDT 2008
//    Avoided this if we're on the Mac.
//
//    Kathleen Bonnell, Wed Apr 30 10:59:18 PDT 2008
//    Windows compiler doesn't like 'and', use '&&' instead.
//
//    Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//    Changed interface to DebugStream to obtain current debug level.
//
//    Mark C. Miller, Mon Aug 26 16:57:27 PDT 2013
//    Adjusted strings to LogCommand to ensure we get stderr output too.
//
//    Mark C. Miller, Wed Aug 28 09:53:15 PDT 2013
//    Don't attempt to log this information in nowin mode.
//
//    Mark C. Miller, Wed Aug 28 09:53:15 PDT 2013
//    Undid previous change.
// ****************************************************************************

static void
LogGlxAndXdpyInfo()
{
#if !defined(_WIN32) && !defined(Q_OS_MAC)
    if (DebugStream::Level5())
    {
        LogCommand("sh -c \"xdpyinfo 2>&1\"", "number of visuals"); // truncate at list of visuals
        LogCommand("sh -c \"glxinfo -v -t 2>&1\"", "^Vis  Vis");    // truncate at table of visuals
    }
#endif
}

