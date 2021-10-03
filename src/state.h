#pragma once

// On Qt 6, you can use <QtQml/qqmlregistration.h> instead.
#include <QtQml/qqml.h>

namespace state {

class AppState : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString qt_version READ qt_version CONSTANT)

private:
    QString _curr_error;

public:
    explicit AppState(QObject * parent = nullptr);

    QString qt_version() const;

public slots:
    /// Should be called once QML is loaded, to show any errors that occurred during
    /// startup.
    void on_loaded();

signals:
    void error(QString message);
};

}
