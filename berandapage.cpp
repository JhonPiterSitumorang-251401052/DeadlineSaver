#include "berandapage.h"
#include <QTimer>

// ══════════════════════════════════════════════════════
// BerandaItem — satu baris tugas di dalam grup
// ══════════════════════════════════════════════════════
BerandaItem::BerandaItem(const TugasItem &item, const QString &warna, QWidget *parent)
    : QWidget(parent)
{
    setObjectName("BerandaItem");
    setFixedHeight(44);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *row = new QHBoxLayout(this);
    row->setContentsMargins(12, 0, 12, 0);
    row->setSpacing(10);

    // Dot
    QLabel *dot = new QLabel(this);
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(
        QString("background-color:%1; border-radius:4px;").arg(warna));

    // Nama tugas
    QLabel *lblNama = new QLabel(item.nama, this);
    lblNama->setObjectName("BerandaItemNama");
    if (item.selesai)
        lblNama->setStyleSheet("color:#4A5F78; text-decoration:line-through;");

    // Deadline
    QString dlText = item.deadline.isValid()
        ? item.deadline.toString("dd MMM HH:mm") : "";
    QLabel *lblDl = new QLabel(dlText, this);
    lblDl->setObjectName("BerandaItemDl");

    // Status badge
    QLabel *lblStatus = new QLabel(item.selesai ? "✓ Selesai" : "Pending", this);
    lblStatus->setObjectName(item.selesai ? "BadgeDone" : "BadgePending");
    lblStatus->setStyleSheet(
        item.selesai
            ? "background-color:#1DD1A1; color:#fff; border-radius:8px; padding:2px 8px; font-size:10px; font-weight:600;"
            : QString("background-color:%1; color:#fff; border-radius:8px; padding:2px 8px; font-size:10px; font-weight:600;").arg(warna)
    );
    lblStatus->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    row->addWidget(dot);
    row->addWidget(lblNama, 1);
    row->addWidget(lblDl);
    row->addWidget(lblStatus);
}

// ══════════════════════════════════════════════════════
// BerandaGroup — satu grup mata kuliah
// ══════════════════════════════════════════════════════
BerandaGroup::BerandaGroup(const QString &mataKuliah,
                           const QString &warna,
                           const QVector<TugasItem> &items,
                           QWidget *parent)
    : QFrame(parent)
{
    setObjectName("BerandaGroup");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);

    // ── Header row ───────────────────────────────────────
    QWidget *header = new QWidget(this);
    header->setObjectName("BerandaGroupHeader");
    header->setFixedHeight(52);
    header->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *hdr = new QHBoxLayout(header);
    hdr->setContentsMargins(14, 0, 14, 0);
    hdr->setSpacing(10);

    // Colour bar
    QFrame *bar = new QFrame(header);
    bar->setFixedSize(4, 32);
    bar->setStyleSheet(
        QString("background-color:%1; border-radius:2px;").arg(warna));

    // Mata kuliah name
    QLabel *lblMk = new QLabel(mataKuliah, header);
    lblMk->setObjectName("BerandaGroupTitle");

    // Count badge
    int total    = items.size();
    int selesai  = 0;
    for (const auto &t : items) if (t.selesai) selesai++;

    QLabel *lblCount = new QLabel(
        QString("%1/%2 selesai").arg(selesai).arg(total), header);
    lblCount->setStyleSheet(
        QString("background-color:%1; color:#fff; border-radius:8px;"
                "padding:3px 10px; font-size:11px; font-weight:600;")
            .arg(warna));
    lblCount->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    // Toggle button
    btnToggle = new QPushButton("▼", header);
    btnToggle->setObjectName("BtnToggle");
    btnToggle->setFixedSize(28, 28);
    connect(btnToggle, &QPushButton::clicked, this, &BerandaGroup::toggleExpand);

    // Also toggle when clicking header
    header->installEventFilter(this);

    hdr->addWidget(bar);
    hdr->addWidget(lblMk, 1);
    hdr->addWidget(lblCount);
    hdr->addWidget(btnToggle);

    // ── Items container ───────────────────────────────────
    itemContainer = new QWidget(this);
    itemContainer->setObjectName("BerandaItemContainer");
    QVBoxLayout *il = new QVBoxLayout(itemContainer);
    il->setContentsMargins(8, 4, 8, 8);
    il->setSpacing(2);

    for (const auto &item : items) {
        BerandaItem *bi = new BerandaItem(item, warna, itemContainer);
        il->addWidget(bi);
    }

    vl->addWidget(header);
    vl->addWidget(itemContainer);
}

void BerandaGroup::toggleExpand()
{
    expanded = !expanded;
    itemContainer->setVisible(expanded);
    btnToggle->setText(expanded ? "▼" : "▶");
}

bool BerandaGroup::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        toggleExpand();
        return true;
    }
    return QFrame::eventFilter(obj, event);
}

// ══════════════════════════════════════════════════════
// BerandaPage
// ══════════════════════════════════════════════════════
BerandaPage::BerandaPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    QWidget *header = new QWidget(this);
    header->setObjectName("PageHeader");
    header->setFixedHeight(64);
    QHBoxLayout *hdr = new QHBoxLayout(header);
    hdr->setContentsMargins(16, 0, 16, 0);

    QLabel *title = new QLabel("BERANDA", header);
    title->setObjectName("PageTitle");
    title->setAlignment(Qt::AlignCenter);
    hdr->addWidget(title);

    // Summary bar
    summaryBar = new QWidget(this);
    summaryBar->setObjectName("SummaryBar");
    summaryBar->setFixedHeight(60);
    // stat diisi oleh refresh() saat pertama kali dipanggil

    // Scroll
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("CardScrollArea");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    listContainer = new QWidget();
    listContainer->setObjectName("ListContainer");
    listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(16, 8, 16, 16);
    listLayout->setSpacing(12);
    listLayout->addStretch();

    scroll->setWidget(listContainer);

    root->addWidget(header);
    root->addWidget(summaryBar);
    root->addWidget(scroll, 1);
}

void BerandaPage::refresh()
{
    // Rebuild summary bar
    QLayout *old = summaryBar->layout();
    if (old) {
        QLayoutItem *item;
        while ((item = old->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete old;
    }

    QHBoxLayout *sl = new QHBoxLayout(summaryBar);
    sl->setContentsMargins(20, 0, 20, 0);
    sl->setSpacing(16);

    auto &tugasList = DataManager::instance().tugas();
    int total   = tugasList.size();
    int selesai = 0;
    for (const auto &t : tugasList) if (t.selesai) selesai++;
    int pending = total - selesai;

    auto makeStat = [&](const QString &val, const QString &lbl, const QString &color) {
        QWidget *w = new QWidget(summaryBar);
        QVBoxLayout *wl = new QVBoxLayout(w);
        wl->setContentsMargins(0, 0, 0, 0);
        wl->setSpacing(2);
        QLabel *lv = new QLabel(val, w);
        lv->setAlignment(Qt::AlignCenter);
        lv->setStyleSheet(QString("color:%1; font-size:22px; font-weight:800;").arg(color));
        QLabel *ll = new QLabel(lbl, w);
        ll->setAlignment(Qt::AlignCenter);
        ll->setStyleSheet("color:#8899BB; font-size:11px; font-weight:500;");
        wl->addWidget(lv);
        wl->addWidget(ll);
        return w;
    };
    auto makeDivider = [&]() {
        QFrame *d = new QFrame(summaryBar);
        d->setFrameShape(QFrame::VLine);
        d->setFixedWidth(1);
        d->setStyleSheet("background-color:#1E2D40;");
        return d;
    };

    sl->addStretch();
    sl->addWidget(makeStat(QString::number(total),   "Total",   "#54A0FF"));
    sl->addWidget(makeDivider());
    sl->addWidget(makeStat(QString::number(pending), "Pending", "#FF9F43"));
    sl->addWidget(makeDivider());
    sl->addWidget(makeStat(QString::number(selesai), "Selesai", "#1DD1A1"));
    sl->addStretch();

    rebuildList();
}

void BerandaPage::rebuildList()
{
    while (listLayout->count() > 1)
        delete listLayout->takeAt(0)->widget();

    auto &list = DataManager::instance().tugas();

    if (list.isEmpty()) {
        QLabel *empty = new QLabel("Belum ada tugas.\nTambahkan dari tab Tugas!", listContainer);
        empty->setObjectName("EmptyLabel");
        empty->setAlignment(Qt::AlignCenter);
        listLayout->insertWidget(0, empty);
        return;
    }

    // Group by mataKuliah
    QMap<QString, QVector<TugasItem>> groups;
    QMap<QString, QString> groupColors;
    for (const auto &t : list) {
        groups[t.mataKuliah].append(t);
        groupColors[t.mataKuliah] = t.warna;
    }

    int idx = 0;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        BerandaGroup *grp = new BerandaGroup(
            it.key(), groupColors[it.key()], it.value(), listContainer);
        listLayout->insertWidget(idx++, grp);
    }
}
