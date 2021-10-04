#pragma once

// On Qt 6, you can use <QtQml/qqmlregistration.h> instead.
#include <QtQml/qqml.h>
#include <unordered_map>

namespace state {

using Settings = std::unordered_map<QString, QVariant>;

class AppState : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString qt_version READ qt_version CONSTANT)
    Q_PROPERTY(QString last_folder READ last_folder NOTIFY last_folder_changed)

private:
    QString _curr_error;
    Settings _settings;

public:
    explicit AppState(QObject * parent = nullptr);

    /// Should be called once QML is loaded, to show any errors that occurred during
    /// startup.
    Q_INVOKABLE void on_loaded();

    QString qt_version() const;

    QString last_folder() const;
    Q_INVOKABLE void on_file_opened(QString file);

signals:
    void error(QString message);
    void last_folder_changed();
};

}
