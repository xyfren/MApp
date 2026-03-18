#include "TimeProfiler.h"

TimeProfiler& TimeProfiler::instance() {
    static TimeProfiler inst;
    return inst;
}

TimeProfiler::TimeProfiler() : m_tag("Log") {
    ProfilerWorker *worker = new ProfilerWorker();
    worker->moveToThread(&m_workerThread);

    // Запускаем цикл обработки при старте потока
    connect(&m_workerThread, &QThread::started, worker, &ProfilerWorker::process);
    // Удаляем воркера при завершении потока
    connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);

    m_workerThread.start();
}

TimeProfiler::~TimeProfiler() {
    m_workerThread.quit();
    m_condition.wakeAll(); // Разбудить поток, если он спит на wait()
    m_workerThread.wait();
}

void TimeProfiler::stamp(const QString &eventName) {
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()).count();

    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(QString("[%1] %2: %3 us").arg(m_tag).arg(eventName).arg(ms));
    m_condition.wakeOne(); // Говорим воркеру, что пора печатать
}

// РЕАЛИЗАЦИЯ WORKER
void ProfilerWorker::process() {
    TimeProfiler &profiler = TimeProfiler::instance();

    while (true) {
        QString message;
        {
            QMutexLocker locker(&profiler.m_mutex);
            // Если очередь пуста, поток засыпает и не ест ресурсы CPU
            if (profiler.m_queue.isEmpty()) {
                profiler.m_condition.wait(&profiler.m_mutex);
            }

            // Проверка на выход (если поток завершается)
            if (profiler.m_workerThread.isInterruptionRequested()) break;
            if (profiler.m_queue.isEmpty()) continue;

            message = profiler.m_queue.dequeue();
        }

        // Вывод. На Android это уйдет в Logcat, на Windows в Output/Console
        qDebug().noquote() << message;
    }
}