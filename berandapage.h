#ifndef BERANDAPAGE_H
#define BERANDAPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QScrollArea>
#include <QMap>
#include <QEvent>
#include "datamanager.h"

// ─── Group card per mata kuliah ───────────────────────────────────────────────
class BerandaGroup : public QFrame {
    Q_OBJECT
public:
    explicit BerandaGroup(const QString &mataKuliah,
                          const QString &warna,
                          const QVector<TugasItem> &items,
                          QWidget *parent = nullptr);
private slots:
    void toggleExpand();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    QWidget     *itemContainer;
    QPushButton *btnToggle;
    bool         expanded = true;
};

// ─── Single item row inside a group ──────────────────────────────────────────
class BerandaItem : public QWidget {
    Q_OBJECT
public:
    explicit BerandaItem(const TugasItem &item, const QString &warna, QWidget *parent = nullptr);
};

// ─── Full Beranda page ────────────────────────────────────────────────────────
class BerandaPage : public QWidget {
    Q_OBJECT
public:
    explicit BerandaPage(QWidget *parent = nullptr);
    void refresh();

private:
    QVBoxLayout *listLayout;
    QWidget     *listContainer;
    QWidget     *summaryBar;
    void rebuildList();
};

#endif // BERANDAPAGE_H
