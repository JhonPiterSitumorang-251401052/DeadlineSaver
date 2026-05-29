#include "tugaspage.h"
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <algorithm>

// ══════════════════════════════════════════════════════
// TugasDialog
// ══════════════════════════════════════════════════════
static const QStringList COLORS = {"#FF6B6B","#FF9F43","#54A0FF","#1DD1A1","#A29BFE","#FD79A8"};
static const QStringList LABELS = {"Merah","Orange","Biru","Hijau","Ungu","Pink"};

QString TugasDialog::warnaFromPrioritas(const QString &p) {
    int i = LABELS.indexOf(p);
    return (i >= 0) ? COLORS[i] : COLORS[0];
}

TugasDialog::TugasDialog(QWidget *parent, const TugasItem *existing)
    : QDialog(parent)
{
    setWindowTitle(existing ? "Edit Tugas" : "Tambah Tugas");
    setMinimumWidth(340);
    setObjectName("AppDialog");

    QFormLayout *form = new QFormLayout(this);
    form->setSpacing(12);
    form->setContentsMargins(20, 20, 20, 20);

    editNama = new QLineEdit(this);
    editNama->setPlaceholderText("Nama tugas...");
    editNama->setObjectName("DialogInput");

    editMatkul = new QLineEdit(this);
    editMatkul->setPlaceholderText("Mata kuliah / kategori...");
    editMatkul->setObjectName("DialogInput");

    editDeadline = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1), this);
    editDeadline->setDisplayFormat("dd/MM/yyyy HH:mm");
    editDeadline->setCalendarPopup(true);
    editDeadline->setObjectName("DialogInput");

    comboPrioritas = new QComboBox(this);
    comboPrioritas->addItems(LABELS);
    comboPrioritas->setObjectName("DialogInput");

    form->addRow("Nama Tugas :", editNama);
    form->addRow("Mata Kuliah :", editMatkul);
    form->addRow("Deadline :", editDeadline);
    form->addRow("Warna :", comboPrioritas);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    btns->button(QDialogButtonBox::Ok)->setText("Simpan");
    btns->button(QDialogButtonBox::Cancel)->setText("Batal");
    btns->setObjectName("DialogBtns");
    form->addRow(btns);

    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (existing) {
        editingId = existing->id;
        editNama->setText(existing->nama);
        editMatkul->setText(existing->mataKuliah);
        editDeadline->setDateTime(existing->deadline);
        int ci = COLORS.indexOf(existing->warna);
        if (ci >= 0) comboPrioritas->setCurrentIndex(ci);
    }
}

TugasItem TugasDialog::result() const {
    TugasItem t;
    t.id         = editingId;
    t.nama       = editNama->text().trimmed();
    t.mataKuliah = editMatkul->text().trimmed();
    t.deadline   = editDeadline->dateTime();
    t.warna      = warnaFromPrioritas(comboPrioritas->currentText());
    t.selesai    = false;
    return t;
}

// ══════════════════════════════════════════════════════
// TugasCard
// ══════════════════════════════════════════════════════
TugasCard::TugasCard(const TugasItem &item, QWidget *parent)
    : QFrame(parent), itemId(item.id)
{
    setObjectName("TugasCard");
    setFixedHeight(78);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *row = new QHBoxLayout(this);
    row->setContentsMargins(14, 10, 14, 10);
    row->setSpacing(12);

    // Left colour bar
    QFrame *bar = new QFrame(this);
    bar->setFixedSize(4, 48);
    bar->setStyleSheet(
        QString("background-color:%1; border-radius:2px;").arg(item.warna));

    // Text block
    QVBoxLayout *textCol = new QVBoxLayout();
    textCol->setSpacing(4);
    textCol->setContentsMargins(0,0,0,0);

    QLabel *lblNama = new QLabel(item.nama, this);
    lblNama->setObjectName("TugasNama");
    if (item.selesai) {
        lblNama->setStyleSheet("color:#4A5F78; text-decoration:line-through;");
    }

    // Badge row: matkul + deadline
    QHBoxLayout *badgeRow = new QHBoxLayout();
    badgeRow->setSpacing(6);
    badgeRow->setContentsMargins(0,0,0,0);

    QLabel *lblMatkul = new QLabel(item.mataKuliah, this);
    lblMatkul->setStyleSheet(
        QString("background-color:%1; color:#fff; border-radius:8px;"
                "padding:2px 8px; font-size:11px; font-weight:600;")
            .arg(item.warna));
    lblMatkul->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QLabel *lblDl = new QLabel(
        item.deadline.isValid()
            ? "⏰ " + item.deadline.toString("dd MMM HH:mm")
            : "", this);
    lblDl->setObjectName("TugasDeadline");

    badgeRow->addWidget(lblMatkul);
    badgeRow->addWidget(lblDl);
    badgeRow->addStretch();

    textCol->addWidget(lblNama);
    textCol->addLayout(badgeRow);

    // Action buttons
    QPushButton *btnEdit = new QPushButton("✏", this);
    btnEdit->setObjectName("BtnCardAction");
    btnEdit->setFixedSize(28, 28);
    btnEdit->setToolTip("Edit");

    QPushButton *btnDel = new QPushButton("🗑", this);
    btnDel->setObjectName("BtnCardDelete");
    btnDel->setFixedSize(28, 28);
    btnDel->setToolTip("Hapus");

    // Checkbox
    QCheckBox *chk = new QCheckBox(this);
    chk->setObjectName("TugasCheck");
    chk->setChecked(item.selesai);

    connect(chk,    &QCheckBox::toggled, this, [=](bool c){
        emit checkToggled(itemId, c);
    });
    connect(btnEdit, &QPushButton::clicked, this, [=]{
        emit editRequested(itemId);
    });
    connect(btnDel,  &QPushButton::clicked, this, [=]{
        emit deleteRequested(itemId);
    });

    row->addWidget(bar);
    row->addLayout(textCol, 1);
    row->addWidget(btnEdit);
    row->addWidget(btnDel);
    row->addWidget(chk);
}

// ══════════════════════════════════════════════════════
// TugasPage
// ══════════════════════════════════════════════════════
TugasPage::TugasPage(QWidget *parent) : QWidget(parent)
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

    QLabel *title = new QLabel("TUGAS", header);
    title->setObjectName("PageTitle");
    title->setAlignment(Qt::AlignCenter);

    QPushButton *btnAdd = new QPushButton("+", header);
    btnAdd->setObjectName("BtnAdd");
    btnAdd->setFixedSize(36, 36);
    connect(btnAdd, &QPushButton::clicked, this, &TugasPage::onAddClicked);

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

void TugasPage::refresh() { rebuildList(); }

void TugasPage::rebuildList()
{
    // clear existing cards
    while (listLayout->count() > 1)
        delete listLayout->takeAt(0)->widget();

    auto &list = DataManager::instance().tugas();

    if (list.isEmpty()) {
        QLabel *empty = new QLabel("Belum ada tugas.\nTap + untuk menambah!", listContainer);
        empty->setObjectName("EmptyLabel");
        empty->setAlignment(Qt::AlignCenter);
        listLayout->insertWidget(0, empty);
        return;
    }

    // Sort: belum selesai dulu, lalu urutkan deadline
    QVector<TugasItem> sorted = list;
    std::sort(sorted.begin(), sorted.end(), [](const TugasItem &a, const TugasItem &b){
        if (a.selesai != b.selesai) return !a.selesai;
        return a.deadline < b.deadline;
    });

    for (const auto &item : sorted) {
        TugasCard *card = new TugasCard(item, listContainer);
        connect(card, &TugasCard::checkToggled,   this, &TugasPage::onCheckToggled);
        connect(card, &TugasCard::editRequested,  this, &TugasPage::onEditRequested);
        connect(card, &TugasCard::deleteRequested,this, &TugasPage::onDeleteRequested);
        listLayout->insertWidget(listLayout->count() - 1, card);
    }
}

void TugasPage::onAddClicked()
{
    TugasDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    TugasItem t = dlg.result();
    if (t.nama.isEmpty()) return;
    DataManager::instance().addTugas(t);
    rebuildList();
}

void TugasPage::onEditRequested(const QString &id)
{
    auto &list = DataManager::instance().tugas();
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const TugasItem &t){ return t.id == id; });
    if (it == list.end()) return;

    TugasDialog dlg(this, &(*it));
    if (dlg.exec() != QDialog::Accepted) return;
    TugasItem updated = dlg.result();
    updated.id      = id;
    updated.selesai = it->selesai;
    DataManager::instance().updateTugas(updated);
    rebuildList();
}

void TugasPage::onDeleteRequested(const QString &id)
{
    auto &list = DataManager::instance().tugas();
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const TugasItem &t){ return t.id == id; });
    QString nama = (it != list.end()) ? it->nama : id;

    QMessageBox mb(this);
    mb.setWindowTitle("Hapus Tugas");
    mb.setText("Hapus \"" + nama + "\"?");
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    mb.setDefaultButton(QMessageBox::No);
    mb.button(QMessageBox::Yes)->setText("Hapus");
    mb.button(QMessageBox::No)->setText("Batal");
    if (mb.exec() != QMessageBox::Yes) return;

    DataManager::instance().removeTugas(id);
    rebuildList();
}

void TugasPage::onCheckToggled(const QString &id, bool checked)
{
    auto &list = DataManager::instance().tugas();
    auto it = std::find_if(list.begin(), list.end(),
                           [&](const TugasItem &t){ return t.id == id; });
    if (it == list.end()) return;
    it->selesai = checked;
    DataManager::instance().save();

    // Defer rebuild — jangan hapus widget saat event checkbox masih berjalan
    QTimer::singleShot(0, this, &TugasPage::rebuildList);
}
