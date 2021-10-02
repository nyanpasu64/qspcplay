#include "util/mv.h"

#include <stx/result.h>

// GUI
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

// Qt utility
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

using stx::Result, stx::Ok, stx::Err;

static Result<QDir, QString> create_config_dir() {
    // AppConfigLocation is the "correct" place to put app data.
    // But on Windows, it returns AppData/Local (intended more for caches than config)
    // rather than Roaming, so use AppDataLocation instead.
    constexpr QStandardPaths::StandardLocation CONFIG_LOCATION =
#ifdef _WIN32
        QStandardPaths::AppDataLocation;
#else
        QStandardPaths::AppConfigLocation;
#endif
    auto config_path = QStandardPaths::writableLocation(CONFIG_LOCATION);
    if (config_path.isEmpty()) {
        return Err(QObject::tr("failed to locate config file dir"));
    }

    auto config_dir = QDir(mv(config_path));
    if (!config_dir.mkpath(".")) {
        return Err(QObject::tr("failed to create config file dir"));
    }

    return Ok(mv(config_dir));
}

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

    {
        auto config_dir = create_config_dir();
        if (config_dir.is_ok()) {
            qDebug() << "config dir:" << config_dir.value();
        }
        if (config_dir.is_err()) {
            qDebug() << "error:" << config_dir.err_value();
        }
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("qtversion", QString(qVersion()));

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
