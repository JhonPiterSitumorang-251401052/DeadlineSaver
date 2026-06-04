#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>

// Map nama preset ke nama file
static QString fileFromPreset(const QString &preset) {
    if (preset == "Bip")     return "bip.wav";
    if (preset == "Buzzer")  return "buzzer.wav";
    if (preset == "Scanner") return "scanner.wav";
    return "alarm.wav"; // Default
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowTitle("Pengaturan");
    setFixedSize(400, 300);

    connect(ui->btnSimpan, &QPushButton::clicked, this, &SettingsDialog::onSimpan);
    connect(ui->btnBatal,  &QPushButton::clicked, this, &QDialog::reject);

    this->setStyleSheet(R"(
        QDialog {
            background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #2c3e50, stop:1 #1a1a2e);
        }
        QLabel {
            color: #ecf0f1;
            font-size: 13px;
            font-weight: normal;
        }
        QLabel#labelJudul {
            font-size: 18px;
            font-weight: bold;
            color: white;
        }
        QComboBox {
            background: rgba(255,255,255,10%);
            color: white;
            border: 1px solid #555;
            border-radius: 6px;
            padding: 6px;
        }
        QComboBox QAbstractItemView {
            background: #2c3e50;
            color: white;
            selection-background-color: #3498db;
        }
        QCheckBox {
            color: #ecf0f1;
            font-size: 13px;
        }
        QPushButton {
            background: #3498db;
            color: white;
            border-radius: 6px;
            font-weight: bold;
            padding: 6px 14px;
        }
        QPushButton:hover { background: #2980b9; }
        QPushButton#btnBatal { background: #555; }
        QPushButton#btnBatal:hover { background: #666; }
    )");

    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadSettings() {
    QSettings s("DeadlineSaver", "Settings");

    // Menit sebelum
    int menit = s.value("menitSebelum", 15).toInt();
    int idx = ui->comboMenit->findText(QString::number(menit) + " menit");
    if (idx >= 0) ui->comboMenit->setCurrentIndex(idx);

    // Preset suara
    QString preset = s.value("soundPreset", "Default").toString();
    int sidx = ui->comboSound->findText(preset);
    if (sidx >= 0) ui->comboSound->setCurrentIndex(sidx);

    // Mode senyap
    bool senyap = s.value("modeSenyap", false).toBool();
    ui->toggleSenyap->setChecked(senyap);
}

void SettingsDialog::saveSettings() {
    QSettings s("DeadlineSaver", "Settings");

    QString menitStr = ui->comboMenit->currentText().split(" ").first();
    s.setValue("menitSebelum", menitStr.toInt());
    s.setValue("soundPreset",  ui->comboSound->currentText());
    s.setValue("soundFile",    fileFromPreset(ui->comboSound->currentText()));
    s.setValue("modeSenyap",   ui->toggleSenyap->isChecked());
}

void SettingsDialog::onSimpan() {
    saveSettings();
    accept();
}

int SettingsDialog::menitSebelum() const {
    QSettings s("DeadlineSaver", "Settings");
    return s.value("menitSebelum", 15).toInt();
}

QString SettingsDialog::soundFile() const {
    QSettings s("DeadlineSaver", "Settings");
    return s.value("soundFile", "alarm.wav").toString();
}

bool SettingsDialog::modeSenyap() const {
    QSettings s("DeadlineSaver", "Settings");
    return s.value("modeSenyap", false).toBool();
}
