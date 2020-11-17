// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISLINESAMPLERWINDOW_H
#define QVISLINESAMPLERWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class LineSamplerAttributes;
class QTabWidget;
class QGroupBox;
class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QRadioButton;
class QListWidget;
class QListWidgetItem;

// ****************************************************************************
// Class: QvisLineSamplerWindow
//
// Purpose:
//    Defines QvisLineSamplerWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisLineSamplerWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisLineSamplerWindow(const int type,
                         LineSamplerAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLineSamplerWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void meshGeometryChanged(int val);
    void arrayConfigurationChanged(int val);
    void boundaryChanged(int val);
    void instanceIdChanged(int val);
    void nArraysProcessText();
    void nChannelsProcessText();
    void toroidalArrayAngleProcessText();
    void channelProjectionChanged(int val);
    void channelOffsetProcessText();
    void channelAngleProcessText();
    void nRowsProcessText();
    void rowOffsetProcessText();
    void arrayOriginProcessText();
    void arrayAxisChanged(int val);
    void poloialAngleProcessText();
    void poloialRTiltProcessText();
    void poloialZTiltProcessText();
    void toroidalAngleProcessText();
    void flipToroidalAngleChanged(bool val);
    void viewGeometryChanged(int val);
    void displayTimeChanged(int val);
    void viewDimensionChanged(int val);
    void donotApplyToAllChanged(bool val);
    void heightPlotScaleProcessText();
    void channelPlotOffsetProcessText();
    void arrayPlotOffsetProcessText();
    void channelGeometryChanged(int val);
    void radiusProcessText();
//     void divergenceProcessText();
    void channelProfileChanged(int val);
    void standardDeviationProcessText();
    void sampleDistanceProcessText();
    void sampleVolumeProcessText();
//     void sampleArcProcessText();
    void channelIntegrationChanged(int val);
    void toroidalIntegrationChanged(int val);
    void toroidalAngleSamplingChanged(int val);
    void toroidalAngleStartProcessText();
    void toroidalAngleStopProcessText();
    void toroidalAngleStrideProcessText();
    void timeSamplingChanged(int val);
    void timeStepStartProcessText();
    void timeStepStopProcessText();
    void timeStepStrideProcessText();
    void addChannel();
    void deleteChannel();
    void deleteChannels();
    void readChannels();
    void readWall();
    void channelListClicked(QListWidgetItem*);
    void channelListDoubleClicked(QListWidgetItem*);
    void channelListTextChanged(const QString &currentText);
    void wallListTextChanged(const QString &currentText);
    void nChannelListArraysProcessText();
    void channelListToroidalArrayAngleProcessText();
    void channelListToroidalAngleProcessText();
    void channelListFlipToroidalAngleChanged(bool val);

    void EnableGeometry(bool flag);
    void EnableList(bool flag);
    void UpdateMeshGeometry();

  private:
    QTabWidget  *propertyTabs;
    QWidget     *mainTab;
    QWidget     *geometryTab;
    QWidget     *listTab;
    QWidget     *samplingTab;
    QWidget     *viewTab;
    QTabWidget  *projectionTabs;
    QWidget     *divergentTab;
    QWidget     *parallelTab;
    QWidget     *gridTab;
    QGroupBox   *toroidalGroup;
    QGroupBox   *oneDPlotGroup;
    QListWidget *wallList;
    QListWidget *channelList;
    QPushButton *channelListAddChannel;
    QPushButton *channelListDeleteChannel;
    QPushButton *channelListDeleteAllChannels;
    QPushButton *channelListReadChannels;
    QPushButton *wallReadFile;
    QWidget      *meshGeometry;
    QButtonGroup *meshGeometryButtonGroup;
    QWidget      *arrayConfiguration;
    QButtonGroup *arrayConfigurationButtonGroup;
    QWidget      *boundary;
    QButtonGroup *boundaryButtonGroup;
    QComboBox *instanceId;
    QLineEdit *nArrays;
    QLineEdit *nDChannels;
    QLineEdit *nPChannels;
    QLineEdit *nGChannels;
    QWidget   *dChannelLayoutType;
    QWidget   *pChannelLayoutType;
    QWidget   *gChannelLayoutType;
    QButtonGroup *dChannelLayoutTypeButtonGroup;
    QButtonGroup *pChannelLayoutTypeButtonGroup;
    QButtonGroup *gChannelLayoutTypeButtonGroup;
    QLineEdit *poloidalAngleStart;
    QLineEdit *poloidalAngleStop;
    QLineEdit *toroidalArrayAngle;
    QWidget      *channelProjection;
    QButtonGroup *channelProjectionButtonGroup;
    QLineEdit *channelParallelOffset;
    QLineEdit *channelGridOffset;
    QLineEdit *channelAngle;
    QLineEdit *nRows;
    QLineEdit *rowOffset;
    QLineEdit *arrayOrigin;
    QWidget      *arrayAxis;
    QButtonGroup *arrayAxisButtonGroup;
    QRadioButton *arrayAxisArrayAxisR;
    QRadioButton *arrayAxisArrayAxisZ;
    QLineEdit *poloialAngle;
    QLineEdit *poloialRTilt;
    QLineEdit *poloialZTilt;
    QLineEdit *toroidalAngle;
    QCheckBox *flipToroidalAngle;
    QWidget      *viewDimension;
    QButtonGroup *viewDimensionButtonGroup;
    QCheckBox    *donotApplyToAll;
    QWidget      *viewGeometry;
    QButtonGroup *viewGeometryButtonGroup;
    QWidget      *displayTime;
    QButtonGroup *displayTimeButtonGroup;
    QLineEdit *heightPlotScale;
    QLineEdit *channelPlotOffset;
    QLineEdit *arrayPlotOffset;
    QWidget      *channelGeometry;
    QButtonGroup *channelGeometryButtonGroup;
    QLineEdit *radius;
//     QLineEdit *divergence;
    QWidget      *channelProfile;
    QButtonGroup *channelProfileButtonGroup;
    QRadioButton *channelProfileChannelTypeTopHat;
    QRadioButton *channelProfileChannelTypeGaussian;
    QLineEdit *standardDeviation;
    QLineEdit *sampleDistance;
    QLineEdit *sampleVolume;
//     QLineEdit *sampleArc;
    QWidget      *channelIntegration;
    QButtonGroup *channelIntegrationButtonGroup;
    QRadioButton *channelIntegrationNone;
    QRadioButton *channelIntegrationSummation;
    QWidget      *toroidalIntegration;
    QButtonGroup *toroidalIntegrationButtonGroup;
    QRadioButton *toroidalIntegrationNone;
    QRadioButton *toroidalIntegrationTime;
    QRadioButton *toroidalIntegrationSummation;
    QWidget      *toroidalAngleSampling;
    QButtonGroup *toroidalAngleSamplingButtonGroup;
    QLineEdit *toroidalAngleStart;
    QLineEdit *toroidalAngleStop;
    QLineEdit *toroidalAngleStride;
    QWidget      *timeSampling;
    QButtonGroup *timeSamplingButtonGroup;
    QLineEdit *timeStepStart;
    QLineEdit *timeStepStop;
    QLineEdit *timeStepStride;
    QLineEdit *nChannelListArrays;
    QLineEdit *channelListToroidalArrayAngle;
    QLineEdit *channelListToroidalAngle;
    QCheckBox *channelListFlipToroidalAngle;
    QLabel *meshGeometryLabel;
    QLabel *arrayConfigurationLabel;
    QLabel *boundaryLabel;
    QLabel *nArraysLabel;
    QLabel *nDChannelsLabel;
    QLabel *nPChannelsLabel;
    QLabel *nGChannelsLabel;
    QLabel *poloidalAngleLabel;
    QLabel *poloidalAngleStartLabel;
    QLabel *poloidalAngleStopLabel;
    QLabel *dChannelLayoutTypeLabel;
    QLabel *pChannelLayoutTypeLabel;
    QLabel *gChannelLayoutTypeLabel;
    QLabel *toroidalArrayAngleLabel;
    QLabel *channelProjectionLabel;
    QLabel *channelParallelOffsetLabel;
    QLabel *channelGridOffsetLabel;
    QLabel *channelAngleLabel;
    QLabel *nRowsLabel;
    QLabel *rowOffsetLabel;
    QLabel *arrayOriginLabel;
    QLabel *arrayAxisLabel;
    QLabel *poloialAngleLabel;
    QLabel *poloialRTiltLabel;
    QLabel *poloialZTiltLabel;
    QLabel *toroidalAngleLabel;
    QLabel *viewGeometryLabel;
    QLabel *displayTimeLabel;
    QLabel *viewDimensionLabel;
    QLabel *heightPlotScaleLabel;
    QLabel *channelPlotOffsetLabel;
    QLabel *arrayPlotOffsetLabel;
    QLabel *channelGeometryLabel;
    QLabel *radiusLabel;
//     QLabel *divergenceLabel;
    QLabel *channelProfileLabel;
    QLabel *standardDeviationLabel;
    QLabel *sampleDistanceLabel;
    QLabel *sampleVolumeLabel;
//     QLabel *sampleArcLabel;
    QLabel *channelIntegrationLabel;
    QLabel *toroidalIntegrationLabel;
    QLabel *toroidalAngleSamplingLabel;
    QLabel *toroidalAngleSampleLabel;
    QLabel *toroidalAngleStartLabel;
    QLabel *toroidalAngleStopLabel;
    QLabel *toroidalAngleStrideLabel;
    QLabel *timeSamplingLabel;
    QLabel *timeStepLabel;
    QLabel *timeStepStartLabel;
    QLabel *timeStepStopLabel;
    QLabel *timeStepStrideLabel;
    QLabel *confFileCoordinateLabel;
    QLabel *wallFileCoordinateLabel;
    QLabel *nChannelListArraysLabel;
    QLabel *channelListToroidalArrayAngleLabel;
    QLabel *channelListToroidalAngleLabel;

    QLabel *cartesianXLayoutLabel;
    QLabel *cartesianZLayoutLabel;
    QLabel *cartesianConfLayoutLabel;
    QLabel *cylindricalRLayoutLabel;
    QLabel *cylindricalZLayoutLabel;
    QLabel *cylindricalConfLayoutLabel;
    QLabel *toroidalRLayoutLabel;
    QLabel *toroidalZLayoutLabel;
    QLabel *toroidalConfLayoutLabel;

    LineSamplerAttributes *atts;
};
#endif
