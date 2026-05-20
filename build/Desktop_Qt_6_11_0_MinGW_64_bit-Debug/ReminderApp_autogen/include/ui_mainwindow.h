/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QLineEdit *InputReminder;
    QDateTimeEdit *dateTimeEdit;
    QPushButton *btnTambah;
    QListWidget *listReminder;
    QPushButton *btnHapus;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(10, 10, 258, 328));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        InputReminder = new QLineEdit(widget);
        InputReminder->setObjectName("InputReminder");

        verticalLayout->addWidget(InputReminder);

        dateTimeEdit = new QDateTimeEdit(widget);
        dateTimeEdit->setObjectName("dateTimeEdit");

        verticalLayout->addWidget(dateTimeEdit);

        btnTambah = new QPushButton(widget);
        btnTambah->setObjectName("btnTambah");

        verticalLayout->addWidget(btnTambah);

        listReminder = new QListWidget(widget);
        new QListWidgetItem(listReminder);
        listReminder->setObjectName("listReminder");

        verticalLayout->addWidget(listReminder, 0, Qt::AlignmentFlag::AlignVCenter);

        btnHapus = new QPushButton(widget);
        btnHapus->setObjectName("btnHapus");

        verticalLayout->addWidget(btnHapus);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Reminder App", nullptr));
        InputReminder->setText(QString());
        btnTambah->setText(QCoreApplication::translate("MainWindow", "Tambah Reminder", nullptr));

        const bool __sortingEnabled = listReminder->isSortingEnabled();
        listReminder->setSortingEnabled(false);
        listReminder->setSortingEnabled(__sortingEnabled);

        btnHapus->setText(QCoreApplication::translate("MainWindow", "Hapus Reminder", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
