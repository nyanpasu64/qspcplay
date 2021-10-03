#include "state.h"

// GUI
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>


int main(int argc, char *argv[]) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor
    );
    QApplication app(argc, argv);

    // The default style on Qt 6 is Fusion, which looks OK.

    qmlRegisterType<state::AppState>("QSpcPlay", 1, 0, "AppState");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
