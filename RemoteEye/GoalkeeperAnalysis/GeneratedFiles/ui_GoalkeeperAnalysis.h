/********************************************************************************
** Form generated from reading UI file 'GoalkeeperAnalysis.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GOALKEEPERANALYSIS_H
#define UI_GOALKEEPERANALYSIS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GoalkeeperAnalysisClass
{
public:
    QWidget *centralWidget;
    QPushButton *startCaptureButton;
    QPushButton *stopCaptureButton;
    QPushButton *showPlaybackButton;
    QPushButton *startCalibrationButton;
    QPushButton *getTestGaze;
    QLabel *label_2;
    QFrame *line;
    QLabel *label_3;
    QFrame *line_2;
    QFrame *line_3;
    QPushButton *saveEyeImages;
    QPushButton *captureFullimage;
    QPushButton *calibrationEvaluation;
    QLabel *profilingLabel;
    QLabel *label_4;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *GoalkeeperAnalysisClass)
    {
        if (GoalkeeperAnalysisClass->objectName().isEmpty())
            GoalkeeperAnalysisClass->setObjectName(QStringLiteral("GoalkeeperAnalysisClass"));
        GoalkeeperAnalysisClass->resize(335, 587);
        centralWidget = new QWidget(GoalkeeperAnalysisClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        startCaptureButton = new QPushButton(centralWidget);
        startCaptureButton->setObjectName(QStringLiteral("startCaptureButton"));
        startCaptureButton->setGeometry(QRect(80, 60, 111, 21));
        stopCaptureButton = new QPushButton(centralWidget);
        stopCaptureButton->setObjectName(QStringLiteral("stopCaptureButton"));
        stopCaptureButton->setGeometry(QRect(200, 140, 111, 23));
        showPlaybackButton = new QPushButton(centralWidget);
        showPlaybackButton->setObjectName(QStringLiteral("showPlaybackButton"));
        showPlaybackButton->setGeometry(QRect(200, 60, 111, 21));
        startCalibrationButton = new QPushButton(centralWidget);
        startCalibrationButton->setObjectName(QStringLiteral("startCalibrationButton"));
        startCalibrationButton->setGeometry(QRect(80, 10, 111, 23));
        getTestGaze = new QPushButton(centralWidget);
        getTestGaze->setObjectName(QStringLiteral("getTestGaze"));
        getTestGaze->setGeometry(QRect(80, 110, 111, 23));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 10, 71, 16));
        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(10, 40, 311, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 60, 47, 13));
        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(10, 90, 311, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(10, 170, 311, 16));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);
        saveEyeImages = new QPushButton(centralWidget);
        saveEyeImages->setObjectName(QStringLiteral("saveEyeImages"));
        saveEyeImages->setGeometry(QRect(200, 110, 111, 23));
        captureFullimage = new QPushButton(centralWidget);
        captureFullimage->setObjectName(QStringLiteral("captureFullimage"));
        captureFullimage->setGeometry(QRect(80, 140, 111, 23));
        calibrationEvaluation = new QPushButton(centralWidget);
        calibrationEvaluation->setObjectName(QStringLiteral("calibrationEvaluation"));
        calibrationEvaluation->setGeometry(QRect(200, 10, 111, 21));
        profilingLabel = new QLabel(centralWidget);
        profilingLabel->setObjectName(QStringLiteral("profilingLabel"));
        profilingLabel->setGeometry(QRect(10, 220, 301, 321));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 110, 47, 13));
        GoalkeeperAnalysisClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(GoalkeeperAnalysisClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 335, 21));
        GoalkeeperAnalysisClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(GoalkeeperAnalysisClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        GoalkeeperAnalysisClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(GoalkeeperAnalysisClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        GoalkeeperAnalysisClass->setStatusBar(statusBar);

        retranslateUi(GoalkeeperAnalysisClass);

        QMetaObject::connectSlotsByName(GoalkeeperAnalysisClass);
    } // setupUi

    void retranslateUi(QMainWindow *GoalkeeperAnalysisClass)
    {
        GoalkeeperAnalysisClass->setWindowTitle(QApplication::translate("GoalkeeperAnalysisClass", "GoalkeeperAnalysis", Q_NULLPTR));
        startCaptureButton->setText(QApplication::translate("GoalkeeperAnalysisClass", "Start capture", Q_NULLPTR));
        stopCaptureButton->setText(QApplication::translate("GoalkeeperAnalysisClass", "Stop ", Q_NULLPTR));
        showPlaybackButton->setText(QApplication::translate("GoalkeeperAnalysisClass", "Show Live Gaze", Q_NULLPTR));
        startCalibrationButton->setText(QApplication::translate("GoalkeeperAnalysisClass", " Calibrate", Q_NULLPTR));
        getTestGaze->setText(QApplication::translate("GoalkeeperAnalysisClass", "Show Face", Q_NULLPTR));
        label_2->setText(QApplication::translate("GoalkeeperAnalysisClass", "Calibration", Q_NULLPTR));
        label_3->setText(QApplication::translate("GoalkeeperAnalysisClass", "Capturing", Q_NULLPTR));
        saveEyeImages->setText(QApplication::translate("GoalkeeperAnalysisClass", "Save Eyevideos", Q_NULLPTR));
        captureFullimage->setText(QApplication::translate("GoalkeeperAnalysisClass", "CaptureFull", Q_NULLPTR));
        calibrationEvaluation->setText(QApplication::translate("GoalkeeperAnalysisClass", "CalibrationACC", Q_NULLPTR));
        profilingLabel->setText(QApplication::translate("GoalkeeperAnalysisClass", "Profiling Info", Q_NULLPTR));
        label_4->setText(QApplication::translate("GoalkeeperAnalysisClass", "Utilities", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class GoalkeeperAnalysisClass: public Ui_GoalkeeperAnalysisClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GOALKEEPERANALYSIS_H
