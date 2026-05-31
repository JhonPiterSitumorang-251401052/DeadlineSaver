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

    // Override style label petunjuk agar tidak ikut QLabel global (20px bold)
    QString styleLabel =
        "color: #cccccc;"
        "font-size: 13px;"
        "font-weight: normal;";

    ui->labelPetunjuk->setStyleSheet(styleLabel);
    ui->labelWaktu->setStyleSheet(styleLabel);

    // Judul jendela
    this->setWindowTitle("DeadlineSaver");

    // Fitur 1: Placeholder text pada InputReminder
    ui->InputReminder->setPlaceholderText("Contoh: Tugas Besar Alpro...");

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

        if (reminder.isEmpty()) {
            QMessageBox::warning(this, "Peringatan", "Nama reminder tidak boleh kosong!");
            return; // Menghentikan fungsi agar tidak lanjut ke bawah
        }

        QString waktu =
            ui->dateTimeEdit->dateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        QString dataMentah = reminder + "|" + waktu;

        QString teksTampil = reminder + " (" + waktu + ")";

        QListWidgetItem *item = new QListWidgetItem(teksTampil);
        item->setData(Qt::UserRole, dataMentah);

        ui->listReminder->addItem(item);
        sortReminders();

        QFile file("reminder.txt");
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for(int i = 0; i < ui->listReminder->count(); ++i) {
                out << ui->listReminder->item(i)->data(Qt::UserRole).toString() << "\n";
            }
            file.close();
        }

        ui->InputReminder->clear();
    });

    connect(ui->btnHapus, &QPushButton::clicked, this, [=]() {

        int baris = ui->listReminder->currentRow();

        delete ui->listReminder->takeItem(baris);
    });

    // Fitur 2: Tombol Edit — kembalikan data ke input lalu hapus item lama
    connect(ui->btnEdit, &QPushButton::clicked, this, [=]() {

        int baris = ui->listReminder->currentRow();

        if (baris < 0) {
            QMessageBox::warning(this, "Peringatan", "Pilih reminder yang ingin diedit terlebih dahulu!");
            return;
        }

        QListWidgetItem *item = ui->listReminder->item(baris);

        // Ambil data mentah (format: "nama|yyyy-MM-dd hh:mm:ss")
        QString dataMentah = item->data(Qt::UserRole).toString();

        // Fallback: kalau UserRole kosong, parse dari text langsung
        if (dataMentah.isEmpty()) {
            dataMentah = item->text();
        }

        QStringList bagian = dataMentah.split("|");

        if (bagian.size() >= 2) {
            ui->InputReminder->setText(bagian[0].trimmed());
            QDateTime dt = QDateTime::fromString(bagian[1].trimmed(), "yyyy-MM-dd hh:mm:ss");
            if (dt.isValid()) {
                ui->dateTimeEdit->setDateTime(dt);
            }
        }

        // Hapus item lama dari list dan file
        delete ui->listReminder->takeItem(baris);

        QFile file("reminder.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < ui->listReminder->count(); ++i) {
                out << ui->listReminder->item(i)->data(Qt::UserRole).toString() << "\n";
            }
            file.close();
        }
    });

    connect(timer, &QTimer::timeout, this, [=]() {

        QString sekarang =
            QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        QDateTime waktuSekarang = QDateTime::currentDateTime();

        for(int i = 0; i < ui->listReminder->count(); i++) {

            QListWidgetItem *currentItem = ui->listReminder->item(i);

            QString itemText = currentItem->text();

            QStringList bagian = itemText.split("|");

            // Fitur 3: Indikator warna overdue
            // Ambil waktu dari UserRole (data mentah) atau teks langsung
            QString dataMentah = currentItem->data(Qt::UserRole).toString();
            if (dataMentah.isEmpty()) dataMentah = itemText;

            QStringList bagianData = dataMentah.split("|");
            if (bagianData.size() >= 2) {
                QDateTime deadlineItem = QDateTime::fromString(
                    bagianData[1].trimmed(), "yyyy-MM-dd hh:mm:ss");

                if (deadlineItem.isValid() && deadlineItem < waktuSekarang) {
                    // Sudah overdue — warna merah redup
                    currentItem->setBackground(QColor(120, 30, 30));
                    currentItem->setForeground(QColor(255, 180, 180));
                } else {
                    // Belum overdue — kembalikan ke default
                    currentItem->setBackground(Qt::transparent);
                    currentItem->setForeground(QColor(0xec, 0xf0, 0xf1));
                }
            }

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
