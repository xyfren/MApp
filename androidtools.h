#ifndef ANDROIDTOOLS_H
#define ANDROIDTOOLS_H

#include <QObject>
#include <QQuickItem>
#include <QString>

struct DisplayParameters {
    Q_GADGET
    Q_PROPERTY(quint16 width MEMBER width)
    Q_PROPERTY(quint16 height MEMBER height)
    Q_PROPERTY(quint16 refreshRate MEMBER refreshRate)

public:
    DisplayParameters() = default;
    DisplayParameters(quint16 w, quint16 h, quint16 rr)
        : width(w), height(h), refreshRate(rr) {}

    quint16 width = 0;
    quint16 height = 0;
    quint16 refreshRate = 0;
};

Q_DECLARE_METATYPE(DisplayParameters)

class AndroidTools : public QObject {
    Q_OBJECT
    QML_ELEMENT // Важно для QML регистрации


public:
    AndroidTools(QObject *parent = nullptr);

    Q_INVOKABLE static DisplayParameters getDisplayParameters();
    Q_INVOKABLE static void setOrientation(int orientation);
    Q_INVOKABLE static void setKeepScreenOn(bool enabled);

public slots:
    static QJniObject getWindowManager();



private slots:
    Q_INVOKABLE static quint16 getDisplayWidth();
    Q_INVOKABLE static quint16 getDisplayHeigth();
    Q_INVOKABLE static quint16 getDisplayRefreshRate();

    Q_INVOKABLE void showToast(const QString &message);
};

#endif // ANDROIDTOOLS_H
