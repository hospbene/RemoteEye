/********************************************************************************
** Form generated from reading UI file 'CalibrationDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CALIBRATIONDIALOG_H
#define UI_CALIBRATIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CalibrationDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *calibrationImageLabel;

    void setupUi(QDialog *CalibrationDialog)
    {
        if (CalibrationDialog->objectName().isEmpty())
            CalibrationDialog->setObjectName(QStringLiteral("CalibrationDialog"));
        CalibrationDialog->resize(400, 300);
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CalibrationDialog->sizePolicy().hasHeightForWidth());
        CalibrationDialog->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(CalibrationDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        calibrationImageLabel = new QLabel(CalibrationDialog);
        calibrationImageLabel->setObjectName(QStringLiteral("calibrationImageLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(calibrationImageLabel->sizePolicy().hasHeightForWidth());
        calibrationImageLabel->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(calibrationImageLabel);


        retranslateUi(CalibrationDialog);

        QMetaObject::connectSlotsByName(CalibrationDialog);
    } // setupUi

    void retranslateUi(QDialog *CalibrationDialog)
    {
        CalibrationDialog->setWindowTitle(QApplication::translate("CalibrationDialog", "Dialog", Q_NULLPTR));
        calibrationImageLabel->setText(QApplication::translate("CalibrationDialog", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class CalibrationDialog: public Ui_CalibrationDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CALIBRATIONDIALOG_H
