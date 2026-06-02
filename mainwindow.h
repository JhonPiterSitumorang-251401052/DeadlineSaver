#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void sortReminders();

private slots:
    void on_btnTambah_clicked();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    int editIndex = -1;

    void saveToFile();
    void updateStatistik();
    void addReminderItem(const QString &dataMentah);
    void hapusItem(int index);
    void editItem(int index);
    void toggleSelesai(int index);
    void updateRowWidget(int index);
    void applyFilter();
};
#endif // MAINWINDOW_H
