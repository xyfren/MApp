#ifndef ANDROIDTOOLS_H
#define ANDROIDTOOLS_H

#include <QObject>
#include <QQuickItem>
#include <QString>

struct DisplayParameters {
    quint16 width = 0;
    quint16 height = 0;
    quint16 refreshRate = 0;
};

class AndroidTools : public QObject {
    Q_OBJECT
    QML_ELEMENT // Важно для QML регистрации


public:
    AndroidTools(QObject *parent = nullptr);

    static DisplayParameters getDisplayParameters();

public slots:
    static QJniObject getWindowManager();



private slots:
    static quint16 getDisplayWidth();
    static quint16 getDisplayHeigth();
    static quint16 getDisplayRefreshRate();

    Q_INVOKABLE void showToast(const QString &message);
};

#endif // ANDROIDTOOLS_H
