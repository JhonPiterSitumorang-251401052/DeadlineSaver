#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

#include "jadwalpage.h"
#include "tugaspage.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void buildUI();
    void buildNavBar();
    QPushButton *navButton(const QString &icon, const QString &label);
    void switchPage(int index);
    void applyGlobalStyle();
    void checkDeadlines();

    QStackedWidget *stack;
    JadwalPage     *jadwalPage;
    TugasPage      *tugasPage;

    QPushButton *navTugas;
    QPushButton *navJadwal;
    QPushButton *navBeranda;
    QPushButton *navPengingat;
    QPushButton *navPengaturan;

    QSystemTrayIcon *trayIcon;
    QTimer          *deadlineTimer;
};

#endif // MAINWINDOW_H
