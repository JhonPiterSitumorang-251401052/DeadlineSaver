#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "settingsdialog.h"
#include <QSettings>
#include <QFileInfo>
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
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>

static QColor warnaDariTag(const QString &tag) {
    if (tag == "Kuliah")    return QColor("#3498db");
    if (tag == "Kerja")     return QColor("#e67e22");
    if (tag == "Pribadi")   return QColor("#9b59b6");
    if (tag == "Kesehatan") return QColor("#2ecc71");
    if (tag == "Belanja")   return QColor("#e91e63");
    if (tag == "Keuangan")  return QColor("#f1c40f");
    return QColor("#95a5a6");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateTimeEdit->setCalendarPopup(true);

    // Setup label style
    QString styleLabel = "color: #bdc3c7; font-size: 13px; font-weight: normal;";
    ui->labelPetunjuk->setStyleSheet(styleLabel);
    ui->labelWaktu->setStyleSheet(styleLabel);
    ui->labelTag->setStyleSheet(styleLabel);

    // Setup list
    ui->listReminder->setSpacing(4);
    ui->listReminder->setUniformItemSizes(false);

    // Tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    trayIcon->show();

    // Sound
    QSoundEffect *sound = new QSoundEffect(this);
    sound->setSource(QUrl::fromLocalFile("alarm.wav"));

    // Load dari file
    QFile file("reminder.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.isEmpty()) continue;

            // Panggil fungsi add item biasa
            addReminderItem(line);

            // Kunci pembeda: Jika data baris tersebut punya flag selesai ("1"), otomatis coret!
            QStringList bagian = line.split("|");
            if (bagian.size() == 4 && bagian[3] == "1") {
                int rowTarget = ui->listReminder->count() - 1;
                // Panggil toggle tanpa membalikkan logika
                QListWidgetItem *lastItem = ui->listReminder->item(rowTarget);
                lastItem->setData(Qt::UserRole + 1, true);

                // Jalankan update tampilan row widgetnya agar dicoret
                QWidget *w = ui->listReminder->itemWidget(lastItem);
                if (w) {
                    QPushButton *btnBulat = w->findChild<QPushButton*>("btnBulat");
                    QLabel *lbl = w->findChild<QLabel*>("lblCountdown");
                    if (btnBulat) btnBulat->setText("✓");
                    if (lbl) {
                        QFont f = lbl->font(); f.setStrikeOut(true); lbl->setFont(f);
                        lbl->setStyleSheet("color: #2ecc71; background: transparent; font-size: 13px;");
                    }
                }
            }
        }
        file.close();
        sortReminders();
        updateStatistik();
        applyFilter();
    }

    // Timer tiap detik
    QTimer *timer = new QTimer(this);
    timer->start(1000);

    connect(timer, &QTimer::timeout, this, [=]() {
        QDateTime sekarang = QDateTime::currentDateTime();
        QString sekarangStr = sekarang.toString("yyyy-MM-dd hh:mm:ss");

        for (int i = 0; i < ui->listReminder->count(); i++) {
            QListWidgetItem *item = ui->listReminder->item(i);
            QString dataMentah = item->data(Qt::UserRole).toString();
            QStringList bagian = dataMentah.split("|");
            if (bagian.size() < 2) continue;

            QString pesan = bagian[0];
            QString waktu = bagian[1];
            QString tag   = bagian.size() >= 3 ? bagian[2] : "Lain-lain";
            bool sudahSelesai = item->data(Qt::UserRole + 1).toBool();

            // Cek pengingat awal
            QSettings sett("DeadlineSaver", "Settings");
            int menitAwal = sett.value("menitSebelum", 15).toInt();
            bool senyap   = sett.value("modeSenyap", false).toBool();
            QString soundPath = sett.value("soundFile", "alarm.wav").toString();

            QDateTime targetWaktuAwal = QDateTime::fromString(waktu, "yyyy-MM-dd hh:mm:ss").addSecs(-menitAwal * 60);
            if (sekarangStr == targetWaktuAwal.toString("yyyy-MM-dd hh:mm:ss")) {
                if (!senyap) sound->play();
                trayIcon->showMessage("Pengingat", pesan + " — " + QString::number(menitAwal) + " menit lagi!", QSystemTrayIcon::Information, 5000);
            }

            // Cek alarm
            if (sekarangStr == waktu) {
                if (!senyap) {
                    sound->setSource(QUrl::fromLocalFile(soundPath));
                    sound->play();
                }
                trayIcon->showMessage("Reminder", pesan, QSystemTrayIcon::Information, 5000);

                if (i == editIndex) {
                    editIndex = -1;
                    ui->btnTambah->setText("Tambah Reminder");
                    ui->InputReminder->clear();
                }

                QWidget *w = ui->listReminder->itemWidget(item);
                if (w) {
                    QLabel *lblCountdown = w->findChild<QLabel*>("lblCountdown");
                    if (lblCountdown) {
                        lblCountdown->setText(pesan + " [" + tag + "]  —  Sudah lewat!");
                        lblCountdown->setStyleSheet(
                            "color: #e74c3c; background: transparent; font-size: 13px;"
                            );
                    }
                }
                updateStatistik();
                continue;
            }

            if (sudahSelesai) continue;

            // Hitung countdown
            QDateTime targetWaktu = QDateTime::fromString(waktu, "yyyy-MM-dd hh:mm:ss");
            qint64 selisihDetik = sekarang.secsTo(targetWaktu);

            QString countdown;
            QString warnaStr;

            if (selisihDetik <= 0) {
                countdown = "Sudah lewat!";
                warnaStr  = "#e74c3c";
            } else {
                qint64 hari  = selisihDetik / 86400;
                qint64 jam   = (selisihDetik % 86400) / 3600;
                qint64 menit = (selisihDetik % 3600) / 60;
                qint64 detik = selisihDetik % 60;

                if (hari > 0)
                    countdown = QString("%1 hari %2 jam lagi").arg(hari).arg(jam);
                else if (jam > 0)
                    countdown = QString("%1 jam %2 menit lagi").arg(jam).arg(menit);
                else if (menit > 0)
                    countdown = QString("%1 menit %2 detik lagi").arg(menit).arg(detik);
                else
                    countdown = QString("%1 detik lagi").arg(detik);

                if (selisihDetik <= 60)        warnaStr = "#e74c3c";
                else if (selisihDetik <= 3600) warnaStr = "#f39c12";
                else                           warnaStr = warnaDariTag(tag).name();
            }

            // Update label countdown di widget baris
            if (i != editIndex) {
                QWidget *w = ui->listReminder->itemWidget(item);
                if (w) {
                    QLabel *lblCountdown = w->findChild<QLabel*>("lblCountdown");
                    if (lblCountdown) {
                        lblCountdown->setText(pesan + " [" + tag + "]  —  " + countdown);
                        lblCountdown->setStyleSheet(
                            QString("color: %1; background: transparent; font-size: 13px;").arg(warnaStr)
                            );
                    }
                }
            }
        }
    });

    // Tombol Tambah
    connect(ui->btnTambah, &QPushButton::clicked, this, [=]() {
        QString reminder = ui->InputReminder->text().trimmed();
        if (reminder.isEmpty()) {
            QMessageBox::warning(this, "Peringatan", "Nama reminder tidak boleh kosong!");
            return;
        }

        QString waktu = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString tag   = ui->comboTag->currentText();
        QString dataMentah = reminder + "|" + waktu + "|" + tag;

        if (editIndex >= 0) {
            // Mode edit
            QListWidgetItem *item = ui->listReminder->item(editIndex);
            item->setData(Qt::UserRole, dataMentah);

            QWidget *w = ui->listReminder->itemWidget(item);
            if (w) {
                QLabel *lbl = w->findChild<QLabel*>("lblCountdown");
                if (lbl) lbl->setText(reminder + " [" + tag + "]");
            }

            editIndex = -1;
            ui->btnTambah->setText("Tambah Reminder");
        } else {
            addReminderItem(dataMentah);
        }

        sortReminders();
        saveToFile();
        applyFilter();
        ui->InputReminder->clear();
    });

    // Search bar
    connect(ui->inputSearch, &QLineEdit::textChanged, this, [=]() {
        applyFilter();
    });

    // Dropdown filter
    connect(ui->comboFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        applyFilter();
    });

    // Tombol Pengaturan
    connect(ui->btnPengaturan, &QPushButton::clicked, this, [=]() {
        SettingsDialog *dialog = new SettingsDialog(this);
        dialog->exec();
        delete dialog;
    });

    // Stylesheet
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

        QLineEdit, QDateTimeEdit, QComboBox {
            background-color: rgba(255, 255, 255, 10%);
            color: white;
            border: 1px solid #555;
            border-radius: 8px;
            padding: 8px;
        }

        QComboBox QAbstractItemView {
            background-color: #2c3e50;
            color: white;
            selection-background-color: #3498db;
        }

        QPushButton {
            background-color: #3498db;
            color: white;
            border-radius: 8px;
            font-weight: bold;
            padding: 8px;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QListWidget {
            background-color: rgba(0, 0, 0, 20%);
            border: none;
            color: #ecf0f1;
        }

        QMessageBox QPushButton {
            min-width: 30px;
            min-height: 15px;
            font-size: 15px;
            padding: 6px 20px;
            background-color: #3498db;
            border-radius: 8px;
            color: white;
            font-weight: bold;
        }

        QMessageBox QPushButton:hover {
            background-color: #2980b9;
        }
    )");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addReminderItem(const QString &dataMentah) {
    QStringList bagian = dataMentah.split("|");
    if (bagian.size() < 2) return;

    QString pesan = bagian[0];
    QString tag   = bagian.size() >= 3 ? bagian[2] : "Lain-lain";

    QListWidgetItem *item = new QListWidgetItem(ui->listReminder);
    item->setData(Qt::UserRole, dataMentah);
    item->setData(Qt::UserRole + 1, false); // belum selesai

    // Widget baris: [○] [teks countdown] [✏] [🗑]
    QWidget *row = new QWidget();
    row->setStyleSheet("background: transparent;");

    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(10);

    // Tombol lingkaran selesai
    QPushButton *btnBulat = new QPushButton("○");
    btnBulat->setObjectName("btnBulat");
    btnBulat->setFixedSize(28, 28);
    btnBulat->setStyleSheet(
        "QPushButton { background: transparent; color: #bdc3c7; font-size: 16px; border: none; }"
        "QPushButton:hover { color: #2ecc71; }"
        );
    btnBulat->setCursor(Qt::PointingHandCursor);

    // Label teks + countdown
    QLabel *lblCountdown = new QLabel(pesan + " [" + tag + "]");
    lblCountdown->setObjectName("lblCountdown");
    lblCountdown->setStyleSheet(
        QString("color: %1; background: transparent; font-size: 13px; font-weight: normal;")
            .arg(warnaDariTag(tag).name())
        );
    lblCountdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Tombol edit
    QPushButton *btnEditRow = new QPushButton("✏");
    btnEditRow->setObjectName("btnEditRow");
    btnEditRow->setFixedSize(28, 28);
    btnEditRow->setStyleSheet(
        "QPushButton { background: transparent; color: #3498db; font-size: 14px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background: rgba(52,152,219,0.2); }"
        );
    btnEditRow->setCursor(Qt::PointingHandCursor);

    // Tombol hapus
    QPushButton *btnHapusRow = new QPushButton("🗑");
    btnHapusRow->setObjectName("btnHapusRow");
    btnHapusRow->setFixedSize(28, 28);
    btnHapusRow->setStyleSheet(
        "QPushButton { background: transparent; color: #e74c3c; font-size: 14px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background: rgba(231,76,60,0.2); }"
        );
    btnHapusRow->setCursor(Qt::PointingHandCursor);

    layout->addWidget(btnBulat);
    layout->addWidget(lblCountdown);
    layout->addWidget(btnEditRow);
    layout->addWidget(btnHapusRow);
    row->setLayout(layout);

    item->setSizeHint(QSize(0, 46));
    ui->listReminder->setItemWidget(item, row);

    // Connect tombol selesai
    connect(btnBulat, &QPushButton::clicked, this, [=]() {
        int idx = ui->listReminder->row(item);
        toggleSelesai(idx);
    });

    // Connect tombol edit
    connect(btnEditRow, &QPushButton::clicked, this, [=]() {
        int idx = ui->listReminder->row(item);
        editItem(idx);
    });

    // Connect tombol hapus
    connect(btnHapusRow, &QPushButton::clicked, this, [=]() {
        int idx = ui->listReminder->row(item);

        QStringList b = item->data(Qt::UserRole).toString().split("|");
        QString nama = b.size() > 0 ? b[0] : "reminder ini";

        QMessageBox::StandardButton jawab = QMessageBox::question(
            this, "Hapus Reminder",
            "Yakin ingin menghapus \"" + nama + "\"?",
            QMessageBox::Yes | QMessageBox::No
            );
        if (jawab == QMessageBox::Yes)
            hapusItem(idx);
    });
}

void MainWindow::hapusItem(int index) {
    if (index < 0 || index >= ui->listReminder->count()) return;
    if (index == editIndex) {
        editIndex = -1;
        ui->btnTambah->setText("Tambah Reminder");
        ui->InputReminder->clear();
    } else if (index < editIndex) {
        editIndex--;
    }
    delete ui->listReminder->takeItem(index);
    saveToFile();
}

void MainWindow::editItem(int index) {
    if (index < 0 || index >= ui->listReminder->count()) return;

    QListWidgetItem *item = ui->listReminder->item(index);
    QString dataMentah = item->data(Qt::UserRole).toString();
    QStringList bagian = dataMentah.split("|");
    if (bagian.size() < 2) return;

    ui->InputReminder->setText(bagian[0]);
    ui->dateTimeEdit->setDateTime(QDateTime::fromString(bagian[1], "yyyy-MM-dd hh:mm:ss"));
    if (bagian.size() >= 3) {
        int idx = ui->comboTag->findText(bagian[2]);
        if (idx >= 0) ui->comboTag->setCurrentIndex(idx);
    }

    // Tandai item yang diedit dengan warna oranye
    QWidget *w = ui->listReminder->itemWidget(item);
    if (w) {
        QLabel *lbl = w->findChild<QLabel*>("lblCountdown");
        if (lbl) lbl->setStyleSheet("color: #f39c12; background: transparent; font-size: 13px;");
    }

    editIndex = index;
    ui->btnTambah->setText("Simpan Perubahan");
    ui->InputReminder->setFocus();
}

void MainWindow::toggleSelesai(int index) {
    if (index < 0 || index >= ui->listReminder->count()) return;

    QListWidgetItem *item = ui->listReminder->item(index);
    bool sudahSelesai = item->data(Qt::UserRole + 1).toBool();
    bool baru = !sudahSelesai;
    item->setData(Qt::UserRole + 1, baru);

    // AMANIN DATA: Ambil data mentah, lalu update statusnya di string agar tersimpan permanen
    QString dataMentah = item->data(Qt::UserRole).toString();
    QStringList bagian = dataMentah.split("|");

    // Pastikan struktur data mentah punya flag status di bagian akhir (indeks ke-3)
    if (bagian.size() >= 3) {
        if (bagian.size() == 3) {
            bagian.append(baru ? "1" : "0");
        } else {
            bagian[3] = baru ? "1" : "0";
        }
        item->setData(Qt::UserRole, bagian.join("|"));
    }

    QWidget *w = ui->listReminder->itemWidget(item);
    if (w) {
        QPushButton *btnBulat = w->findChild<QPushButton*>("btnBulat");
        QLabel *lbl = w->findChild<QLabel*>("lblCountdown");

        if (baru) {
            if (btnBulat) {
                btnBulat->setText("✓");
                btnBulat->setStyleSheet("QPushButton { background: transparent; color: #2ecc71; font-size: 16px; border: none; }");
            }
            if (lbl) {
                QFont f = lbl->font();
                f.setStrikeOut(true);
                lbl->setFont(f);
                lbl->setStyleSheet("color: #2ecc71; background: transparent; font-size: 13px;");
            }
        } else {
            QString tag = bagian.size() >= 3 ? bagian[2] : "Lain-lain";
            if (btnBulat) {
                btnBulat->setText("○");
                btnBulat->setStyleSheet("QPushButton { background: transparent; color: #bdc3c7; font-size: 16px; border: none; } QPushButton:hover { color: #2ecc71; }");
            }
            if (lbl) {
                QFont f = lbl->font();
                f.setStrikeOut(false);
                lbl->setFont(f);
                lbl->setStyleSheet(QString("color: %1; background: transparent; font-size: 13px;").arg(warnaDariTag(tag).name()));
            }
        }
    }
    updateStatistik();
}

void MainWindow::applyFilter() {
    QString keyword = ui->inputSearch->text().trimmed().toLower();
    int filterIdx = ui->comboFilter->currentIndex();
    QDateTime sekarang = QDateTime::currentDateTime();
    QDate hari_ini = sekarang.date();

    for (int i = 0; i < ui->listReminder->count(); i++) {
        QListWidgetItem *item = ui->listReminder->item(i);
        QString dataMentah = item->data(Qt::UserRole).toString();
        QStringList bagian = dataMentah.split("|");

        QString nama = bagian.size() > 0 ? bagian[0].toLower() : "";
        QDateTime waktu = bagian.size() > 1
                              ? QDateTime::fromString(bagian[1], "yyyy-MM-dd hh:mm:ss")
                              : QDateTime();
        bool sudahSelesai = item->data(Qt::UserRole + 1).toBool();

        bool cocokKeyword = keyword.isEmpty() || nama.contains(keyword);

        bool cocokFilter = false;
        switch (filterIdx) {
        case 0: cocokFilter = true; break; // Semua
        case 1: cocokFilter = !sudahSelesai && waktu.date() == hari_ini; break; // Hari Ini
        case 2: cocokFilter = !sudahSelesai && waktu > sekarang; break; // Upcoming
        case 3: cocokFilter = sudahSelesai; break; // Selesai
        }

        item->setHidden(!(cocokKeyword && cocokFilter));
    }
}

void MainWindow::updateStatistik() {
    int aktif = 0, selesai = 0, terlewat = 0;
    QDateTime sekarang = QDateTime::currentDateTime();

    for (int i = 0; i < ui->listReminder->count(); i++) {
        QListWidgetItem *item = ui->listReminder->item(i);
        bool sudahSelesai = item->data(Qt::UserRole + 1).toBool();

        if (sudahSelesai) {
            selesai++;
        } else {
            QString dataMentah = item->data(Qt::UserRole).toString();
            QStringList bagian = dataMentah.split("|");
            if (bagian.size() >= 2) {
                QDateTime waktu = QDateTime::fromString(bagian[1], "yyyy-MM-dd hh:mm:ss");
                if (waktu < sekarang) terlewat++;
                else aktif++;
            }
        }
    }

    ui->labelStatistik->setText(
        "📊 Aktif: " + QString::number(aktif) +
        "   ✅ Selesai: " + QString::number(selesai) +
        "   ⚠ Terlewat: " + QString::number(terlewat)
        );
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
    updateStatistik();
}

void MainWindow::sortReminders() {
    QList<QPair<QString, QListWidgetItem*>> items;
    for (int i = 0; i < ui->listReminder->count(); ++i) {
        QListWidgetItem *item = ui->listReminder->item(i);
        QString waktu = item->data(Qt::UserRole).toString().split("|").value(1);
        items.append({waktu, item});
    }

    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });

    // Rebuild list dengan urutan baru
    QList<QString> dataList;
    for (auto &pair : items) dataList.append(pair.second->data(Qt::UserRole).toString());

    ui->listReminder->clear();
    for (const QString &data : dataList) addReminderItem(data);
}

void MainWindow::on_btnTambah_clicked() {}