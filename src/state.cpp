#include "state.h"
#include "util/mv.h"

#include <stx/result.h>

// Qt utility
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QSqlDatabase>

namespace state {

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

    /* """
    Warning: If you add a connection with the same name as an existing connection, the
    new connection replaces the old one. If you call this function more than once
    without specifying connectionName, the default connection will be the one replaced.
    */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(settings_path);

    if (!db.open()) {
        return Err(
            QObject::tr("failed to open settings file \"%1\"").arg(settings_path)
        );
    }

    return Ok(mv(db));
}

AppState::AppState(QObject * parent)
    : QObject(parent)
{
    // This is wrong; if you create multiple AppState, they'll trample over each other.
    auto result = open_settings();
    if (result.is_ok()) {
        qDebug() << "settings file:" << result.value().databaseName();
    }
    if (result.is_err()) {
        _curr_error = mv(result.err_value());
    }
}

QString AppState::qt_version() const {
    return QString(qVersion());
}

void AppState::on_loaded() {
    if (!_curr_error.isEmpty()) {
        emit error(mv(_curr_error));
        _curr_error = QString();
    }
}

}
