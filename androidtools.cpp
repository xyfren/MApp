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

void AndroidTools::enableEdgeToEdge(){
    qDebug() << "HEEELLOOOO";
    // Получаем текущую Activity
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        // Выполняем в основном потоке Android (UI Thread)
        QNativeInterface::QAndroidApplication::runOnAndroidMainThread([activity]() {
            QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
            if (window.isValid()) {
                // Аналог WindowCompat.setDecorFitsSystemWindows(window, false)
                window.callMethod<void>("setDecorFitsSystemWindows", "(Z)V", false);

                // Делаем статус-бар и навигацию прозрачными
                window.callMethod<void>("setStatusBarColor", "(I)V", 0); // 0 = Color.TRANSPARENT
                window.callMethod<void>("setNavigationBarColor", "(I)V", 0);
            }
        });
    }
}
void AndroidTools::setFullScreen(bool enable) {
#ifdef Q_OS_ANDROID
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([enable]() {
        // 1. Получаем Activity и Window
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

        if (!window.isValid()) return;

        if (enable) {
            // Разрешаем контенту заходить под вырезы (чёлка/punch-hole камера)
            // LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES = 1 (API 28+)
            QJniObject layoutParams = window.callObjectMethod("getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
            if (layoutParams.isValid()) {
                layoutParams.setField<jint>("layoutInDisplayCutoutMode", 1);
                window.callMethod<void>("setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V", layoutParams.object());
            }

            // Режим "Под куполом": контент заходит под вырезы и панели
            // WindowInsetsController.setDecorFitsSystemWindows(false)
            window.callMethod<void>("setDecorFitsSystemWindows", "(Z)V", false);

            // Получаем WindowInsetsController (Android 11+)
            QJniObject controller = window.callObjectMethod("getInsetsController", "()Landroid/view/WindowInsetsController;");

            if (controller.isValid()) {
                // Типы: statusBars (1) | navigationBars (2) = 3
                jint types = 3;
                controller.callMethod<void>("hide", "(I)V", types);

                // Чтобы панели не вылезали сами (только по свайпу)
                // BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE = 2
                controller.callMethod<void>("setSystemBarsBehavior", "(I)V", 2);
            } else {
                // Старый метод для Android 10 и ниже (SystemUI Flags)
                QJniObject decorView = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
                // SYSTEM_UI_FLAG_FULLSCREEN (4) | SYSTEM_UI_FLAG_HIDE_NAVIGATION (2) | SYSTEM_UI_FLAG_IMMERSIVE_STICKY (4096)
                jint flags = 4 | 2 | 4096;
                decorView.callMethod<void>("setSystemUiVisibility", "(I)V", flags);
            }
        } else {
            // Возвращаем стандартный режим вырезов
            // LAYOUT_IN_DISPLAY_CUTOUT_MODE_DEFAULT = 0
            QJniObject layoutParams = window.callObjectMethod("getAttributes", "()Landroid/view/WindowManager$LayoutParams;");
            if (layoutParams.isValid()) {
                layoutParams.setField<jint>("layoutInDisplayCutoutMode", 0);
                window.callMethod<void>("setAttributes", "(Landroid/view/WindowManager$LayoutParams;)V", layoutParams.object());
            }

            // Возвращаем всё как было
            window.callMethod<void>("setDecorFitsSystemWindows", "(Z)V", true);
            QJniObject controller = window.callObjectMethod("getInsetsController", "()Landroid/view/WindowInsetsController;");
            if (controller.isValid()) {
                controller.callMethod<void>("show", "(I)V", 3);
            }
        }
    });
#endif
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
