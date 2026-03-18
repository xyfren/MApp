#ifndef TIMEPROFILER_H
#define TIMEPROFILER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QString>
#include <QDebug>
#include <chrono>

// Класс-исполнитель, который будет работать в отдельном потоке
class ProfilerWorker : public QObject {
    Q_OBJECT
public slots:
    void process(); // Цикл обработки очереди
signals:
    void finished();
};

class TimeProfiler : public QObject {
    Q_OBJECT
public:
    static TimeProfiler& instance();

    // Замер времени (вызывать из любого места)
    void stamp(const QString &eventName);

    // Тег для логов (например, "ANDROID" или "PC")
    void setTag(const QString &tag) { m_tag = tag; }

private:
    TimeProfiler();
    ~TimeProfiler();

    friend class ProfilerWorker; // Чтобы воркер видел очередь

    QString m_tag;
    QQueue<QString> m_queue;
    QMutex m_mutex;
    QWaitCondition m_condition;
    QThread m_workerThread;
};

#endif // TIMEPROFILER_H