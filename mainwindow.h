#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

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
};
#endif // MAINWINDOW_H
