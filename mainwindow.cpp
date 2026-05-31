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

        QString reminder = ui->InputReminder->text().trimmed();

        if (reminder.isEmpty()) {
            QMessageBox::warning(this, "Peringatan", "Nama reminder tidak boleh kosong!");
            return;
        }

        QString waktu =
            ui->dateTimeEdit->dateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        QString dataMentah = reminder + "|" + waktu;
        QString teksTampil = reminder + " (" + waktu + ")";

        if (editIndex >= 0) {
            // Mode edit: perbarui item yang sudah ada
            QListWidgetItem *item = ui->listReminder->item(editIndex);
            item->setText(teksTampil);
            item->setData(Qt::UserRole, dataMentah);
            item->setForeground(Qt::white);

            editIndex = -1;
            ui->btnTambah->setText("Tambah Reminder");
            ui->btnEdit->setEnabled(false);
        } else {
            // Mode tambah: buat item baru
            QListWidgetItem *item = new QListWidgetItem(teksTampil);
            item->setData(Qt::UserRole, dataMentah);
            ui->listReminder->addItem(item);
        }

        sortReminders();
        saveToFile();
        ui->InputReminder->clear();
    });

    connect(ui->btnEdit, &QPushButton::clicked, this, [=]() {

        int baris = ui->listReminder->currentRow();
        if (baris < 0) {
            QMessageBox::warning(this, "Peringatan", "Pilih reminder yang ingin diedit!");
            return;
        }

        QListWidgetItem *item = ui->listReminder->item(baris);
        QString dataMentah = item->data(Qt::UserRole).toString();
        QStringList bagian = dataMentah.split("|");

        if (bagian.size() < 2) return;

        // Isi form dengan data reminder yang dipilih
        ui->InputReminder->setText(bagian[0]);
        ui->dateTimeEdit->setDateTime(
            QDateTime::fromString(bagian[1], "yyyy-MM-dd hh:mm:ss")
        );

        // Tandai item yang sedang diedit dengan warna oranye
        item->setForeground(QColor("#f39c12"));

        editIndex = baris;
        ui->btnTambah->setText("Simpan Perubahan");
        ui->btnEdit->setEnabled(false);

        ui->InputReminder->setFocus();
    });

    connect(ui->btnHapus, &QPushButton::clicked, this, [=]() {

        int baris = ui->listReminder->currentRow();
        if (baris < 0) return;

        // Batalkan mode edit jika item yang dihapus adalah yang sedang diedit
        if (baris == editIndex) {
            editIndex = -1;
            ui->btnTambah->setText("Tambah Reminder");
            ui->btnEdit->setEnabled(true);
            ui->InputReminder->clear();
        }

        delete ui->listReminder->takeItem(baris);
        saveToFile();
    });

    // Aktifkan btnEdit saat user memilih item dari list
    connect(ui->listReminder, &QListWidget::currentRowChanged, this, [=](int row) {
        ui->btnEdit->setEnabled(row >= 0 && editIndex < 0);
    });

    connect(timer, &QTimer::timeout, this, [=]() {

        QString sekarang =
            QDateTime::currentDateTime()
                .toString("yyyy-MM-dd hh:mm:ss");

        for(int i = 0; i < ui->listReminder->count(); i++) {

            QString dataMentah =
                ui->listReminder->item(i)->data(Qt::UserRole).toString();

            QStringList bagian = dataMentah.split("|");

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

                // Reset mode edit jika item yang berbunyi sedang diedit
                if (i == editIndex) {
                    editIndex = -1;
                    ui->btnTambah->setText("Tambah Reminder");
                    ui->InputReminder->clear();
                }

                delete ui->listReminder->takeItem(i);
                saveToFile();
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

        #btnEdit {
            background-color: #27ae60;
        }

        #btnEdit:hover {
            background-color: #1e8449;
        }

        #btnEdit:disabled {
            background-color: #555;
            color: #999;
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

void MainWindow::saveToFile() {
    QFile file("reminder.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < ui->listReminder->count(); ++i) {
            out << ui->listReminder->item(i)->data(Qt::UserRole).toString() << "\n";
        }
        file.close();
    }
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
