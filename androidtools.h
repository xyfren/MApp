#ifndef ANDROIDTOOLS_H
#define ANDROIDTOOLS_H

#include <QObject>
#include <QQuickItem>
#include <QString>

class AndroidTools : public QObject {
    Q_OBJECT
    QML_ELEMENT // Важно для QML регистрации

public:
    AndroidTools(QObject *parent = nullptr);

    static qint16 getDisplayWidth();
    static qint16 getDisplayHeigth();
    static qint16 getDisplayRefreshRate();

public slots:
    static QJniObject getWindowManager();

    Q_INVOKABLE void showToast(const QString &message);
};

#endif // ANDROIDTOOLS_H
