#include "state.h"
#include "util/mv.h"

#include <stx/result.h>

// Qt utility
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <optional>

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
#define SETTINGS "settings"

static Result<Settings, QString> open_settings() {
    TRY_OK(config_dir, create_config_dir());

    QString settings_path = config_dir.filePath(SETTINGS_NAME);

    /* """
    Warning: If you add a connection with the same name as an existing connection, the
    new connection replaces the old one. If you call this function more than once
    without specifying connectionName, the default connection will be the one replaced.
    */
    // Sets the global database.
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(settings_path);

    if (!db.open()) {
        return Err(
            QObject::tr("failed to open settings file \"%1\"").arg(settings_path)
        );
    }

    // Implicitly uses the global database.
    QSqlQuery query;
    if (!query.exec("create table if not exists " SETTINGS " (\n"
        "key text primary key not null,\n"
        "value blob not null\n"
    ")")) {
        return Err(
            QObject::tr("failed to initialize settings \"%1\", error: %2")
                .arg(settings_path, query.lastError().text())
        );
    }

    if (!query.exec("select key, value from " SETTINGS)) {
        return Err(
            QObject::tr("failed to load settings \"%1\", error: %2")
                .arg(settings_path, query.lastError().text())
        );
    }

    Settings settings;
    int size = query.size();
    if (size >= 0) {
        settings.reserve((size_t) size);
    } else {
        qDebug() << "unknown setting count";
    }

    while (query.next()) {
        settings.insert_or_assign(query.value(0).toString(), query.value(1));
    }
    return Ok(mv(settings));
}

AppState::AppState(QObject * parent)
    : QObject(parent)
{
    // This is wrong; if you create multiple AppState, they'll trample over each other.
    auto result = open_settings();
    if (result.is_ok()) {
        _settings = mv(result.value());
    }
    if (result.is_err()) {
        _curr_error = mv(result.err_value());
    }
}

void AppState::on_loaded() {
    if (!_curr_error.isEmpty()) {
        emit error(mv(_curr_error));
        _curr_error = QString();
    }
}

QString AppState::qt_version() const {
    return QString(qVersion());
}

static const QString LAST_FOLDER = QStringLiteral("last_folder");

static std::optional<QSqlError> insert_setting(QString key, QVariant value) {
    // TODO make function non-static, and handle _settings and database and emit error?
    QSqlQuery query;
    query.prepare("insert or replace into " SETTINGS " (key, value) values (?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        return query.lastError();
    }
    return {};
}

QString URL_PREFIX = QStringLiteral("file://");


QString AppState::last_folder() const {
    auto it = _settings.find(LAST_FOLDER);
    if (it != _settings.end()) {
        return URL_PREFIX + it->second.toString();
    } else {
        return
            URL_PREFIX +
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
}

void AppState::on_file_opened(QString file) {
    if (file.startsWith(URL_PREFIX)) {
        file.remove(0, URL_PREFIX.size());
    }

    // Get parent directory.
    QString folder = QFileInfo(file).dir().path();

    // clone
    _settings.insert_or_assign(LAST_FOLDER, folder);
    if (auto err = insert_setting(LAST_FOLDER, mv(folder))) {
        emit error(err->text());
    }

    emit last_folder_changed();
}

}
