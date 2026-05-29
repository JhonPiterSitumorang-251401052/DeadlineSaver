#include "jadwalpage.h"
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <algorithm>

static const QStringList WARNA_COLORS = {"#FF6B6B","#FF9F43","#54A0FF","#1DD1A1","#A29BFE","#FD79A8"};
static const QStringList WARNA_LABELS = {"Merah","Orange","Biru","Hijau","Ungu","Pink"};
static const QStringList HARI_LIST    = {"Setiap Hari","Senin","Selasa","Rabu","Kamis","Jumat","Sabtu","Minggu"};

// ══════════════════════════════════════════════════════
// JadwalDialog
// ══════════════════════════════════════════════════════
JadwalDialog::JadwalDialog(QWidget *parent, const JadwalItem *existing)
    : QDialog(parent)
{
    setWindowTitle(existing ? "Edit Jadwal" : "Tambah Jadwal");
    setMinimumWidth(320);
    setObjectName("AppDialog");

    QFormLayout *form = new QFormLayout(this);
    form->setSpacing(12);
    form->setContentsMargins(20, 20, 20, 20);

    editNama = new QLineEdit(this);
    editNama->setPlaceholderText("Nama kegiatan...");
    editNama->setObjectName("DialogInput");

    editJam = new QTimeEdit(QTime::currentTime(), this);
    editJam->setDisplayFormat("HH:mm");
    editJam->setObjectName("DialogInput");

    comboHari = new QComboBox(this);
    comboHari->addItems(HARI_LIST);
    comboHari->setObjectName("DialogInput");

    comboWarna = new QComboBox(this);
    comboWarna->addItems(WARNA_LABELS);
    comboWarna->setObjectName("DialogInput");

    form->addRow("Nama :", editNama);
    form->addRow("Jam :", editJam);
    form->addRow("Hari :", comboHari);
    form->addRow("Warna :", comboWarna);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btns->button(QDialogButtonBox::Ok)->setText("Simpan");
    btns->button(QDialogButtonBox::Cancel)->setText("Batal");
    form->addRow(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (existing) {
        editingId = existing->id;
        editNama->setText(existing->nama);
        editJam->setTime(QTime::fromString(existing->jam, "HH:mm"));
        int hi = HARI_LIST.indexOf(existing->hari);
        if (hi >= 0) comboHari->setCurrentIndex(hi);
        int wi = WARNA_COLORS.indexOf(existing->warna);
        if (wi >= 0) comboWarna->setCurrentIndex(wi);
    }
}

JadwalItem JadwalDialog::result() const {
    JadwalItem j;
    j.id    = editingId;
    j.nama  = editNama->text().trimmed();
    j.jam   = editJam->time().toString("HH:mm");
    j.hari  = comboHari->currentText();
    int wi  = WARNA_LABELS.indexOf(comboWarna->currentText());
    j.warna = (wi >= 0) ? WARNA_COLORS[wi] : WARNA_COLORS[2];
    return j;
}

// ══════════════════════════════════════════════════════
// JadwalCard
// ══════════════════════════════════════════════════════
JadwalCard::JadwalCard(const JadwalItem &item, QWidget *parent)
    : QFrame(parent), itemId(item.id)
{
    setObjectName("JadwalCard");
    setFixedHeight(68);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *row = new QHBoxLayout(this);
    row->setContentsMargins(14, 8, 14, 8);
    row->setSpacing(12);

    // Jam
    QLabel *lblJam = new QLabel(item.jam, this);
    lblJam->setObjectName("CardJam");
    lblJam->setFixedWidth(48);
    lblJam->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    // Divider
    QFrame *div = new QFrame(this);
    div->setFrameShape(QFrame::VLine);
    div->setObjectName("CardDivider");
    div->setFixedWidth(2);

    // Text col
    QVBoxLayout *textCol = new QVBoxLayout();
    textCol->setSpacing(3);
    textCol->setContentsMargins(0,0,0,0);

    QLabel *lblNama = new QLabel(item.nama, this);
    lblNama->setObjectName("CardNama");

    QLabel *lblHari = new QLabel(item.hari, this);
    lblHari->setStyleSheet(
        QString("background-color:%1; color:#fff; border-radius:8px;"
                "padding:2px 8px; font-size:11px; font-weight:600;")
            .arg(item.warna));
    lblHari->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    textCol->addWidget(lblNama);
    textCol->addWidget(lblHari);

    // Badge dot
    QLabel *dot = new QLabel(this);
    dot->setFixedSize(12, 12);
    dot->setStyleSheet(
        QString("background-color:%1; border-radius:6px;").arg(item.warna));

    // Edit / Delete buttons
    QPushButton *btnEdit = new QPushButton("✏", this);
    btnEdit->setObjectName("BtnCardAction");
    btnEdit->setFixedSize(28, 28);

    QPushButton *btnDel = new QPushButton("🗑", this);
    btnDel->setObjectName("BtnCardDelete");
    btnDel->setFixedSize(28, 28);

    connect(btnEdit, &QPushButton::clicked, this, [=]{ emit editRequested(itemId); });
    connect(btnDel,  &QPushButton::clicked, this, [=]{ emit deleteRequested(itemId); });

    row->addWidget(lblJam);
    row->addWidget(div);
    row->addLayout(textCol, 1);
    row->addWidget(dot);
    row->addWidget(btnEdit);
    row->addWidget(btnDel);
}

// ══════════════════════════════════════════════════════
// JadwalPage
// ══════════════════════════════════════════════════════
JadwalPage::JadwalPage(QWidget *parent) : QWidget(parent)
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

    QLabel *title = new QLabel("JADWAL", header);
    title->setObjectName("PageTitle");
    title->setAlignment(Qt::AlignCenter);

    QPushButton *btnAdd = new QPushButton("+", header);
    btnAdd->setObjectName("BtnAdd");
    btnAdd->setFixedSize(36, 36);
    connect(btnAdd, &QPushButton::clicked, this, &JadwalPage::onAddClicked);

    hdr->addWidget(title, 1);
    hdr->addWidget(btnAdd);

    // Scroll
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setObjectName("CardScrollArea");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    listContainer = new QWidget();
    listContainer->setObjectName("ListContainer");
    listLayout = new QVBoxLayout(listContainer);
    listLayout->setContentsMargins(16, 16, 16, 16);
    listLayout->setSpacing(10);
    listLayout->addStretch();

    scroll->setWidget(listContainer);

    root->addWidget(header);
    root->addWidget(scroll, 1);

    refresh();
}

void JadwalPage::refresh() { rebuildList(); }

void JadwalPage::rebuildList()
{
    while (listLayout->count() > 1)
        delete listLayout->takeAt(0)->widget();

    auto &list = DataManager::instance().jadwal();

    if (list.isEmpty()) {
        QLabel *empty = new QLabel("Belum ada jadwal.\nTap + untuk menambah!", listContainer);
        empty->setObjectName("EmptyLabel");
        empty->setAlignment(Qt::AlignCenter);
        listLayout->insertWidget(0, empty);
        return;
    }

    QVector<JadwalItem> sorted = list;
    std::sort(sorted.begin(), sorted.end(), [](const JadwalItem &a, const JadwalItem &b){
        return a.jam < b.jam;
    });

    for (const auto &item : sorted) {
        JadwalCard *card = new JadwalCard(item, listContainer);
        connect(card, &JadwalCard::editRequested,   this, &JadwalPage::onEditRequested);
        connect(card, &JadwalCard::deleteRequested, this, &JadwalPage::onDeleteRequested);
        listLayout->insertWidget(listLayout->count() - 1, card);
    }
}

void JadwalPage::onAddClicked()
{
    JadwalDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    JadwalItem j = dlg.result();
    if (j.nama.isEmpty()) return;
    DataManager::instance().addJadwal(j);
    rebuildList();
}

void JadwalPage::onEditRequested(const QString &id)
{
    auto &list = DataManager::instance().jadwal();
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const JadwalItem &j){ return j.id == id; });
    if (it == list.end()) return;

    JadwalDialog dlg(this, &(*it));
    if (dlg.exec() != QDialog::Accepted) return;
    JadwalItem updated = dlg.result();
    updated.id = id;
    DataManager::instance().updateJadwal(updated);
    rebuildList();
}

void JadwalPage::onDeleteRequested(const QString &id)
{
    auto &list = DataManager::instance().jadwal();
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const JadwalItem &j){ return j.id == id; });
    QString nama = (it != list.end()) ? it->nama : id;

    QMessageBox mb(this);
    mb.setWindowTitle("Hapus Jadwal");
    mb.setText("Hapus \"" + nama + "\"?");
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.button(QMessageBox::Yes)->setText("Hapus");
    mb.button(QMessageBox::No)->setText("Batal");
    if (mb.exec() != QMessageBox::Yes) return;

    DataManager::instance().removeJadwal(id);
    rebuildList();
}
