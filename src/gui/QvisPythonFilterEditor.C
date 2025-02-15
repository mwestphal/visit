// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisPythonFilterEditor.h>
#include <QtCore>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>
#ifdef _WIN32
#include <QTemporaryFile>
#endif

#include <QvisPythonSyntaxHighlighter.h>
#include <visit-config.h>
#include <sstream>
#include <Environment.h>
#include <InstallationFunctions.h>

#include <iostream>
using namespace std;

// ****************************************************************************
// Method: QvisPythonFilterEditor::QvisPythonFilterEditor
//
// Purpose:
//   This is the constructor for the QvisPythonFilterEditor class.
//
// Arguments:
//   parent: Parent Widget.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QvisPythonFilterEditor::QvisPythonFilterEditor(QWidget *parent)
: QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    editor = new QTextEdit(this);
    highlighter = new QvisPythonSyntaxHighlighter(editor->document());

    QHBoxLayout *button_layout = new QHBoxLayout();
    cmdLoad = new QPushButton(tr("Load script"),this);

    cmdLoadMenu = new QMenu(this);

    QMenu *loadTempsMenu    = cmdLoadMenu->addMenu(tr("&Template"));

    loadFile       = cmdLoadMenu->addAction(tr("&File"));
    loadTempSimple = loadTempsMenu->addAction(tr("&Simple filter"));
    loadTempAdv    = loadTempsMenu->addAction(tr("&Advanced filter"));

    cmdLoad->setMenu(cmdLoadMenu);

    cmdSave = new QPushButton(tr("Save script"),this);


    button_layout->addWidget(cmdLoad);
    button_layout->addWidget(cmdSave);
    button_layout->addStretch(10);

    layout->addWidget(editor);
    layout->addLayout(button_layout);

    connect(editor, SIGNAL(textChanged()),
            this, SIGNAL(sourceTextChanged()));

    connect(cmdSave, SIGNAL(clicked()),
            this, SLOT(cmdSaveClick()));

    connect(cmdLoadMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(loadMenuEvent(QAction *)));

}

// ****************************************************************************
// Method: QvisPythonFilterEditor::~QvisPythonFilterEditor
//
// Purpose:
//   The destructor for the QvisPythonFilterEditor class.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QvisPythonFilterEditor::~QvisPythonFilterEditor()
{

}

// ****************************************************************************
// Method: QvisPythonFilterEditor::getSource
//
// Purpose:
//   Gets the script from the source editor.
//   Optionally escapes the output.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

QString
QvisPythonFilterEditor::getSource(bool escape)
{
    QString res = editor->toPlainText();
    res = res.trimmed() + QString("\n");

    if(escape)
    {
        res.replace(QString("\""),QString("\\\""));
        res.replace(QString("\n"),QString("\\n"));
        res.replace(QString(" "),QString("\\s"));
    }

    return res;
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::setSource
//
// Purpose:
//   Sets the script in the source editor.
//   Optionally handles escaped input.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::setSource(const QString &py_script, bool escaped)
{
    if(escaped)
    {
        QString res = py_script;
        res.replace(QString("\\\""),QString("\""));
        res.replace(QString("\\n"),QString("\n"));
        res.replace(QString("\\s"),QString(" "));
        editor->setText(res);
    }
    else
        editor->setText(py_script);
}


// ****************************************************************************
// Method: QvisPythonFilterEditor::saveScript
//
// Purpose:
//   Saves a script to a file.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::saveScript(const QString &py_script)
{
    QFile file(py_script);
    if ( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        stream << getSource();
        file.close();
    }
    /*
    else
    ERROR
    */
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::loadScript
//
// Purpose:
//   Loads a script from a file.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
QvisPythonFilterEditor::loadScript(const QString &py_script)
{
    QFile file(py_script);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QTextStream stream( &file );
        setSource(stream.readAll());
        file.close();
    }
    /*
        else
        ERROR
    */
}


// ****************************************************************************
// Method: QvisPythonFilterEditor::cmdSaveClick
//
// Purpose:
//   Slot handler for save click.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//   Cyrus Harrison, Fri May 21 09:51:44 PDT 2010
//   Added 'All Files' to the filter, to support scripts without a '.py'
//   extension.
//
//   Kathleen Bonnell, Fri May 13 13:28:45 PDT 2011
//   On Windows, explicitly test writeability of the 'cwd' before passing it
//   to getSaveFileName (eg don't present user with a place to save a file if
//   they cannot save there!)
//
//   Cyrus Harrison, Wed Apr 11 15:03:17 PDT 2012
//   Add other common python filter extentsions (vpe & vpq)
//
//   Kathleen Biagas, Wed Feb 26 14:40:09 MST 2013
//   Replace commas with spaces in filter string.
//
// ****************************************************************************

void
QvisPythonFilterEditor::cmdSaveClick()
{
    QString useDir = QDir::current().path();

#ifdef _WIN32
    { // new scope
        // force a temporary file creation in cwd
        QTemporaryFile tf("mytemp");
        if (!tf.open())
        {
            useDir = GetUserVisItDirectory().c_str();
        }
    }
#endif

    QString default_file = useDir + "/" + QString("visit_filter.py");

    // Get the name of the file that the user saved.
    QString filter(tr("Python script file") +  QString("  (*.py *vpe *vpq);;")
                   + tr("All files") + QString(" (*)"));

    QString res = QFileDialog::getSaveFileName(this,
                                               tr("Save Python filter script"),
                                               default_file,
                                               filter);
    if(!res.isNull())
        saveScript(res);
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::loadMenuEvent
//
// Purpose:
//   Slot handler for load menu actions.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//   Cyrus Harrison, Fri May 21 09:51:44 PDT 2010
//   Added 'All Files' to the filter, to support scripts without a '.py'
//   extension.
//
//   Cyrus Harrison, Wed Apr 11 15:03:17 PDT 2012
//   Add other common python filter extentsions (vpe & vpq)
//
//   Kathleen Biagas, Wed Feb 26 14:40:09 MST 2013
//   Replace commas with spaces in filter string.
//
// ****************************************************************************

void
QvisPythonFilterEditor::loadMenuEvent(QAction *action)
{
    if(action == loadFile)
    {
        QString filter(tr("Python script file") + QString(" (*.py *vpe *vpq);;")
                       + tr("All files") + QString(" (*)"));
        QString res = QFileDialog::getOpenFileName(this,
                                                   tr("Load Python filter"),
                                                   QDir::current().path(),
                                                   filter);
        if(!res.isNull())
            loadScript(res);
    }
    else if(action == loadTempSimple)
        emit templateSelected(QString("Simple"));
    else if(action == loadTempAdv)
        emit templateSelected(QString("Advanced"));
}

// ****************************************************************************
// Method: QvisPythonFilterEditor::templateDirectory
//
// Purpose:
//   Provides the path to the filter template directory.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Feb 11 09:35:54 PST 2010
//
// Modifications:
//   Kathleen Bonnell, Wed Mar 24 16:28:37 MST 2010
//   Retrieve VISITARCHHOME via GetVisItArchitectureDirectory.
//
//   Cyrus Harrison, Wed Apr 11 15:03:17 PDT 2012
//   Update to reflect change loc of templates.
//
//   Kathleen Biagas, Fri May 4 14:05:27 PDT 2012
//   Call GetVisItLibraryDirectory instead of GetVisItArchitectureDirectory.
//
// ****************************************************************************

QString
QvisPythonFilterEditor::templateDirectory()
{
    QString res(GetVisItLibraryDirectory().c_str());
    res += QString(VISIT_SLASH_CHAR)
           + QString("site-packages")
           + QString(VISIT_SLASH_CHAR)
           + QString("pyavt")
           + QString(VISIT_SLASH_CHAR)
           + QString("templates")
           + QString(VISIT_SLASH_CHAR);

    return res;
}


