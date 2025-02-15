// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisModelFitWindow.h"

#include <ModelFitAtts.h>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QGroupBox>
#include <QWidget>
#include <QPushButton>

#include <QLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QvisVariableButton.h>

#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisModelFitWindow::QvisModelFitWindow
//
// Purpose: 
//   Constructor
//
// Note:       Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QvisModelFitWindow::QvisModelFitWindow(const int type,
                         ModelFitAtts *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
    num_relats = 0;
    modelNumbers.push_back(0);
    selection_type.push_back(0);
    distance_type.push_back(0);
    input_space.push_back(0);
    inPrepareTable = false;
}


// ****************************************************************************
// Method: QvisModelFitWindow::~QvisModelFitWindow
//
// Purpose: 
//   Destructor
//
// Note:       Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QvisModelFitWindow::~QvisModelFitWindow()
{
}


// ****************************************************************************
// Method: QvisModelFitWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Note:       Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisModelFitWindow::CreateWindowContents()
{
    
    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *rightSide = new QVBoxLayout();

    QGroupBox *modelBox = new QGroupBox();
    modelBox->setTitle(tr("Models"));
    modelBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    rightSide->addWidget(modelBox);

    QGridLayout *modelLayout = new QGridLayout(modelBox);

    models = new QTableWidget();

    models->setSelectionMode(QAbstractItemView::SingleSelection);
    models->horizontalHeader()->hide();
    models->verticalHeader()->hide();
    models->setShowGrid(false);
    models->setMaximumWidth(150);
    connect(models, SIGNAL(cellChanged(int, int)),
        this, SLOT(storeModelNames(int, int)));
    connect(models, SIGNAL(currentCellChanged(int, int, int, int)),
        this, SLOT(prepareTable(int, int, int, int)));

    modelLayout->addWidget(models, 0, 0, 1, 1);

    //QGridLayout *addModelLayout = new QGridLayout(modelBox);

    addRelat = new QPushButton(QString(tr("Add Model")));
    connect(addRelat, SIGNAL(clicked()),
        this, SLOT(addRelationship()));
    modelLayout->addWidget(addRelat, 1, 0);

    
    //QPushButton *addSpace = new QPushButton(QString(tr("Add Space")));
    //addSpace->setEnabled(false);
    //connect(addSpace, SIGNAL(clicked()),
    //      this, SLOT(addSpaces()));
    //addModelLayout->addWidget(addSpace, 0, 1);
    

    deleteMod = new QPushButton(QString(tr("Delete")));
    connect(deleteMod, SIGNAL(clicked()),
        this, SLOT(deleteRelationship()));
    modelLayout->addWidget(deleteMod, 2, 0);

    //modelLayout->addLayout(addModelLayout);

    rightSide->setStretchFactor(modelBox, 1000);
    mainLayout->addLayout(rightSide);
    QVBoxLayout *calculationLayout = new QVBoxLayout();

    QGroupBox *iSpaceBox = new QGroupBox(tr("Select Input Space:"));
    calculationLayout->addWidget(iSpaceBox);
    QHBoxLayout *iSpaceLayout = new QHBoxLayout(iSpaceBox);
    QButtonGroup *iSpaceType = new QButtonGroup(iSpaceBox);
    QWidget *iSpaceWidget = new QWidget(iSpaceBox);

    vSpace = new QRadioButton(tr("Variable"), iSpaceWidget);
    vSpace->setChecked(true);
    iSpaceType->addButton(vSpace, 0);
    iSpaceLayout->addWidget(vSpace);
    nSpace = new QRadioButton(tr("0-1"), iSpaceWidget);
    iSpaceType->addButton(nSpace, 1);
    iSpaceLayout->addWidget(nSpace);
    lSpace = new QRadioButton(tr("log"), iSpaceWidget);
    iSpaceType->addButton(lSpace, 2);
    iSpaceLayout->addWidget(lSpace);
    pSpace = new QRadioButton(tr("Probability"), iSpaceWidget);
    iSpaceType->addButton(pSpace, 3);
    iSpaceLayout->addWidget(pSpace);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(iSpaceType, SIGNAL(buttonClicked(int)), this,
        SLOT(inputSpaceChanged(int)));
#else
    connect(iSpaceType, SIGNAL(idClicked(int)), this,
        SLOT(inputSpaceChanged(int)));
#endif
    
    QGroupBox *selectionBox = new QGroupBox(tr("Select Calculation Space:"));
    calculationLayout->addWidget(selectionBox);
    QHBoxLayout *selectionLayout = new QHBoxLayout(selectionBox);
    QButtonGroup *selectType = new QButtonGroup(selectionBox);
    QWidget *selectWidget = new QWidget(selectionBox);

    varSpace = new QRadioButton(tr("Variable"), selectWidget);
    varSpace->setChecked(true);
    selectType->addButton(varSpace, 0);
    selectionLayout->addWidget(varSpace);
    normSpace = new QRadioButton(tr("0-1"), selectWidget);
    selectType->addButton(normSpace, 1);
    selectionLayout->addWidget(normSpace);
    logSpace = new QRadioButton(tr("log"), selectWidget);
    selectType->addButton(logSpace, 2);
    selectionLayout->addWidget(logSpace);
    probSpace = new QRadioButton(tr("Probability"), selectWidget);
    selectType->addButton(probSpace, 3);
    selectionLayout->addWidget(probSpace);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(selectType, SIGNAL(buttonClicked(int)), this,
        SLOT(selectTypeChanged(int)));
#else
    connect(selectType, SIGNAL(idClicked(int)), this,
        SLOT(selectTypeChanged(int)));
#endif

    distanceBox = new QGroupBox(tr("Distance Selections:"));
    calculationLayout->addWidget(distanceBox);
    QHBoxLayout *distanceLayout = new QHBoxLayout(distanceBox);
    QButtonGroup *distanceType = new QButtonGroup(distanceBox);
    QWidget *distanceWidget = new QWidget(distanceBox);

    euclidean = new QRadioButton(tr("Euclidean"), distanceWidget);
    euclidean->setChecked(true);
    distanceType->addButton(euclidean, 0);
    distanceLayout->addWidget(euclidean);
    manhattan = new QRadioButton(tr("Manhattan"), distanceWidget);
    distanceType->addButton(manhattan, 1);
    distanceLayout->addWidget(manhattan);
    chebyshev = new QRadioButton(tr("Maximum"), distanceWidget);
    distanceType->addButton(chebyshev, 2);
    distanceLayout->addWidget(chebyshev);
    QRadioButton *invisible = new QRadioButton(tr(""), distanceWidget);
    invisible->setEnabled(false);
    distanceLayout->addWidget(invisible);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(distanceType, SIGNAL(buttonClicked(int)), this,
        SLOT(distanceTypeChanged(int)));
#else
    connect(distanceType, SIGNAL(idClicked(int)), this,
        SLOT(distanceTypeChanged(int)));
#endif
        
    QGroupBox *ModelFitBox = new QGroupBox();
    ModelFitBox->setTitle(tr("Define Models"));
    ModelFitBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    calculationLayout->addWidget(ModelFitBox);
    QGridLayout *ModelFitLayout = new QGridLayout(ModelFitBox);
    ModelFit = new QTableWidget(ModelFitBox);
    ModelFitLayout->addWidget(ModelFit, 1, 0, 1, 3);
    ModelFit->setSelectionMode(QAbstractItemView::SingleSelection);
    ModelFit->horizontalHeader()->show();
    ModelFit->verticalHeader()->show();
    ModelFit->setMinimumWidth(400);
    ModelFit->setMinimumHeight(200);
    connect(ModelFit, SIGNAL(cellChanged(int, int)),
        this, SLOT(storeTableContents(int, int)));

    addVar = new QvisVariableButton(false,
        true, false, QvisVariableButton::Scalars, ModelFitBox);
    addVar->setText(tr("Add Variable"));
    addVar->setChangeTextOnVariableChange(false);
    addVar->setEnabled(false);
    connect(addVar, SIGNAL(activated(const QString &)), this,
        SLOT(varAdded(const QString &)));
    ModelFitLayout->addWidget(addVar, 2, 1);

    /*addModel = new QvisVariableButton(false,
        true, false, QvisVariableButton::Scalars, ModelFitBox);
    addModel->setText(tr("Add Model"));
    addModel->setChangeTextOnVariableChange(false);
    addModel->setEnabled(false);
    ModelFitLayout->addWidget(addModel, 2, 1);*/
    addTup = new QPushButton(QString(tr("Add Point")),
        ModelFitBox);
    addTup->setEnabled(false);
    connect(addTup, SIGNAL(clicked()),
        this, SLOT(addTuple()));
    ModelFitLayout->addWidget(addTup, 2, 2);
    deleteVar = new QPushButton(QString(tr("Delete Variable")),
        ModelFitBox);
    deleteVar->setEnabled(false);
    connect(deleteVar, SIGNAL(clicked()),
        this, SLOT(deleteVariable()));
    ModelFitLayout->addWidget(deleteVar, 3, 1);
    deleteTup = new QPushButton(QString(tr("Delete Point")),
        ModelFitBox);
    deleteTup->setEnabled(false);
    connect(deleteTup, SIGNAL(clicked()),
        this, SLOT(deletePoint()));
    ModelFitLayout->addWidget(deleteTup, 3, 2);

    calculationLayout->setStretchFactor(ModelFitBox, 1000);
    mainLayout->addLayout(calculationLayout);

    topLayout->addLayout(mainLayout, 100);
}


// ****************************************************************************
// Method: QvisModelFitWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Note:       Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
QvisModelFitWindow::UpdateWindow(bool doAll)
{
    numVars.clear();
    numVars   = atts->GetNumVars();
    num_relats = (int)numVars.size();
    numPoints.clear();
    numPoints = atts->GetNumTups(); 
    modelNumbers = atts->GetModelNums();

    modelNames = atts->GetModelNames();

    stringVector Vars = atts->GetVars();
    doubleVector tuples = atts->GetTuples();
    doubleVector thold  = atts->GetThold();
    unsignedCharVector statTuples = atts->GetStatTuples();

    guiVarNames.clear();
    int cur_pos = 0;
    for(size_t i = 0; i < numVars.size(); i++)
        for(int j = 0; j < numVars[i]; j++)
            guiVarNames.insert(std::pair<int, string>((int)i, Vars[cur_pos++]));
    
    theTable.clear();
    char tableString[20];
    cur_pos = 0;
    for(size_t i = 0; i < numVars.size(); i++)
        for(int j = 0; j < numVars[i]; j++)
            for(int k = 0; k < numPoints[i]; k++)
            {
                if(statTuples[cur_pos] != '-')
                    sprintf(tableString, "%c", statTuples[cur_pos]);
                else
                {
                    if(thold[cur_pos] > 0)
                        sprintf(tableString, "%.5lf-%.5lf", tuples[cur_pos]-thold[cur_pos],
                                tuples[cur_pos]+thold[cur_pos]);
                    else
                        sprintf(tableString, "%.5lf", tuples[cur_pos]);
                }
                theTable.insert(std::pair<int, tableEntry>((int)i, tableEntry(j, k, tableString)));
                cur_pos++;
            }

    selection_type = atts->GetSelectionType();
    input_space = atts->GetInputSpace();
    distance_type = atts->GetDistanceType();

    if(modelNames.size())
    {
        if(cur_row >= 0)
            prepareTable(cur_row, 0, 0, 0);
        else
            prepareTable(0, 0, 0, 0);
    }
    else
        prepareTable(-1, 0, 0, 0);
}

// ****************************************************************************
// Method: QvisModelFitWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Note:       Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
QvisModelFitWindow::GetCurrentValues(int which_widget)
{
    if(!num_relats)
        numPoints.push_back(-1);

    stringVector Vars;
    doubleVector Tuples, thold;
    unsignedCharVector StatTuples;
    unsigned char tupleTemp;
    int table_start;

    std::string inTable;
    size_t found;
    std::string snum1, snum2;
    float num1, num2;

    for(int i = 0; i < num_relats; i++)
    {
        for(std::multimap<int, std::string>::iterator j = guiVarNames.begin(); j != guiVarNames.end(); j++)
            if( (*j).first == i)
                Vars.push_back((*j).second);
        for(int j = 0; j < numPoints[i]*numVars[i]; j++)
        {
            Tuples.push_back(0);
            StatTuples.push_back('-');
        }

        for(std::multimap<int, tableEntry>::iterator j = theTable.begin(); j != theTable.end(); j++)
            if( (*j).first == i)
            {
                table_start = 0;
                for(int k = 0; k < i; k++)
                    table_start += numPoints[k]*numVars[k];

                tupleTemp = (*j).second.getContents().c_str()[0];
                if(tupleTemp == 'a' || tupleTemp == 'M' || tupleTemp == 'm')
                {
                    StatTuples[table_start + (*j).second.getRow()*numPoints[i] + (*j).second.getCol()] = tupleTemp;
                    thold.push_back(-1);
                }
                else
                {
                    inTable = (*j).second.getContents().c_str();
                    found = inTable.find('-', 0);
                    if(found != inTable.npos)
                    {
                        snum1 = inTable.substr(0, found);
                        snum2 = inTable.substr(found+1, inTable.npos);

                        num1 = atof(snum1.c_str());
                        num2 = atof(snum2.c_str());         

                        Tuples[table_start + (*j).second.getRow()*numPoints[i] + (*j).second.getCol()] = num1 + (num2 - num1)/2;
                        thold.push_back((num2 - num1)/2);
                    }
                    else
                    {
                        Tuples[table_start + (*j).second.getRow()*numPoints[i] + (*j).second.getCol()] = atof((*j).second.getContents().c_str());
                        thold.push_back(-1);
                    }
                }
            }
    }

    atts->SetNumVars(numVars);
    atts->SetNumTups(numPoints);
    atts->SetVars(Vars);
    atts->SetTuples(Tuples);
    atts->SetThold(thold);
    atts->SetStatTuples(StatTuples);
    atts->SetSelectionType(selection_type);
    atts->SetInputSpace(input_space);
    atts->SetDistanceType(distance_type);
    atts->SetModelNames(modelNames);
    atts->SetModelNums(modelNumbers);
}


//
// Qt Slot functions
//

void
QvisModelFitWindow::addRelationship()
{
    numVars.push_back(0);
    numPoints.push_back(0);

    int nrows = models->rowCount();
    models->setRowCount(nrows+1);
    models->setColumnCount(1);
    models->horizontalHeader()->resizeSections(QHeaderView::Stretch);

    char modelName[20];
    if(!modelNumbers.size())
        modelNumbers.push_back(0);
    modelNumbers.push_back(modelNumbers[modelNumbers.size()-1]+1);
    sprintf(modelName, "Model %d", modelNumbers[modelNumbers.size()-1]);
    modelNames.push_back(modelName);
    QTableWidgetItem *name_item = new QTableWidgetItem(tr(modelName));

    name_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    models->setItem(nrows, 0, name_item);
    models->setCurrentCell(nrows, 0);

    num_relats++;
    if(num_relats != (int)input_space.size())
    {
        selection_type.push_back(0);
        distance_type.push_back(0);
        input_space.push_back(0);
    }

    prepareTable(nrows, 0, 0, 0);
}

void
QvisModelFitWindow::varAdded(const QString &varName)
{
    bool already_contained = false;

    for(std::multimap<int, string>::iterator i = guiVarNames.begin(); i != guiVarNames.end(); i++)
        if((*i).first == cur_row)
            if((*i).second == varName.toStdString())
                already_contained= true;

    if(already_contained)
    {
        prepareTable(cur_row, 0, 0, 0);
        return;
    }

    numVars[cur_row]++;
    if(!numPoints[cur_row])
        numPoints[cur_row]++;
    guiVarNames.insert(std::pair<int, std::string>(cur_row, varName.toStdString()));

    prepareTable(cur_row, 0, 0, 0);
}

void
QvisModelFitWindow::addTuple()
{
    numPoints[cur_row]++;
    prepareTable(cur_row, 0, 0, 0);
}

void
QvisModelFitWindow::selectTypeChanged(int buttonID)
{
    selection_type[cur_row] = buttonID;
    prepareTable(cur_row, 0, 0, 0);
}

void
QvisModelFitWindow::inputSpaceChanged(int buttonID)
{
    input_space[cur_row] = buttonID;
    prepareTable(cur_row, 0, 0, 0);
}

void 
QvisModelFitWindow::distanceTypeChanged(int buttonID)
{
    distance_type[cur_row] = buttonID;
    prepareTable(cur_row, 0, 0, 0);
}

void
QvisModelFitWindow::deleteVariable()
{
    int currentRow = ModelFit->currentRow();
    int currentCol = ModelFit->currentColumn();

    int nextSelected = currentRow;

    if(currentRow == -1)
        currentRow = ModelFit->rowCount()-1;
    if(currentRow == -1)
        return;

    std::multimap<int, tableEntry>   td;  //temp for deletion
    std::multimap<int, std::string> afd;  //adjusted for delete

    for(std::multimap<int, string>::iterator i = guiVarNames.begin(); i!= guiVarNames.end(); i++)
    {
        if( (*i).first == cur_row)
        {
            if( (*i).second != vHeaderLbls[currentRow].toStdString())
                afd.insert(std::pair<int, std::string>((*i).first, (*i).second));
        }
        else
            afd.insert(std::pair<int, std::string>((*i).first, (*i).second));
    }
    
    guiVarNames.clear();
    for(std::multimap<int, std::string>::iterator i = afd.begin(); i != afd.end(); i++)
        guiVarNames.insert(std::pair<int, std::string>((*i).first, (*i).second));
    afd.clear();

    for(std::multimap<int, tableEntry>::iterator i = theTable.begin(); i != theTable.end(); i++)
    {
        if((*i).first == cur_row)
        {
            if( (*i).second.getRow() != currentRow)
            {
                if((*i).second.getRow() > currentRow)
                    td.insert(std::pair<int, tableEntry>(cur_row, tableEntry((*i).second.getRow() - 1, (*i).second.getCol(), (*i).second.getContents())));
                else
                    td.insert(std::pair<int, tableEntry>(cur_row, tableEntry((*i).second.getRow(),     (*i).second.getCol(), (*i).second.getContents())));
            }
        }
        else
            td.insert(std::pair<int, tableEntry>((*i).first, tableEntry((*i).second.getRow(), (*i).second.getCol(),   (*i).second.getContents())));
    }

    theTable.clear();
    for(std::multimap<int, tableEntry>::iterator i = td.begin(); i != td.end(); i++)
        theTable.insert(std::pair<int, tableEntry>((*i).first, (*i).second));
    td.clear();

    ModelFit->removeRow(currentRow);
    numVars[cur_row]--;
    if(!numVars[cur_row])
        numPoints[cur_row] = 0;

    prepareTable(cur_row, 0, 0, 0);

    if(nextSelected == ModelFit->rowCount())
        nextSelected--;
    if(nextSelected != -1)
        ModelFit->setCurrentCell(nextSelected, currentCol);
}

void 
QvisModelFitWindow::deletePoint()
{
    int currentRow = ModelFit->currentRow();
    int currentCol = ModelFit->currentColumn();

    int nextSelected = currentCol;

    if(currentCol == -1)
        currentCol = ModelFit->columnCount()-1;
    if(currentCol == -1)
        return;

    std::multimap<int, tableEntry> td;  //temp for deletion

    for(std::multimap<int, tableEntry>::iterator i = theTable.begin(); i != theTable.end(); i++)
    {
        if((*i).first == cur_row)
        {
            if( (*i).second.getCol() != currentCol)
            {
                if((*i).second.getCol() > currentCol)
                    td.insert(std::pair<int, tableEntry>(cur_row, tableEntry((*i).second.getRow(), (*i).second.getCol()-1, (*i).second.getContents())));
                else
                    td.insert(std::pair<int, tableEntry>(cur_row, tableEntry((*i).second.getRow(), (*i).second.getCol(),   (*i).second.getContents())));
            }
        }
        else
            td.insert(std::pair<int, tableEntry>((*i).first, tableEntry((*i).second.getRow(), (*i).second.getCol(),   (*i).second.getContents())));
    }

    theTable.clear();
    for(std::multimap<int, tableEntry>::iterator i = td.begin(); i != td.end(); i++)
        theTable.insert(std::pair<int, tableEntry>((*i).first, (*i).second));
    td.clear();

    ModelFit->removeColumn(currentCol);
    numPoints[cur_row]--;

    prepareTable(cur_row, 0, 0, 0);

    if(nextSelected == ModelFit->columnCount())
        nextSelected--;
    if(nextSelected != -1)
        ModelFit->setCurrentCell(currentRow, nextSelected);
}

void
QvisModelFitWindow::deleteRelationship()
{
    int currentRow = models->currentRow();
    if(currentRow == -1)
        return;

    models->removeRow(currentRow);
    numVars.erase(numVars.begin()+currentRow);
    numPoints.erase(numPoints.begin()+currentRow);
    input_space.erase(input_space.begin()+currentRow);
    modelNumbers.erase(modelNumbers.begin()+currentRow+1);
    modelNames.erase(modelNames.begin()+currentRow);
    distance_type.erase(distance_type.begin()+currentRow);
    selection_type.erase(selection_type.begin()+currentRow);

    std::multimap<int, string>afd;     //adjusted for delete
    std::multimap<int, tableEntry> td; //temp for deletions

    for(std::multimap<int, string>::iterator i = guiVarNames.begin(); i != guiVarNames.end(); i++)
        if( (*i).first != currentRow)
        {
            if( (*i).first > currentRow)
                afd.insert(std::pair<int, std::string>((*i).first - 1, (*i).second));
            else
                afd.insert(std::pair<int, std::string>((*i).first    , (*i).second));
        }

    guiVarNames.clear();
    for(std::multimap<int, string>::iterator i = afd.begin(); i != afd.end(); i++)
        guiVarNames.insert(std::pair<int, std::string>((*i).first, (*i).second));
    afd.clear();

    for(std::multimap<int, tableEntry>::iterator i = theTable.begin(); i != theTable.end(); i++)
        if( (*i).first != currentRow)
        {
            if( (*i).first > currentRow)
                td.insert(std::pair<int, tableEntry>((*i).first - 1, (*i).second));
            else
                td.insert(std::pair<int, tableEntry>((*i).first    , (*i).second));
        }

    theTable.clear();
    for(std::multimap<int, tableEntry>::iterator i = td.begin(); i != td.end(); i++)
        theTable.insert(std::pair<int, tableEntry>((*i).first, (*i).second));
    td.clear();

    num_relats--;     
    if(!num_relats)
    {
        selection_type.push_back(0);
        distance_type.push_back(0);
        input_space.push_back(0);
    }

    prepareTable(models->currentRow(), 0, 0, 0);
}

void
QvisModelFitWindow::fillTables()
{
    QTableWidgetItem *fill;
    std::multimap<int, tableEntry>::iterator it;

    for(int i = 0; i < ModelFit->rowCount(); i++)
        for(int j = 0; j < ModelFit->columnCount(); j++)
            if(ModelFit->item(i, j) == 0)
            {
                fill = new QTableWidgetItem(tr("-"));
                fill->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
                fill->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                ModelFit->setItem(i, j, fill);
                for(it = theTable.begin(); it != theTable.end(); it++)
                    if( (*it).first == cur_row && (*it).second.getRow() == i &&
                        (*it).second.getCol() == j)
                        break;
                if(it == theTable.end())
                    theTable.insert(std::pair<int, tableEntry>(cur_row, tableEntry(i, j, "-")));
            }

    for(int i = 0; i < models->rowCount(); i++)
        if(models->item(i, 0) == 0)
        {
                fill = new QTableWidgetItem(tr("-"));
                fill->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
                models->setItem(i, 0, fill);
        }
}

void
QvisModelFitWindow::storeTableContents(int row, int col)
{
    for(std::multimap<int, tableEntry>::iterator i = theTable.begin(); i != theTable.end(); i++)
        if( (*i).first == cur_row && (*i).second.getRow() == row &&
                (*i).second.getCol() == col)
            if(ModelFit->item(row, col)->text().toStdString() != "-")
                (*i).second.setContents(ModelFit->item(row, col)->text().toStdString());
}

void
QvisModelFitWindow::storeModelNames(int row, int col)
{
    if(inPrepareTable)
        return;

    modelNames.clear();
    for(int i = 0; i < models->rowCount(); i++)
        modelNames.push_back(models->item(i, 0)->text().toStdString());
}

void 
QvisModelFitWindow::prepareTable(int row, int col, int na1, int na2)
{
    cur_row = row;
    int nrows = 0;
    int ncols = 0;
    char pointName[20];

    if(inPrepareTable)
        return;
    inPrepareTable = true;
        
    ModelFit->setRowCount(0);
    if(vHeaderLbls.size())
    {
        vHeaderLbls.clear();
        ModelFit->setVerticalHeaderLabels(vHeaderLbls);
    }
    ModelFit->setColumnCount(0);
    if(hHeaderLbls.size())
    {
        hHeaderLbls.clear();
        ModelFit->setHorizontalHeaderLabels(hHeaderLbls);
    }

    if(cur_row == -1)
    {
        addVar->setEnabled(false);
        addTup->setEnabled(false);
        deleteVar->setEnabled(false);
        deleteTup->setEnabled(false);
    }

    addVar->setEnabled(true);
        
    for(std::multimap<int, string>::iterator i = guiVarNames.begin(); i != guiVarNames.end(); i++)
        if( (*i).first == cur_row)
        {
            vHeaderLbls << tr( (*i).second.c_str());
            nrows++;
        }
    
    ModelFit->setRowCount(nrows);
    ModelFit->setVerticalHeaderLabels(vHeaderLbls);
    
    if(vHeaderLbls.size() > 0)
    {
        addTup->setEnabled(true);
        deleteVar->setEnabled(true);
    }
    else
    {
        addTup->setEnabled(false);
        deleteVar->setEnabled(false);
    }
    if(cur_row < 0 || numPoints[cur_row] < 2)
        deleteTup->setEnabled(false);
    else
        deleteTup->setEnabled(true);
    
    for(int i = 0; cur_row >= 0 && i < numPoints[cur_row]; i++)
    {
        sprintf(pointName, "p %d", i+1);
        hHeaderLbls << tr(pointName);
        ncols++;
    }

    models->setRowCount((int)modelNames.size());
    ModelFit->setColumnCount(ncols);
    ModelFit->setHorizontalHeaderLabels(hHeaderLbls);
    
    fillTables();

    for(int i = 0; i < models->rowCount(); i++)
        models->item(i, 0)->setText(QString(modelNames[i].c_str()));

    models->setCurrentCell(cur_row, 0);
    
    for(std::multimap<int, tableEntry>::iterator i = theTable.begin(); i != theTable.end(); i++)
        if((*i).first == cur_row)
            ModelFit->item( (*i).second.getRow(), (*i).second.getCol())->setText(QString((*i).second.getContents().c_str()));

    inPrepareTable = false;

    if(cur_row < 0)
        return;
    switch(input_space[cur_row])
    {
        case 0:
            vSpace->setChecked(true);
            break;
        case 1:
            nSpace->setChecked(true);
            break;
        case 2:
            lSpace->setChecked(true);
            break;
        case 3:
            pSpace->setChecked(true);
    }
    switch(selection_type[cur_row])
    {
        case 0:
            varSpace->setChecked(true);
            break;
        case 1:
            normSpace->setChecked(true);
            break;
        case 2:
            logSpace->setChecked(true);
            break;
        case 3:
            probSpace->setChecked(true);
    }
    switch(distance_type[cur_row])
    {
        case 0:
            euclidean->setChecked(true);
            break;
        case 1:
            manhattan->setChecked(true);
            break;
        case 2:
            chebyshev->setChecked(true);
    }
}
