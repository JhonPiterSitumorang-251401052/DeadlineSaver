#ifndef JADWALPAGE_H
#define JADWALPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QScrollArea>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTimeEdit>
#include "datamanager.h"

// ─── Dialog tambah / edit jadwal ─────────────────────────────────────────────
class JadwalDialog : public QDialog {
    Q_OBJECT
public:
    explicit JadwalDialog(QWidget *parent = nullptr, const JadwalItem *existing = nullptr);
    JadwalItem result() const;

private:
    QLineEdit  *editNama;
    QTimeEdit  *editJam;
    QComboBox  *comboHari;
    QComboBox  *comboWarna;
    QString     editingId;
};

// ─── Single jadwal card ───────────────────────────────────────────────────────
class JadwalCard : public QFrame {
    Q_OBJECT
public:
    explicit JadwalCard(const JadwalItem &item, QWidget *parent = nullptr);

signals:
    void deleteRequested(const QString &id);
    void editRequested(const QString &id);

private:
    QString itemId;
};

// ─── Full Jadwal page ─────────────────────────────────────────────────────────
class JadwalPage : public QWidget {
    Q_OBJECT
public:
    explicit JadwalPage(QWidget *parent = nullptr);
    void refresh();

private slots:
    void onAddClicked();
    void onEditRequested(const QString &id);
    void onDeleteRequested(const QString &id);

private:
    QVBoxLayout *listLayout;
    QWidget     *listContainer;
    void rebuildList();
};

#endif // JADWALPAGE_H
