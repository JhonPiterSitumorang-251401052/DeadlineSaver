#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QApplication>
#include <QSoundEffect>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTimer *timer = new QTimer(this);

    timer->start(1000);

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(style()->standardIcon(
        QStyle::SP_ComputerIcon));

    trayIcon->show();

    QSoundEffect *sound = new QSoundEffect(this);

    sound->setSource(
        QUrl::fromLocalFile(
            "alarm.wav"
            )
        );

    qDebug() << sound->source();

    sound->setLoopCount(1);

    sound->setVolume(1.0);

    QFile file("reminder.txt");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QTextStream in(&file);

        while(!in.atEnd()) {

            QString line = in.readLine();

            ui->listReminder->addItem(line);
        }

        file.close();
    }

    connect(ui->btnTambah, &QPushButton::clicked, this, [=]() {

        QString reminder = ui->InputReminder->text();

        QString waktu =
            ui->dateTimeEdit->dateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        QString hasil = reminder + "|" + waktu;

        ui->listReminder->addItem(hasil);

        QFile file("reminder.txt");

        if(file.open(QIODevice::Append | QIODevice::Text)) {

            QTextStream out(&file);

            out << hasil << "\n";

            file.close();
        }

        ui->InputReminder->clear();
    });

    connect(ui->btnHapus, &QPushButton::clicked, this, [=]() {

        int baris = ui->listReminder->currentRow();

        delete ui->listReminder->takeItem(baris);
    });

    connect(timer, &QTimer::timeout, this, [=]() {

        QString sekarang =
            QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        for(int i = 0; i < ui->listReminder->count(); i++) {

            QString itemText =
                ui->listReminder->item(i)->text();

            QStringList bagian = itemText.split("|");

            if(bagian.size() < 2)
                continue;

            QString pesan = bagian[0];
            QString waktu = bagian[1];

            if(sekarang == waktu) {

                sound->play();

                trayIcon->showMessage(
                    "Reminder",
                    pesan,
                    QSystemTrayIcon::Information,
                    5000
                    );

                delete ui->listReminder->takeItem(i);

                i--;
            }
        }
    });
    this->setStyleSheet(R"(

        QMainWindow {
            background-color: #1e1e1e;
        }

        QLabel {
            color: white;
            font-size: 20px;
            font-weight: bold;
        }

        QLineEdit {
            background-color: #2d2d2d;
            color: white;
            border: 2px solid #444;
            border-radius: 10px;
            padding: 8px;
            font-size: 14px;
        }

        QDateTimeEdit {
            background-color: #2d2d2d;
            color: white;
            border: 2px solid #444;
            border-radius: 10px;
            padding: 8px;
            font-size: 14px;
        }

        QPushButton {
            background-color: #3a86ff;
            color: white;
            border-radius: 10px;
            padding: 10px;
            font-size: 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #539dff;
        }

        QListWidget {
            background-color: #2d2d2d;
            color: white;
            border-radius: 10px;
            padding: 5px;
            font-size: 14px;
        }

    )");
}

MainWindow::~MainWindow()
{
    delete ui;
}