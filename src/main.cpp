#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor
    );
    QGuiApplication app(argc, argv);

    // The default QQC2 style on Qt 5 is Basic, which is ugly.
    // The default style on Qt 6 is Fusion, which looks OK.
    // However, on Qt 6, it's not accessible by the name "fusion", so don't set it.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // If QT_QUICK_CONTROLS_STYLE is set, use the style override.
    // Otherwise, default to Fusion.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle("fusion");
    }
#endif

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("qtversion", QString(qVersion()));

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
