#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>

// ─── Data models ─────────────────────────────────────────────────────────────

struct TugasItem {
    QString  id;
    QString  nama;
    QString  mataKuliah;
    QString  warna;
    QDateTime deadline;
    bool     selesai;

    QJsonObject toJson() const;
    static TugasItem fromJson(const QJsonObject &obj);
};

struct JadwalItem {
    QString id;
    QString jam;       // "08:00"
    QString nama;
    QString warna;
    QString hari;      // "Senin" / "Setiap Hari" etc.

    QJsonObject toJson() const;
    static JadwalItem fromJson(const QJsonObject &obj);
};

// ─── Singleton data manager ───────────────────────────────────────────────────

class DataManager {
public:
    static DataManager &instance();

    void load();
    void save();

    // Tugas
    QVector<TugasItem> &tugas();
    void addTugas(const TugasItem &item);
    void updateTugas(const TugasItem &item);
    void removeTugas(const QString &id);

    // Jadwal
    QVector<JadwalItem> &jadwal();
    void addJadwal(const JadwalItem &item);
    void updateJadwal(const JadwalItem &item);
    void removeJadwal(const QString &id);

private:
    DataManager() = default;
    QString filePath() const;

    QVector<TugasItem>  m_tugas;
    QVector<JadwalItem> m_jadwal;
};

#endif // DATAMANAGER_H
