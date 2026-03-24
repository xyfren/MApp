#include "androidtools.h"
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>
#endif

AndroidTools::AndroidTools(QObject *parent) : QObject(parent) {}

void AndroidTools::showToast(const QString &message) {
    qDebug() << "Hello";
#ifdef Q_OS_ANDROID
    // Создаем Java строку из QString
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([message](){
        QJniObject javaString = QJniObject::fromString(message);
        QJniObject toast = QJniObject::callStaticObjectMethod("android/widget/Toast", "makeText",
                                                              "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;",
                                                              QNativeInterface::QAndroidApplication::context().object(),
                                                              javaString.object(),
                                                              jint(1));
        toast.callMethod<void>("show");
    });
#endif
}

// Вспомогательная функция для получения объекта WindowManager
QJniObject AndroidTools::getWindowManager() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject serviceName = QJniObject::fromString("window");
    return activity.callObjectMethod("getSystemService",
                                     "(Ljava/lang/String;)Ljava/lang/Object;",
                                     serviceName.object<jstring>());
}

DisplayParameters AndroidTools::getDisplayParameters(){
    QJniObject display = getWindowManager().callObjectMethod("getDefaultDisplay", "()Landroid/view/Display;");
    DisplayParameters param;

    QJniObject point("android/graphics/Point");
    display.callMethod<void>("getSize", "(Landroid/graphics/Point;)V", point.object());

    param.width = static_cast<quint16>(point.getField<int>("x"));
    param.height = static_cast<quint16>(point.getField<int>("y"));
    if (param.width < param.height){
        quint16 t = param.width;
        param.width = param.height;
        param.height = t;
    }
    float refreshRate = display.callMethod<jfloat>("getRefreshRate");
    param.refreshRate = static_cast<qint16>(refreshRate);
    return param;
}

void AndroidTools::setOrientation(int orientation) {
    auto activity = QJniObject::fromString("android/app/Activity");
    activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        activity.callMethod<void>("setRequestedOrientation", "(I)V", orientation);
    }
}

void AndroidTools::setKeepScreenOn(bool enabled) {
    // Все манипуляции с Window в Android должны происходить в основном потоке (UI Thread)
    // Используем лямбду для выполнения в UI-потоке Android
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([enabled]() {
        // 1. Получаем activity как QJniObject
        QJniObject activity = QNativeInterface::QAndroidApplication::context();

        if (activity.isValid()) {
            // 2. Получаем окно (Window)
            // Сигнатура: ()Landroid/view/Window;
            QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

            if (window.isValid()) {
                const int FLAG_KEEP_SCREEN_ON = 128; // WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON

                if (enabled) {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                } else {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
    });
}

quint16 AndroidTools::getDisplayWidth() {
    QJniObject display = getWindowManager().callObjectMethod("getDefaultDisplay", "()Landroid/view/Display;");

    // В современном Android рекомендуется использовать WindowMetrics,
    // но для простоты и совместимости используем getSize через Point
    QJniObject point("android/graphics/Point");
    display.callMethod<void>("getSize", "(Landroid/graphics/Point;)V", point.object());

    return static_cast<quint16>(point.getField<int>("x"));
}

quint16 AndroidTools::getDisplayHeigth() {
    QJniObject display = getWindowManager().callObjectMethod("getDefaultDisplay", "()Landroid/view/Display;");

    QJniObject point("android/graphics/Point");
    display.callMethod<void>("getSize", "(Landroid/graphics/Point;)V", point.object());

    return static_cast<quint16>(point.getField<int>("y"));
}

quint16 AndroidTools::getDisplayRefreshRate() {
    QJniObject display = getWindowManager().callObjectMethod("getDefaultDisplay", "()Landroid/view/Display;");

    // Возвращает float, приводим к quint16
    float refreshRate = display.callMethod<jfloat>("getRefreshRate");
    return static_cast<qint16>(refreshRate);
}
