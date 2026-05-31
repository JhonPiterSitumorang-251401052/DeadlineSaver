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
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>

// Delegate untuk memberi padding pada setiap item di listReminder
class PaddedItemDelegate : public QStyledItemDelegate {
public:
    explicit PaddedItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem opt = option;
        opt.rect.adjust(10, 6, -10, -6);
        QStyledItemDelegate::paint(painter, opt, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(size.height() + 16);
        return size;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Padding untuk item di listReminder
    ui->listReminder->setItemDelegate(new PaddedItemDelegate(this));

    // Override style labelPetunjuk agar tidak ikut QLabel global (20px bold)
    ui->labelPetunjuk->setStyleSheet(
        "color: #cccccc;"
        "font-size: 13px;"
        "font-weight: normal;"
    );

    QTimer *timer = new QTimer(this);

    timer->start(1000);

    trayIcon = new QSystemTrayIcon(this);

    trayIcon->setIcon(style()->standardIcon(
        QStyle::SP_ComputerIcon));

    trayIcon->show();

    QSoundEffect *sound = new QSoundEffect(this);

    sound->setSource(
        QUrl::fromLocalFile(
            QCoreApplication::applicationDirPath() + "/alarm.wav"
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
        sortReminders();
    }

    connect(ui->btnTambah, &QPushButton::clicked, this, [=]() {

        QString reminder = ui->InputReminder->text();

        QString waktu =
            ui->dateTimeEdit->dateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        QString hasil = reminder + "|" + waktu;

        ui->listReminder->addItem(hasil);
        sortReminders();

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
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #2c3e50, stop:1 #000000);
        }

        QLabel {
            color: white;
            font-size: 20px;
            font-weight: bold;
        }

        #label {
            padding-bottom: 10px;
            min-height: 40px;
        }


        QLineEdit, QDateTimeEdit {
            background-color: rgba(255, 255, 255, 10%); /* Transparan */
            color: white;
            border: 1px solid #555;
            border-radius: 8px;
            padding: 8px;
        }

        /* Tombol dengan warna yang lebih pop-up */
        QPushButton {
            background-color: #3498db;
            color: white;
            border-radius: 8px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        /* List Widget agar menyatu dengan background */
        QListWidget {
            background-color: rgba(0, 0, 0, 20%);
            border: none;
            color: #ecf0f1;
        }
    )");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sortReminders() {
    QList<QListWidgetItem*> items;
    for(int i = 0; i < ui->listReminder->count(); ++i) {
        items.append(ui->listReminder->takeItem(0));
    }

    std::sort(items.begin(), items.end(), [](QListWidgetItem* a, QListWidgetItem* b) {
        QString waktuA = a->text().split("|").last();
        QString waktuB = b->text().split("|").last();
        return waktuA < waktuB;
    });

    for(auto item : items) {
        ui->listReminder->addItem(item);
    }
}

void MainWindow::on_btnTambah_clicked()
{

}
