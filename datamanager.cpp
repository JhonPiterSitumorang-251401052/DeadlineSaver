#include "datamanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>

// ── TugasItem JSON ────────────────────────────────────────────────────────────
QJsonObject TugasItem::toJson() const {
    QJsonObject o;
    o["id"]          = id;
    o["nama"]        = nama;
    o["mataKuliah"]  = mataKuliah;
    o["warna"]       = warna;
    o["deadline"]    = deadline.toString(Qt::ISODate);
    o["selesai"]     = selesai;
    return o;
}

TugasItem TugasItem::fromJson(const QJsonObject &o) {
    TugasItem t;
    t.id         = o["id"].toString();
    t.nama       = o["nama"].toString();
    t.mataKuliah = o["mataKuliah"].toString();
    t.warna      = o["warna"].toString();
    t.deadline   = QDateTime::fromString(o["deadline"].toString(), Qt::ISODate);
    t.selesai    = o["selesai"].toBool();
    return t;
}

// ── JadwalItem JSON ───────────────────────────────────────────────────────────
QJsonObject JadwalItem::toJson() const {
    QJsonObject o;
    o["id"]    = id;
    o["jam"]   = jam;
    o["nama"]  = nama;
    o["warna"] = warna;
    o["hari"]  = hari;
    return o;
}

JadwalItem JadwalItem::fromJson(const QJsonObject &o) {
    JadwalItem j;
    j.id    = o["id"].toString();
    j.jam   = o["jam"].toString();
    j.nama  = o["nama"].toString();
    j.warna = o["warna"].toString();
    j.hari  = o["hari"].toString();
    return j;
}

// ── DataManager ───────────────────────────────────────────────────────────────
DataManager &DataManager::instance() {
    static DataManager dm;
    return dm;
}

QString DataManager::filePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/data.json";
}

void DataManager::load() {
    QFile f(filePath());
    if (!f.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonObject root  = doc.object();

    m_tugas.clear();
    for (auto v : root["tugas"].toArray())
        m_tugas.append(TugasItem::fromJson(v.toObject()));

    m_jadwal.clear();
    for (auto v : root["jadwal"].toArray())
        m_jadwal.append(JadwalItem::fromJson(v.toObject()));
}

void DataManager::save() {
    QJsonArray ta, ja;
    for (auto &t : m_tugas)  ta.append(t.toJson());
    for (auto &j : m_jadwal) ja.append(j.toJson());

    QJsonObject root;
    root["tugas"]  = ta;
    root["jadwal"] = ja;

    QFile f(filePath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(root).toJson());
}

// ── Tugas CRUD ────────────────────────────────────────────────────────────────
QVector<TugasItem> &DataManager::tugas() { return m_tugas; }

void DataManager::addTugas(const TugasItem &item) {
    TugasItem t = item;
    if (t.id.isEmpty()) t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_tugas.append(t);
    save();
}

void DataManager::updateTugas(const TugasItem &item) {
    for (auto &t : m_tugas) {
        if (t.id == item.id) { t = item; break; }
    }
    save();
}

void DataManager::removeTugas(const QString &id) {
    m_tugas.erase(
        std::remove_if(m_tugas.begin(), m_tugas.end(),
                       [&](const TugasItem &t){ return t.id == id; }),
        m_tugas.end()
    );
    save();
}

// ── Jadwal CRUD ───────────────────────────────────────────────────────────────
QVector<JadwalItem> &DataManager::jadwal() { return m_jadwal; }

void DataManager::addJadwal(const JadwalItem &item) {
    JadwalItem j = item;
    if (j.id.isEmpty()) j.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_jadwal.append(j);
    save();
}

void DataManager::updateJadwal(const JadwalItem &item) {
    for (auto &j : m_jadwal) {
        if (j.id == item.id) { j = item; break; }
    }
    save();
}

void DataManager::removeJadwal(const QString &id) {
    m_jadwal.erase(
        std::remove_if(m_jadwal.begin(), m_jadwal.end(),
                       [&](const JadwalItem &j){ return j.id == id; }),
        m_jadwal.end()
    );
    save();
}
