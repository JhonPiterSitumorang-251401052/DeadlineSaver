#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Getter untuk dibaca MainWindow
    int  menitSebelum() const;
    QString soundFile() const;
    bool modeSenyap() const;

private slots:
    void onSimpan();

private:
    Ui::SettingsDialog *ui;

    void loadSettings();
    void saveSettings();
};

#endif // SETTINGSDIALOG_H
