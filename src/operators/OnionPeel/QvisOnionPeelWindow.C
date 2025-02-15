// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisOnionPeelWindow.h"

#include <OnionPeelAttributes.h>
#include <ViewerProxy.h>
#include <avtSIL.h>
#include <SILRestrictionAttributes.h>
#include <QvisSILSetSelector.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimer>
#include <vectortypes.h>



// ****************************************************************************
// Method: QvisOnionPeelWindow::QvisOnionPeelWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Added observation of SILRestrictionAttributes, data member silUseSet.
//   
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Removed defaultItem, silTopSet, silNumSets, silNumCollections, silAtts.
//
//   Brad Whitlock, Fri Jul 18 08:54:49 PDT 2008
//   Initialize adjacencyType button groups.
//
// ****************************************************************************

QvisOnionPeelWindow::QvisOnionPeelWindow(const int type,
                         OnionPeelAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad) 
{
    atts = subj;
    adjacencyType = 0;
    seedType = 0;
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::~QvisOnionPeelWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 08:55:09 PDT 2008
//   Delete button groups.
//
// ****************************************************************************

QvisOnionPeelWindow::~QvisOnionPeelWindow()
{
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004
//   Added useGlobalId checkbox.
//   
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005 
//   Changed maximum for requestedLayer spin box. 
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group. 
//
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replace categoryName/subsetName combo boxes with QvisSILSetSelector.
//
//   Brad Whitlock, Fri Apr 25 08:54:57 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Fri Jul 18 08:54:13 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Tue Jul 22 19:32:57 MST 2014
//   Added honorOriginalMesh comboBox.
//
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisOnionPeelWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(0);
    mainLayout->setContentsMargins(0,0,0,0);
    topLayout->addLayout(mainLayout);

    //
    // Adjacency
    //
    mainLayout->addWidget(new QLabel(tr("Adjacency"), central),0,0);
    adjacencyType = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(adjacencyType, SIGNAL(buttonClicked(int)),
            this, SLOT(adjacencyTypeChanged(int)));
#else
    connect(adjacencyType, SIGNAL(idClicked(int)),
            this, SLOT(adjacencyTypeChanged(int)));
#endif

    QRadioButton *rb = new QRadioButton(tr("Node"), central);
    adjacencyType->addButton(rb, 0);
    mainLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("Face"), central);
    adjacencyType->addButton(rb, 2);
    mainLayout->addWidget(rb, 0, 2);

    //
    // silSet (category/subset)
    //
    intVector roles;
    roles.push_back(SIL_DOMAIN);
    roles.push_back(SIL_BLOCK);
    silSet = new QvisSILSetSelector(central, 
        GetViewerState()->GetSILRestrictionAttributes(), roles);
    connect(silSet, SIGNAL(categoryChanged(const QString &)),
            this, SLOT(categoryChanged(const QString &)));
    connect(silSet, SIGNAL(subsetChanged(const QString &)),
            this, SLOT(subsetChanged(const QString &)));
    mainLayout->addWidget(silSet, 1, 0, 1, 4);

    //
    // Seed
    //
    mainLayout->addWidget(new QLabel(tr("Seed"), central),2,0);
    seedType = new QButtonGroup(central);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(seedType, SIGNAL(buttonClicked(int)),
            this, SLOT(seedTypeChanged(int)));
#else
    connect(seedType, SIGNAL(idClicked(int)),
            this, SLOT(seedTypeChanged(int)));
#endif

    rb = new QRadioButton(tr("Cell"), central);
    seedType->addButton(rb, 0);
    mainLayout->addWidget(rb, 2, 1);
    rb = new QRadioButton(tr("Node"), central);
    seedType->addButton(rb, 1);
    mainLayout->addWidget(rb, 2, 2);
    
    //
    // Index
    //
    mainLayout->addWidget(new QLabel(tr("Seed # or i j [k]"), 
                central),3,0);
    index = new QLineEdit(central);
    index->setText(QString("1"));
    connect(index, SIGNAL(returnPressed()),
            this, SLOT(indexChanged()));
    mainLayout->addWidget(index, 3, 1, 1, 3);

    //
    // UseGlobalId
    //
    useGlobalId = new QCheckBox(tr("Seed # is Global"), central);
    useGlobalId->setChecked(false);
    connect(useGlobalId, SIGNAL(toggled(bool)),
            this, SLOT(useGlobalIdToggled(bool)));
    mainLayout->addWidget(useGlobalId, 4, 0, 1, 3);

    //
    // Layers
    //
    mainLayout->addWidget(new QLabel(tr("Layers"), central),5,0);
    requestedLayer = new QSpinBox(central);
    requestedLayer->setKeyboardTracking(false);
    requestedLayer->setMinimum(0);
    requestedLayer->setMaximum(10000);
    connect(requestedLayer, SIGNAL(valueChanged(int)), 
            this, SLOT(requestedLayerChanged(int)));
    mainLayout->addWidget(requestedLayer, 5, 1, 1, 3);

    //
    // honorOriginalMesh
    //
    honorOriginalMesh = new QComboBox(central);
    honorOriginalMesh->addItem(tr("Honor actual mesh"));
    honorOriginalMesh->addItem(tr("Honor original mesh"));
    connect(honorOriginalMesh, SIGNAL(activated(int)),
            this, SLOT(honorOriginalMeshChanged(int)));
    mainLayout->addWidget(honorOriginalMesh, 6, 0 );
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Feb 26 13:19:40 PST 2004
//   Only update the ComboBoxes if the SILRestrictionAttributes have changed.
//   
//   Kathleen Bonnell, Tue May  4 17:47:50 PDT 2004 
//   Also update the ComboBoxes if doAll is true. 
//    
//   Kathleen Bonnell, Fri Dec 10 14:28:14 PST 2004
//   Added useGlobalId checkbox.
//   
//   Kathleen Bonnell, Wed Jan 19 15:45:38 PST 2005 
//   Added 'seedType' button group. 
//
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Removed calls to UpdateComboBoxes, replace category/subset with silSet.
//
//   Brad Whitlock, Fri Jul 18 09:03:53 PDT 2008
//   Qt 4.
//
//   Kathleen Biagas, Tue Jul 22 19:32:57 MST 2014
//   Added honorOriginalMesh.
//
// ****************************************************************************

void
QvisOnionPeelWindow::UpdateWindow(bool doAll)
{
    QString temp;
    intVector ivec;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case OnionPeelAttributes::ID_adjacencyType:
            adjacencyType->blockSignals(true);
            adjacencyType->button(atts->GetAdjacencyType())->setChecked(true);
            adjacencyType->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_useGlobalId :
            useGlobalId->blockSignals(true);
            useGlobalId->setChecked(atts->GetUseGlobalId());
            silSet->setEnabled(!atts->GetUseGlobalId());
            useGlobalId->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_categoryName:
            silSet->blockSignals(true);
            silSet->SetCategoryName(atts->GetCategoryName().c_str());
            silSet->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_subsetName:
            silSet->blockSignals(true);
            silSet->SetSubsetName(atts->GetSubsetName().c_str());
            silSet->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_index:
            index->blockSignals(true);
            ivec = atts->GetIndex(); 
            char buff[80];
            temp = "";
            for (size_t j = 0; j < ivec.size(); j++) 
            {
                sprintf(buff, "%d ", ivec[j]);
                temp += buff;
            }
            index->setText(temp); 
            index->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_logical:
            break;
        case OnionPeelAttributes::ID_requestedLayer:
            requestedLayer->blockSignals(true);
            requestedLayer->setValue(atts->GetRequestedLayer());
            requestedLayer->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_seedType:
            seedType->blockSignals(true);
            seedType->button(atts->GetSeedType())->setChecked(true);
            seedType->blockSignals(false);
            break;
        case OnionPeelAttributes::ID_honorOriginalMesh:
            honorOriginalMesh->blockSignals(true);
            honorOriginalMesh->setCurrentIndex(atts->GetHonorOriginalMesh()? 1 : 0);
            honorOriginalMesh->blockSignals(false);
            break;

        }
    } // end for
}


// ****************************************************************************
// Method: QvisOnionPeelWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Thu Aug 8 14:29:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005
//   Added logic for requestedLayer.
//   
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced category/subset with silSet.
//
//   Brad Whitlock, Fri Jul 18 09:12:10 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisOnionPeelWindow::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);
    QString msg, temp;

    // Do categoryName && subsetName
    if(doAll)
    {
        atts->SetCategoryName(silSet->GetCategoryName().toStdString());
        atts->SetSubsetName(silSet->GetSubsetName().toStdString());
    }

    // Do index
    if(which_widget == OnionPeelAttributes::ID_index || doAll)
    {
        intVector ivec;
        bool okay = false;
        if(LineEditGetInts(index, ivec))
            okay = ivec.size() > 0 && ivec.size() <= 3;
        if(okay)
        {
            atts->SetLogical(ivec.size() != 1 && !atts->GetUseGlobalId());
            atts->SetIndex(ivec);
        } 
        else 
        {
            msg = tr("The values for the index were invalid. "
                     "Resetting to the last good value.");
            Message(msg);
            atts->SetLogical(atts->GetLogical());
            atts->SetIndex(atts->GetIndex());
        }
    }

    if(which_widget == OnionPeelAttributes::ID_requestedLayer || doAll)
    {
        if (atts->GetRequestedLayer() != requestedLayer->value())
            atts->SetRequestedLayer(requestedLayer->value());
    }
}


//
// Qt Slot functions
//

// ****************************************************************************
// Method: QvisOnionPeelWindow::delayedApply
//
// Purpose: 
//   This is a Qt slot function that delays the Apply when autoupdate is used.
//   We do this because there is an internal timer in the spin boxes that
//   keeps adding signals for the spin boxes when we press the arrows. This
//   causes problems when the plot has no OnionPeel operator and we want to
//   add one with the "Add operator" dialog. Once the operator is added, the
//   pent up signals are processed causing the operator window to go nuts. By
//   having this slot called on a delayed timer, we avoid the problem.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 26 17:41:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisOnionPeelWindow::delayedApply()
{
    Apply(true);
}

void
QvisOnionPeelWindow::adjacencyTypeChanged(int val)
{
    if(val != atts->GetAdjacencyType())
    {
        atts->SetAdjacencyType(OnionPeelAttributes::NodeFace(val));
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

void
QvisOnionPeelWindow::seedTypeChanged(int val)
{
    if(val != atts->GetSeedType())
    {
        atts->SetSeedType(OnionPeelAttributes::SeedIdType(val));
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Changed name from 'categoryNameChanged' to 'categoryChanged'. 
//   SetCategory name directly instead of calling GetCurrentValues. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::categoryChanged(const QString &cname)
{
    atts->SetCategoryName(cname.toStdString());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Changed name from 'subsetNameChanged' to 'subsetChanged'. SetSubsetname
//   directly instead of calling GetCurrentValues. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::subsetChanged(const QString &sname)
{
    atts->SetSubsetName(sname.toStdString());
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

void
QvisOnionPeelWindow::indexChanged()
{
    GetCurrentValues(OnionPeelAttributes::ID_index);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}


// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Tue Jan 18 19:37:46 PST 2005
//   Added call to GetCurrentValues. 
//   
// ****************************************************************************

void
QvisOnionPeelWindow::requestedLayerChanged(int val)
{
    GetCurrentValues(OnionPeelAttributes::ID_requestedLayer);
    if (AutoUpdate())
        QTimer::singleShot(100, this, SLOT(delayedApply()));
    else
        Apply();
}

// ****************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Jun  6 17:22:08 PDT 2007 
//   Replaced 'UpdateComboBoxesEnabled' with 'silSet->setEnabled'.
//   
// ****************************************************************************

void
QvisOnionPeelWindow::useGlobalIdToggled(bool val)
{
    if(val != atts->GetUseGlobalId())
    {
        atts->SetUseGlobalId(val);
        silSet->setEnabled(!val);
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

void
QvisOnionPeelWindow::honorOriginalMeshChanged(int val)
{
    if(val != (int) atts->GetHonorOriginalMesh())
    {
        atts->SetHonorOriginalMesh((bool)val);
        if (AutoUpdate())
            QTimer::singleShot(100, this, SLOT(delayedApply()));
        else
            Apply();
    }
}

