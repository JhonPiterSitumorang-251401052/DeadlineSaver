#ifndef TUGASPAGE_H
#define TUGASPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QCheckBox>
#include <QScrollArea>
#include <QTimer>
#include <QDialog>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include "datamanager.h"

// ─── Dialog tambah / edit tugas ───────────────────────────────────────────────
class TugasDialog : public QDialog {
    Q_OBJECT
public:
    explicit TugasDialog(QWidget *parent = nullptr, const TugasItem *existing = nullptr);
    TugasItem result() const;

private:
    QLineEdit      *editNama;
    QLineEdit      *editMatkul;
    QDateTimeEdit  *editDeadline;
    QComboBox      *comboPrioritas;
    QString         editingId;

    static QString warnaFromPrioritas(const QString &p);
};

// ─── Single task card ─────────────────────────────────────────────────────────
class TugasCard : public QFrame {
    Q_OBJECT
public:
    explicit TugasCard(const TugasItem &item, QWidget *parent = nullptr);

signals:
    void checkToggled(const QString &id, bool checked);
    void deleteRequested(const QString &id);
    void editRequested(const QString &id);

private:
    QString itemId;
};

// ─── Full Tugas page ──────────────────────────────────────────────────────────
class TugasPage : public QWidget {
    Q_OBJECT
public:
    explicit TugasPage(QWidget *parent = nullptr);
    void refresh();

private slots:
    void onAddClicked();
    void onEditRequested(const QString &id);
    void onDeleteRequested(const QString &id);
    void onCheckToggled(const QString &id, bool checked);

private:
    QVBoxLayout *listLayout;
    QWidget     *listContainer;
    void rebuildList();
};

#endif // TUGASPAGE_H
