// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vector>

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QTabWidget>
#include <QWidget>

#include <visit-config.h>

#include <QvisMeshManagementWindow.h>
#include <MeshManagementAttributes.h>
#include <ViewerProxy.h>

using std::vector;

// ****************************************************************************
// Method: QvisMeshManagementWindow::QvisMeshManagementWindow
//
// Purpose: 
//   This is the constructor for the QvisMeshManagementWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:37:42 PDT 2003
//   Added playbackModeButtonGroup.
//
//   Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//   Added AllExtraButtons
//
//   Brad Whitlock, Wed Apr  9 11:34:45 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisMeshManagementWindow::QvisMeshManagementWindow(MeshManagementAttributes *subj,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons)
{
    mmAtts = subj;
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::~QvisMeshManagementWindow
//
// Purpose: 
//   This is the destructor for the QvisMeshManagementWindow class.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Oct 7 09:38:31 PDT 2003
//   Deleted playbackModeButtonGroup since it has no parent.
//
// ****************************************************************************

QvisMeshManagementWindow::~QvisMeshManagementWindow()
{
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:39:42 PDT 2002
//   Added a slider for the animation playback speed.
//
//   Brad Whitlock, Mon Oct 6 16:21:12 PST 2003
//   Added radio buttons that allow the user to set the animation style.
//
//   Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//   Changed initialization based on existence of FI library 
//
//   Mark C. Miller, Wed Dec 19 11:32:58 PST 2007
//   Made Qt objects for tolerances a little more both in code and in the
//   running GUI. Changed discretizationTolerances to smallestZone and
//   flatEnough.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Wed Jul  2 11:16:25 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Thu Dec 18 09:28:16 PST 2008
//   Removed "selected" signal connection for the tab widget and the 
//   tabSelected slot. This signal does not exist in Qt4 & the slot code 
//   was empty.
//   
//   Jeremy Meredith, Fri Feb 26 14:13:08 EST 2010
//   Added a new "multi-pass" discretization algorithm
//
//   Mark C. Miller, Wed Mar  3 07:59:15 PST 2010
//   Changed form of conditional compilation check for HAVE_BILIB from
//   numeric test to existence test.
//
//   Cyrus Harrison, Mon Jan 26 21:26:34 PST 2015
//   Changed check from boost interval template lib to boost proper.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::CreateWindowContents()
{

    tabs = new QTabWidget(central);
    topLayout->setSpacing(5);
    topLayout->addWidget(tabs);

    pageCSGGroup = new QWidget(central);
    
    tabs->addTab(pageCSGGroup, tr("CSG Meshing"));

    QVBoxLayout *internalLayoutCSGGroup = new QVBoxLayout(pageCSGGroup);
    internalLayoutCSGGroup->addSpacing(10);
    QGridLayout *layoutCSGGroup = new QGridLayout();
    internalLayoutCSGGroup->addLayout(layoutCSGGroup);
    layoutCSGGroup->setSpacing(5);

    renderCSGDirect = new QCheckBox(tr("Don't discretize. Pass native CSG down pipeline"),
                                    pageCSGGroup);
    connect(renderCSGDirect, SIGNAL(toggled(bool)),
            this, SLOT(renderCSGDirectChanged(bool)));
    layoutCSGGroup->addWidget(renderCSGDirect, 0, 0, 1, 4);
    renderCSGDirect->setEnabled(false);

    discretizeBoundaryOnly = new QCheckBox(tr("Discretize boundary only"),
                                           pageCSGGroup);
    connect(discretizeBoundaryOnly, SIGNAL(toggled(bool)),
            this, SLOT(discretizeBoundaryOnlyChanged(bool)));
    layoutCSGGroup->addWidget(discretizeBoundaryOnly, 1, 0, 1, 4);

    discretizeModeLabel = new QLabel(tr("Discretization mode"), pageCSGGroup);
    layoutCSGGroup->addWidget(discretizeModeLabel, 2, 0);
    discretizationMode = new QButtonGroup(pageCSGGroup);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(discretizationMode, SIGNAL(buttonClicked(int)),
            this, SLOT(discretizationModeChanged(int)));
#else
    connect(discretizationMode, SIGNAL(idClicked(int)),
            this, SLOT(discretizationModeChanged(int)));
#endif
            
    discretizeUniform = new QRadioButton(tr("Uniform"), pageCSGGroup);
    discretizationMode->addButton(discretizeUniform,0);
    layoutCSGGroup->addWidget(discretizeUniform, 2, 1);
    discretizeAdaptive = new QRadioButton(tr("Adaptive"), pageCSGGroup);
    discretizationMode->addButton(discretizeAdaptive,1);
#ifndef HAVE_BOOST
    discretizeAdaptive->setEnabled(false);
#endif
    layoutCSGGroup->addWidget(discretizeAdaptive, 2, 2);
    discretizeMultiPass = new QRadioButton(tr("Multi-pass"), pageCSGGroup);
    discretizationMode->addButton(discretizeMultiPass,2);
    layoutCSGGroup->addWidget(discretizeMultiPass, 2, 3);

    smallestZoneLabel = new QLabel(tr("Smallest zone (% bbox diag)"), pageCSGGroup);
    layoutCSGGroup->addWidget(smallestZoneLabel, 3, 0);
    smallestZoneLineEdit = new QLineEdit(pageCSGGroup);
    connect(smallestZoneLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSmallestZoneText()));
    connect(smallestZoneLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSmallestZoneText(const QString &)));
    layoutCSGGroup->addWidget(smallestZoneLineEdit, 3, 1, 1, 3);

    flatEnoughLabel = new QLabel(tr("Flat enough (recip. curvature)"), pageCSGGroup);
    layoutCSGGroup->addWidget(flatEnoughLabel, 4, 0);
    flatEnoughLineEdit = new QLineEdit(pageCSGGroup);
    connect(flatEnoughLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processFlatEnoughText()));
    connect(flatEnoughLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processFlatEnoughText(const QString &)));
    layoutCSGGroup->addWidget(flatEnoughLineEdit, 4, 1, 1, 3);
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 14 11:49:44 PDT 2002
//   Added animation timeout slider.
//
//   Brad Whitlock, Mon Oct 6 16:24:09 PST 2003
//   Added the animation style button group.
//
//   Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//   Changed behavior based on existence of FI library 
//
//   Mark C. Miller, Wed Dec 19 11:32:58 PST 2007
//   Made Qt objects for tolerances a little more both in code and in the
//   running GUI. Changed discretizationTolerances to smallestZone and
//   flatEnough.
//
//   Brad Whitlock, Wed Dec 19 15:01:30 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Wed Jul  2 11:16:25 PDT 2008
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Fri Feb 26 14:13:08 EST 2010
//   Added a new "multi-pass" discretization algorithm
//
//   Mark C. Miller, Wed Mar  3 07:59:15 PST 2010
//   Changed form of conditional compilation check for HAVE_BILIB from
//   numeric test to existence test.
//
//   Cyrus Harrison, Mon Jan 26 21:26:34 PST 2015
//   Changed check from boost interval template lib to boost proper.
//
// ****************************************************************************

void
QvisMeshManagementWindow::UpdateWindow(bool doAll)
{
    MeshManagementAttributes *atts = (MeshManagementAttributes *)subject;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!atts->IsSelected(i) && !doAll)
            continue;

        QString temp;

        switch(i)
        {
        case MeshManagementAttributes::ID_discretizationTolerance:
            {   const vector<double> tols = atts->GetDiscretizationTolerance();
                char tmp[32];
                snprintf(tmp, sizeof(tmp), "%g ", tols[0]);
                temp += tmp;
                smallestZoneLineEdit->setText(temp);
                temp = "";
                snprintf(tmp, sizeof(tmp), "%g ", tols[1]);
                temp += tmp;
                flatEnoughLineEdit->setText(temp);
            }
            break;
        case MeshManagementAttributes::ID_discretizationToleranceX:
            break;
        case MeshManagementAttributes::ID_discretizationToleranceY:
            break;
        case MeshManagementAttributes::ID_discretizationToleranceZ:
            break;
        case MeshManagementAttributes::ID_discretizationMode:
            {
                MeshManagementAttributes::DiscretizationModes dMode;
                dMode = atts->GetDiscretizationMode();
                discretizationMode->blockSignals(true);
                if (dMode == MeshManagementAttributes::Uniform)
                {
                    discretizationMode->button(0)->setChecked(true);
                    flatEnoughLineEdit->setEnabled(false);
                }
                else if (dMode == MeshManagementAttributes::Adaptive)
                {
#ifdef HAVE_BOOST
                    discretizationMode->button(1)->setChecked(true);
                    flatEnoughLineEdit->setEnabled(true);
#else
                    GUIBase::Warning(tr("Adaptive not available. "
                                     "Missing boost interval template library. "
                                     "Overriding to Uniform."));
                    discretizationMode->button(0)->setChecked(true);
                    flatEnoughLineEdit->setEnabled(false);
#endif
                }
                else if (dMode == MeshManagementAttributes::MultiPass)
                {
                    discretizationMode->button(2)->setChecked(true);
                    flatEnoughLineEdit->setEnabled(false);
                }
                discretizationMode->blockSignals(false);
            }
            break;
        case MeshManagementAttributes::ID_discretizeBoundaryOnly:
            discretizeBoundaryOnly->blockSignals(true);
            discretizeBoundaryOnly->setChecked(atts->GetDiscretizeBoundaryOnly());
            discretizeBoundaryOnly->blockSignals(false);
            break;
        case MeshManagementAttributes::ID_passNativeCSG:
            renderCSGDirect->blockSignals(true);
            renderCSGDirect->setChecked(atts->GetPassNativeCSG());
            renderCSGDirect->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::GetCurrentValues
//
// Purpose:
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Mark C. Miller
// Creation:   Thu Oct 24 10:03:40 PDT 2002
//
// Modifications:
//    Brad Whitlock, Wed Nov 23 11:42:26 PDT 2005
//    Fixed for Qt 3.0.2.
//
//    Cyrus Harrison, Wed Jul  2 11:16:25 PDT 2008
//    Initial Qt4 Port.
//
//    Jeremy Meredith, Fri Feb 26 14:13:08 EST 2010
//    Added a new "multi-pass" discretization algorithm
//
// ****************************************************************************
void
QvisMeshManagementWindow::GetCurrentValues(const QWidget *widget)
{
    const bool doAll = widget == 0;

    double temp;
    if (doAll || widget == smallestZoneLineEdit)
    {
        bool okay = sscanf(smallestZoneLineEdit->displayText().toStdString().c_str(), 
                           "%lg", &temp) == 1;
        if (okay && mmAtts->GetDiscretizationTolerance()[0] != temp)
        {
            vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
            temp1[0] = temp;
            mmAtts->SetDiscretizationTolerance(temp1);
        }
    }

    if (doAll || widget == discretizeAdaptive ||
        widget == discretizeUniform || widget == discretizeAdaptive)
    {
        
        int selectedId = discretizationMode->id(discretizationMode->checkedButton());
        if (selectedId == 0 &&
            mmAtts->GetDiscretizationMode() != MeshManagementAttributes::Uniform)
            mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
        else if (selectedId == 1 &&
                 mmAtts->GetDiscretizationMode() != MeshManagementAttributes::Adaptive)
            mmAtts->SetDiscretizationMode(MeshManagementAttributes::Adaptive);
        else if (selectedId == 2 &&
                 mmAtts->GetDiscretizationMode() != MeshManagementAttributes::MultiPass)
            mmAtts->SetDiscretizationMode(MeshManagementAttributes::MultiPass);
    }

    if (doAll || widget == discretizeBoundaryOnly)
    {
        if (discretizeBoundaryOnly->isChecked() != mmAtts->GetDiscretizeBoundaryOnly())
            mmAtts->SetDiscretizeBoundaryOnly(discretizeBoundaryOnly->isChecked());
    }

    if (doAll || widget == renderCSGDirect)
    {
        if (renderCSGDirect->isChecked() != mmAtts->GetPassNativeCSG())
            mmAtts->SetPassNativeCSG(renderCSGDirect->isChecked());
    }
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::Apply
//
// Purpose: 
//   This method is called when the animation attributes are updated.
//
// Arguments:
//   ignore : Whether or not to ignore the apply.
//
// Programmer: Eric Brugger
// Creation:   Mon Nov 19 14:15:03 PST 2001
//
// ****************************************************************************

void
QvisMeshManagementWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Tell the viewer to set the mesh management attributes.
        if(mmAtts->NumAttributesSelected() > 0 || ignore)
        {
            GetCurrentValues();
            mmAtts->Notify();
        }

        GetViewerMethods()->SetMeshManagementAttributes();
        GUIBase::Warning(tr("Note:  These settings only apply to new plots.  "
                            "To apply them to current plots, re-open the file."));

    }
    else
    {
        // Send the new state to the viewer.
        mmAtts->Notify();
    }
}
 
//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisMeshManagementWindow::apply
//
// Purpose:
//   Qt slot function called when apply button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::makeDefault
//
// Purpose:
//   Qt slot function called when "Make default" button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::makeDefault()
{
    GetCurrentValues();
    mmAtts->Notify();
    GetViewerMethods()->SetDefaultMeshManagementAttributes();
}

// ****************************************************************************
// Method: QvisMeshManagementWindow::makeDefault
//
// Purpose:
//   Qt slot function called when "Make default" button is clicked.
//
// ****************************************************************************

void
QvisMeshManagementWindow::reset()
{
    GetViewerMethods()->ResetMeshManagementAttributes();
}


void
QvisMeshManagementWindow::renderCSGDirectChanged(bool val)
{
    mmAtts->SetPassNativeCSG(val);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::discretizeBoundaryOnlyChanged(bool val)
{
    mmAtts->SetDiscretizeBoundaryOnly(val);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::discretizationModeChanged(int val)
{
    flatEnoughLineEdit->setEnabled(false);
    if (val == 0)
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
    else if (val == 1)
    {
#ifdef HAVE_BOOST
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Adaptive);
        flatEnoughLineEdit->setEnabled(true);
#else
        GUIBase::Warning(tr("Adaptive not available. "
                         "Missing boost interval template library. "
                         "Overriding to Uniform."));
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::Uniform);
#endif
    }
    else if (val == 2)
        mmAtts->SetDiscretizationMode(MeshManagementAttributes::MultiPass);
    SetUpdate(false);
    Apply();
}

void
QvisMeshManagementWindow::processSmallestZoneText()
{
    double temp = -1.0;
    bool okay = sscanf(smallestZoneLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
        temp1[0] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processSmallestZoneText(const QString &tols)
{
    double temp = -1.0;
    bool okay = sscanf(smallestZoneLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
        temp1[0] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processFlatEnoughText()
{
    double temp = -1.0;
    bool okay = sscanf(flatEnoughLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
        temp1[1] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}

void
QvisMeshManagementWindow::processFlatEnoughText(const QString &tols)
{
    double temp = -1.0;
    bool okay = sscanf(flatEnoughLineEdit->displayText().toStdString().c_str(),
                       "%lg", &temp) == 1;

    if (okay && temp >= 0.0)
    {
        vector<double> temp1 = mmAtts->GetDiscretizationTolerance();
        temp1[1] = temp;
        mmAtts->SetDiscretizationTolerance(temp1);
    }
}
