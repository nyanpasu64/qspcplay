#include "state.h"
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
#include <QSqlDatabase>

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
        return Err(QObject::tr("failed to locate config dir"));
    }

    auto config_dir = QDir(config_path);
    if (!config_dir.mkpath(".")) {
        return Err(
            QObject::tr("failed to create config dir \"%1\"").arg(config_path)
        );
    }

    return Ok(mv(config_dir));
}

static const QString SETTINGS_NAME = QStringLiteral("settings.sqlite3");

static Result<QSqlDatabase, QString> open_settings() {
    TRY_OK(config_dir, create_config_dir());

    QString settings_path = config_dir.filePath(SETTINGS_NAME);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(settings_path);

    if (!db.open()) {
        return Err(
            QObject::tr("failed to open settings file \"%1\"").arg(settings_path)
        );
    }

    return Ok(mv(db));
}

int main(int argc, char *argv[]) {
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor
    );
    QGuiApplication app(argc, argv);

    // The default style on Qt 6 is Fusion, which looks OK.

    QSqlDatabase settings;

    {
        auto result = open_settings();
        if (result.is_ok()) {
            qDebug() << "settings file:" << result.value().databaseName();
            settings = mv(result.value());
        }
        if (result.is_err()) {
            qDebug() << "error:" << qUtf8Printable(result.err_value());
        }
    }

    qmlRegisterType<state::AppState>("QSpcPlay", 1, 0, "AppState");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
