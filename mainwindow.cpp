#include "mainwindow.h"
#include "datamanager.h"
#include <QApplication>
#include <QMenu>

// ══════════════════════════════════════════════════════
// MainWindow
// ══════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Deadline Saver");
    setMinimumSize(400, 700);
    resize(420, 760);

    DataManager::instance().load();

    applyGlobalStyle();
    buildUI();

    // ── System tray ──────────────────────────────────────
    trayIcon = new QSystemTrayIcon(
        style()->standardIcon(QStyle::SP_ComputerIcon), this);
    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("Buka", this, [=]{ showNormal(); raise(); activateWindow(); });
    trayMenu->addSeparator();
    trayMenu->addAction("Keluar", qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip("Deadline Saver");
    trayIcon->show();

    // ── Deadline checker – every minute ──────────────────
    deadlineTimer = new QTimer(this);
    connect(deadlineTimer, &QTimer::timeout, this, &MainWindow::checkDeadlines);
    deadlineTimer->start(60000);
    checkDeadlines(); // run once on start

    switchPage(0);
}

MainWindow::~MainWindow() {}

void MainWindow::checkDeadlines()
{
    QDateTime now = QDateTime::currentDateTime();
    for (const auto &t : DataManager::instance().tugas()) {
        if (t.selesai) continue;
        if (!t.deadline.isValid()) continue;

        qint64 diff = now.secsTo(t.deadline);
        // notify if deadline within next 60 minutes and not past
        if (diff >= 0 && diff <= 3600) {
            int menit = static_cast<int>(diff / 60);
            trayIcon->showMessage(
                "⏰ Deadline Mendekat!",
                QString("\"%1\" deadline %2 menit lagi!")
                    .arg(t.nama)
                    .arg(menit > 0 ? QString::number(menit) : "kurang dari 1"),
                QSystemTrayIcon::Warning,
                5000
            );
        }
        // notify if deadline is now or overdue (within last 5 min)
        if (diff < 0 && diff >= -300) {
            trayIcon->showMessage(
                "🚨 Deadline Terlewat!",
                QString("\"%1\" sudah melewati deadline!").arg(t.nama),
                QSystemTrayIcon::Critical,
                8000
            );
        }
    }
}

void MainWindow::buildUI()
{
    QWidget *central = new QWidget(this);
    central->setObjectName("Central");
    setCentralWidget(central);

    QVBoxLayout *vbox = new QVBoxLayout(central);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // App title bar
    QWidget *appBar = new QWidget(central);
    appBar->setObjectName("AppBar");
    appBar->setFixedHeight(52);
    QHBoxLayout *abLayout = new QHBoxLayout(appBar);
    abLayout->setContentsMargins(20, 0, 20, 0);
    QLabel *appTitle = new QLabel("DEADLINE SAVER", appBar);
    appTitle->setObjectName("AppTitle");
    appTitle->setAlignment(Qt::AlignCenter);
    abLayout->addWidget(appTitle);

    // Stack
    stack = new QStackedWidget(central);
    stack->setObjectName("PageStack");

    // Page 0 – Tugas ✅
    tugasPage = new TugasPage();
    stack->addWidget(tugasPage);

    // Page 1 – Jadwal ✅
    jadwalPage = new JadwalPage();
    stack->addWidget(jadwalPage);

    // Placeholder factory
    auto makePlaceholder = [](const QString &text) {
        QWidget *w = new QWidget();
        w->setObjectName("PlaceholderPage");
        QVBoxLayout *l = new QVBoxLayout(w);
        QLabel *lbl = new QLabel(text, w);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color:#4A5F78; font-size:16px; font-weight:600;");
        l->addWidget(lbl);
        return w;
    };

    // Page 2 – Beranda ✅
    berandaPage = new BerandaPage();
    stack->addWidget(berandaPage);
    stack->addWidget(makePlaceholder("Pengingat\n(coming soon)"));
    stack->addWidget(makePlaceholder("Pengaturan\n(coming soon)"));

    // Nav bar
    QWidget *navBar = new QWidget(central);
    navBar->setObjectName("NavBar");
    navBar->setFixedHeight(70);
    buildNavBar();

    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);
    navLayout->addWidget(navTugas);
    navLayout->addWidget(navJadwal);
    navLayout->addWidget(navBeranda);
    navLayout->addWidget(navPengingat);
    navLayout->addWidget(navPengaturan);

    vbox->addWidget(appBar);
    vbox->addWidget(stack, 1);
    vbox->addWidget(navBar);
}

void MainWindow::buildNavBar()
{
    navTugas      = navButton("📋", "Tugas");
    navJadwal     = navButton("📅", "Jadwal");
    navBeranda    = navButton("🏠", "Beranda");
    navPengingat  = navButton("🔔", "Pengingat");
    navPengaturan = navButton("⚙",  "Pengaturan");

    connect(navTugas,      &QPushButton::clicked, this, [=]{ switchPage(0); });
    connect(navJadwal,     &QPushButton::clicked, this, [=]{ switchPage(1); });
    connect(navBeranda,    &QPushButton::clicked, this, [=]{ switchPage(2); });
    connect(navPengingat,  &QPushButton::clicked, this, [=]{ switchPage(3); });
    connect(navPengaturan, &QPushButton::clicked, this, [=]{ switchPage(4); });
}

QPushButton *MainWindow::navButton(const QString &icon, const QString &label)
{
    QPushButton *btn = new QPushButton(this);
    btn->setObjectName("NavBtn");
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setCheckable(true);

    QVBoxLayout *vl = new QVBoxLayout(btn);
    vl->setContentsMargins(0, 6, 0, 6);
    vl->setSpacing(2);

    QLabel *ico = new QLabel(icon, btn);
    ico->setObjectName("NavIcon");
    ico->setAlignment(Qt::AlignCenter);

    QLabel *lbl = new QLabel(label, btn);
    lbl->setObjectName("NavLabel");
    lbl->setAlignment(Qt::AlignCenter);

    vl->addWidget(ico);
    vl->addWidget(lbl);
    return btn;
}

void MainWindow::switchPage(int index)
{
    stack->setCurrentIndex(index);
    // refresh data pages when switching to them
    if (index == 0) tugasPage->refresh();
    if (index == 1) jadwalPage->refresh();
    if (index == 2) berandaPage->refresh();

    QList<QPushButton*> btns = {
        navTugas, navJadwal, navBeranda, navPengingat, navPengaturan
    };
    for (int i = 0; i < btns.size(); ++i)
        btns[i]->setChecked(i == index);
}

void MainWindow::applyGlobalStyle()
{
    setStyleSheet(R"(
        QMainWindow, QWidget#Central, QWidget#PlaceholderPage {
            background-color: #0D1B2A;
        }
        QWidget#AppBar {
            background-color: #0D1B2A;
            border-bottom: 1px solid #1E2D40;
        }
        QLabel#AppTitle {
            color: #FFFFFF;
            font-size: 18px;
            font-weight: 900;
            letter-spacing: 4px;
        }
        QStackedWidget#PageStack { background-color: #0D1B2A; }

        /* Page header */
        QWidget#PageHeader { background-color: #0D1B2A; }
        QLabel#PageTitle {
            color: #FFFFFF;
            font-size: 15px;
            font-weight: 700;
            letter-spacing: 2px;
        }
        QPushButton#BtnBack, QPushButton#BtnAdd {
            background-color: #1E2D40;
            color: #FFFFFF;
            border: none;
            border-radius: 18px;
            font-size: 18px;
            font-weight: bold;
        }
        QPushButton#BtnBack:hover, QPushButton#BtnAdd:hover {
            background-color: #2A3F58;
        }

        /* Scroll */
        QScrollArea#CardScrollArea, QWidget#ListContainer {
            background-color: transparent;
        }
        QScrollBar:vertical { background:transparent; width:4px; }
        QScrollBar::handle:vertical { background:#2A3F58; border-radius:2px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }

        /* Empty state */
        QLabel#EmptyLabel {
            color: #4A5F78;
            font-size: 15px;
            font-weight: 500;
        }

        /* Jadwal card */
        QFrame#JadwalCard {
            background-color: #162032;
            border-radius: 12px;
            border: 1px solid #1E2D40;
        }
        QLabel#CardJam {
            color: #8899BB;
            font-size: 13px;
            font-weight: 600;
        }
        QFrame#CardDivider { background-color: #2A3F58; }
        QLabel#CardNama {
            color: #FFFFFF;
            font-size: 14px;
            font-weight: 500;
        }

        /* Tugas card */
        QFrame#TugasCard {
            background-color: #162032;
            border-radius: 12px;
            border: 1px solid #1E2D40;
        }
        QLabel#TugasNama {
            color: #FFFFFF;
            font-size: 14px;
            font-weight: 600;
        }
        QLabel#TugasDeadline {
            color: #8899BB;
            font-size: 11px;
        }

        /* Card action buttons */
        QPushButton#BtnCardAction {
            background-color: #1E2D40;
            border: none;
            border-radius: 6px;
            font-size: 13px;
        }
        QPushButton#BtnCardAction:hover { background-color: #2A3F58; }
        QPushButton#BtnCardDelete {
            background-color: #2D1A1A;
            border: none;
            border-radius: 6px;
            font-size: 13px;
        }
        QPushButton#BtnCardDelete:hover { background-color: #3D2020; }

        /* Checkbox */
        QCheckBox#TugasCheck { spacing:0; }
        QCheckBox#TugasCheck::indicator {
            width:22px; height:22px;
            border-radius:11px;
            border:2px solid #2A3F58;
            background-color:#0D1B2A;
        }
        QCheckBox#TugasCheck::indicator:checked {
            background-color:#54A0FF;
            border-color:#54A0FF;
        }
        QCheckBox#TugasCheck::indicator:hover { border-color:#54A0FF; }

        /* Dialog */
        QDialog#AppDialog {
            background-color: #0D1B2A;
        }
        QDialog#AppDialog QLabel {
            color: #8899BB;
            font-size: 13px;
        }
        QLineEdit#DialogInput, QDateTimeEdit#DialogInput,
        QTimeEdit#DialogInput, QComboBox#DialogInput {
            background-color: #162032;
            color: #FFFFFF;
            border: 1px solid #2A3F58;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: 13px;
        }
        QLineEdit#DialogInput:focus, QDateTimeEdit#DialogInput:focus,
        QTimeEdit#DialogInput:focus, QComboBox#DialogInput:focus {
            border-color: #54A0FF;
        }
        QComboBox#DialogInput QAbstractItemView {
            background-color: #162032;
            color: #FFFFFF;
            selection-background-color: #54A0FF;
        }
        QDialogButtonBox QPushButton {
            background-color: #54A0FF;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 13px;
            font-weight: 600;
            min-width: 80px;
        }
        QDialogButtonBox QPushButton:hover { background-color: #2980B9; }
        QDialogButtonBox QPushButton[text="Batal"] {
            background-color: #1E2D40;
        }
        QDialogButtonBox QPushButton[text="Batal"]:hover {
            background-color: #2A3F58;
        }

        /* Beranda */
        QWidget#SummaryBar {
            background-color: #111C2B;
            border-bottom: 1px solid #1E2D40;
        }
        QFrame#BerandaGroup {
            background-color: #162032;
            border-radius: 12px;
            border: 1px solid #1E2D40;
        }
        QWidget#BerandaGroupHeader {
            background-color: transparent;
            border-radius: 12px;
        }
        QWidget#BerandaGroupHeader:hover {
            background-color: #1A2840;
        }
        QLabel#BerandaGroupTitle {
            color: #FFFFFF;
            font-size: 14px;
            font-weight: 700;
        }
        QPushButton#BtnToggle {
            background-color: #1E2D40;
            color: #8899BB;
            border: none;
            border-radius: 6px;
            font-size: 11px;
        }
        QPushButton#BtnToggle:hover { background-color: #2A3F58; }
        QWidget#BerandaItemContainer {
            background-color: transparent;
        }
        QWidget#BerandaItem {
            background-color: #0D1B2A;
            border-radius: 8px;
        }
        QWidget#BerandaItem:hover {
            background-color: #1A2840;
        }
        QLabel#BerandaItemNama {
            color: #CCDDEE;
            font-size: 13px;
        }
        QLabel#BerandaItemDl {
            color: #8899BB;
            font-size: 11px;
        }

        /* Nav bar */
        QWidget#NavBar {
            background-color: #111C2B;
            border-top: 1px solid #1E2D40;
        }
        QPushButton#NavBtn {
            background-color: transparent;
            border: none;
            border-radius: 0;
        }
        QPushButton#NavBtn:checked { background-color: #162032; }
        QPushButton#NavBtn:hover   { background-color: #1A2840; }
        QLabel#NavIcon  { color:#4A5F78; font-size:20px; }
        QLabel#NavLabel { color:#4A5F78; font-size:10px; font-weight:500; }
        QPushButton#NavBtn:checked QLabel#NavIcon  { color:#54A0FF; }
        QPushButton#NavBtn:checked QLabel#NavLabel { color:#54A0FF; }
    )");
}
