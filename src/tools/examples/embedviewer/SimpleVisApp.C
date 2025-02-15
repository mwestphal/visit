// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SimpleVisApp.h>
#include <VisItViewer.h>
#include <ViewerMethods.h>

#include <QApplication>
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMenuBar>
#include <QMenu>
#include <QRadioButton>
#include <QSpinBox>
#include <QWidget>

// State objects that we use.
#include <avtDatabaseMetaData.h>
#include <SaveWindowAttributes.h>

// Include this last since something about it on X11 systems seems to
// interfere with some of our other enums. X11 headers must have some
// naughty #define directives.
#include <vtkQtRenderWindow.h>

#include "CommandParser.h"

QWidget *
create_application_main_window(VisItViewer *v, int *argc, char ***argv)
{
    return new SimpleVisApp(v, argc, argv);
}

void
show_application_main_window(QWidget *w)
{
    // Do it like this since virtual show isn't working...
    SimpleVisApp *app = (SimpleVisApp *)w;
    app->show();
    app->raise();
    app->execFile(1);
}

vtkQtRenderWindow *
SimpleVisApp::ReturnVisWin(void *data)
{
    SimpleVisApp *This = (SimpleVisApp *)data;
    return This->viswin;
}

// ****************************************************************************
// Method: SimpleVisApp::SimpleVisApp
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   v : The VisItViewer object that we'll use to control the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 21 10:23:40 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri Nov 21 10:23:46 PST 2008
//   Fixed some slots for Qt 4.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

SimpleVisApp::SimpleVisApp(VisItViewer *v, int *argc, char ***argv)
    : QMainWindow()
{
    viewer = v;
    setWindowTitle(tr("Simple visualization"));
    plotType = 0;

    // Create the window.
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *hLayout = new QHBoxLayout(central);
    hLayout->setContentsMargins(10,10,10,10);
    hLayout->setSpacing(10);
    QVBoxLayout *leftLayout = new QVBoxLayout(0);
    leftLayout->setSpacing(10);
    hLayout->addLayout(leftLayout);

    scalarLabel = new QLabel(tr("Scalar variables"), central);
    leftLayout->addWidget(scalarLabel);

    variables = new QListWidget(central);
    leftLayout->addWidget(variables);
    connect(variables, SIGNAL(currentTextChanged(const QString &)),
            this, SLOT(changeVariable(const QString &)));

    plotTypeWidget = new QWidget(central);
    leftLayout->addWidget(plotTypeWidget);
    QHBoxLayout *ptLayout = new QHBoxLayout(plotTypeWidget);
    ptLayout->setSpacing(10);
    ptLayout->addWidget(new QLabel(tr("Plot type"), plotTypeWidget));
    plotType = new QButtonGroup(plotTypeWidget);
    QRadioButton *rb = new QRadioButton(tr("Pseudocolor"), plotTypeWidget);
    plotType->addButton(rb, 0);
    ptLayout->addWidget(rb);
    rb = new QRadioButton(tr("Contour"), plotTypeWidget);
    plotType->addButton(rb, 1);
    ptLayout->addWidget(rb);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(plotType, SIGNAL(buttonClicked(int)),
            this, SLOT(changePlotType(int)));
#else
    connect(plotType, SIGNAL(idClicked(int)),
            this, SLOT(changePlotType(int)));
#endif

    contourWidget = new QWidget(central);
    leftLayout->addWidget(contourWidget);
    QHBoxLayout *cLayout = new QHBoxLayout(contourWidget);
    cLayout->setSpacing(10);
    nContours = new QSpinBox(contourWidget);
    nContours->setRange(1,40);
    nContours->setValue(10);
    connect(nContours, SIGNAL(valueChanged(int)),
            this, SLOT(setNContours(int)));
    cLayout->addWidget(new QLabel(tr("Number of contours"), contourWidget));
    cLayout->addWidget(nContours);

    // Create the vis window directly.
    viswin = new vtkQtRenderWindow(central);
    viswin->setMinimumSize(QSize(500,500));
    hLayout->addWidget(viswin, 100);

    // Create menus
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("Open . . ."), this, SLOT(selectFile()));
    fileMenu->addAction(tr("Open commands. . ."), this, SLOT(execFile()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Save window"), this, SLOT(saveWindow()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Quit"), this, SLOT(quitApp()));
    
    QMenu *controlsMenu = menuBar()->addMenu(tr("Controls"));
    controlsMenu->addAction(tr("Open GUI"), this, SLOT(openGUI()));

    //
    // Register a window creation function (before Setup) that will
    // return the vtkQtRenderWindow objects that we've already
    // parented into our interface.
    //
    viewer->SetWindowCreationCallback(ReturnVisWin, (void *)this);

    // Set the initial widget sensitivity.
    resetWindow();

    cmd = 0;

    if (*argc > 1)
        clfilename = (*argv)[1];
}

SimpleVisApp::~SimpleVisApp()
{
    delete cmd;
}

void
SimpleVisApp::quitApp()
{
    qApp->quit();
}

void
SimpleVisApp::resetWindow()
{
    variables->blockSignals(true);
    variables->clear();
    variables->blockSignals(false);

    plotType->blockSignals(true);
    plotType->button(0)->setChecked(true);
    plotType->blockSignals(false);

    scalarLabel->setEnabled(false);
    variables->setEnabled(false);
    plotTypeWidget->setEnabled(false);
    contourWidget->setEnabled(false);

    viewer->Methods()->DeleteActivePlots();
}

void
SimpleVisApp::show()
{
    // Tell the viewer to hide toolbars in all windows.
    viewer->Methods()->HideToolbars(true);
    // Tell the viewer to show all windows. This does not show our windows
    // since our windows are embedded but it does do some extra setup for
    // the windows and thus needs to be called.
    viewer->Methods()->ShowAllWindows();

    QMainWindow::show();
}

//
// Qt slots
//

// ****************************************************************************
// Method: SimpleVisApp::selectFile
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 21 10:23:40 PST 2008
//
// Modifications:
//   Kathleen Bonnell, Fri Oct 8 08:55:44 PDT 2010
//   Make sure path separators are converted to native format. 
//
// ****************************************************************************

void
SimpleVisApp::selectFile()
{
    // Get a filename from the file dialog.
    QString filename = QFileDialog::getOpenFileName(this,
               tr("Open File"),
               QDir::current().path(),
               tr("Data files (*.silo *.vtk *.cgns *.nc *.h5 *.pdb *.visit)"));

    filename = QDir::toNativeSeparators(filename);

    openFile(filename);
}

void
SimpleVisApp::openFile(const QString &filename)
{
    if(!filename.isEmpty())
    {
        // Open the file.
        viewer->Methods()->OpenDatabase(filename.toStdString());

        // Get the file's metadata and populate the variable list.
        const avtDatabaseMetaData *md = viewer->GetMetaData(filename.toStdString());
        if(md != 0)
        {
            variables->blockSignals(true);
            for(int i = 0; i < md->GetNumScalars(); ++i)
                variables->addItem(md->GetScalar(i)->name.c_str());
            variables->setCurrentRow(0);
            variables->blockSignals(false);
            if(md->GetNumScalars() > 0)
            {
                variables->setEnabled(true);
                plotTypeWidget->setEnabled(true);
                contourWidget->setEnabled(plotType->checkedId() == 1);
                // Add a plot of the first variable
                changePlotType(plotType->checkedId());
            }
            scalarLabel->setEnabled(true);
            variables->setEnabled(true);
        }
        else
            resetWindow();
    }
    else
        resetWindow();
}

void
SimpleVisApp::changeVariable(const QString &var)
{
    viewer->Methods()->ChangeActivePlotsVar(var.toStdString());
}

void
SimpleVisApp::changeVariableAndUpdate(const QString &var)
{
    changeVariable(var);

    variables->blockSignals(true);
    for(int i = 0; i < variables->count(); ++i)
    {
        if(variables->item(i)->text() == var)
        {
            variables->setCurrentRow(i);
            break;
        }
    }
    variables->blockSignals(false);
}

void
SimpleVisApp::changePlotType(int val)
{
    if(variables->currentRow() == -1)
        return;

    // Determine the variable.
    std::string var(variables->currentItem()->text().toStdString());

    // Delete the active plots.
    viewer->Methods()->DeleteActivePlots();
    if(val == 0)
    {
        int Pseudocolor = viewer->GetPlotIndex("Pseudocolor");
        viewer->Methods()->AddPlot(Pseudocolor, var);
    }
    else
    {
        int Contour = viewer->GetPlotIndex("Contour");
        viewer->Methods()->AddPlot(Contour, var);
    }
    contourWidget->setEnabled(val == 1);
    viewer->Methods()->DrawPlots();
}

void
SimpleVisApp::setNContours(int nc)
{
    int Contour = viewer->GetPlotIndex("Contour");
    AttributeSubject *contourAtts = viewer->DelayedState()->GetPlotAttributes(Contour);
    if(contourAtts != 0)
    {
        contourAtts->SetValue("contourNLevels", nc);
        contourAtts->Notify();
        viewer->DelayedMethods()->SetPlotOptions(Contour);
    }
}

void
SimpleVisApp::saveWindow()
{
    // Set the output format to JPEG
    SaveWindowAttributes *swa = viewer->State()->GetSaveWindowAttributes();
    swa->SetFormat(SaveWindowAttributes::JPEG);
    swa->SetScreenCapture(true);

    // Save the window
    viewer->Methods()->SaveWindow();
}

void
SimpleVisApp::openGUI()
{
    stringVector args;
    viewer->DelayedMethods()->OpenClient("GUI", viewer->GetVisItCommand(), args);
}

void
SimpleVisApp::execFile(int openClFile)
{
    QString filename;
    if (openClFile != 0)
    {
        filename = clfilename;
    }
    else
    {
        // Get a filename from the file dialog.
        filename = QFileDialog::getOpenFileName(this,
            tr("Open File"),
            QDir::current().path(),
            tr("command files (*.txt)"));
    }
        
    if(!filename.isEmpty())
    {
        if(cmd == NULL)
        {
            // Create the command parser.
            cmd = new CommandParser(viewer);
            connect(cmd, SIGNAL(openFile(const QString &)),
                    this, SLOT(openFile(const QString &)));
            connect(cmd, SIGNAL(changeVariable(const QString &)),
                    this, SLOT(changeVariableAndUpdate(const QString &)));
            connect(cmd, SIGNAL(changePlotType(int)),
                    this, SLOT(changePlotType(int)));
            connect(cmd, SIGNAL(setNContours(int)),
                    this, SLOT(setNContours(int)));
            connect(cmd, SIGNAL(saveWindow()),
                    this, SLOT(saveWindow()));
            connect(cmd, SIGNAL(quitApp()),
                    this, SLOT(quitApp()));
        }

        cmd->ProcessCommands(filename);
    }
}
